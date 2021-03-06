// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import * as Comlink from '../lib/comlink.js';
import {
  AsyncWriter,  // eslint-disable-line no-unused-vars
} from './async_writer.js';

/**
 * A no-op video processor.
 * TODO(shik): Convert the video format in the processor.
 */
class VideoProcessor {
  /**
   * @param {!AsyncWriter} output
   */
  constructor(output) {
    /**
     * @type {!AsyncWriter}
     * @private
     */
    this.output_ = output;
  }

  /**
   * @param {!Blob} blob
   */
  write(blob) {
    this.output_.write(blob);
  }

  /**
   * @return {!Promise}
   */
  async close() {
    await this.output_.close();
  }
}

Comlink.expose(VideoProcessor);
