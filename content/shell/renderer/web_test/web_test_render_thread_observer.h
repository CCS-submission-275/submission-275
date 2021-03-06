// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_RENDERER_WEB_TEST_WEB_TEST_RENDER_THREAD_OBSERVER_H_
#define CONTENT_SHELL_RENDERER_WEB_TEST_WEB_TEST_RENDER_THREAD_OBSERVER_H_

#include <memory>

#include "base/macros.h"
#include "content/public/renderer/render_thread_observer.h"
#include "content/shell/common/web_test.mojom.h"
#include "mojo/public/cpp/bindings/associated_receiver.h"
#include "mojo/public/cpp/bindings/pending_associated_receiver.h"

namespace test_runner {
class TestInterfaces;
}

namespace content {

class WebTestRenderThreadObserver : public RenderThreadObserver,
                                    public mojom::WebTestControl {
 public:
  static WebTestRenderThreadObserver* GetInstance();

  WebTestRenderThreadObserver();
  ~WebTestRenderThreadObserver() override;

  test_runner::TestInterfaces* test_interfaces() const {
    return test_interfaces_.get();
  }

  // content::RenderThreadObserver:
  void RegisterMojoInterfaces(
      blink::AssociatedInterfaceRegistry* associated_interfaces) override;
  void UnregisterMojoInterfaces(
      blink::AssociatedInterfaceRegistry* associated_interfaces) override;

 private:
  // mojom::WebTestControl implementation.
  void ReplicateWebTestRuntimeFlagsChanges(
      base::Value changed_layout_test_runtime_flags) override;

  // Helper to bind this class as the mojom::WebTestControl.
  void OnWebTestControlAssociatedRequest(
      mojo::PendingAssociatedReceiver<mojom::WebTestControl> receiver);

  std::unique_ptr<test_runner::TestInterfaces> test_interfaces_;

  mojo::AssociatedReceiver<mojom::WebTestControl> receiver_{this};

  DISALLOW_COPY_AND_ASSIGN(WebTestRenderThreadObserver);
};

}  // namespace content

#endif  // CONTENT_SHELL_RENDERER_WEB_TEST_WEB_TEST_RENDER_THREAD_OBSERVER_H_
