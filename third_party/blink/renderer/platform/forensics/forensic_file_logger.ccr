//
// Created by  on 2/22/21.
//

#include "third_party/blink/renderer/platform/forensics/forensic_file_logger.h"
#include <fstream>
#include "base/base_switches.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/path_service.h"
#include "base/process/process_handle.h"
#include "base/single_thread_task_runner.h"
#include "base/task/task_traits.h"
#include "base/threading/platform_thread.h"
#include "chrome/common/chrome_paths.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace blink {

ForensicFileLogger::ForensicFileLogger()
    : loggedMsgCount_(0),
      log_file_(nullptr),
      single_thread_task_runner_(nullptr) {
  VLOG(7) << "Creating ForensicFile Logger "
  << base::PlatformThread::GetName() << std::endl;
}

ForensicFileLogger* ForensicFileLogger::GetInstance() {
  return base::Singleton<ForensicFileLogger>::get();
}

scoped_refptr<base::SequencedTaskRunner>
ForensicFileLogger::GetOrCreateFileThread() {
  if (!single_thread_task_runner_)
    single_thread_task_runner_ =
        base::ThreadPool::CreateSequencedTaskRunner({base::MayBlock()});
  return single_thread_task_runner_;
}

void ForensicFileLogger::log(std::string const& msg) {
  // GetOrCreateFileThread()->PostTask(
  //     FROM_HERE, base::BindOnce(&ForensicFileLogger::write,
  //                               base::Unretained(GetInstance()), msg));
  write(msg+"\n");
}

void ForensicFileLogger::write(std::string const& msg) {
  if (log_file_) {
    log_file_->WriteAtCurrentPos(msg.c_str(), msg.size());
    loggedMsgCount_++;
    if (loggedMsgCount_ > 1000) {
      log_file_->Flush();
      loggedMsgCount_ = 0;
    }

  } else {
    auto* command_line = base::CommandLine::ForCurrentProcess();
    std::string filename = "/tmp/jscapsule-default.log";
    if (command_line->HasSwitch("forensic-log-file")) {
      filename = command_line->GetSwitchValueASCII("forensic-log-file")
                 + "_" + std::to_string(base::GetCurrentProcId())
                 + "_" + std::to_string(base::PlatformThread::CurrentId());
    }
    VLOG(7) << __func__ << " Logging to " << filename;

    base::FilePath jscapsule = base::FilePath::FromUTF8Unsafe(filename);

    log_file_ = std::make_unique<base::File>(
        jscapsule, base::File::FLAG_APPEND | base::File::FLAG_OPEN_ALWAYS);

    VLOG(7) << __func__ << " create file at " << jscapsule.AsUTF8Unsafe()
            << " is valid: " << log_file_->IsValid();
    if (!log_file_->IsValid()) {
      VLOG(7) << log_file_->error_details();
    } else {
      loggedMsgCount_ = 0;
      write(msg);
    }
  }
}

void ForensicFileLogger::close() {
  log_file_->Close();
}

}  // namespace blink
