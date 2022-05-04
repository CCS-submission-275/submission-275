/*
 * Copyright (C) 2020 Georgia Institute of Technology. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/jallen89/JSCapsule/blob/master/LICENSE.txt
 *
 */
//

#ifndef V8_INSTRUMENTATION_PLATFORM_INSTRUMENTATION_H
#define V8_INSTRUMENTATION_PLATFORM_INSTRUMENTATION_H

#include <string>
#include <vector>
#include <unordered_map>

#include "include/v8.h"

#include "src/base/utils/random-number-generator.h"
#include "src/utils/boxed-float.h"
#include "v8-internal.h"

namespace v8 {
namespace internal {

class V8ForensicRecordValueList : public std::vector<double> {
  public:
    V8ForensicRecordValueList() {index_ = 0;}
    explicit V8ForensicRecordValueList(const std::vector<double>& rhs);
    double* next();
    void reset() {index_ = 0;}

  private:
    size_t index_;
};
using V8ForensicRecordValueMap = std::unordered_map<std::string, V8ForensicRecordValueList>;

class PlatformInstrumentation {
 public:
  enum State { OFF, RECORDING, REPLAYING };

  static void init();
  static PlatformInstrumentation* current();
  static void TearDown();
  ~PlatformInstrumentation();

  static double TimeCurrentMillis();

  void HandleTimeCurrentMillis(double val);
  double NextTimeCurrentMills();

  static Address InstrumentMathRandom(Address raw_native_context);
  void setSecurityToken(const std::string& token);

  void HandleMathRandomVals(double);
  double NextMathRandomVals();

  double recordStartTime();
  double replayStartTime();
  double recordStartTimeMs();
  double replayStartTimeMs();
  void setRecordStartTime(double t);
  void setReplayStartTime(double t);

  void startRecording();
  void startReplay();
  void stop();
  State state();
  void clear();
  void reset();
  PlatformInstrumentationData dump();
  PlatformInstrumentationData dumpForReplay();
  void load(const PlatformInstrumentationData& data);

 private:
  PlatformInstrumentation();

  void addToMap(V8ForensicRecordValueMap&, double val);
  void resetMap(V8ForensicRecordValueMap&);
  void copyMap(const V8ForensicRecordValueMap&, std::unordered_map<std::string, std::vector<double>>&);
  void copyMap(const std::unordered_map<std::string, std::vector<double>>&, V8ForensicRecordValueMap&);
  double* nextFromMap(V8ForensicRecordValueMap&);

  State currentState;
  V8ForensicRecordValueMap math_random_values_;
  V8ForensicRecordValueMap current_timestamps_;

  V8ForensicRecordValueMap replayed_math_random_values_;
  V8ForensicRecordValueMap replayed_current_timestamps_;

  double m_recordStartTime;
  double m_replayStartTime;

  std::string security_token_;
};
}  // namespace internal
}  // namespace v8

#endif  // V8_BASE_INSTRUMENTATION_PLATFORM_INSTRUMENTATION_H
