// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_TAB_STATE_TAB_STATE_DB_H_
#define CHROME_BROWSER_TAB_STATE_TAB_STATE_DB_H_

#include <string>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/sequenced_task_runner.h"
#include "chrome/browser/tab/state/tab_state_db_content.pb.h"
#include "components/leveldb_proto/public/proto_database.h"

namespace leveldb_proto {
class ProtoDatabaseProvider;
}  // namespace leveldb_proto

class TabStateDBTest;

// TabStateDatabase is leveldb backend store for NonCriticalPersistedTabData.
// NonCriticalPersistedTabData is an extension of TabState where data for
// new features which are not critical to the core functionality of the app
// are acquired and persisted across restarts. The intended key format is
// <NonCriticalPersistedTabData id>_<Tab id>

// NonCriticalPersistedTabData is stored in key/value pairs.
// TODO(crbug.com/1061258) add a KeyedService so TabStateDB is per profile
class TabStateDB {
 public:
  using KeyAndValue = std::pair<std::string, std::vector<uint8_t>>;

  // Callback when content is acquired
  using LoadCallback = base::OnceCallback<void(bool, std::vector<KeyAndValue>)>;

  // Used for confirming an operation was completed successfully (e.g.
  // insert, delete). This will be invoked on a different SequenceRunner
  // to TabStateDB.
  using OperationCallback = base::OnceCallback<void(bool)>;

  // Entry in the database
  using ContentEntry = leveldb_proto::ProtoDatabase<
      tab_state_db::TabStateContentProto>::KeyEntryVector;

  // Initializes the database
  TabStateDB(leveldb_proto::ProtoDatabaseProvider* proto_database_provider,
             const base::FilePath& profile_directory,
             base::OnceClosure closure);

  ~TabStateDB();

  // Returns true if initialization has finished successfully, otherwise false.
  bool IsInitialized() const;

  // Loads the content data for the key and passes them to the callback
  void LoadContent(const std::string& key, LoadCallback callback);

  // Inserts a value for a given key and passes the result (success/failure) to
  // OperationCallback
  void InsertContent(const std::string& key,
                     const std::vector<uint8_t>& value,
                     OperationCallback callback);

  // Deletes content in the database, matching all keys which have a prefix
  // that matches the key
  void DeleteContent(const std::string& key, OperationCallback callback);

  // Delete all content in the database
  void DeleteAllContent(OperationCallback callback);

 private:
  friend class ::TabStateDBTest;
  // Used for tests
  explicit TabStateDB(std::unique_ptr<leveldb_proto::ProtoDatabase<
                          tab_state_db::TabStateContentProto>> storage_database,
                      scoped_refptr<base::SequencedTaskRunner> task_runner,
                      base::OnceClosure closure);

  // Passes back database status following database initialization
  void OnDatabaseInitialized(base::OnceClosure closure,
                             leveldb_proto::Enums::InitStatus status);

  // Callback when content is loaded
  void OnLoadContent(
      LoadCallback callback,
      bool success,
      std::unique_ptr<std::vector<tab_state_db::TabStateContentProto>> content);

  // Callback when an operation (e.g. insert or delete) is called
  void OnOperationCommitted(OperationCallback callback, bool success);

  // Status of the database initialization.
  leveldb_proto::Enums::InitStatus database_status_;

  // The database for storing content storage information.
  std::unique_ptr<
      leveldb_proto::ProtoDatabase<tab_state_db::TabStateContentProto>>
      storage_database_;

  base::WeakPtrFactory<TabStateDB> weak_ptr_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(TabStateDB);
};

#endif  // CHROME_BROWSER_TAB_STATE_TAB_STATE_DB_H_
