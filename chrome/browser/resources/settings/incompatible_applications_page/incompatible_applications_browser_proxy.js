// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
// #import {addSingletonGetter, sendWithPromise} from 'chrome://resources/js/cr.m.js';
// clang-format on

cr.define('settings', function() {
  /**
   * All possible actions to take on an incompatible application.
   *
   * Must be kept in sync with BlacklistMessageType in
   * chrome/browser/win/conflicts/proto/module_list.proto
   * @readonly
   * @enum {number}
   */
  /* #export */ const ActionTypes = {
    UNINSTALL: 0,
    MORE_INFO: 1,
    UPGRADE: 2,
  };

  /**
   * @typedef {{
   *   name: string,
   *   actionType: {settings.ActionTypes},
   *   actionUrl: string,
   * }}
   */
  /* #export */ let IncompatibleApplication;

  /** @interface */
  /* #export */ class IncompatibleApplicationsBrowserProxy {
    /**
     * Get the list of incompatible applications.
     * @return {!Promise<!Array<!settings.IncompatibleApplication>>}
     */
    requestIncompatibleApplicationsList() {}

    /**
     * Launches the Apps & Features page that allows uninstalling
     * 'applicationName'.
     * @param {string} applicationName
     */
    startApplicationUninstallation(applicationName) {}

    /**
     * Opens the specified URL in a new tab.
     * @param {!string} url
     */
    openURL(url) {}

    /**
     * Requests the plural string for the subtitle of the Incompatible
     * Applications subpage.
     * @param {number} numApplications
     * @return {!Promise<string>}
     */
    getSubtitlePluralString(numApplications) {}

    /**
     * Requests the plural string for the subtitle of the Incompatible
     * Applications subpage, when the user does not have administrator rights.
     * @param {number} numApplications
     * @return {!Promise<string>}
     */
    getSubtitleNoAdminRightsPluralString(numApplications) {}

    /**
     * Requests the plural string for the title of the list of Incompatible
     * Applications.
     * @param {number} numApplications
     * @return {!Promise<string>}
     */
    getListTitlePluralString(numApplications) {}
  }

  /** @implements {settings.IncompatibleApplicationsBrowserProxy} */
  /* #export */ class IncompatibleApplicationsBrowserProxyImpl {
    /** @override */
    requestIncompatibleApplicationsList() {
      return cr.sendWithPromise('requestIncompatibleApplicationsList');
    }

    /** @override */
    startApplicationUninstallation(applicationName) {
      chrome.send('startApplicationUninstallation', [applicationName]);
    }

    /** @override */
    openURL(url) {
      window.open(url);
    }

    /** @override */
    getSubtitlePluralString(numApplications) {
      return cr.sendWithPromise('getSubtitlePluralString', numApplications);
    }

    /** @override */
    getSubtitleNoAdminRightsPluralString(numApplications) {
      return cr.sendWithPromise(
          'getSubtitleNoAdminRightsPluralString', numApplications);
    }

    /** @override */
    getListTitlePluralString(numApplications) {
      return cr.sendWithPromise('getListTitlePluralString', numApplications);
    }
  }

  cr.addSingletonGetter(IncompatibleApplicationsBrowserProxyImpl);

  // #cr_define_end
  return {
    ActionTypes,
    IncompatibleApplication,
    IncompatibleApplicationsBrowserProxy,
    IncompatibleApplicationsBrowserProxyImpl,
  };
});
