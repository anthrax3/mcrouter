/*
 *  Copyright (c) 2016, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include <stdio.h>

#include <memory>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include <folly/dynamic.h>
#include <folly/FileUtil.h>
#include <folly/io/async/EventBase.h>
#include <folly/Memory.h>

#include "mcrouter/config.h"
#include "mcrouter/lib/network/test/ClientSocket.h"
#include "mcrouter/lib/network/test/ListenSocket.h"
#include "mcrouter/McrouterInstance.h"
#include "mcrouter/proxy.h"
#include "mcrouter/ProxyThread.h"
#include "mcrouter/test/cpp_unit_tests/mcrouter_test_client.h"
#include "mcrouter/test/cpp_unit_tests/MemcacheLocal.h"

using namespace facebook::memcache::mcrouter;
using namespace facebook::memcache::test;
using namespace folly;

using facebook::memcache::McrouterOptions;

/*
 * * * * * * * * * SPECIAL NOTE : SPECIAL NOTE : SPECIAL NOTE * * * * * * * * *
 *
 * This tester program tests the functional behavior of libmcrouter, which
 * communicates with a running memcached server. To deterministically test the
 * functional behavior, libmcrouter is configured to use a local memcached
 * server instead of the production servers. The configuration string for
 * libmcrouter is generated by generateConfigString(char *buf, int port)
 * function, which assumes memcached is running in the localhost.
 *
 * Before starting the tests, we need the "Local Memcached" up, and generate
 * the configuration string for libmcrouer. The try block in the main(..)
 * function takes care of these tasks.
 *
 * * * * * * * * RECOMMENDED PRACTICES : RECOMMENDED PRACTICES * * * * * * * * *
 *
 *
 * Before calling RUN_ALL_TESTS(..) first check memcacheLocal variable
 * as a gate. This variable should not be null when the tests are run.
 *
 */

std::string configString;
std::unique_ptr<MemcacheLocal> memcacheLocal = nullptr; // local memcached
std::string kInvalidPoolConfig =
  "mcrouter/test/cpp_unit_tests/files/libmcrouter_invalid_pools.json";

TEST(libmcrouter, sanity) {
  auto opts = defaultTestOptions();
  opts.config = configString;

  MCRouterTestClient client("sanity", opts);

  int nkeys = 100;
  dynamic keys = dynamic::array;
  dynamic kv_pairs = dynamic::object;
  keys.resize(nkeys);
  for (dynamic i = 0; i < nkeys; i ++) {
    keys[i] = "rajeshn-testkey" + i.asString();
    kv_pairs[keys[i]] = "value" + i.asString();
  }

  // clean everything out
  dynamic delete_results = dynamic::object;
  client.del(keys, true, delete_results);

  // start the test
  dynamic set_results = dynamic::object;
  EXPECT_TRUE(client.set(kv_pairs, set_results) == nkeys);

  for (auto& res : set_results.values()) {
    EXPECT_TRUE(res.asBool());
  }

  dynamic get_results = dynamic::object;
  EXPECT_TRUE(client.get(keys, get_results) == nkeys);

  // make sure we get what we set
  for (auto& res_pair : get_results.items()) {
    auto pos = kv_pairs.find(res_pair.first);
    EXPECT_FALSE(pos == get_results.items().end());
    EXPECT_TRUE(kv_pairs[res_pair.first] == res_pair.second);
  }

  delete_results = dynamic::object;
  EXPECT_TRUE(client.del(keys, true, delete_results) == nkeys);

  for (auto& res : delete_results.values()) {
    EXPECT_TRUE(res.asBool());
  }

  delete_results = dynamic::object;
  EXPECT_TRUE(client.del(keys, true, delete_results) == 0);

  for (auto& res : delete_results.values()) {
    EXPECT_FALSE(res.asBool());
  }
}

static std::atomic<int> on_reply_count{0};
void on_reply(mcrouter_msg_t* router_req,
              void* context) {
  on_reply_count++;
  mc_msg_decref(router_req->req);
}

static std::atomic<int> on_cancel_count{0};
void on_cancel(void* request_context,
               void* client_context) {
  on_cancel_count++;
}

TEST(libmcrouter, premature_disconnect) {
  auto opts = defaultTestOptions();
  opts.config = configString;
  auto router = McrouterInstance::init("test_premature_disconnect", opts);

  for (int i = 0; i < 10; i++) {
    on_reply_count = 0;
    on_cancel_count = 0;

    {
      auto client = router->createClient(
        {on_reply, on_cancel, nullptr},
        nullptr, 0, false);

      const char key[] = "__mockmc__.want_timeout(50)";
      mc_msg_t *mc_msg = mc_msg_new(sizeof(key));
      mc_msg->key.str = (char*) &mc_msg[1];
      strcpy(mc_msg->key.str, key);
      mc_msg->key.len = strlen(key);
      mc_msg->op = mc_op_get;
      mcrouter_msg_t router_msg;
      router_msg.req = mc_msg;
      client->send(&router_msg, 1);
      mc_msg_decref(mc_msg);
    }

    for (size_t j = 0; on_cancel_count + on_reply_count == 0 && j < 20; ++j) {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(0, on_reply_count);
    EXPECT_EQ(1, on_cancel_count);
  }
}

TEST(libmcrouter, invalid_pools) {
  auto opts = defaultTestOptions();
  std::string configStr;
  EXPECT_TRUE(folly::readFile(kInvalidPoolConfig.data(), configStr));
  opts.config = configStr;
  opts.default_route = "/a/b/";
  auto router = McrouterInstance::init("test_invalid_pools", opts);
  EXPECT_TRUE(router == nullptr);
}

TEST(libmcrouter, listenSock) {
  /* Create a listen socket, pass it to a child mcrouter and
     check that communication through the socket works */
  using namespace facebook::memcache;

  ListenSocket listenSock;

  std::vector<std::string> args{MCROUTER_INSTALL_PATH "mcrouter/mcrouter",
        "--listen-sock-fd", folly::to<std::string>(listenSock.getSocketFd()),
        "--config-str", configString };
  auto testArgs = defaultTestCommandLineArgs();
  args.insert(args.end(), testArgs.begin(), testArgs.end());
  folly::Subprocess mcr(args);

  SCOPE_EXIT {
    mcr.terminate();
    mcr.wait();
  };

  const std::string kSetRequest = "set testkey 0 0 1\r\nv\r\n";
  const std::string kStoredReply = "STORED\r\n";
  const std::string kGetRequest = "get testkey\r\n";
  const std::string kGetReply = "VALUE testkey 0 1\r\nv\r\nEND\r\n";

  ClientSocket mcrSock(listenSock.getPort());
  EXPECT_EQ(kStoredReply,
            mcrSock.sendRequest(kSetRequest, kStoredReply.size()));
  EXPECT_EQ(kGetReply, mcrSock.sendRequest(kGetRequest, kGetReply.size()));

  ClientSocket mcSock(memcacheLocal->getPort());
  EXPECT_EQ(kGetReply, mcSock.sendRequest(kGetRequest, kGetReply.size()));
}

// for backward compatibility with gflags
namespace gflags { }
namespace google { using namespace gflags; }

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  try {
    //blocks until local memcache server ready
    memcacheLocal = folly::make_unique<MemcacheLocal> ();
    configString = memcacheLocal->generateMcrouterConfigString();
  } catch (const SubprocessError& e) { // declared in Subprocess.h
    LOG(ERROR) << "SubprocessError:" << std::endl << e.what();
  }

  EXPECT_TRUE(memcacheLocal != nullptr); // gate to check if memcached is up

  return RUN_ALL_TESTS();
}
