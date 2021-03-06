// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
// #import {TestBrowserProxy} from 'chrome://test/test_browser_proxy.m.js';
// clang-format on

cr.define('android_info', function() {
  /**
   * Test value for messages for web permissions origin.
   */
  /* #export */ const TEST_ANDROID_SMS_ORIGIN = 'http://foo.com';

  /** @implements {settings.AndroidInfoBrowserProxy} */
  /* #export */ class TestAndroidInfoBrowserProxy extends TestBrowserProxy {
    constructor() {
      super([
        'getAndroidSmsInfo',
      ]);
      this.androidSmsInfo = {origin: TEST_ANDROID_SMS_ORIGIN, enabled: true};
    }

    /** @override */
    getAndroidSmsInfo() {
      this.methodCalled('getAndroidSmsInfo');
      return Promise.resolve(this.androidSmsInfo);
    }
  }

  // #cr_define_end
  return {
    TestAndroidInfoBrowserProxy: TestAndroidInfoBrowserProxy,
    TEST_ANDROID_SMS_ORIGIN: TEST_ANDROID_SMS_ORIGIN,
  };
});
