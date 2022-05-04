/*
 * Copyright (C) 2020 Georgia Institute of Technology. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/jallen89/JSCapsule/blob/master/LICENSE.txt
 *
 */
//

#include "src/instrumentation/platform_instrumentation.h"

#include <sstream>
#include <unordered_map>
#include <vector>

#include "src/api/api.h"
#include "src/base/debug/stack_trace.h"
#include "src/base/logging.h"
#include "src/base/platform/platform.h"
#include "src/common/assert-scope.h"
#include "src/execution/frames-inl.h"
#include "src/execution/frames.h"
#include "src/execution/isolate.h"
#include "src/numbers/hash-seed-inl.h"
#include "src/objects/objects-inl.h"
#include "src/runtime/runtime.h"
#include "v8-internal.h"

#define REPLAY_TIME_TOLERANCE 100  // 100ms tolerance

namespace v8 {
namespace internal {
static PlatformInstrumentation* platformInst = nullptr;

double* V8ForensicRecordValueList::next() {
  if (index_ < this->size()) {
    return &((*this)[index_++]);
  }
  return nullptr;
}
V8ForensicRecordValueList::V8ForensicRecordValueList(
    const std::vector<double>& rhs) {
  reset();
  for (auto val : rhs) {
    this->push_back(val);
  }
}

void PlatformInstrumentation::init() {
  if (!platformInst) {
    platformInst = new PlatformInstrumentation();
    v8::base::OS::Print("-- JSCAPSULE -- V8::PlatformInstrumentation::Init\n");
    platformInst->startRecording();
  }
}

PlatformInstrumentation* PlatformInstrumentation::current() {
  return platformInst;
}

void PlatformInstrumentation::TearDown() {
  if (platformInst) {
    v8::base::OS::Print(
        "-- JSCAPSULE -- V8::PlatformInstrumentation::TearDown\n");
    delete platformInst;
  }
}
PlatformInstrumentation::PlatformInstrumentation()
    : currentState(OFF),
      math_random_values_(),
      current_timestamps_(),
      replayed_math_random_values_(),
      replayed_current_timestamps_(),
      security_token_("Unset") {
  reset();
}

PlatformInstrumentation::~PlatformInstrumentation() {
  v8::base::OS::Print("-- JSCAPSULE -- V8::PlatformInstrumentation::~\n");
}
double PlatformInstrumentation::TimeCurrentMillis() {
  if (platformInst) {
    switch (platformInst->state()) {
      case RECORDING: {
        double retval = V8::GetCurrentPlatform()->CurrentClockTimeMillis();
        platformInst->HandleTimeCurrentMillis(retval);
        return retval;
      } break;
      case REPLAYING: {
        double retval = platformInst->NextTimeCurrentMills();
        platformInst->addToMap(platformInst->replayed_current_timestamps_,
                               retval);
        return retval;
      }
      case OFF:
        break;
    }
  }
  return V8::GetCurrentPlatform()->CurrentClockTimeMillis();
}

void PlatformInstrumentation::HandleTimeCurrentMillis(double val) {
  addToMap(current_timestamps_, val);
}

double PlatformInstrumentation::NextTimeCurrentMills() {
  double nowRecTime = (V8::GetCurrentPlatform()->CurrentClockTimeMillis() -
                       replayStartTimeMs()) +
                      recordStartTimeMs();
  double* next = nextFromMap(current_timestamps_);
  if (!next) {
    return nowRecTime;
  }
  return *next;
}

Address PlatformInstrumentation::InstrumentMathRandom(
    Address raw_native_context) {
  Context native_context = Context::cast(Object(raw_native_context));
  int index = native_context.math_random_index().value();
  int r = -1;
  if (platformInst) {
    switch (platformInst->state()) {
      case RECORDING: {
        // recording
        // std::cout << __func__ << " recording"
        //           << " val:"
        //           << native_context.math_random_cache().get_scalar(index)
        //           << " index:" << index << std::endl;
        platformInst->HandleMathRandomVals(
            native_context.math_random_cache().get_scalar(index));
        r = -1;
        break;
      }
      case REPLAYING: {
        double val = platformInst->NextMathRandomVals();
        platformInst->addToMap(platformInst->replayed_math_random_values_, val);
        // std::cout << __func__ << " replaying "
        //           << "val:" << val << " index:" << index << std::endl;
        r = 0;
        if (val > -1) {
          // replaying
          // std::cout << __func__ << " replaying" << std::endl;
          native_context.math_random_cache().set(index, val);
          r = 1;
        }
        break;
      }
      case OFF:
        break;
    }
  }

  Smi ret = Smi::FromInt(r);
  return ret.ptr();
}

void PlatformInstrumentation::HandleMathRandomVals(double val) {
  if (platformInst && platformInst->state() == RECORDING) {
    addToMap(math_random_values_, val);
  }
}

double PlatformInstrumentation::NextMathRandomVals() {
  if (platformInst && platformInst->state() == REPLAYING) {
    double* next = nextFromMap(math_random_values_);
    if (!next) {
      return -1;
    }
    return *next;
  }
  return -1;
}

double PlatformInstrumentation::recordStartTime() { return m_recordStartTime; }

double PlatformInstrumentation::recordStartTimeMs() {
  return m_recordStartTime * 1000;
}

double PlatformInstrumentation::replayStartTime() { return m_replayStartTime; }

double PlatformInstrumentation::replayStartTimeMs() {
  return m_replayStartTime * 1000;
}

void PlatformInstrumentation::setRecordStartTime(
    double t /* expects value in seconds */) {
  m_recordStartTime = t;
}

void PlatformInstrumentation::setReplayStartTime(
    double t /* expects value in seconds */) {
  m_replayStartTime = t;
}

void PlatformInstrumentation::startRecording() {
  v8::base::OS::Print(
      "-- JSCAPSULE -- V8::PlatformInstrumentation::startRecording Start "
      "Recording for %s\n",
      security_token_.c_str());
  currentState = RECORDING;
}

void PlatformInstrumentation::startReplay() {
  v8::base::OS::Print(
      "-- JSCAPSULE -- V8::PlatformInstrumentation::startReplay Start "
      "Replay\n");
  reset();
  currentState = REPLAYING;
}

void PlatformInstrumentation::stop() {
  v8::base::OS::Print(
      "-- JSCAPSULE -- V8::PlatformInstrumentation::stop Stopping\n");
  currentState = OFF;
}

PlatformInstrumentation::State PlatformInstrumentation::state() {
  return currentState;
}

void PlatformInstrumentation::clear() {
  // v8::base::OS::Print(
  //     "-- JSCAPSULE -- V8::PlatformInstrumentation::clear clear data for %s \n",
  //     security_token_.c_str());
  switch (state()) {
    case REPLAYING:
      replayed_math_random_values_.clear();
      replayed_current_timestamps_.clear();
      break;
    case RECORDING:
      math_random_values_.clear();
      current_timestamps_.clear();
      break;
    case OFF:
      replayed_math_random_values_.clear();
      replayed_current_timestamps_.clear();
      math_random_values_.clear();
      current_timestamps_.clear();
      break;
  }
}

void PlatformInstrumentation::reset() {
  resetMap(math_random_values_);
  resetMap(current_timestamps_);
}

PlatformInstrumentationData PlatformInstrumentation::dump() {
  PlatformInstrumentationData data;

  copyMap(current_timestamps_, data.current_timestamps_);
  copyMap(math_random_values_, data.math_random_values_);
  // std::cout << __func__ << " " << security_token_ << " " << data.size()
  //           << std::endl;
  return data;
}

PlatformInstrumentationData PlatformInstrumentation::dumpForReplay() {
  PlatformInstrumentationData data;

  copyMap(replayed_current_timestamps_, data.current_timestamps_);
  copyMap(replayed_math_random_values_, data.math_random_values_);
  // std::cout << __func__ << " " << security_token_ << " " << data.size()
  //           << std::endl;
  return data;
}

void PlatformInstrumentation::load(const PlatformInstrumentationData& data) {
  clear();
  v8::base::OS::Print(
      "-- JSCAPSULE -- V8::PlatformInstrumentation::load data from blink\n");
  copyMap(data.math_random_values_, math_random_values_);
  copyMap(data.current_timestamps_, current_timestamps_);
}

void PlatformInstrumentation::setSecurityToken(const std::string& token) {
  security_token_ = token;
}

void PlatformInstrumentation::copyMap(
    const V8ForensicRecordValueMap& src,
    std::unordered_map<std::string, std::vector<double>>& tar) {
  for (auto it = src.begin(); it != src.end(); ++it) {
    tar.insert({it->first, {it->second.begin(), it->second.end()}});
  }
}
void PlatformInstrumentation::copyMap(
    const std::unordered_map<std::string, std::vector<double>>& src,
    V8ForensicRecordValueMap& tar) {
  for (auto it = src.begin(); it != src.end(); ++it) {
    V8ForensicRecordValueList l(it->second);
    tar.insert({it->first, l});
  }
}

void PlatformInstrumentation::addToMap(V8ForensicRecordValueMap& map,
                                       double val) {
  auto it = map.find(security_token_);
  if (it != map.end()) {
    it->second.emplace_back(val);
  } else {
    V8ForensicRecordValueList l;
    l.emplace_back(val);
    map.insert({security_token_, l});
  }
}
void PlatformInstrumentation::resetMap(V8ForensicRecordValueMap& map) {
  for (auto it = map.begin(); it != map.end(); ++it) {
    it->second.reset();
  }
}
double* PlatformInstrumentation::nextFromMap(V8ForensicRecordValueMap& map) {
  auto it = map.find(security_token_);
  if (it != map.end()) {
    return it->second.next();
  }
  v8::base::OS::Print(
      "-- JSCAPSULE -- V8::PlatformInstrumentation::nextFromMap cannot find "
      "the token: %s\n",
      security_token_.c_str());
  return nullptr;
}

}  // namespace internal
}  // namespace v8
