// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_BROWSER_WEBVIEW_CLIENT_WEBVIEW_H_
#define CHROMECAST_BROWSER_WEBVIEW_CLIENT_WEBVIEW_H_

#include <map>
#include <string>
#include <vector>

#include "base/files/file_descriptor_watcher_posix.h"
#include "base/run_loop.h"
#include "base/threading/thread.h"
#include "chromecast/browser/webview/proto/webview.grpc.pb.h"
#include "components/exo/wayland/clients/client_base.h"
#include "third_party/grpc/src/include/grpcpp/grpcpp.h"

namespace chromecast {
namespace client {

// Sample Wayland client to manipulate webviews
class WebviewClient : public exo::wayland::clients::ClientBase {
 public:
  struct BufferCallback {
    WebviewClient* client;
    Buffer* buffer;
  };

  WebviewClient();
  ~WebviewClient() override;
  bool HasAvailableBuffer();
  void Run(const InitParams& params, const std::string& channel_directory);
  void SchedulePaint();

 private:
  using WebviewRequestResponseClient =
      ::grpc::ClientReaderWriterInterface<chromecast::webview::WebviewRequest,
                                          chromecast::webview::WebviewResponse>;
  struct Webview {
    Webview();
    ~Webview();
    std::unique_ptr<ClientBase::Buffer> buffer;
    std::unique_ptr<wl_surface> surface;
    std::unique_ptr<wl_subsurface> subsurface;
    std::unique_ptr<WebviewRequestResponseClient> client;
    std::unique_ptr<::grpc::ClientContext> context;
  };

  void AllocateBuffers(const InitParams& params);
  void CreateWebview(const std::vector<std::string>& tokens);
  void DestroyWebview(const std::vector<std::string>& tokens);
  void HandleMode(void* data,
                  struct wl_output* wl_output,
                  uint32_t flags,
                  int32_t width,
                  int32_t height,
                  int32_t refresh) override;
  void InputCallback();
  void ListActiveWebviews();
  void Paint();
  void SendNavigationRequest(const std::vector<std::string>& tokens);
  void SendResizeRequest(const std::vector<std::string>& tokens);
  void SetPosition(const std::vector<std::string>& tokens);
  void TakeExclusiveAccess();
  void WlDisplayCallback();

  int32_t drm_format_ = 0;
  int32_t bo_usage_ = 0;

  std::map<int, std::unique_ptr<Webview>> webviews_;

  std::unique_ptr<wl_callback> frame_callback_;
  std::vector<std::unique_ptr<BufferCallback>> buffer_callbacks_;
  const scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  std::unique_ptr<base::FileDescriptorWatcher::Controller> stdin_controller_;
  std::unique_ptr<base::FileDescriptorWatcher::Controller>
      wl_display_controller_;
  base::FileDescriptorWatcher file_descriptor_watcher_;
  base::RunLoop run_loop_;

  std::unique_ptr<chromecast::webview::PlatformViewsService::Stub> stub_;
  DISALLOW_COPY_AND_ASSIGN(WebviewClient);
};

}  // namespace client
}  // namespace chromecast

#endif  // CHROMECAST_BROWSER_WEBVIEW_CLIENT_WEBVIEW_H_
