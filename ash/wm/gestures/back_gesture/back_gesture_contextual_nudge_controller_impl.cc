// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wm/gestures/back_gesture/back_gesture_contextual_nudge_controller_impl.h"

#include "ash/public/cpp/back_gesture_contextual_nudge_delegate.h"
#include "ash/session/session_controller_impl.h"
#include "ash/shelf/contextual_tooltip.h"
#include "ash/shell.h"
#include "ash/shell_delegate.h"
#include "ash/wm/gestures/back_gesture/back_gesture_contextual_nudge.h"
#include "ash/wm/window_util.h"
#include "components/prefs/pref_service.h"
#include "ui/aura/client/window_types.h"
#include "ui/wm/public/activation_client.h"

namespace ash {

namespace {

PrefService* GetActivePrefService() {
  return Shell::Get()->session_controller()->GetActivePrefService();
}

}  // namespace

BackGestureContextualNudgeControllerImpl::
    BackGestureContextualNudgeControllerImpl() {
  tablet_mode_observer_.Add(Shell::Get()->tablet_mode_controller());
}

BackGestureContextualNudgeControllerImpl::
    ~BackGestureContextualNudgeControllerImpl() {
  DoCleanUp();
}

void BackGestureContextualNudgeControllerImpl::OnBackGestureStarted() {
  if (nudge_) {
    nudge_->CancelAnimationOrFadeOutToHide();
    contextual_tooltip::LogNudgeDismissedMetrics(
        contextual_tooltip::TooltipType::kBackGesture,
        contextual_tooltip::DismissNudgeReason::kBackGestureStarted);
  }
}

void BackGestureContextualNudgeControllerImpl::OnActiveUserSessionChanged(
    const AccountId& account_id) {
  UpdateWindowMonitoring(/*can_show_nudge_immediately=*/true);
}

void BackGestureContextualNudgeControllerImpl::OnSessionStateChanged(
    session_manager::SessionState state) {
  UpdateWindowMonitoring(/*can_show_nudge_immediately=*/true);
}

void BackGestureContextualNudgeControllerImpl::OnTabletModeStarted() {
  UpdateWindowMonitoring(/*can_show_nudge_immediately=*/true);
}

void BackGestureContextualNudgeControllerImpl::OnTabletModeEnded() {
  UpdateWindowMonitoring(/*can_show_nudge_immediately=*/false);
}

void BackGestureContextualNudgeControllerImpl::OnTabletControllerDestroyed() {
  DoCleanUp();
}

void BackGestureContextualNudgeControllerImpl::OnWindowActivated(
    ActivationReason reason,
    aura::Window* gained_active,
    aura::Window* lost_active) {
  if (!gained_active)
    return;

  // If another window is activated when the nudge is waiting to be shown or
  // is currently being shown, cancel the animation.
  if (nudge_) {
    nudge_->CancelAnimationOrFadeOutToHide();
    contextual_tooltip::LogNudgeDismissedMetrics(
        contextual_tooltip::TooltipType::kBackGesture,
        contextual_tooltip::DismissNudgeReason::kActiveWindowChanged);
  }

  if (!nudge_ || !nudge_->ShouldNudgeCountAsShown()) {
    // Start tracking |gained_active|'s navigation status and show the
    // contextual nudge ui if applicable.
    nudge_delegate_->MaybeStartTrackingNavigation(gained_active);
  }
}

void BackGestureContextualNudgeControllerImpl::NavigationEntryChanged(
    aura::Window* window) {
  // If navigation entry changed when the nudge is waiting to be shown or is
  // currently being shown, cancel the animation.
  if (nudge_) {
    nudge_->CancelAnimationOrFadeOutToHide();
    contextual_tooltip::LogNudgeDismissedMetrics(
        contextual_tooltip::TooltipType::kBackGesture,
        contextual_tooltip::DismissNudgeReason::kNavigationEntryChanged);
  }

  MaybeShowNudgeUi(window);
}

bool BackGestureContextualNudgeControllerImpl::CanShowNudge(
    base::TimeDelta* recheck_delay) const {
  if (!Shell::Get()->IsInTabletMode())
    return false;

  if (Shell::Get()->session_controller()->GetSessionState() !=
      session_manager::SessionState::ACTIVE) {
    return false;
  }

  return contextual_tooltip::ShouldShowNudge(
      GetActivePrefService(), contextual_tooltip::TooltipType::kBackGesture,
      recheck_delay);
}

void BackGestureContextualNudgeControllerImpl::MaybeShowNudgeUi(
    aura::Window* window) {
  if ((!nudge_ || !nudge_->ShouldNudgeCountAsShown()) &&
      window->type() == aura::client::WINDOW_TYPE_NORMAL &&
      !window->is_destroying() &&
      Shell::Get()->shell_delegate()->CanGoBack(window) &&
      CanShowNudge(nullptr)) {
    contextual_tooltip::SetBackGestureNudgeShowing(true);
    nudge_ = std::make_unique<BackGestureContextualNudge>(base::BindOnce(
        &BackGestureContextualNudgeControllerImpl::OnNudgeAnimationFinished,
        weak_ptr_factory_.GetWeakPtr()));
  }
}

void BackGestureContextualNudgeControllerImpl::UpdateWindowMonitoring(
    bool can_show_nudge_immediately) {
  base::TimeDelta recheck_delay;
  const bool should_monitor = CanShowNudge(&recheck_delay);

  // If the nudge cannot be shown at this time, but could become available after
  // some time, schedule a timer to re-evaluate window monitoring state.
  if (!should_monitor && !recheck_delay.is_zero()) {
    auto_show_timer_.Start(
        FROM_HERE, recheck_delay,
        base::BindOnce(
            &BackGestureContextualNudgeControllerImpl::UpdateWindowMonitoring,
            base::Unretained(this), /*can_show_nudge_immediately=*/false));
  }

  if (is_monitoring_windows_ == should_monitor)
    return;
  is_monitoring_windows_ = should_monitor;

  if (should_monitor) {
    // Start monitoring window.
    nudge_delegate_ = Shell::Get()
                          ->shell_delegate()
                          ->CreateBackGestureContextualNudgeDelegate(this);
    // If there is an active window at this moment and we should monitor its
    // navigation status, start monitoring it now.
    aura::Window* active_window = window_util::GetActiveWindow();
    if (active_window) {
      if (can_show_nudge_immediately)
        MaybeShowNudgeUi(active_window);
      nudge_delegate_->MaybeStartTrackingNavigation(active_window);
    }

    Shell::Get()->activation_client()->AddObserver(this);
    return;
  }

  // Stop monitoring window.
  nudge_delegate_.reset();
  Shell::Get()->activation_client()->RemoveObserver(this);
  // Cancel any in-waiting animation or in-progress animation.
  if (nudge_) {
    nudge_->CancelAnimationOrFadeOutToHide();

    if (!Shell::Get()->IsInTabletMode()) {
      contextual_tooltip::LogNudgeDismissedMetrics(
          contextual_tooltip::TooltipType::kBackGesture,
          contextual_tooltip::DismissNudgeReason::kSwitchToClamshell);
    } else if (Shell::Get()->session_controller()->GetSessionState() !=
               session_manager::SessionState::ACTIVE) {
      contextual_tooltip::LogNudgeDismissedMetrics(
          contextual_tooltip::TooltipType::kBackGesture,
          contextual_tooltip::DismissNudgeReason::kUserSessionInactive);
    } else {
      contextual_tooltip::LogNudgeDismissedMetrics(
          contextual_tooltip::TooltipType::kBackGesture,
          contextual_tooltip::DismissNudgeReason::kOther);
    }
  }
}

void BackGestureContextualNudgeControllerImpl::OnNudgeAnimationFinished() {
  const bool count_as_shown = nudge_->ShouldNudgeCountAsShown();
  // UpdateWindowMonitoring() might attempt to cancel any in-progress nudge,
  // which would switch the nudge into an invalid state. Reset the nudge before
  // window monitoring is updated.
  nudge_.reset();

  contextual_tooltip::SetBackGestureNudgeShowing(false);

  if (count_as_shown) {
    contextual_tooltip::HandleNudgeShown(
        GetActivePrefService(), contextual_tooltip::TooltipType::kBackGesture);
    contextual_tooltip::LogNudgeDismissedMetrics(
        contextual_tooltip::TooltipType::kBackGesture,
        contextual_tooltip::DismissNudgeReason::kTimeout);
    UpdateWindowMonitoring(/*can_show_nudge_immediately=*/false);

    // Set a timer to monitoring windows and show nudge ui again.
    auto_show_timer_.Start(
        FROM_HERE, contextual_tooltip::kMinInterval,
        base::BindOnce(
            &BackGestureContextualNudgeControllerImpl::UpdateWindowMonitoring,
            base::Unretained(this),
            /*can_show_nudge_immediately=*/false));
  }
}

void BackGestureContextualNudgeControllerImpl::DoCleanUp() {
  tablet_mode_observer_.RemoveAll();

  if (is_monitoring_windows_) {
    Shell::Get()->activation_client()->RemoveObserver(this);
    nudge_delegate_.reset();
  }

  nudge_.reset();
  contextual_tooltip::SetBackGestureNudgeShowing(false);
  is_monitoring_windows_ = false;
}

}  // namespace ash
