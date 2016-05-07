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

TEST(Argument, basic) {
  std::vector<const std::string> seq = {"a", "b"};

  argparse_internal::ArgumentProcessor proc;
  auto arg = proc.add_argument("test").nargs(1);
  
  std::vector<argparse_internal::Var*> options;
  size_t idx = arg.parse(seq, 0, &options);
  EXPECT_EQ(1, idx);
  EXPECT_EQ(1, options.size());
  argparse_internal::Var *opt = options[0];
  EXPECT_EQ("a", opt->to_s());
}

TEST(Argument, Integer) {
  std::vector<const std::string> seq_ok1 = {"10", "b"};
  std::vector<const std::string> seq_ok2 = {"0", "b"};
  std::vector<const std::string> seq_ng1 = {"a", "b"};
  std::vector<const std::string> seq_ng2 = {"-1", "b"};
  
  std::vector<argparse_internal::Var*> options1, options2, options3, options4;
  argparse_internal::ArgumentProcessor proc;
  auto arg = proc.add_argument("test").nargs(1).type(argparse::ArgType::INT);
  
  size_t r1 = arg.parse(seq_ok1, 0, &options1);
  EXPECT_EQ(1, r1);
  EXPECT_EQ(10, options1[0]->to_i());

  size_t r2 = arg.parse(seq_ok2, 0, &options2);
  EXPECT_EQ(1, r2);
  EXPECT_EQ(0, options2[0]->to_i());

  EXPECT_THROW(arg.parse(seq_ng1, 0, &options3),
               argparse::exception::ParseError);
  EXPECT_THROW(arg.parse(seq_ng2, 0, &options4),
               argparse::exception::ParseError);
}

TEST(Argument, help) {
  argparse_internal::ArgumentProcessor proc;
  argparse::Argument arg(&proc);
  arg.help("save us");
  EXPECT_EQ("save us", arg.get_help());
}

TEST(Argument, set_type_bool) {
  argparse_internal::ArgumentProcessor proc;
  argparse::Argument *arg = &(proc.add_argument("-a").type("bool"));
  EXPECT_EQ(argparse::ArgType::BOOL, arg->get_type());
}

TEST(Argument, set_type_str) {
  argparse_internal::ArgumentProcessor proc;
  argparse::Argument *arg = &(proc.add_argument("-a").type("str"));
  EXPECT_EQ(argparse::ArgType::STR, arg->get_type());
}

TEST(Argument, set_type_int) {
  argparse_internal::ArgumentProcessor proc;
  argparse::Argument *arg = &(proc.add_argument("-a").type("int"));
  EXPECT_EQ(argparse::ArgType::INT, arg->get_type());
}

TEST(Argument, set_type_invalid) {
  argparse_internal::ArgumentProcessor proc;
  EXPECT_THROW(proc.add_argument("-a").type("invalid"),
               argparse::exception::ConfigureError);
}

class ArgumentUsage : public ::testing::Test {
public:
  argparse_internal::ArgumentProcessor proc;
  argparse::Argument *arg;
  virtual void SetUp() {
    arg = &(this->proc.add_argument("-a"));
  }
  virtual void TearDown() {
  }
};

TEST_F(ArgumentUsage, store_with_one) {
  EXPECT_EQ("-a VAL", arg->usage());
  EXPECT_TRUE(arg->usage2().empty());
}

TEST_F(ArgumentUsage, store_with_three) {
  arg->nargs(3);
  
  EXPECT_EQ("-a VAL1 VAL2 VAL3", arg->usage());
}

TEST_F(ArgumentUsage, store_with_asterisk) {
  arg->nargs("*");
  
  EXPECT_EQ("-a [VAL [VAL ...]]", arg->usage());
}

TEST_F(ArgumentUsage, store_with_question) {
  arg->nargs("?");
  
  EXPECT_EQ("-a [VAL]", arg->usage());
}

TEST_F(ArgumentUsage, store_with_plus) {
  arg->nargs("+");
  EXPECT_EQ("-a VAL [VAL ...]", arg->usage());
}

TEST_F(ArgumentUsage, store_with_metavar) {
  arg->nargs("+").metavar("A");
  EXPECT_EQ("-a A [A ...]", arg->usage());
}

TEST_F(ArgumentUsage, store_true) {
  arg->action("store_true");
  EXPECT_EQ("-a", arg->usage());
}

TEST_F(ArgumentUsage, store_false) {
  arg->action("store_false");
  EXPECT_EQ("-a", arg->usage());
}

TEST_F(ArgumentUsage, append) {
  arg->action("append");
  EXPECT_EQ("-a VAL", arg->usage());
}

TEST_F(ArgumentUsage, append_const) {
  arg->action("append_const");
  EXPECT_EQ("-a", arg->usage());
}

TEST_F(ArgumentUsage, count) {
  arg->action("count");
  EXPECT_EQ("-a", arg->usage());
}

TEST_F(ArgumentUsage, name_2nd) {
  arg->name("--aim");
  EXPECT_EQ("-a VAL", arg->usage());
  EXPECT_EQ("--aim VAL", arg->usage2());
}

TEST_F(ArgumentUsage, seq_name) {
  argparse::Argument& arg2 = proc.add_argument("b");
  EXPECT_EQ("b", arg2.usage());
}

TEST_F(ArgumentUsage, seq_metavar) {
  argparse::Argument& arg2 = proc.add_argument("b");
  arg2.metavar("ALPHA");
  EXPECT_EQ("ALPHA", arg2.usage());
}

