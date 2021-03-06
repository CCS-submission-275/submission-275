// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICES_NETWORK_PUBLIC_CPP_BLOCKED_BY_RESPONSE_REASON_H_
#define SERVICES_NETWORK_PUBLIC_CPP_BLOCKED_BY_RESPONSE_REASON_H_

namespace network {

// This enum is used by to communicate the reason a request was blocked from
// the network service to the browser. The blocking reasons pertain to
// security features such as CORP, COEP, and COOP.
enum class BlockedByResponseReason : int {
  kCoepFrameResourceNeedsCoepHeader,
  kCoopSandboxedIFrameCannotNavigateToCoopPage,
  kCorpNotSameOrigin,
  kCorpNotSameOriginAfterDefaultedToSameOriginByCoep,
  kCorpNotSameSite,
  // `kMaxValue` needs be assigned to the max value in the enum.
  kMaxValue = kCorpNotSameSite,
};

}  // namespace network

#endif  // SERVICES_NETWORK_PUBLIC_CPP_BLOCKED_BY_RESPONSE_REASON_H_
