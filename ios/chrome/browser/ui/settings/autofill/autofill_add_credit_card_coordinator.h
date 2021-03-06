// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_SETTINGS_AUTOFILL_AUTOFILL_ADD_CREDIT_CARD_COORDINATOR_H_
#define IOS_CHROME_BROWSER_UI_SETTINGS_AUTOFILL_AUTOFILL_ADD_CREDIT_CARD_COORDINATOR_H_

#include <UIKit/UIKit.h>

#import "ios/chrome/browser/ui/coordinators/chrome_coordinator.h"

// The coordinator for add credit card screen.
@interface AutofillAddCreditCardCoordinator : ChromeCoordinator

// Use -initWithBaseViewController:browser:
- (instancetype)initWithBaseViewController:(UIViewController*)viewController
    NS_UNAVAILABLE;

// Use -initWithBaseViewController:browser:
- (instancetype)initWithBaseViewController:(UIViewController*)viewController
                              browserState:(ChromeBrowserState*)browserState
    NS_UNAVAILABLE;

@end

#endif  // IOS_CHROME_BROWSER_UI_SETTINGS_AUTOFILL_AUTOFILL_ADD_CREDIT_CARD_COORDINATOR_H_
