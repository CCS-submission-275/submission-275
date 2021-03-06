// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_RELAUNCH_NOTIFICATION_WALL_CLOCK_TIMER_H_
#define CHROME_BROWSER_UI_VIEWS_RELAUNCH_NOTIFICATION_WALL_CLOCK_TIMER_H_

#include "base/bind.h"
#include "base/callback.h"
#include "base/location.h"
#include "base/macros.h"
#include "base/power_monitor/power_observer.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace base {
class Clock;
class TickClock;
}  // namespace base

// WallClockTimer is based on OneShotTimer and provides a simple timer API
// which is mostly similar to OneShotTimer's API. The main difference is that
// WallClockTimer is using Time (which is system-dependent) to schedule task.
// WallClockTimer calls you back once scheduled time has come.
//
// Comparison with OneShotTimer: WallClockTimer runs |user_task_| after |delay_|
// expires according to usual time, while OneShotTimer runs |user_task_| after
// |delay_| expires according to TimeTicks which freezes when power suspends
// (desktop falls asleep).
//
// The API is not thread safe. All methods must be called from the same
// sequence (not necessarily the construction sequence), except for the
// destructor.
// - The destructor may be called from any sequence when the timer is not
// running and there is no scheduled task active.
class WallClockTimer : public base::PowerObserver {
 public:
  WallClockTimer();
  WallClockTimer(const base::Clock* clock, const base::TickClock* tick_clock);

  ~WallClockTimer() override;

  // Start the timer to run at the given |delay| from now. If the timer is
  // already running, it will be replaced to call the given |user_task|.
  virtual void Start(const base::Location& posted_from,
                     base::Time desired_run_time,
                     base::OnceClosure user_task);

  // Start the timer to run at the given |delay| from now. If the timer is
  // already running, it will be replaced to call a task formed from
  // |reviewer->*method|.
  template <class Receiver>
  void Start(const base::Location& posted_from,
             base::Time desired_run_time,
             Receiver* receiver,
             void (Receiver::*method)()) {
    Start(posted_from, desired_run_time,
          base::BindOnce(method, base::Unretained(receiver)));
  }

  void Stop();

  bool IsRunning() const;

  // base::PowerObserver:
  void OnResume() override;

  base::Time desired_run_time() const { return desired_run_time_; }

 private:
  void AddObserver();

  void RemoveObserver();

  // Actually run scheduled task
  void RunUserTask();

  // Returns the current time count.
  base::Time Now() const;

  bool observer_added_ = false;

  // Location in user code.
  base::Location posted_from_;

  // The desired run time of |user_task_|.
  base::Time desired_run_time_;

  base::OnceClosure user_task_;

  // Timer which should notify to run task in the period while system awake
  base::OneShotTimer timer_;

  // The clock used to calculate the run time for scheduled tasks.
  const base::Clock* const clock_;

  DISALLOW_COPY_AND_ASSIGN(WallClockTimer);
};

#endif  // CHROME_BROWSER_UI_VIEWS_RELAUNCH_NOTIFICATION_WALL_CLOCK_TIMER_H_
