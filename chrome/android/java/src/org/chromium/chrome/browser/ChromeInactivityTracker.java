// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser;

import androidx.annotation.VisibleForTesting;

import org.chromium.chrome.browser.lifecycle.ActivityLifecycleDispatcher;
import org.chromium.chrome.browser.lifecycle.Destroyable;
import org.chromium.chrome.browser.lifecycle.PauseResumeWithNativeObserver;
import org.chromium.chrome.browser.lifecycle.StartStopWithNativeObserver;
import org.chromium.chrome.browser.preferences.SharedPreferencesManager;

/**
 * Manages pref that can track the delay since the last stop of the tracked activity.
 */
public class ChromeInactivityTracker
        implements StartStopWithNativeObserver, PauseResumeWithNativeObserver, Destroyable {
    private static final String TAG = "InactivityTracker";

    private static final long UNKNOWN_LAST_BACKGROUNDED_TIME = -1;

    private final String mPrefName;
    private final ActivityLifecycleDispatcher mLifecycleDispatcher;

    /**
     * Creates an inactivity tracker without a timeout callback. This is useful if clients only
     * want to query the inactivity state manually.
     * @param prefName the location in shared preferences that the timestamp is stored.
     * @param lifecycleDispatcher tracks the lifecycle of the Activity of interest, and calls
     *     observer methods on ChromeInactivityTracker.
     */
    public ChromeInactivityTracker(
            String prefName, ActivityLifecycleDispatcher lifecycleDispatcher) {
        mPrefName = prefName;
        mLifecycleDispatcher = lifecycleDispatcher;
        mLifecycleDispatcher.register(this);
    }

    /**
     * Updates the shared preferences to contain the given time. Used internally and for tests.
     * @param timeInMillis the time to record.
     */
    @VisibleForTesting
    public void setLastBackgroundedTimeInPrefs(long timeInMillis) {
        SharedPreferencesManager.getInstance().writeLong(mPrefName, timeInMillis);
    }

    /**
     * @return The last backgrounded time in millis.
     */
    public long getLastBackgroundedTimeMs() {
        return SharedPreferencesManager.getInstance().readLong(
                mPrefName, UNKNOWN_LAST_BACKGROUNDED_TIME);
    }

    /**
     * @return the time interval in millis since the last backgrounded time.
     */
    public long getTimeSinceLastBackgroundedMs() {
        long lastBackgroundedTimeMs = getLastBackgroundedTimeMs();
        if (lastBackgroundedTimeMs == UNKNOWN_LAST_BACKGROUNDED_TIME) {
            return UNKNOWN_LAST_BACKGROUNDED_TIME;
        }
        return System.currentTimeMillis() - lastBackgroundedTimeMs;
    }

    @Override
    public void onStartWithNative() {}

    @Override
    public void onResumeWithNative() {
        // We clear the backgrounded time here, rather than in #onStartWithNative, to give
        // handlers the chance to respond to inactivity during any onStartWithNative handler
        // regardless of ordering. onResume is always called after onStart, and it should be fine to
        // consider Chrome active if it reaches onResume.
        setLastBackgroundedTimeInPrefs(UNKNOWN_LAST_BACKGROUNDED_TIME);
    }

    @Override
    public void onPauseWithNative() {}

    @Override
    public void onStopWithNative() {
        // Always track the last backgrounded time in case others are using the pref.
        setLastBackgroundedTimeInPrefs(System.currentTimeMillis());
    }

    @Override
    public void destroy() {
        mLifecycleDispatcher.unregister(this);
    }
}
