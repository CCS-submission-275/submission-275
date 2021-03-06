// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.background_sync;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.chrome.browser.webapps.WebappRegistry;

import java.util.Locale;

/**
 * The {@link BackgroundSyncPwaDetector} singleton is responsible for detecting
 * installed PWAs (TWA and WebAPK) on Android.
 * This is used to limit access to Periodic Background Sync APIs.
 *
 * Thread model: This class is to be run on the UI thread only.
 */
public class BackgroundSyncPwaDetector {
    /**
     * Returns true if there's at least one PWA installed for the given origin, false otherwise.
     * Installed PWAs are associated with a scope, while permissions are attributed to an origin.
     * Because we're only passed the requesting origin from the permissions infrastructure, we can't
     * match the scope of installed PWAs to the exact URL of the permission request. We instead
     * look for any installed PWA for the requesting origin. With this logic, if there's already a
     * PWA installed for travel.google.com, and a request to register Periodic Background Sync comes
     * in from maps.google.com, this method will return true and registration will succeed, provided
     * other required conditions are met.
     *
     * @param origin The origin for which to look for a PWA.
     */
    @CalledByNative
    public static boolean isPwaInstalled(String origin) {
        return WebappRegistry.getInstance().hasAtLeastOneWebApkForOrigin(
                origin.toLowerCase(Locale.getDefault()));
    }

    /**
     * Returns true if there's a TWA installed for the given origin, false otherwise.
     * @param origin The origin for which to look for a TWA.
     */
    @CalledByNative
    public static boolean isTwaInstalled(String origin) {
        return WebappRegistry.getInstance().getTrustedWebActivityPermissionStore().isTwaInstalled(
                origin.toLowerCase(Locale.getDefault()));
    }
}