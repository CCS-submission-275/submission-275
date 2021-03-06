// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_UPDATER_MAC_SETUP_SETUP_H_
#define CHROME_UPDATER_MAC_SETUP_SETUP_H_

namespace updater {

// Set up the updater by copying the bundle, creating launchd plists for
// scheduled tasks and xpc service, and start both launchd jobs.
int SetupUpdater();

// Remove the launchd plists for scheduled tasks and xpc service. Delete the
// updater bundle from its installed location.
int Uninstall(bool is_machine);

}  // namespace updater

#endif  // CHROME_UPDATER_MAC_SETUP_SETUP_H_
