// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_POLICY_MINIMUM_VERSION_POLICY_HANDLER_DELEGATE_IMPL_H_
#define CHROME_BROWSER_CHROMEOS_POLICY_MINIMUM_VERSION_POLICY_HANDLER_DELEGATE_IMPL_H_

#include "base/version.h"
#include "chrome/browser/chromeos/policy/minimum_version_policy_handler.h"

namespace policy {

class MinimumVersionPolicyHandlerDelegateImpl
    : public MinimumVersionPolicyHandler::Delegate {
 public:
  MinimumVersionPolicyHandlerDelegateImpl();
  bool IsKioskMode() const override;
  bool IsEnterpriseManaged() const override;
  const base::Version& GetCurrentVersion() const override;
};

}  // namespace policy

#endif  // CHROME_BROWSER_CHROMEOS_POLICY_MINIMUM_VERSION_POLICY_HANDLER_DELEGATE_IMPL_H_
