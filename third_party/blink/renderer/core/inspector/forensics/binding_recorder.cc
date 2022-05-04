#include "third_party/blink/renderer/core/inspector/forensics/binding_recorder.h"
#include "third_party/blink/renderer/bindings/core/v8/script_source_code.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/core/dom/node.h"
#include "third_party/blink/renderer/core/page/chrome_client.h"
#include "third_party/blink/renderer/platform/bindings/v8_binding.h"

// :: Forensics ::
#include "third_party/blink/renderer/core/inspector/forensics/dom_modification_recorder.h"
#include "third_party/blink/renderer/core/inspector/forensics/forensic_replay_engine.h"
#include "third_party/blink/renderer/core/inspector/forensics/v8_recorder.h"
// Forensics

#include <sstream>

namespace blink {
using std::string;
// done
void BindingRecorder::DidCompileScriptInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const int scriptID,
    const int line,
    const int column,
    const String& url,
    const String& code) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("scriptID", scriptID);
  message->SetInteger("line", line + 1);
  message->SetInteger("column", column + 1);
  message->SetString("url", url);
  const void* bytes = code.Bytes();
  size_t size = code.length();
  uint32_t hash = base::PersistentHash(bytes, size);
  message->SetInteger("hash", hash);
  // message->SetString("code", code);
  message->SetInteger("size", size);
  AddThreadData(thread_data, message);

  VLOG(7) << code << " " << hash << " " << size;

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

// Called when a script has been compiled and right before being executed
void BindingRecorder::DidCompileScript(LocalFrame* frame,
                                       const ScriptSourceCode& source,
                                       const int scriptID) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&BindingRecorder::DidCompileScriptInternal,
                                GetThreadData(), copyFrameData(frame), scriptID,
                                source.StartPosition().line_.OneBasedInt(),
                                source.StartPosition().column_.OneBasedInt(),
                                source.Url().GetString().IsolatedCopy(),
                                source.Source().ToString().IsolatedCopy()));
}
// done
void BindingRecorder::DidRunCompiledScriptBeginInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const int scriptID) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("scriptID", scriptID);
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

// Called when a compiled script has stopped running
// The time delta between DidCompileScript and DidRunCompiledScript
// should represent the actual script execution time,
// including nested script execution
void BindingRecorder::DidRunCompiledScriptBegin(LocalFrame* frame,
                                                const int scriptID,
                                                v8::Local<v8::Value> result) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&BindingRecorder::DidRunCompiledScriptBeginInternal,
                     GetThreadData(), copyFrameData(frame), scriptID));
}

void BindingRecorder::DidRunCompiledScriptEndInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const int scriptID,
    const String& result) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("scriptID", scriptID);
  message->SetString("result", result);
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void BindingRecorder::DidRunCompiledScriptEnd(LocalFrame* frame,
                                              const int scriptID,
                                              v8::Local<v8::Value> result) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  // :: Forensics ::
  if (!ForensicReplayEngine::GetInstance()->replay()) {
    v8::PlatformInstrumentationData v8_records =
        v8::V8::DumpAndClearPlatformInstRecording();
    V8Recorder::DumpV8Records(v8_records, "RunScriptEnd");
  } else {
    v8::PlatformInstrumentationData v8_records =
        v8::V8::DumpAndClearPlatformInstReplaying();
    V8Recorder::DumpV8Records(v8_records, "RunScriptEnd");
  }
  //
  
  auto resultStr = result.IsEmpty() ? String("") : getStringFromV8Value(result);


  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&BindingRecorder::DidRunCompiledScriptEndInternal,
                     GetThreadData(), copyFrameData(frame), scriptID,
                     resultStr.IsolatedCopy()));
}

void BindingRecorder::DidCallFunctionBeginInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    std::unique_ptr<ForensicFunctionData> function,
    std::vector<String> args) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("scriptID", function->scriptID);
  message->SetInteger("line", function->line);
  message->SetInteger("column", function->column);
  AddThreadData(thread_data, message);
  // message->SetString("callbackDebugName", function->debug_name);

  auto argList = std::make_unique<JSONArray>();

  for (auto a : args) {
    argList->PushString(a);
  }

  message->SetArray("argList", std::move(argList));

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

// static
void BindingRecorder::DidCallFunctionBegin(
    ExecutionContext* context,
    v8::Local<v8::Function> function,
    v8::Local<v8::Value> receiver,
    std::vector<v8::Local<v8::Value>> args) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!context->ExecutingWindow())
    return;

  LocalFrame* frame = context->ExecutingWindow()->GetFrame();
  std::unique_ptr<ForensicFunctionData> func_feat =
      TranslateV8Function(function);
  std::vector<String> argList;
  for (auto a : args) {
    argList.emplace_back(getStringFromV8Value(a).IsolatedCopy());
  }
  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&BindingRecorder::DidCallFunctionBeginInternal,
                                GetThreadData(), copyFrameData(frame),
                                std::move(func_feat), argList));
}

