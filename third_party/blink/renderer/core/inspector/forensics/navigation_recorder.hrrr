
#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_FRAME_RECORDER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_FRAME_RECORDER_H_

#include "third_party/blink/public/web/web_frame_load_type.h"
#include "third_party/blink/public/web/web_navigation_type.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"

namespace blink {

class Frame;

class KURL;

class CORE_EXPORT NavigationRecorder : ForensicRecorder {
  DISALLOW_COPY(NavigationRecorder);

 public:
  static void WindowOpen(Document* document,
                         const KURL& url,
                         const AtomicString& window_name,
                         const WebWindowFeatures& window_features,
                         bool user_gesture);

  static void WindowCreated(LocalFrame* frame, LocalFrame* opener_frame);

  static void WillDownloadURL(LocalFrame* frame,
                              const KURL& url,
                              const String& suggested_name);

  static void DidCreateDocumentLoader(
                                      LocalFrame* frame,
                                      const KURL& url,
                                      WebFrameLoadType,
                                      WebNavigationType);

  static void WillLoadFrame(LocalFrame* frame,
                            const KURL& url,
                            int navigation_policy_enum,
                            int request_context_enum,
                            int frame_load_type_enum,
                            Event* triggering_event,
                            std::string event_type);

  static void FrameRequestedNavigation(LocalFrame* frame,
                                       Frame* target_frame,
                                       const KURL& url,
                                       ClientNavigationReason reason,
                                       bool replace_current_item = false);

  static void FrameScheduledNavigation(LocalFrame* frame,
                                       const KURL& url,
                                       ClientNavigationReason reason,
                                       bool replace_current_item = false);

  static void FrameClearedScheduledNavigation(LocalFrame* frame);
  static void BeginNavigation(LocalFrame* frame, const KURL& url);

  static void DidCommitLoad(LocalFrame* frame, DocumentLoader* document_loader);

 private:
  static void DidCreateDocumentLoaderInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const String& url,
      int navigation_type);
  static void WindowOpenInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const String& url,
      const String& windowName,
      bool user_gesture);
  static void WindowCreatedInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const String& created_frame_id);
  static void FrameRequestedNavigationInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const String& url,
      ClientNavigationReason reason,
      bool replace_current_item = false);
};
}  // namespace blink
#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_FRAME_RECORDER_H_
