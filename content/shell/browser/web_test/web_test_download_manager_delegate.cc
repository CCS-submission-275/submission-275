// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/shell/browser/web_test/web_test_download_manager_delegate.h"

#if defined(OS_WIN)
#include <windows.h>
#include <commdlg.h>
#endif

#include "base/bind.h"
#include "base/command_line.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "build/build_config.h"
#include "components/download/public/common/download_item.h"
#include "content/public/browser/download_item_utils.h"
#include "content/public/browser/download_manager.h"
#include "content/shell/browser/web_test/blink_test_controller.h"
#include "net/base/filename_util.h"

#if defined(OS_WIN)
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#endif

namespace content {

WebTestDownloadManagerDelegate::WebTestDownloadManagerDelegate()
    : ShellDownloadManagerDelegate() {}

WebTestDownloadManagerDelegate::~WebTestDownloadManagerDelegate() {}

bool WebTestDownloadManagerDelegate::ShouldOpenDownload(
    download::DownloadItem* item,
    DownloadOpenDelayedCallback callback) {
  if (BlinkTestController::Get() &&
      BlinkTestController::Get()->IsMainWindow(
          DownloadItemUtils::GetWebContents(item)) &&
      item->GetMimeType() == "text/html") {
    BlinkTestController::Get()->OpenURL(
        net::FilePathToFileURL(item->GetFullPath()));
  }
  return true;
}

void WebTestDownloadManagerDelegate::CheckDownloadAllowed(
    const content::WebContents::Getter& web_contents_getter,
    const GURL& url,
    const std::string& request_method,
    base::Optional<url::Origin> request_initiator,
    bool from_download_cross_origin_redirect,
    bool content_initiated,
    content::CheckDownloadAllowedCallback check_download_allowed_cb) {
  auto* test_controller = BlinkTestController::Get();
  base::Optional<bool> should_wait_until_external_url_load =
      test_controller->accumulated_web_test_runtime_flags_changes()
          .FindBoolPath("wait_until_external_url_load");

  // The if clause below catches all calls to this method not
  // initiated by content, or even if it does, whose web_test
  // does not call TestRunner::WaitUntilExternalUrlLoad().
  if (!content_initiated || !should_wait_until_external_url_load.has_value() ||
      !should_wait_until_external_url_load.value()) {
    ShellDownloadManagerDelegate::CheckDownloadAllowed(
        web_contents_getter, url, request_method, request_initiator,
        from_download_cross_origin_redirect, content_initiated,
        std::move(check_download_allowed_cb));
    return;
  }

  test_controller->printer()->AddMessageRaw("Download started\n");
  test_controller->OnTestFinishedInSecondaryRenderer();
  std::move(check_download_allowed_cb).Run(false);
}

}  // namespace content