void BindingRecorder::DidCallFunctionEndInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    std::unique_ptr<ForensicFunctionData> function) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetInteger("scriptID", function->scriptID);
  message->SetInteger("line", function->line);
  message->SetInteger("column", function->column);
  AddThreadData(thread_data, message);
  // message->SetString("callbackDebugName", function->debug_name);

  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void BindingRecorder::DidCallFunctionEnd(
    ExecutionContext* context,
    v8::Local<v8::Function> function,
    v8::Local<v8::Value> receiver,
    std::vector<v8::Local<v8::Value>> args) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!context->ExecutingWindow())
    return;

  // :: Forensics ::
  if (!ForensicReplayEngine::GetInstance()->replay()) {
    v8::PlatformInstrumentationData v8_records =
        v8::V8::DumpAndClearPlatformInstRecording();
    V8Recorder::DumpV8Records(v8_records, "CallFunctionEnd");
  } else {
    v8::PlatformInstrumentationData v8_records =
        v8::V8::DumpAndClearPlatformInstReplaying();
    V8Recorder::DumpV8Records(v8_records, "CallFunctionEnd");
  }

  //

  LocalFrame* frame = context->ExecutingWindow()->GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&BindingRecorder::DidCallFunctionEndInternal,
                                GetThreadData(), copyFrameData(frame),
                                TranslateV8Function(function)));
}

void BindingRecorder::DidCallCallbackBeginInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const std::string& interface,
    const std::string& attribute) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("interface", String(interface.c_str(), interface.size()));
  message->SetString("attribute", String(attribute.c_str(), attribute.size()));
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void BindingRecorder::DidCallCallbackBegin(ExecutionContext* context,
                                           const std::string& interface,
                                           const std::string& attribute) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!context->ExecutingWindow())
    return;

  LocalFrame* frame = context->ExecutingWindow()->GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&BindingRecorder::DidCallCallbackBeginInternal,
                                GetThreadData(), copyFrameData(frame),
                                interface, attribute));
}

void BindingRecorder::DidCallCallbackEndInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const std::string& interface,
    const std::string& attribute) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("interface", String(interface.c_str(), interface.size()));
  message->SetString("attribute", String(attribute.c_str(), attribute.size()));
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void BindingRecorder::DidCallCallbackEnd(ExecutionContext* context,
                                         const std::string& interface,
                                         const std::string& attribute) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!context->ExecutingWindow())
    return;

  LocalFrame* frame = context->ExecutingWindow()->GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&BindingRecorder::DidCallCallbackEndInternal,
                                GetThreadData(), copyFrameData(frame),
                                interface, attribute));
}

void BindingRecorder::DidCallV8GetAttributeInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const std::string& interface,
    const std::string& attribute,
    const String& value) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("interface", String(interface.c_str(), interface.size()));
  message->SetString("attribute", String(attribute.c_str(), attribute.size()));
  // Note: probably we just need a string for value
  message->SetString("returnValue", value);
  message->SetString("returnType", "String");
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void BindingRecorder::DidCallV8GetAttribute(ExecutionContext* execution_context,
                                            const std::string& interface,
                                            const std::string& attribute,
                                            const std::string& cpp_class,
                                            v8::Local<v8::Value> v8Value) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  if (!execution_context->ExecutingWindow())
    return;

  LocalFrame* frame = execution_context->ExecutingWindow()->GetFrame();
  std::unique_ptr<ForensicFrameData> frame_data = copyFrameData(frame);

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&BindingRecorder::DidCallV8GetAttributeInternal,
                     GetThreadData(), copyFrameData(frame), interface,
                     attribute, getStringFromV8Value(v8Value)));
}

void BindingRecorder::DidCallV8SetAttributeInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    ChromeClient* client,
    const std::string& interface,
    const std::string& attribute,
    const String& value) {
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("interface", String(interface.c_str(), interface.size()));
  message->SetString("attribute", String(attribute.c_str(), attribute.size()));
  // Note: probably we just need a string for value
  message->SetString("returnValue", value);
  message->SetString("returnType", "String");
  AddThreadData(thread_data, message);
  std::ostringstream log_str;
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

// static
void BindingRecorder::DidCallV8SetAttribute(ExecutionContext* execution_context,
                                            const string& interface,
                                            const string& attribute,
                                            const string& cpp_class,
                                            v8::Local<v8::Value> v8Value) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!execution_context->ExecutingWindow())
    return;

  LocalFrame* frame = execution_context->ExecutingWindow()->GetFrame();

  GetTaskRunner()->PostTask(
      FROM_HERE, base::BindOnce(&BindingRecorder::DidCallV8SetAttributeInternal,
                                GetThreadData(), copyFrameData(frame),
                                GetChromeClient(frame), interface, attribute,
                                getStringFromV8Value(v8Value)));
}

