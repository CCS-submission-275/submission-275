// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_SIGNALING_MOCK_SIGNALING_TRACKER_H_
#define REMOTING_SIGNALING_MOCK_SIGNALING_TRACKER_H_

#include "remoting/signaling/signaling_tracker.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace remoting {

class MockSignalingTracker final : public SignalingTracker {
 public:
  MockSignalingTracker();
  ~MockSignalingTracker() override;

  MOCK_METHOD0(OnChannelActive, void());
  MOCK_CONST_METHOD0(IsChannelActive, bool());
  MOCK_CONST_METHOD0(GetLastReportedChannelActiveDuration, base::TimeDelta());
};

}  // namespace remoting

#endif  // REMOTING_SIGNALING_MOCK_SIGNALING_TRACKER_H_