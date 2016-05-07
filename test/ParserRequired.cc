/*
 * Copyright 2016, Masayoshi Mizutani, mizutani@sfc.wide.ad.jp
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <vector>
#include <string>

#include "./gtest.h"
#include "../argparse.hpp"

class ParserRequired : public ::testing::Test {
public:
  argparse::Parser psr;
  argparse::Argument *arg;
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").required(true));
  }
};

TEST_F(ParserRequired, store_ok) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("v1", val.get("a", 0));
}

TEST_F(ParserRequired, store_ng) {
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserRequired, append_ok) {
  arg->action("append");
  argparse::Argv seq = {"./test", "-a", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("v1", val.get("a", 0));
}

TEST_F(ParserRequired, append_ng) {
  arg->action("append");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserRequired, store_const_ok) {
  arg->action("store_const").set_const("c");
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
}

TEST_F(ParserRequired, store_const_ng) {
  arg->action("store_const").set_const("c");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserRequired, append_const_ok) {
  arg->action("append_const").set_const("c");
  argparse::Argv seq = {"./test", "-a", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(2, val.size("a"));
}

TEST_F(ParserRequired, append_const_ng) {
  arg->action("append_const").set_const("c");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserRequired, store_true) {
  arg->action("store_true");
  argparse::Argv seq = {"./test"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
}

TEST_F(ParserRequired, store_false) {
  arg->action("store_false");
  argparse::Argv seq = {"./test"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
}

TEST_F(ParserRequired, count_ok) {
  arg->action("count");
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
}

TEST_F(ParserRequired, count_ng) {
  arg->action("count");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}
