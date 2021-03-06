// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_COMMON_CREDENTIAL_PROVIDER_ARCHIVABLE_CREDENTIAL_STORE_H_
#define IOS_CHROME_COMMON_CREDENTIAL_PROVIDER_ARCHIVABLE_CREDENTIAL_STORE_H_

#import <Foundation/Foundation.h>

#import "ios/chrome/common/credential_provider/credential_store.h"

@class ArchivableCredential;

// Credential store built on top of keyed archiver and unarchiver to persist
// data. Can also be used as a memory only store. Use |saveDataWithCompletion:|
// to update the data on disk. All operations will be held in memory until saved
// to disk, making it possible to batch multiple operations.
@interface ArchivableCredentialStore : NSObject <CredentialStore>

// Initializes the store. |fileURL| is where the store should live in disk. If
// the file doesn't exist, it will be created on first save. If the file was not
// created by this class the store won't be able to load it and the behavior
// will be unexpected. In general |fileURL| can be a temp file for testing, or a
// shared resource path in order to be used in an extension. Initialize the
// store with |fileURL| = nil to effectively create a memory only store.
- (instancetype)initWithFileURL:(NSURL*)fileURL NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

// Adds a credential to the memory storage. Use |saveDataWithCompletion:| to
// update the data on disk.
- (void)addCredential:(ArchivableCredential*)credential;

// Updates a credential in the memory storage. Use |saveDataWithCompletion:| to
// update the data on disk.
- (void)updateCredential:(ArchivableCredential*)credential;

// Removes a credential from the memory storage. Use |saveDataWithCompletion:|
// to update the data on disk.
- (void)removeCredential:(ArchivableCredential*)credential;

@end

#endif  // IOS_CHROME_COMMON_CREDENTIAL_PROVIDER_ARCHIVABLE_CREDENTIAL_STORE_H_
