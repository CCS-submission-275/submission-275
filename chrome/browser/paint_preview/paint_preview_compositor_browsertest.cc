// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/run_loop.h"
#include "base/test/gtest_util.h"
#include "base/unguessable_token.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "components/paint_preview/browser/compositor_utils.h"
#include "components/paint_preview/browser/paint_preview_base_service_test_factory.h"
#include "components/paint_preview/browser/paint_preview_compositor_client_impl.h"
#include "components/paint_preview/browser/paint_preview_compositor_service_impl.h"
#include "components/paint_preview/public/paint_preview_compositor_client.h"
#include "components/paint_preview/public/paint_preview_compositor_service.h"
#include "components/services/paint_preview_compositor/public/mojom/paint_preview_compositor.mojom.h"
#include "content/public/browser/service_process_host.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace paint_preview {

namespace {

// These functions cast the public API for the CompositorService and
// CompositorClient to the *Impl versions. This exposes some internal test-only
// methods to validate internal state.

std::unique_ptr<PaintPreviewCompositorServiceImpl> ToCompositorServiceImpl(
    std::unique_ptr<PaintPreviewCompositorService> service) {
  return std::unique_ptr<PaintPreviewCompositorServiceImpl>(
      reinterpret_cast<PaintPreviewCompositorServiceImpl*>(service.release()));
}

std::unique_ptr<PaintPreviewCompositorClientImpl> ToCompositorClientImpl(
    std::unique_ptr<PaintPreviewCompositorClient> client) {
  return std::unique_ptr<PaintPreviewCompositorClientImpl>(
      reinterpret_cast<PaintPreviewCompositorClientImpl*>(client.release()));
}

}  // namespace

class PaintPreviewCompositorBrowserTest : public InProcessBrowserTest {
 public:
  PaintPreviewCompositorBrowserTest() = default;
  ~PaintPreviewCompositorBrowserTest() override = default;

  PaintPreviewCompositorBrowserTest(const PaintPreviewCompositorBrowserTest&) =
      delete;
  PaintPreviewCompositorBrowserTest& operator=(
      const PaintPreviewCompositorBrowserTest&) = delete;

 protected:
  void CreateServiceInstance() {
    key_ = std::make_unique<SimpleFactoryKey>(
        browser()->profile()->GetPath(),
        browser()->profile()->IsOffTheRecord());
    PaintPreviewBaseServiceTestFactory::GetInstance()->SetTestingFactory(
        key_.get(),
        base::BindRepeating(&PaintPreviewBaseServiceTestFactory::Build));
  }

  PaintPreviewBaseService* GetBaseService() {
    return PaintPreviewBaseServiceTestFactory::GetForKey(key_.get());
  }

 private:
  std::unique_ptr<SimpleFactoryKey> key_ = nullptr;
};

// Test that a "true" initialization works and doesn't crash.
IN_PROC_BROWSER_TEST_F(PaintPreviewCompositorBrowserTest,
                       TestInitializationSuccess) {
  base::RunLoop loop;
  mojo::Remote<mojom::PaintPreviewCompositorCollection> compositor_collection =
      CreateCompositorCollection();
  EXPECT_TRUE(compositor_collection.is_bound());
  EXPECT_TRUE(compositor_collection.is_connected());

  // If the compositor_collection hasn't crashed during initialization due to
  // lacking font support then this call should succeed.
  compositor_collection->ListCompositors(base::BindOnce(
      [](base::OnceClosure quit,
         const std::vector<base::UnguessableToken>& ids) {
        EXPECT_EQ(ids.size(), 0U);
        std::move(quit).Run();
      },
      loop.QuitClosure()));
  loop.Run();
  compositor_collection.reset();
}

