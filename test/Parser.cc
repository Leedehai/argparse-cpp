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
  argparse::Argv args = {"./test", "-a"};
  psr->add_argument("-a").action("store_true");
  
  argparse::Values val = psr->parse_args(args);
  EXPECT_TRUE(val.is_true("a"));
  
}

TEST(Parser, basic_argument) {
  argparse::Parser *psr = new argparse::Parser("test");
  argparse::Argv args = {"./test", "-a", "v"};
  psr->add_argument("-a");
  argparse::Values val = psr->parse_args(args);
  EXPECT_EQ("v", val.get("a"));
  EXPECT_EQ("v", val.get("a", 0));
  EXPECT_EQ("v", val["a"]); // ["x"] is same with get("x", 0)
}

TEST(Parser, nargs1) {
  argparse::Parser *psr = new argparse::Parser("test");
  psr->add_argument("-a").nargs(1);
  argparse::Argv args1 = {"./test", "-a", "v"};
  argparse::Argv args2 = {"./test", "-a", "v", "w"};
  
  argparse::Values val = psr->parse_args(args1);
  EXPECT_EQ(val.get("a"), "v");
  EXPECT_THROW(val.get("a", 1), argparse::exception::IndexError);
  
  EXPECT_THROW(psr->parse_args(args2), argparse::exception::ParseError);
}

TEST(Parser, nargs2) {
  argparse::Parser *psr = new argparse::Parser("test");
  psr->add_argument("-a").nargs(2);
  argparse::Argv args1 = {"./test", "-a", "v"};
  argparse::Argv args2 = {"./test", "-a", "v", "w"};
  argparse::Argv args3 = {"./test", "-a", "v", "w", "x"};
  
  argparse::Values val = psr->parse_args(args2);
  EXPECT_EQ(val.get("a"), "v");
  EXPECT_EQ(val.get("a", 0), "v");
  EXPECT_EQ(val.get("a", 1), "w");
  EXPECT_THROW(val.get("a", 2), argparse::exception::IndexError);
  
  EXPECT_THROW(psr->parse_args(args1), argparse::exception::ParseError);
  EXPECT_THROW(psr->parse_args(args3), argparse::exception::ParseError);
}

TEST(Parser, nargs_asterisk) {
  argparse::Parser *psr = new argparse::Parser("test");
  psr->add_argument("-a").nargs("*");
  psr->add_argument("-b");
  
  // Stop parsing by end of arguments.
  argparse::Argv ok1 = {"./test", "-a", "v1", "v2"};
  argparse::Values v1 = psr->parse_args(ok1);
  EXPECT_EQ(2, v1.size("a"));
  EXPECT_EQ("v1", v1.get("a", 0));
  EXPECT_EQ("v2", v1.get("a", 1));
  
  // Stop parsing by an other option.
  argparse::Argv ok2 = {"./test", "-a", "v1", "v2", "-b", "r1"};
  argparse::Values v2 = psr->parse_args(ok2);
  EXPECT_EQ(2, v2.size("a"));
  EXPECT_EQ("v1", v2.get("a", 0));
  EXPECT_EQ("v2", v2.get("a", 1));
  EXPECT_EQ(1, v2.size("b"));
  EXPECT_EQ("r1", v2.get("b"));

  // No option value.
  argparse::Argv ok3 = {"./test", "-a", "-b", "r1"};
  argparse::Values v3 = psr->parse_args(ok3);
  EXPECT_EQ(0, v3.size("a"));
  EXPECT_TRUE(v3.is_set("a"));
  EXPECT_EQ(1, v3.size("b"));
  EXPECT_EQ("r1", v3.get("b"));
}

TEST(Parser, name2) {
  argparse::Parser *psr = new argparse::Parser("test");
  psr->add_argument("-a").name("--action");
  argparse::Argv ok1 = {"./test", "-a", "v"};
  argparse::Argv ok2 = {"./test", "--action", "v"};
  argparse::Argv ng1 = {"./test", "-a"}; // Not enough arguments
  argparse::Argv ng2 = {"./test", "--action"}; // Same as above

  argparse::Values v1 = psr->parse_args(ok1);
  argparse::Values v2 = psr->parse_args(ok2);
  EXPECT_EQ(v1.get("a"), "v"); // Using "name" instead of "dest".
  EXPECT_EQ(v2.get("a"), "v"); // Same if the option is set by 2nd name (--aciton)

  EXPECT_THROW(psr->parse_args(ng1), argparse::exception::ParseError);
  EXPECT_THROW(psr->parse_args(ng1), argparse::exception::ParseError);
}