void BindingRecorder::DidCallV8MethodCallbackInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    const std::string& interface,
    const std::string& attribute) {
  std::ostringstream log_str;
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("interface", String(interface.c_str(), interface.size()));
  message->SetString("attribute", String(attribute.c_str(), attribute.size()));
  AddThreadData(thread_data, message);
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}
// static
void BindingRecorder::DidCallV8MethodCallback(
    ExecutionContext* execution_context,
    const std::string& interface,
    const std::string& attribute) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!execution_context->ExecutingWindow())
    return;
  LocalFrame* frame = execution_context->ExecutingWindow()->GetFrame();
  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&BindingRecorder::DidCallV8MethodCallbackInternal,
                     GetThreadData(), copyFrameData(frame), interface,
                     attribute));
}

// static
void BindingRecorder::DidCallV8Method(ExecutionContext* execution_context,
                                      const string& interface,
                                      const string& attribute,
                                      const string& cpp_class,
                                      void* cpp_impl_ptr,
                                      std::vector<ArgNameValPair> args_list,
                                      v8::Local<v8::Value> ret_value) {
  DidCallV8MethodTemplate(execution_context, interface, attribute, cpp_class,
                          cpp_impl_ptr, args_list, ret_value);
}

// static
void BindingRecorder::DidCallV8Method(ExecutionContext* execution_context,
                                      const string& interface,
                                      const string& attribute,
                                      const string& cpp_class,
                                      Node* cpp_impl_ptr,
                                      std::vector<ArgNameValPair> args_list,
                                      v8::Local<v8::Value> ret_value) {
  DidCallV8MethodTemplate(execution_context, interface, attribute, cpp_class,
                          cpp_impl_ptr, args_list, ret_value);
}

// static
void BindingRecorder::DidCallV8Method(ExecutionContext* execution_context,
                                      const string& interface,
                                      const string& attribute,
                                      const string& cpp_class,
                                      EventTarget* cpp_impl_ptr,
                                      std::vector<ArgNameValPair> args_list,
                                      v8::Local<v8::Value> ret_value) {
  DidCallV8MethodTemplate(execution_context, interface, attribute, cpp_class,
                          cpp_impl_ptr, args_list, ret_value);
}

template <typename T>
void BindingRecorder::DidCallV8MethodTemplate(
    ExecutionContext* execution_context,
    const string& interface,
    const string& attribute,
    const string& cpp_class,
    T* cpp_impl_ptr,
    std::vector<ArgNameValPair> args_list,
    v8::Local<v8::Value> ret_value) {
  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_

  if (!execution_context)
    return;

  if (!execution_context->ExecutingWindow())
    return;

  LocalFrame* frame = execution_context->ExecutingWindow()->GetFrame();

  std::ostringstream log_str;
  auto message = std::make_unique<JSONObject>();

  message->SetString("interface", interface.c_str());
  message->SetString("attribute", attribute.c_str());
  message->SetString("cppClass", cpp_class.c_str());
  message->SetString("cppImplPtr", AddrToString(cpp_impl_ptr));

  auto argList = std::make_unique<JSONArray>();

  for (auto a : args_list) {
    argList->PushObject(V8ValueToJSONObject(GetChromeClient(frame), a.second));
  }

  message->SetArray("argList", std::move(argList));
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str());
}

//-------------- PRIVATE ---------------//

std::unique_ptr<JSONObject> BindingRecorder::V8ValueToJSONObject(
    ChromeClient* client,
    v8::Local<v8::Value> v8Value) {
  // TODO (): to real JSON format
  auto message = std::make_unique<JSONObject>();
  if (!client)
    return message;

  if (v8Value.IsEmpty())
    return message;

  if (v8Value->IsNull()) {
    message->SetString("valueType", "null");
    return message;
  }

  if (v8Value->IsUndefined()) {
    message->SetString("valueType", "undefined");
    return message;
  }

  if (v8Value->IsFunction()) {
    std::unique_ptr<ForensicFunctionData> func_feat =
        TranslateV8Function(v8Value.As<v8::Function>());
    message->SetInteger("scriptID", func_feat->scriptID);
    message->SetInteger("line", func_feat->line);
    message->SetInteger("column", func_feat->column);
    // message->SetString("callbackDebugName", func_feat->debug_name);
    message->SetString("valueType", "function");

    return message;
  }

  if (v8Value->IsObject()) {
    message->SetString("valueType", "object");
  }

  std::unique_ptr<base::Value> v = client->TranslateV8Value(v8Value);

  if (v == nullptr) {
    message->SetString("value", "unknown");
    return message;
  }

  std::string jsonStr;
  JSONStringValueSerializer json(&jsonStr);
  if (json.Serialize(*v) && !jsonStr.empty()) {
    message->SetString("value", jsonStr.c_str());
  } else {
    message->SetString("error", "Object cannot be serialized!");
  }
  return message;
}