IN_PROC_BROWSER_TEST_F(PaintPreviewCompositorBrowserTest, CompositorCreate) {
  CreateServiceInstance();
  auto* base_service = GetBaseService();
  auto compositor_service = ToCompositorServiceImpl(
      base_service->StartCompositorService(base::DoNothing()));

  base::RunLoop loop;
  auto compositor = ToCompositorClientImpl(
      compositor_service->CreateCompositor(loop.QuitClosure()));
  loop.Run();
  EXPECT_TRUE(compositor_service->HasActiveClients());
  EXPECT_TRUE(base::Contains(compositor_service->ActiveClientsForTesting(),
                             compositor->Token()));
  EXPECT_TRUE(compositor->IsBoundAndConnected());
  compositor.reset();

  EXPECT_FALSE(compositor_service->HasActiveClients());
}

IN_PROC_BROWSER_TEST_F(PaintPreviewCompositorBrowserTest,
                       MultipleCompositorCreate) {
  CreateServiceInstance();
  auto* base_service = GetBaseService();
  auto compositor_service = ToCompositorServiceImpl(
      base_service->StartCompositorService(base::DoNothing()));
  EXPECT_EQ(0U, compositor_service->ActiveClientsForTesting().size());
  EXPECT_FALSE(compositor_service->HasActiveClients());

  base::RunLoop loop_0;
  auto compositor_0 = ToCompositorClientImpl(
      compositor_service->CreateCompositor(loop_0.QuitClosure()));
  loop_0.Run();
  EXPECT_TRUE(compositor_service->HasActiveClients());
  EXPECT_EQ(1U, compositor_service->ActiveClientsForTesting().size());
  EXPECT_TRUE(base::Contains(compositor_service->ActiveClientsForTesting(),
                             compositor_0->Token()));
  EXPECT_TRUE(compositor_0->IsBoundAndConnected());

  base::RunLoop loop_1;
  auto compositor_1 = ToCompositorClientImpl(
      compositor_service->CreateCompositor(loop_1.QuitClosure()));
  loop_1.Run();
  EXPECT_TRUE(compositor_service->HasActiveClients());
  EXPECT_EQ(2U, compositor_service->ActiveClientsForTesting().size());
  EXPECT_TRUE(base::Contains(compositor_service->ActiveClientsForTesting(),
                             compositor_1->Token()));
  EXPECT_TRUE(compositor_1->IsBoundAndConnected());
  EXPECT_NE(compositor_0->Token(), compositor_1->Token());

  compositor_0.reset();
  EXPECT_TRUE(compositor_service->HasActiveClients());
  EXPECT_EQ(1U, compositor_service->ActiveClientsForTesting().size());
  EXPECT_TRUE(base::Contains(compositor_service->ActiveClientsForTesting(),
                             compositor_1->Token()));

  compositor_1.reset();
  EXPECT_FALSE(compositor_service->HasActiveClients());
}

IN_PROC_BROWSER_TEST_F(PaintPreviewCompositorBrowserTest,
                       KillWithActiveCompositors) {
  CreateServiceInstance();
  auto* base_service = GetBaseService();
  // NOTE: the disconnect handler for the service as a whole only triggers if
  // the service is killed unexpectedly. Here the |compositor_service| object
  // is deleted (performing a graceful shutdown) so the handler won't run.
  auto compositor_service = ToCompositorServiceImpl(
      base_service->StartCompositorService(base::DoNothing()));

  base::RunLoop loop;
  auto compositor = ToCompositorClientImpl(
      compositor_service->CreateCompositor(loop.QuitClosure()));
  loop.Run();
  EXPECT_TRUE(compositor_service->HasActiveClients());
  EXPECT_TRUE(base::Contains(compositor_service->ActiveClientsForTesting(),
                             compositor->Token()));
  EXPECT_TRUE(compositor->IsBoundAndConnected());

  base::RunLoop disconnect_loop;
  compositor->SetDisconnectHandler(disconnect_loop.QuitClosure());

  // Kill before releasing active compositors.
  compositor_service.reset();
  disconnect_loop.Run();
  EXPECT_FALSE(compositor->IsBoundAndConnected());
}

}  // namespace paint_preview
