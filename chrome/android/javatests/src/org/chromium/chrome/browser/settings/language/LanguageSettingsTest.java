// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.settings.language;

import static android.support.test.espresso.Espresso.onView;
import static android.support.test.espresso.action.ViewActions.click;
import static android.support.test.espresso.assertion.ViewAssertions.doesNotExist;
import static android.support.test.espresso.assertion.ViewAssertions.matches;
import static android.support.test.espresso.matcher.ViewMatchers.hasSibling;
import static android.support.test.espresso.matcher.ViewMatchers.isDisplayed;
import static android.support.test.espresso.matcher.ViewMatchers.withId;
import static android.support.test.espresso.matcher.ViewMatchers.withText;

import static org.hamcrest.core.AllOf.allOf;

import android.support.test.InstrumentationRegistry;
import android.support.test.espresso.contrib.RecyclerViewActions;
import android.view.View;
import android.widget.ImageView;

import androidx.appcompat.widget.SwitchCompat;
import androidx.recyclerview.widget.RecyclerView;
import androidx.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.chromium.base.test.util.CommandLineFlags;
import org.chromium.base.test.util.Restriction;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.flags.ChromeSwitches;
import org.chromium.chrome.browser.language.settings.LanguageItem;
import org.chromium.chrome.browser.language.settings.LanguageListBaseAdapter;
import org.chromium.chrome.browser.language.settings.LanguageSettings;
import org.chromium.chrome.browser.preferences.Pref;
import org.chromium.chrome.browser.preferences.PrefServiceBridge;
import org.chromium.chrome.browser.settings.SettingsActivity;
import org.chromium.chrome.browser.settings.SettingsActivityTest;
import org.chromium.chrome.browser.translate.TranslateBridge;
import org.chromium.chrome.test.ChromeJUnit4ClassRunner;
import org.chromium.chrome.test.util.browser.RecyclerViewTestUtils;
import org.chromium.components.browser_ui.widget.listmenu.ListMenuButton;
import org.chromium.content_public.browser.test.util.TestThreadUtils;
import org.chromium.ui.test.util.UiRestriction;

/**
 * Tests for the "Languages" settings screen.
 */
@RunWith(ChromeJUnit4ClassRunner.class)
// clang-format off
@CommandLineFlags.Add({ChromeSwitches.DISABLE_FIRST_RUN_EXPERIENCE})
@Restriction(UiRestriction.RESTRICTION_TYPE_PHONE)
public class LanguageSettingsTest {
    // clang-format on

    private SettingsActivity mActivity;

    @Before
    public void setUp() throws Exception {
        mActivity = SettingsActivityTest.startSettingsActivity(
                InstrumentationRegistry.getInstrumentation(), LanguageSettings.class.getName());
    }

    private void addLanguage() {
        onView(withId(R.id.add_language)).check(matches(isDisplayed()));
        RecyclerView acceptLanguageList = mActivity.findViewById(R.id.language_list);
        int originalAcceptLanguageCount = acceptLanguageList.getChildCount();
        // Disable animation to reduce flakiness.
        acceptLanguageList.setItemAnimator(null);

        // Enter "Add language" screen.
        onView(withId(R.id.add_language)).perform(click());
        onView(withId(R.id.language_list)).check(matches(isDisplayed()));
        onView(withId(R.id.language_list))
                .perform(RecyclerViewActions.actionOnItemAtPosition(0, click()));

        // Back to "Language" screen.
        Assert.assertEquals(mActivity.getString(R.string.language_settings), mActivity.getTitle());
        acceptLanguageList = mActivity.findViewById(R.id.language_list);
        Assert.assertEquals("Failed to add a new language.", originalAcceptLanguageCount + 1,
                acceptLanguageList.getChildCount());
    }

    @Test
    @SmallTest
    public void testRemoveLanguage() {
        RecyclerView acceptLanguageList = mActivity.findViewById(R.id.language_list);
        int originalAcceptLanguageCount = acceptLanguageList.getChildCount();

        // Enter "Add language" screen.
        addLanguage();

        View newLangView =
                acceptLanguageList.findViewHolderForAdapterPosition(originalAcceptLanguageCount)
                        .itemView;

        // Toggle popup menu to remove a language.
        TestThreadUtils.runOnUiThreadBlocking(
                () -> { newLangView.findViewById(R.id.more).performClick(); });
        onView(withText(R.string.remove)).perform(click());
        Assert.assertEquals("The language is not removed.", originalAcceptLanguageCount,
                acceptLanguageList.getChildCount());
    }

