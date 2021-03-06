// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.omnibox.suggestions;

import org.chromium.ui.modelutil.MVCListAdapter.ModelList;
import org.chromium.ui.modelutil.PropertyKey;
import org.chromium.ui.modelutil.PropertyModel.WritableBooleanPropertyKey;
import org.chromium.ui.modelutil.PropertyModel.WritableObjectPropertyKey;

/**
 * The properties controlling the state of the list of suggestion items.
 */
public class SuggestionListProperties {
    /**
     * Interface that will receive notifications and callbacks from OmniboxSuggestionList.
     */
    public interface SuggestionListObserver {
        /**
         * Invoked whenever the height of suggestion list changes.
         * The height may change as a result of eg. soft keyboard popping up.
         *
         * @param newHeightPx New height of the suggestion list in pixels.
         */
        void onSuggestionListHeightChanged(int newHeightPx);

        /**
         * Invoked whenever the User scrolls the list.
         */
        void onSuggestionListScroll();
    }

    /** Whether the suggestion list is visible. */
    public static final WritableBooleanPropertyKey VISIBLE = new WritableBooleanPropertyKey();

    /** The embedder for the suggestion list. */
    public static final WritableObjectPropertyKey<OmniboxSuggestionListEmbedder> EMBEDDER =
            new WritableObjectPropertyKey<>();

    /**
     * The list of models controlling the state of the suggestion items. This should never be
     * bound to the same view more than once.
     */
    public static final WritableObjectPropertyKey<ModelList> SUGGESTION_MODELS =
            new WritableObjectPropertyKey<>(true);

    /** Whether the suggestion list should have a dark background. */
    public static final WritableBooleanPropertyKey IS_INCOGNITO = new WritableBooleanPropertyKey();

    /** Observer that will receive notifications and callbacks from Suggestion List. */
    public static final WritableObjectPropertyKey<SuggestionListObserver> OBSERVER =
            new WritableObjectPropertyKey<>();

    public static final PropertyKey[] ALL_KEYS =
            new PropertyKey[] {VISIBLE, EMBEDDER, SUGGESTION_MODELS, IS_INCOGNITO, OBSERVER};
}
