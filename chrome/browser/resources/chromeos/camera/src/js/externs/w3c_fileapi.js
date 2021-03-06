// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(inker): Put it to the externs/browser/w3c_fileapi.js of the upstream
// Closure Compiler, uprev the one bundled in Chromium repo, and remove the
// definition here.

/* eslint-disable valid-jsdoc */

/**
 * @see http://www.w3.org/TR/FileAPI/#arraybuffer-method-algo
 * @return {!Promise<!ArrayBuffer>}
 * @nosideeffects
 */
Blob.prototype.arrayBuffer = function() {};
