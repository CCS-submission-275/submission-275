// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Tests for shared Polymer 3 components. */

// Polymer BrowserTest fixture.
GEN_INCLUDE(['//chrome/test/data/webui/polymer_browser_test_base.js']);
GEN('#include "services/network/public/cpp/features.h"');

/** Test fixture for shared Polymer 3 components. */
// eslint-disable-next-line no-var
var CrComponentsV3BrowserTest = class extends PolymerTest {
  /** @override */
  get browsePreload() {
    return 'chrome://dummyurl';
  }

  /** @override */
  get extraLibraries() {
    return [
      '//third_party/mocha/mocha.js',
      '//chrome/test/data/webui/mocha_adapter.js',
    ];
  }

  /** @override */
  get webuiHost() {
    return 'dummyurl';
  }
};

// eslint-disable-next-line no-var
var CrComponentsManagedFootnoteV3Test =
    class extends CrComponentsV3BrowserTest {
  /** @override */
  get browsePreload() {
    return 'chrome://test?module=cr_components/managed_footnote_test.m.js';
  }
};

TEST_F('CrComponentsManagedFootnoteV3Test', 'All', function() {
  mocha.run();
});

GEN('#if defined(USE_NSS_CERTS)');

/**
 * Test fixture for chrome://settings/certificates. This tests the
 * certificate-manager component in the context of the Settings privacy page.
 */
// eslint-disable-next-line no-var
var CrComponentsCertificateManagerV3Test =
    class extends CrComponentsV3BrowserTest {
  /** @override */
  get browsePreload() {
    return 'chrome://settings/test_loader.html?module=cr_components/certificate_manager_test.m.js';
  }

  /** @override */
  get featureList() {
    return {enabled: ['network::features::kOutOfBlinkCors']};
  }
};

TEST_F('CrComponentsCertificateManagerV3Test', 'All', function() {
  mocha.run();
});

GEN('#endif  // defined(USE_NSS_CERTS)');
