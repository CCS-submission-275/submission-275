// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/smb_client/smbfs_share.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/strings/strcat.h"
#include "base/strings/string_util.h"
#include "base/unguessable_token.h"
#include "chrome/browser/chromeos/file_manager/volume_manager.h"
#include "chrome/browser/chromeos/smb_client/smb_service_helper.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/webui/chromeos/smb_shares/smb_credentials_dialog.h"
#include "storage/browser/file_system/external_mount_points.h"

namespace chromeos {
namespace smb_client {

namespace {

constexpr char kMountDirPrefix[] = "smbfs-";
constexpr base::TimeDelta kAllowCredentialsTimeout =
    base::TimeDelta::FromSeconds(5);

SmbMountResult MountErrorToMountResult(smbfs::mojom::MountError mount_error) {
  switch (mount_error) {
    case smbfs::mojom::MountError::kOk:
      return SmbMountResult::kSuccess;
    case smbfs::mojom::MountError::kTimeout:
      return SmbMountResult::kAborted;
    case smbfs::mojom::MountError::kInvalidUrl:
      return SmbMountResult::kInvalidUrl;
    case smbfs::mojom::MountError::kInvalidOptions:
      return SmbMountResult::kInvalidOperation;
    case smbfs::mojom::MountError::kNotFound:
      return SmbMountResult::kNotFound;
    case smbfs::mojom::MountError::kAccessDenied:
      return SmbMountResult::kAuthenticationFailed;
    case smbfs::mojom::MountError::kInvalidProtocol:
      return SmbMountResult::kUnsupportedDevice;
    case smbfs::mojom::MountError::kUnknown:
    default:
      return SmbMountResult::kUnknownFailure;
  }
}

}  // namespace

SmbFsShare::SmbFsShare(Profile* profile,
                       const SmbUrl& share_url,
                       const std::string& display_name,
                       const MountOptions& options)
    : profile_(profile),
      share_url_(share_url),
      display_name_(display_name),
      options_(options),
      mount_id_(
          base::ToLowerASCII(base::UnguessableToken::Create().ToString())) {
  DCHECK(share_url_.IsValid());
}

SmbFsShare::~SmbFsShare() {
  Unmount(base::DoNothing());
}

void SmbFsShare::Mount(SmbFsShare::MountCallback callback) {
  DCHECK(!mounter_);
  DCHECK(!host_);

  if (unmount_pending_) {
    LOG(WARNING) << "Cannot mount a shared that is being unmounted";
    std::move(callback).Run(SmbMountResult::kMountExists);
    return;
  }

  // TODO(amistry): Come up with a scheme for consistent mount paths between
  // sessions.
  const std::string mount_dir = base::StrCat({kMountDirPrefix, mount_id_});
  if (mounter_creation_callback_for_test_) {
    mounter_ = mounter_creation_callback_for_test_.Run(
        share_url_.ToString(), mount_dir, options_, this);
  } else {
    mounter_ = std::make_unique<smbfs::SmbFsMounter>(
        share_url_.ToString(), mount_dir, options_, this,
        chromeos::disks::DiskMountManager::GetInstance());
  }
  mounter_->Mount(base::BindOnce(&SmbFsShare::OnMountDone,
                                 base::Unretained(this), std::move(callback)));
}

void SmbFsShare::Remount(const MountOptions& options,
                         SmbFsShare::MountCallback callback) {
  if (IsMounted() || unmount_pending_) {
    std::move(callback).Run(SmbMountResult::kMountExists);
    return;
  }

  options_ = options;

  Mount(std::move(callback));
}

void SmbFsShare::Unmount(SmbFsShare::UnmountCallback callback) {
  if (unmount_pending_) {
    LOG(WARNING) << "Cannot unmount a shared that is being unmounted";
    std::move(callback).Run(chromeos::MountError::MOUNT_ERROR_INTERNAL);
    return;
  }

  unmount_pending_ = true;

  // Cancel any pending mount request.
  mounter_.reset();

  if (!host_) {
    LOG(WARNING) << "Cannot unmount as the share is already unmounted";
    std::move(callback).Run(chromeos::MountError::MOUNT_ERROR_PATH_NOT_MOUNTED);
    return;
  }

  // Remove volume from VolumeManager.
  file_manager::VolumeManager::Get(profile_)->RemoveSmbFsVolume(
      host_->mount_path());

  storage::ExternalMountPoints* const mount_points =
      storage::ExternalMountPoints::GetSystemInstance();
  DCHECK(mount_points);
  bool success = mount_points->RevokeFileSystem(mount_id_);
  CHECK(success);

  // Get cros-disks to unmount cleanly. In the process it will kill smbfs which
  // may result in OnDisconnected() being called, but reentrant calls to
  // Unmount() will be aborted as unmount_pending_ == true.
  host_->Unmount(base::BindOnce(&SmbFsShare::OnUnmountDone,
                                base::Unretained(this), std::move(callback)));
}

void SmbFsShare::OnUnmountDone(SmbFsShare::UnmountCallback callback,
                               chromeos::MountError result) {
  host_.reset();

  // Must do this *after* destroying SmbFsHost so that reentrant calls to
  // Unmount() exit early.
  unmount_pending_ = false;

  // Callback to SmbService::OnSuspendUnmountDone().
  std::move(callback).Run(result);
}

void SmbFsShare::OnMountDone(MountCallback callback,
                             smbfs::mojom::MountError mount_error,
                             std::unique_ptr<smbfs::SmbFsHost> smbfs_host) {
  // Don't need the mounter any more.
  mounter_.reset();

  if (mount_error != smbfs::mojom::MountError::kOk) {
    std::move(callback).Run(MountErrorToMountResult(mount_error));
    return;
  }

  DCHECK(smbfs_host);
  host_ = std::move(smbfs_host);

  storage::ExternalMountPoints* const mount_points =
      storage::ExternalMountPoints::GetSystemInstance();
  DCHECK(mount_points);
  bool success = mount_points->RegisterFileSystem(
      mount_id_, storage::kFileSystemTypeSmbFs,
      storage::FileSystemMountOption(), host_->mount_path());
  CHECK(success);

  file_manager::VolumeManager::Get(profile_)->AddSmbFsVolume(
      host_->mount_path(), display_name_);
  std::move(callback).Run(SmbMountResult::kSuccess);
}

void SmbFsShare::OnDisconnected() {
  Unmount(base::DoNothing());
}

void SmbFsShare::AllowCredentialsRequest() {
  allow_credential_request_ = true;
  allow_credential_request_expiry_ =
      base::TimeTicks::Now() + kAllowCredentialsTimeout;
}

void SmbFsShare::RequestCredentials(RequestCredentialsCallback callback) {
  if (allow_credential_request_expiry_ < base::TimeTicks::Now()) {
    allow_credential_request_ = false;
  }

  if (!allow_credential_request_) {
    std::move(callback).Run(true /* cancel */, "" /* username */,
                            "" /* workgroup */, "" /* password */);
    return;
  }

  smb_dialog::SmbCredentialsDialog::Show(
      mount_id_, share_url_.ToString(),
      base::BindOnce(
          [](RequestCredentialsCallback callback, bool canceled,
             const std::string& username, const std::string& password) {
            if (canceled) {
              std::move(callback).Run(true /* cancel */, "" /* username */,
                                      "" /* workgroup */, "" /* password */);
              return;
            }

            std::string parsed_username = username;
            std::string workgroup;
            ParseUserName(username, &parsed_username, &workgroup);
            std::move(callback).Run(false /* cancel */, parsed_username,
                                    workgroup, password);
          },
          std::move(callback)));
  // Reset the allow dialog state to prevent showing another dialog to the user
  // immediately after they've dismissed one.
  allow_credential_request_ = false;
}

void SmbFsShare::SetMounterCreationCallbackForTest(
    MounterCreationCallback callback) {
  mounter_creation_callback_for_test_ = std::move(callback);
}

}  // namespace smb_client
}  // namespace chromeos
