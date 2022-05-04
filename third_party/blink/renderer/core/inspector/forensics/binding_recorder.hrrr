#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_BINDING_RECORDER_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_BINDING_RECORDER_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_recorder.h"
#include "base/json/json_string_value_serializer.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "v8/include/v8.h"

#include <memory>


namespace blink {

class Frame;
class ExecutionContext;
class EventListener;
class KURL;


using ArgNameValPair = std::pair<std::string, v8::Local<v8::Value>>;
using ArgNameValPairString = std::pair<std::string, String>;
using ArgNameListenerPair = std::pair<std::string, EventListener*>;

struct ForensicFunctionData {
  int scriptID = 0;
  int line = 0;
  int column = 0;
  String debug_name;
  // we may add hash, scriptUrl later
};

class CORE_EXPORT BindingRecorder : ForensicRecorder {
  DISALLOW_COPY(BindingRecorder);

 public:
  // Methods called when compiling or executing JS code
  static void DidCompileScript(LocalFrame* frame,
                               const ScriptSourceCode& source,
                               const int scriptID);

  static void DidRunCompiledScriptBegin(LocalFrame* frame,
                                        const int scriptID,
                                        v8::Local<v8::Value> result);

  static void DidRunCompiledScriptEnd(LocalFrame* frame,
                                      const int scriptID,
                                      v8::Local<v8::Value> result);

  static void DidCallFunctionBegin(ExecutionContext* context,
                                   v8::Local<v8::Function> function,
                                   v8::Local<v8::Value> receiver,
                                   std::vector<v8::Local<v8::Value>> args);

  static void DidCallFunctionEnd(ExecutionContext* context,
                                 v8::Local<v8::Function> function,
                                 v8::Local<v8::Value> receiver,
                                 std::vector<v8::Local<v8::Value>> args);

  static void DidCallCallbackBegin(ExecutionContext* context,
                                   const std::string& interface,
                                   const std::string& attribute);

  static void DidCallCallbackEnd(ExecutionContext* context,
                                 const std::string& interface,
                                 const std::string& attribute);

  // Methods called from the Bindings instrumentation.
  //
  static void DidCallV8GetAttribute(ExecutionContext* execution_context,
                                    const std::string& interface,
                                    const std::string& attribute,
                                    const std::string& cpp_class,
                                    v8::Local<v8::Value> v8Value);


  static void DidCallV8GetAttributeString(ExecutionContext* execution_context,
                                          const std::string& interface,
                                          const std::string& attribute,
                                          const std::string& cpp_class,
                                          const WTF::String return_value);


  static void DidCallV8GetAttributeint(ExecutionContext* execution_context,
                                       const std::string& interface,
                                       const std::string& attribute,
                                       const std::string& cpp_class,
                                       int return_value);


  static void DidCallV8GetAttributedouble(ExecutionContext* execution_context,
                                          const std::string& interface,
                                          const std::string& attribute,
                                          const std::string& cpp_class,
                                          double return_value);


  static void DidCallV8SetAttribute(ExecutionContext* execution_context,
                                    const std::string& interface,
                                    const std::string& attribute,
                                    const std::string& cpp_class,
                                    v8::Local<v8::Value> v8Value);

  static void RecordPerformanceNowEvent(ExecutionContext* context,
                                        v8::Local<v8::Value> now);



static void DidCallV8MethodCallback(ExecutionContext* execution_context,
                                      const std::string& interface,
                                      const std::string& attribute);

  static void DidCallV8Method(ExecutionContext* execution_context,
                              const std::string& interface,
                              const std::string& attribute,
                              const std::string& cpp_class,
                              void* cpp_impl_ptr,
                              std::vector<ArgNameValPair> args_list,
                              v8::Local<v8::Value> ret_value);

  static void DidCallV8Method(ExecutionContext* execution_context,
                              const std::string& interface,
                              const std::string& attribute,
                              const std::string& cpp_class,
                              Node* cpp_impl_ptr,
                              std::vector<ArgNameValPair> args_list,
                              v8::Local<v8::Value> ret_value);

  static void DidCallV8Method(ExecutionContext* execution_context,
                              const std::string& interface,
                              const std::string& attribute,
                              const std::string& cpp_class,
                              EventTarget* cpp_impl_ptr,
                              std::vector<ArgNameValPair> args_list,
                              v8::Local<v8::Value> ret_value);

  static void DidCallV8Method(ExecutionContext* execution_context,
                              const std::string& interface,
                              const std::string& attribute,
                              const std::string& cpp_class,
                              std::vector<ArgNameValPair> args_list,
                              v8::Local<v8::Value> ret_value);

 private:
  static std::unique_ptr<JSONObject> V8ValueToJSONObject(
      ChromeClient* client,
      v8::Local<v8::Value> v8Value);

  static std::unique_ptr<ForensicFunctionData> TranslateV8Function(
      v8::Local<v8::Function> func);


  template <typename T>
  static void DidCallV8MethodTemplate(ExecutionContext* execution_context,
                                      const std::string& interface,
                                      const std::string& attribute,
                                      const std::string& cpp_class,
                                      T* cpp_impl_ptr,
                                      std::vector<ArgNameValPair> args_list,
                                      v8::Local<v8::Value> ret_value);

  static void DidCompileScriptInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const int scriptID,
      const int line,
      const int column,
      const String& url,
      const String& code);

  static void DidRunCompiledScriptBeginInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const int scriptID);

  static void DidRunCompiledScriptEndInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const int scriptID,
      const String& result);

  static void DidCallFunctionBeginInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      std::unique_ptr<ForensicFunctionData> function,
      std::vector<String> args);

  static void DidCallFunctionEndInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      std::unique_ptr<ForensicFunctionData> function);

  static void DidCallCallbackBeginInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const std::string& interface,
      const std::string& attribute);

  static void DidCallCallbackEndInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const std::string& interface,
      const std::string& attribute);

  static void DidCallV8GetAttributeInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const std::string& interface,
      const std::string& attribute,
      const String& v8Value);
  static void DidCallV8SetAttributeInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      ChromeClient* client,
      const std::string& interface,
      const std::string& attribute,
      const String& v8Value);
  static void DidCallV8MethodCallbackInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      const std::string& interface,
      const std::string& attribute);
  static void DidCallV8MethodInternal(
      std::unique_ptr<ThreadData> thread_data,
      std::unique_ptr<ForensicFrameData> frame_data,
      ChromeClient* client,
      const std::string& interface,
      const std::string& attribute,
      const std::string& cpp_class,
      std::unique_ptr<std::vector<ArgNameValPairString>> args_list,
      const String& value);

  static void RecordPerformanceNowEventInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    WTF::String now);

  inline static String getStringFromV8Value(v8::Local<v8::Value> value);
};
}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_INSPECTOR_FORENSICS_BINDING_RECORDER_H_
