// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.autofill_assistant;

import android.app.Activity;
import android.widget.TextView;

import org.chromium.chrome.browser.compositor.CompositorViewHolder;
import org.chromium.ui.KeyboardVisibilityDelegate.KeyboardVisibilityListener;
import org.chromium.ui.base.ActivityKeyboardVisibilityDelegate;

/**
 * Coordinator responsible for enabling or disabling the soft keyboard.
 */
class AssistantKeyboardCoordinator {
    private final Activity mActivity;
    private final ActivityKeyboardVisibilityDelegate mKeyboardDelegate;
    private final CompositorViewHolder mCompositorViewHolder;
    private final KeyboardVisibilityListener mKeyboardVisibilityListener =
            this::onKeyboardVisibilityChanged;
    private boolean mAllowShowingSoftKeyboard = true;
    private Delegate mDelegate;

    interface Delegate {
        void onKeyboardVisibilityChanged(boolean visible);
    }

    AssistantKeyboardCoordinator(Activity activity,
            ActivityKeyboardVisibilityDelegate keyboardVisibilityDelegate,
            CompositorViewHolder compositorViewHolder, AssistantModel model, Delegate delegate) {
        mActivity = activity;
        mKeyboardDelegate = keyboardVisibilityDelegate;
        mCompositorViewHolder = compositorViewHolder;
        mDelegate = delegate;

        model.addObserver((source, propertyKey) -> {
            if (AssistantModel.VISIBLE == propertyKey) {
                if (model.get(AssistantModel.VISIBLE)) {
                    enableListenForKeyboardVisibility(true);
                } else {
                    enableListenForKeyboardVisibility(false);
                }
            } else if (AssistantModel.ALLOW_SOFT_KEYBOARD == propertyKey) {
                allowShowingSoftKeyboard(model.get(AssistantModel.ALLOW_SOFT_KEYBOARD));
            }
        });
    }

    /** Returns whether the keyboard is currently shown. */
    boolean isKeyboardShown() {
        return mKeyboardDelegate.isKeyboardShowing(mActivity, mCompositorViewHolder);
    }

    /** Hides the keyboard. */
    void hideKeyboard() {
        mKeyboardDelegate.hideKeyboard(mCompositorViewHolder);
    }

    /** Hides the keyboard after a delay if the focus is not on a TextView */
    void hideKeyboardIfFocusNotOnText() {
        if (!(mActivity.getCurrentFocus() instanceof TextView)) {
            hideKeyboard();
        }
    }

    /**
     * Enable or disable the soft keyboard.
     */
    private void allowShowingSoftKeyboard(boolean allowed) {
        mAllowShowingSoftKeyboard = allowed;
        if (!allowed) {
            mKeyboardDelegate.hideKeyboard(mCompositorViewHolder);
        }
    }

    /**
     * Start or stop listening for keyboard visibility changes.
     */
    private void enableListenForKeyboardVisibility(boolean enabled) {
        if (enabled) {
            mKeyboardDelegate.addKeyboardVisibilityListener(mKeyboardVisibilityListener);
        } else {
            mKeyboardDelegate.removeKeyboardVisibilityListener(mKeyboardVisibilityListener);
        }
    }

    // TODO(crbug.com/806868): Current solution only hides the keyboard once it has been already
    // shown. We should improve it and prevent from the showing in the first place.
    private void onKeyboardVisibilityChanged(boolean isShowing) {
        mDelegate.onKeyboardVisibilityChanged(isShowing);
        if (isShowing && !mAllowShowingSoftKeyboard) {
            hideKeyboard();
        }
    }
}
