// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
// #import {ImportDataBrowserProxyImpl, ImportDataStatus} from 'chrome://settings/lazy_load.js';
// #import {TestBrowserProxy} from 'chrome://test/test_browser_proxy.m.js';
// #import {flush} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';
// clang-format on

/** @implements {settings.ImportDataBrowserProxy} */
class TestImportDataBrowserProxy extends TestBrowserProxy {
  constructor() {
    super([
      'initializeImportDialog',
      'importFromBookmarksFile',
      'importData',
    ]);

    /** @private {!Array<!settings.BrowserProfile} */
    this.browserProfiles_ = [];
  }

  /** @param {!Array<!settings.BrowserProfile} browserProfiles */
  setBrowserProfiles(browserProfiles) {
    this.browserProfiles_ = browserProfiles;
  }

  /** @override */
  initializeImportDialog() {
    this.methodCalled('initializeImportDialog');
    return Promise.resolve(this.browserProfiles_.slice());
  }

  /** @override */
  importFromBookmarksFile() {
    this.methodCalled('importFromBookmarksFile');
  }

  /** @override */
  importData(browserProfileIndex, types) {
    this.methodCalled('importData', [browserProfileIndex, types]);
  }
}

suite('ImportDataDialog', function() {
  /** @type {!Array<!settings.BrowserProfile} */
  const browserProfiles = [
    {
      autofillFormData: true,
      favorites: true,
      history: true,
      index: 0,
      name: 'Mozilla Firefox',
      passwords: true,
      search: true
    },
    {
      autofillFormData: true,
      favorites: true,
      history: true,
      index: 1,
      name: 'Mozilla Firefox',
      passwords: true,
      profileName: 'My profile',
      search: true
    },
    {
      autofillFormData: false,
      favorites: true,
      history: false,
      index: 2,
      name: 'Bookmarks HTML File',
      passwords: false,
      search: false
    },
  ];

  function createBooleanPref(name) {
    return {
      key: name,
      type: chrome.settingsPrivate.PrefType.BOOLEAN,
      value: true,
    };
  }

  const prefs = {};
  ['import_dialog_history', 'import_dialog_bookmarks',
   'import_dialog_saved_passwords', 'import_dialog_search_engine',
   'import_dialog_autofill_form_data',
  ].forEach(function(name) {
    prefs[name] = createBooleanPref(name);
  });

  let dialog = null;

  let browserProxy = null;

  setup(function() {
    browserProxy = new TestImportDataBrowserProxy();
    browserProxy.setBrowserProfiles(browserProfiles);
    settings.ImportDataBrowserProxyImpl.instance_ = browserProxy;
    PolymerTest.clearBody();
    dialog = document.createElement('settings-import-data-dialog');
    dialog.set('prefs', prefs);
    document.body.appendChild(dialog);
    return browserProxy.whenCalled('initializeImportDialog').then(function() {
      assertTrue(dialog.$.dialog.open);
      Polymer.dom.flush();
    });
  });

  function simulateBrowserProfileChange(index) {
    dialog.$.browserSelect.selectedIndex = index;
    dialog.$.browserSelect.dispatchEvent(new CustomEvent('change'));
  }

  test('Initialization', function() {
    assertFalse(dialog.$.import.hidden);
    assertFalse(dialog.$.import.disabled);
    assertFalse(dialog.$.cancel.hidden);
    assertFalse(dialog.$.cancel.disabled);
    assertTrue(dialog.$.done.hidden);
    assertTrue(dialog.$.successIcon.parentElement.hidden);

    // Check that the displayed text correctly combines browser name and profile
    // name (if any).
    const expectedText = [
      'Mozilla Firefox',
      'Mozilla Firefox - My profile',
      'Bookmarks HTML File',
    ];

    Array.from(dialog.$.browserSelect.options).forEach((option, i) => {
      assertEquals(expectedText[i], option.textContent.trim());
    });
  });

  test('ImportButton', function() {
    assertFalse(dialog.$.import.disabled);

    // Flip all prefs to false.
    Object.keys(prefs).forEach(function(prefName) {
      dialog.set('prefs.' + prefName + '.value', false);
    });
    assertTrue(dialog.$.import.disabled);

    // Change browser selection to "Import from Bookmarks HTML file".
    simulateBrowserProfileChange(2);
    assertTrue(dialog.$.import.disabled);

    // Ensure everything except |import_dialog_bookmarks| is ignored.
    dialog.set('prefs.import_dialog_history.value', true);
    assertTrue(dialog.$.import.disabled);

    dialog.set('prefs.import_dialog_bookmarks.value', true);
    assertFalse(dialog.$.import.disabled);
  });

  function assertInProgressButtons() {
    assertFalse(dialog.$.import.hidden);
    assertTrue(dialog.$.import.disabled);
    assertFalse(dialog.$.cancel.hidden);
    assertTrue(dialog.$.cancel.disabled);
    assertTrue(dialog.$.done.hidden);
    assertTrue(dialog.$$('paper-spinner-lite').active);
    assertFalse(dialog.$$('paper-spinner-lite').hidden);
  }

  function assertSucceededButtons() {
    assertTrue(dialog.$.import.hidden);
    assertTrue(dialog.$.cancel.hidden);
    assertFalse(dialog.$.done.hidden);
    assertFalse(dialog.$$('paper-spinner-lite').active);
    assertTrue(dialog.$$('paper-spinner-lite').hidden);
  }

  /** @param {!settings.ImportDataStatus} status */
  function simulateImportStatusChange(status) {
    cr.webUIListenerCallback('import-data-status-changed', status);
  }

  test('ImportFromBookmarksFile', function() {
    simulateBrowserProfileChange(2);
    dialog.$.import.click();
    return browserProxy.whenCalled('importFromBookmarksFile').then(function() {
      simulateImportStatusChange(settings.ImportDataStatus.IN_PROGRESS);
      assertInProgressButtons();

      simulateImportStatusChange(settings.ImportDataStatus.SUCCEEDED);
      assertSucceededButtons();

      assertFalse(dialog.$.successIcon.parentElement.hidden);
      assertFalse(dialog.$$('settings-toggle-button').parentElement.hidden);
    });
  });

  test('ImportFromBrowserProfile', function() {
    dialog.set('prefs.import_dialog_bookmarks.value', false);
    dialog.set('prefs.import_dialog_search_engine.value', true);

    const expectedIndex = 0;
    simulateBrowserProfileChange(expectedIndex);
    dialog.$.import.click();

    const importCalled = browserProxy.whenCalled('importData');
    return importCalled.then(([actualIndex, types]) => {
      assertEquals(expectedIndex, actualIndex);
      assertFalse(types['import_dialog_bookmarks']);
      assertTrue(types['import_dialog_search_engine']);

      simulateImportStatusChange(settings.ImportDataStatus.IN_PROGRESS);
      assertInProgressButtons();

      simulateImportStatusChange(settings.ImportDataStatus.SUCCEEDED);
      assertSucceededButtons();

      assertFalse(dialog.$.successIcon.parentElement.hidden);
      assertTrue(dialog.$$('settings-toggle-button').parentElement.hidden);
    });
  });

  test('ImportError', function() {
    simulateImportStatusChange(settings.ImportDataStatus.FAILED);
    assertFalse(dialog.$.dialog.open);
  });
});
