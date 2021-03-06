// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/services/local_search_service/local_search_service_impl.h"

#include <utility>

#include "base/bind.h"
#include "chrome/services/local_search_service/index_impl.h"
#include "chrome/services/local_search_service/public/mojom/types.mojom.h"

namespace local_search_service {

LocalSearchServiceImpl::LocalSearchServiceImpl() = default;

LocalSearchServiceImpl::~LocalSearchServiceImpl() = default;

void LocalSearchServiceImpl::BindReceiver(
    mojo::PendingReceiver<mojom::LocalSearchService> receiver) {
  receivers_.Add(this, std::move(receiver));
}

void LocalSearchServiceImpl::GetIndex(
    mojom::LocalSearchService::IndexId index_id,
    mojo::PendingReceiver<mojom::Index> index) {
  if (index_id != mojom::LocalSearchService::IndexId::CROS_SETTINGS)
    return;

  auto* it = IndexLookupOrCreate(local_search_service::IndexId::kCrosSettings);
  it->BindReceiver(std::move(index));
}

IndexImpl* LocalSearchServiceImpl::GetIndexImpl(
    local_search_service::IndexId index_id) {
  return IndexLookupOrCreate(index_id);
}

IndexImpl* LocalSearchServiceImpl::IndexLookupOrCreate(
    local_search_service::IndexId index_id) {
  auto it = indices_.find(index_id);
  if (it == indices_.end())
    it = indices_.emplace(index_id, std::make_unique<IndexImpl>()).first;

  DCHECK(it != indices_.end());
  DCHECK(it->second);

  return it->second.get();
}

}  // namespace local_search_service
