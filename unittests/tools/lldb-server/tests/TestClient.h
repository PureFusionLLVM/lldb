//===-- TestClient.h --------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_SERVER_TESTS_TESTCLIENT_H
#define LLDB_SERVER_TESTS_TESTCLIENT_H

#include "MessageObjects.h"
#include "Plugins/Process/gdb-remote/GDBRemoteCommunicationClient.h"
#include "lldb/Target/ProcessLaunchInfo.h"
#include "lldb/Utility/ArchSpec.h"
#include "lldb/Utility/Connection.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/FormatVariadic.h"
#include <memory>
#include <string>

#if LLDB_SERVER_IS_DEBUGSERVER
#define LLGS_TEST(x) DISABLED_ ## x
#define DS_TEST(x) x
#else
#define LLGS_TEST(x) x
#define DS_TEST(x) DISABLED_ ## x
#endif

namespace llgs_tests {
class TestClient
    : public lldb_private::process_gdb_remote::GDBRemoteCommunicationClient {
public:
  static bool IsDebugServer() { return LLDB_SERVER_IS_DEBUGSERVER; }
  static bool IsLldbServer() { return !IsDebugServer(); }

  /// Launches the server, connects it to the client and returns the client. If
  /// Log is non-empty, the server will write it's log to this file.
  static llvm::Expected<std::unique_ptr<TestClient>> launch(llvm::StringRef Log);

  /// Launches the server, while specifying the inferior on its command line.
  /// When the client connects, it already has a process ready.
  static llvm::Expected<std::unique_ptr<TestClient>>
  launch(llvm::StringRef Log, llvm::ArrayRef<llvm::StringRef> InferiorArgs);

  /// Allows user to pass additional arguments to the server. Be careful when
  /// using this for generic tests, as the two stubs have different
  /// command-line interfaces.
  static llvm::Expected<std::unique_ptr<TestClient>>
  launchCustom(llvm::StringRef Log, llvm::ArrayRef<llvm::StringRef> ServerArgs, llvm::ArrayRef<llvm::StringRef> InferiorArgs);


  ~TestClient() override;
  llvm::Error SetInferior(llvm::ArrayRef<std::string> inferior_args);
  llvm::Error ListThreadsInStopReply();
  llvm::Error SetBreakpoint(unsigned long address);
  llvm::Error ContinueAll();
  llvm::Error ContinueThread(unsigned long thread_id);
  const ProcessInfo &GetProcessInfo();
  llvm::Optional<JThreadsInfo> GetJThreadsInfo();
  const StopReply &GetLatestStopReply();
  template <typename T> llvm::Expected<const T &> GetLatestStopReplyAs() {
    assert(m_stop_reply);
    if (const auto *Reply = llvm::dyn_cast<T>(m_stop_reply.get()))
      return *Reply;
    return llvm::make_error<llvm::StringError>(
        llvm::formatv("Unexpected Stop Reply {0}", m_stop_reply->getKind()),
        llvm::inconvertibleErrorCode());
  }
  llvm::Error SendMessage(llvm::StringRef message);
  llvm::Error SendMessage(llvm::StringRef message,
                          std::string &response_string);
  llvm::Error SendMessage(llvm::StringRef message, std::string &response_string,
                          PacketResult expected_result);
  unsigned int GetPcRegisterId();

private:
  TestClient(std::unique_ptr<lldb_private::Connection> Conn);

  llvm::Error QueryProcessInfo();
  llvm::Error Continue(llvm::StringRef message);
  std::string FormatFailedResult(
      const std::string &message,
      lldb_private::process_gdb_remote::GDBRemoteCommunication::PacketResult
          result);

  llvm::Optional<ProcessInfo> m_process_info;
  std::unique_ptr<StopReply> m_stop_reply;
  unsigned int m_pc_register = UINT_MAX;
};

} // namespace llgs_tests

#endif // LLDB_SERVER_TESTS_TESTCLIENT_H
