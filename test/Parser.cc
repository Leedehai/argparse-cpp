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
 * * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
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

TEST(Parser, basic_usage) {
  argparse::Parser *psr = new argparse::Parser("test");
  std::vector<std::string> args = {"./test", "-a"};
  psr->add_argument("-a").action("store_true");

  argparse::Values val = psr->parse_args(args);
  EXPECT_TRUE(val.is_true("a"));
  
}

TEST(Parser, nargs1) {
  argparse::Parser *psr = new argparse::Parser("test");
  psr->add_argument("-a").nargs(1);
  std::vector<std::string> args1 = {"./test", "-a", "v"};
  std::vector<std::string> args2 = {"./test", "-a", "v", "w"};
  
  argparse::Values val = psr->parse_args(args1);
  EXPECT_EQ(val.str("a"), "v");
  EXPECT_THROW(val.str("a", 1), argparse::exception::IndexError);
  
  EXPECT_THROW(psr->parse_args(args2), argparse::exception::ParseError);
}

TEST(Parser, nargs2) {
  argparse::Parser *psr = new argparse::Parser("test");
  psr->add_argument("-a").nargs(2);
  std::vector<std::string> args1 = {"./test", "-a", "v"};
  std::vector<std::string> args2 = {"./test", "-a", "v", "w"};
  std::vector<std::string> args3 = {"./test", "-a", "v", "w", "x"};
  
  argparse::Values val = psr->parse_args(args2);
  EXPECT_EQ(val.str("a"), "v");
  EXPECT_EQ(val.str("a", 0), "v");
  EXPECT_EQ(val.str("a", 1), "w");
  EXPECT_THROW(val.str("a", 2), argparse::exception::IndexError);
  
  EXPECT_THROW(psr->parse_args(args1), argparse::exception::ParseError);
  EXPECT_THROW(psr->parse_args(args3), argparse::exception::ParseError);
}