    @Test
    @SmallTest
    public void testToggleOfferToTranslate() {
        RecyclerView acceptLanguageList = mActivity.findViewById(R.id.language_list);
        int originalAcceptLanguageCount = acceptLanguageList.getChildCount();

        // Enter "Add language" screen.
        addLanguage();

        Assert.assertEquals(mActivity.getString(R.string.language_settings), mActivity.getTitle());
        acceptLanguageList = mActivity.findViewById(R.id.language_list);
        Assert.assertEquals("Failed to add a new language.", originalAcceptLanguageCount + 1,
                acceptLanguageList.getChildCount());
        View newLangView =
                acceptLanguageList.findViewHolderForAdapterPosition(originalAcceptLanguageCount)
                        .itemView;
        LanguageItem languageItem = ((LanguageListBaseAdapter) acceptLanguageList.getAdapter())
                                            .getLanguageItemList()
                                            .get(originalAcceptLanguageCount);

        // Turn on "offer to translate".
        TestThreadUtils.runOnUiThreadBlocking(
                () -> { newLangView.findViewById(R.id.more).performClick(); });
        onView(withText(R.string.languages_item_option_offer_to_translate)).perform(click());

        // Verify that the "offer to translate" is on.
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            Assert.assertFalse("Language should not be blocked when 'offer to translate' is on.",
                    TranslateBridge.isBlockedLanguage(languageItem.getCode()));
        });

        RecyclerViewTestUtils.waitForStableRecyclerView(acceptLanguageList);
        // Open popup menu to verify the drawable (blue tick) is visible.
        TestThreadUtils.runOnUiThreadBlocking(
                () -> { newLangView.findViewById(R.id.more).performClick(); });

        onView(withText(R.string.languages_item_option_offer_to_translate))
                .check(matches(isDisplayed()));
        onView(allOf(hasSibling(withText(R.string.languages_item_option_offer_to_translate)),
                       withId(R.id.menu_item_end_icon)))
                .check((v, e) -> {
                    Assert.assertNotNull(
                            "There should exist an icon next to the text to indicate 'offer to translate' is on",
                            ((ImageView) v).getDrawable());
                });

        // Turn off "offer to translate".
        onView(withText(R.string.languages_item_option_offer_to_translate)).perform(click());

        TestThreadUtils.runOnUiThreadBlocking(() -> {
            Assert.assertTrue("Language should be blocked when 'offer to translate' is off.",
                    TranslateBridge.isBlockedLanguage(languageItem.getCode()));
        });

        // Open popup menu to verify the drawable (blue tick) is invisible.
        TestThreadUtils.runOnUiThreadBlocking(
                () -> { newLangView.findViewById(R.id.more).performClick(); });

        onView(allOf(hasSibling(withText(R.string.languages_item_option_offer_to_translate)),
                       withId(R.id.menu_item_end_icon)))
                .check((v, e) -> { Assert.assertNull(((ImageView) v).getDrawable()); });

        // Reset states by toggling popup menu to remove a language.
        TestThreadUtils.runOnUiThreadBlocking(
                () -> { newLangView.findViewById(R.id.more).performClick(); });
        onView(withText(R.string.remove)).perform(click());
    }

    @Test
    @SmallTest
    public void testEnabledAndDisableOfferToTranslate() {
        RecyclerView acceptLanguageList = mActivity.findViewById(R.id.language_list);
        View langView = acceptLanguageList.findViewHolderForAdapterPosition(0).itemView;
        ListMenuButton moreButton = langView.findViewById(R.id.more);
        SwitchCompat pref = mActivity.findViewById(R.id.switchWidget);

        // Restore this after test.
        boolean enabledInDefault = pref.isChecked();
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            boolean enabled =
                    PrefServiceBridge.getInstance().getBoolean(Pref.OFFER_TRANSLATE_ENABLED);
            Assert.assertEquals(
                    "The state of switch widget is different from local preference of 'offer to translate'.",
                    enabledInDefault, enabled);
        });

        // Verify that "offer to translate" is hidden or visible.
        TestThreadUtils.runOnUiThreadBlocking((Runnable) moreButton::performClick);
        onView(withText(R.string.languages_item_option_offer_to_translate))
                .check(enabledInDefault ? matches(isDisplayed()) : doesNotExist());

        // Dismiss the popup window.
        TestThreadUtils.runOnUiThreadBlocking(moreButton::dismiss);

        // Toggle the switch.
        onView(withId(R.id.switchWidget)).perform(click());
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            Assert.assertEquals(
                    "Preference of 'offer to translate' should be toggled when switch widget is clicked.",
                    !enabledInDefault,
                    PrefServiceBridge.getInstance().getBoolean(Pref.OFFER_TRANSLATE_ENABLED));
        });

        TestThreadUtils.runOnUiThreadBlocking((Runnable) moreButton::performClick);

        onView(withText(R.string.languages_item_option_offer_to_translate))
                .check(!enabledInDefault ? matches(isDisplayed()) : doesNotExist());

        // Reset state.
        TestThreadUtils.runOnUiThreadBlocking(() -> {
            PrefServiceBridge.getInstance().setBoolean(
                    Pref.OFFER_TRANSLATE_ENABLED, enabledInDefault);
        });
    }
}
