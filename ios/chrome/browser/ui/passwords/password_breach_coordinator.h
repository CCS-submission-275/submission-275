// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_PASSWORDS_PASSWORD_BREACH_COORDINATOR_H_
#define IOS_CHROME_BROWSER_UI_PASSWORDS_PASSWORD_BREACH_COORDINATOR_H_

#import "ios/chrome/browser/ui/coordinators/chrome_coordinator.h"

// Presents and stops the Password Breach feature, which consists in alerting
// the user that Chrome detected a leaked credential. In some scenarios it
// prompts for a checkup of the stored passwords.
@interface PasswordBreachCoordinator : ChromeCoordinator

// Unavailable, use -initWithBaseViewController:browser:.
- (instancetype)initWithBaseViewController:(UIViewController*)viewController
    NS_UNAVAILABLE;
// Unavailable, use -initWithBaseViewController:browser:.
- (instancetype)initWithBaseViewController:(UIViewController*)viewController
                              browserState:(ChromeBrowserState*)browserState
    NS_UNAVAILABLE;

@end

#endif  // IOS_CHROME_BROWSER_UI_PASSWORDS_PASSWORD_BREACH_COORDINATOR_H_
