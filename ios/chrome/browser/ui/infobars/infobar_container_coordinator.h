// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_INFOBARS_INFOBAR_CONTAINER_COORDINATOR_H_
#define IOS_CHROME_BROWSER_UI_INFOBARS_INFOBAR_CONTAINER_COORDINATOR_H_

#import "ios/chrome/browser/ui/coordinators/chrome_coordinator.h"

#import "ios/chrome/browser/ui/infobars/banners/infobar_banner_presentation_state.h"

namespace web {
class WebState;
}

@protocol InfobarPositioner;
@protocol SyncPresenter;

// Coordinator that owns and manages an InfobarContainer.
@interface InfobarContainerCoordinator : ChromeCoordinator

// Unavailable, use -initWithBaseViewController:browser:.
- (instancetype)initWithBaseViewController:(UIViewController*)viewController
    NS_UNAVAILABLE;

// Unavailable, use -initWithBaseViewController:browser:.
- (instancetype)initWithBaseViewController:(UIViewController*)viewController
                              browserState:(ChromeBrowserState*)browserState
    NS_UNAVAILABLE;

// Sets the visibility of the container to |hidden|.
- (void)hideContainer:(BOOL)hidden;

// The InfobarContainer Legacy View.
- (UIView*)legacyContainerView;

// Updates the InfobarContainer according to the positioner information.
- (void)updateInfobarContainer;

// Notifies the coordinator that its baseViewController's viewDidAppear. This
// means the view is now visible and part of the main window hierarchy.
- (void)baseViewDidAppear;

// Notifies the coordinator that its baseViewController's ViewWillDisappear.
// This means the view is not visible and no longer part of the main window
// hierarchy.
- (void)baseViewWillDisappear;

// YES if an Infobar is being presented for |webState|.
- (BOOL)isInfobarPresentingForWebState:(web::WebState*)webState;

// Dismisses the InfobarBanner. If the presentation is taking place it will stop
// it and dismiss the banner. If none is being presented |completion| will still
// run.
- (void)dismissInfobarBannerAnimated:(BOOL)animated
                          completion:(void (^)())completion;

// The delegate used to position the InfobarContainer in the view.
@property(nonatomic, weak) id<InfobarPositioner> positioner;

// The SyncPresenter delegate for this Coordinator.
@property(nonatomic, weak) id<SyncPresenter> syncPresenter;

// The current InfobarBanner presentation state (Only one Infobar Banner can be
// presented at the time).
@property(nonatomic, assign, readonly)
    InfobarBannerPresentationState infobarBannerState;

@end

#endif  // IOS_CHROME_BROWSER_UI_INFOBARS_INFOBAR_CONTAINER_COORDINATOR_H_
