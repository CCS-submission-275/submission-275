//
// Created by  on 2/22/21.
//

#ifndef THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_FORENSIC_FILE_LOGGER_H_
#define THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_FORENSIC_FILE_LOGGER_H_

#include "third_party/blink/renderer/platform/platform_export.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "base/memory/singleton.h"
#include "third_party/blink/renderer/platform/heap/heap_allocator.h"
#include "third_party/blink/renderer/platform/heap/persistent.h"


namespace base {
class File;
class SingleThreadTaskRunner;
}

namespace blink {

class PLATFORM_EXPORT ForensicFileLogger final {
 USING_FAST_MALLOC(ForensicFileLogger);

 public:
  static ForensicFileLogger* GetInstance();
  ForensicFileLogger();
  void log(std::string const& msg);
  scoped_refptr<base::SequencedTaskRunner> GetOrCreateFileThread();
  void close();

 private:
  size_t loggedMsgCount_;
  std::unique_ptr<base::File> log_file_;
  scoped_refptr<base::SequencedTaskRunner> single_thread_task_runner_;

  void write(std::string const& msg);

  friend base::DefaultSingletonTraits<ForensicFileLogger>;
};

}


#endif //THIRD_PARTY_BLINK_RENDERER_PLATFORM_FORENSICS_FORENSIC_FILE_LOGGER_H_
