// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.tab.state;

import android.support.test.filters.SmallTest;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import org.chromium.base.Callback;
import org.chromium.base.ThreadUtils;
import org.chromium.base.test.BaseJUnit4ClassRunner;
import org.chromium.chrome.test.ChromeBrowserTestRule;

import java.io.File;
import java.util.concurrent.Semaphore;

/**
 * Tests relating to  {@link FilePersistedTabDataStorage}
 */
@RunWith(BaseJUnit4ClassRunner.class)
public class FilePersistedTabDataStorageTest {
    @Rule
    public final ChromeBrowserTestRule mBrowserTestRule = new ChromeBrowserTestRule();

    @Mock
    private Callback<byte[]> mByteArrayCallback;

    private static final int TAB_ID = 1;
    private static final byte[] DATA = {13, 14};
    private static final String DATA_ID = "DataId";

    private byte[] mResult;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @SmallTest
    @Test
    public void testFilePersistedDataStorageNonEncrypted() throws InterruptedException {
        testFilePersistedDataStorage(new FilePersistedTabDataStorage());
    }

    @SmallTest
    @Test
    public void testFilePersistedDataStorageEncrypted() throws InterruptedException {
        testFilePersistedDataStorage(new EncryptedFilePersistedTabDataStorage());
    }

    private void testFilePersistedDataStorage(FilePersistedTabDataStorage persistedTabDataStorage)
            throws InterruptedException {
        final Semaphore semaphore = new Semaphore(0);
        Callback<byte[]> callback = new Callback<byte[]>() {
            @Override
            public void onResult(byte[] res) {
                mResult = res;
                semaphore.release();
            }
        };
        ThreadUtils.runOnUiThreadBlocking(() -> {
            persistedTabDataStorage.save(TAB_ID, DATA_ID, DATA);
            persistedTabDataStorage.restore(TAB_ID, DATA_ID, callback);
        });
        semaphore.acquire();
        Assert.assertEquals(mResult.length, 2);
        Assert.assertArrayEquals(mResult, DATA);
        File file = persistedTabDataStorage.getFile(TAB_ID, DATA_ID);
        Assert.assertTrue(file.exists());

        ThreadUtils.runOnUiThreadBlocking(() -> {
            persistedTabDataStorage.delete(TAB_ID, DATA_ID);
            semaphore.release();
        });
        semaphore.acquire();
        Assert.assertFalse(file.exists());
    }
}