// static
std::unique_ptr<ForensicFunctionData> BindingRecorder::TranslateV8Function(
    v8::Local<v8::Function> func) {
  std::unique_ptr<ForensicFunctionData> func_feat =
      std::make_unique<ForensicFunctionData>();

  if (!func.IsEmpty()) {
    func_feat->scriptID = func->ScriptId();
    func_feat->line = func->GetScriptLineNumber() + 1;
    func_feat->column = func->GetScriptColumnNumber() + 1;
    // func_feat->debug_name =
    //     ToCoreString(v8::Local<v8::String>::Cast(func->GetDebugName()))
    //         .IsolatedCopy();
  }
  return func_feat;
}

void BindingRecorder::DidCallV8MethodInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    ChromeClient* client,
    const std::string& interface,
    const std::string& attribute,
    const std::string& cpp_class,
    std::unique_ptr<std::vector<ArgNameValPairString>> args_list,
    const String& value) {
  std::ostringstream log_str;
  auto message = std::make_unique<JSONObject>();
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("interface", String(interface.c_str(), interface.size()));
  message->SetString("attribute", String(attribute.c_str(), attribute.size()));
  message->SetString("return", value);
  AddThreadData(thread_data, message);

  auto argList = std::make_unique<JSONArray>();

  for (auto a : *args_list) {
    //  argList->PushObject(V8ValueToJSONObject(client, a.second));
    argList->PushString(a.second);
  }

  message->SetArray("argList", std::move(argList));

  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();

  LogInfo(log_str.str(), thread_data->timestamp);
}

void BindingRecorder::DidCallV8Method(ExecutionContext* execution_context,
                                      const string& interface,
                                      const string& attribute,
                                      const string& cpp_class,
                                      std::vector<ArgNameValPair> args_list,
                                      v8::Local<v8::Value> ret_value) {
  // Note: we don't need to log result when the JS function execution is
  // deterministic

  TRACE_EVENT0("blink.forensic_recording", __func__);
  _IFA_DISABLE_
  if (!execution_context)
    return;

  if (!execution_context->ExecutingWindow())
    return;

  LocalFrame* frame = execution_context->ExecutingWindow()->GetFrame();
  auto argList = std::make_unique<std::vector<ArgNameValPairString>>();

  for (auto a : args_list) {
    argList->emplace_back(a.first, getStringFromV8Value(a.second));
  }

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&BindingRecorder::DidCallV8MethodInternal, GetThreadData(),
                     copyFrameData(frame), GetChromeClient(frame), interface,
                     attribute, cpp_class, std::move(argList),
                     getStringFromV8Value(ret_value)));
}
String BindingRecorder::getStringFromV8Value(v8::Local<v8::Value> value) {
  if (value->IsString())
    return ToCoreString(v8::Local<v8::String>::Cast(value)).IsolatedCopy();
  else if (value->IsNumber())
    return WTF::String::Number(value.As<v8::Number>()->Value(), 16);
  else if (value->IsBoolean())
    return (value->IsBoolean() ? "true" : "false");
  else if (value->IsNullOrUndefined())
    return "Undefined";
  else if (value->IsObject())
    return "Object";
  else
    return "Unknown";
}

void BindingRecorder::RecordPerformanceNowEventInternal(
    std::unique_ptr<ThreadData> thread_data,
    std::unique_ptr<ForensicFrameData> frame_data,
    WTF::String now) {
  auto message = std::make_unique<JSONObject>();
  AddThreadData(thread_data, message);
  AddFrameInfo(std::move(frame_data), message);
  message->SetString("now", now);
  std::ostringstream log_str;
  VLOG(7) << "RecordPerformanceNowEventInternal: " << message->ToJSONString();
  log_str << _IFA_LOG_PREFIX_ << message->ToJSONString();
  LogInfo(log_str.str(), thread_data->timestamp);
}

void BindingRecorder::RecordPerformanceNowEvent(ExecutionContext* context,
                                                v8::Local<v8::Value> now) {
  auto* window = DynamicTo<LocalDOMWindow>(context);
  LocalFrame* frame = nullptr;
  if (window)
    frame = window->GetFrame();
  else
    return;

  GetTaskRunner()->PostTask(
      FROM_HERE,
      base::BindOnce(&BindingRecorder::RecordPerformanceNowEventInternal,
                     GetThreadData(), copyFrameData(frame),
                     getStringFromV8Value(now).IsolatedCopy()));
}

}  // namespace blink
