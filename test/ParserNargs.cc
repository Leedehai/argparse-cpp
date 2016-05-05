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

class ParserNargsNumber : public ::testing::Test {
public:
  argparse::Parser *psr;
  argparse::Argument *arg;
  virtual void SetUp() {
    psr = new argparse::Parser("test");
    arg = &(psr->add_argument("-a").nargs(2));
    psr->add_argument("-b");
  }
  
  virtual void TearDown() { delete psr; }
};


TEST_F(ParserNargsNumber, ok1) {
  argparse::Argv seq = {"./test", "-a", "v1", "v2"};
  argparse::Values val = psr->parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(2, val.size("a"));
  EXPECT_EQ("v1", val.get("a", 0));
  EXPECT_EQ("v2", val.get("a", 1));
}

TEST_F(ParserNargsNumber, ng1_too_many_arg) {
  argparse::Argv seq = {"./test", "-a", "v1", "v2", "v3"};
  EXPECT_THROW(psr->parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserNargsNumber, ng2_not_enough_arg) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  EXPECT_THROW(psr->parse_args(seq), argparse::exception::ParseError);
}



class ParserNargsAsterisk : public ::testing::Test {
public:
  argparse::Parser *psr;
  argparse::Argument *arg;
  virtual void SetUp() {
    psr = new argparse::Parser("test");
    arg = &(psr->add_argument("-a").nargs("*"));
    psr->add_argument("-b");
  }
  
  virtual void TearDown() { delete psr; }
};

TEST_F(ParserNargsAsterisk, no_option) {
  argparse::Argv ok1 = {"./test"};
  argparse::Values v1 = psr->parse_args(ok1);
  EXPECT_FALSE(v1.is_set("a"));
}

TEST_F(ParserNargsAsterisk, ok1) {
  argparse::Argv ok1 = {"./test", "-a", "v1", "v2"};
  argparse::Values v1 = psr->parse_args(ok1);
  EXPECT_EQ(2, v1.size("a"));
  EXPECT_EQ("v1", v1.get("a", 0));
  EXPECT_EQ("v2", v1.get("a", 1));
}

TEST_F(ParserNargsAsterisk, ok2) {
  // Stop parsing by an other option.
  argparse::Argv ok2 = {"./test", "-a", "v1", "v2", "-b", "r1"};
  argparse::Values v2 = psr->parse_args(ok2);
  EXPECT_EQ(2, v2.size("a"));
  EXPECT_EQ("v1", v2.get("a", 0));
  EXPECT_EQ("v2", v2.get("a", 1));
  EXPECT_EQ(1, v2.size("b"));
  EXPECT_EQ("r1", v2.get("b"));
}

TEST_F(ParserNargsAsterisk, ok3) {
  // No option value.
  argparse::Argv ok3 = {"./test", "-a", "-b", "r1"};
  argparse::Values v3 = psr->parse_args(ok3);
  EXPECT_EQ(0, v3.size("a"));
  EXPECT_TRUE(v3.is_set("a"));
  EXPECT_EQ(1, v3.size("b"));
  EXPECT_EQ("r1", v3.get("b"));
}




class ParserNargsQuestion : public ::testing::Test {
public:
  argparse::Parser *psr;
  argparse::Argument *arg;
  virtual void SetUp() {
    psr = new argparse::Parser("test");
    arg = &(psr->add_argument("-a").nargs("?")); // 0 or 1 value
    psr->add_argument("-b");
  }
  
  virtual void TearDown() { delete psr; }
};

TEST_F(ParserNargsQuestion, ok0) {
  argparse::Argv seq = {"./test"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_FALSE(v.is_set("a"));
  EXPECT_EQ(0, v.size("a"));
  EXPECT_FALSE(v.is_set("b"));
}

TEST_F(ParserNargsQuestion, ok1) {
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
  EXPECT_FALSE(v.is_set("b"));
}

TEST_F(ParserNargsQuestion, ok2) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
  EXPECT_EQ("v1", v["a"]);
  EXPECT_FALSE(v.is_set("b"));
}

TEST_F(ParserNargsQuestion, ok3) {
  argparse::Argv seq = {"./test", "-a", "v1", "-b", "r1"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
  EXPECT_EQ("v1", v["a"]);
  EXPECT_TRUE(v.is_set("b"));
}

TEST_F(ParserNargsQuestion, with_default1) {
  arg->set_default("d");
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
}

TEST_F(ParserNargsQuestion, with_default2) {
  arg->set_default("d");
  argparse::Argv seq = {"./test"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
  EXPECT_EQ("d", v["a"]);
}

TEST_F(ParserNargsQuestion, with_const) {
  arg->set_const("c");
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
  EXPECT_EQ("c", v["a"]);
}

TEST_F(ParserNargsQuestion, with_default_and_const) {
  arg->set_const("c").set_default("d");
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
  // 'const' has priority over 'default'
  EXPECT_EQ("c", v["a"]);
}

class ParserNargsPlus : public ::testing::Test {
public:
  argparse::Parser *psr;
  argparse::Argument *arg;
  virtual void SetUp() {
    psr = new argparse::Parser("test");
    arg = &(psr->add_argument("-a").nargs("+")); // 0 or 1 value
    psr->add_argument("-b");
  }
  
  virtual void TearDown() { delete psr; }
};

TEST_F(ParserNargsPlus, ok0) {
  argparse::Argv seq = {"./test"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_FALSE(v.is_set("a"));
  EXPECT_EQ(0, v.size("a"));
  EXPECT_FALSE(v.is_set("b"));
}

TEST_F(ParserNargsPlus, ok1) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(1, v.size("a"));
  EXPECT_EQ("v1", v.get("a", 0));
  EXPECT_FALSE(v.is_set("b"));
}

TEST_F(ParserNargsPlus, ok2) {
  argparse::Argv seq = {"./test", "-a", "v1", "v2"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(2, v.size("a"));
  EXPECT_EQ("v1", v.get("a", 0));
  EXPECT_EQ("v2", v.get("a", 1));
  EXPECT_FALSE(v.is_set("b"));
}

TEST_F(ParserNargsPlus, ok3) {
  argparse::Argv seq = {"./test", "-a", "v1", "v2", "v3", "-b", "r1"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_TRUE(v.is_set("a"));
  EXPECT_EQ(3, v.size("a"));
  EXPECT_EQ("v1", v.get("a", 0));
  EXPECT_EQ("v2", v.get("a", 1));
  EXPECT_EQ("v3", v.get("a", 2));
  EXPECT_TRUE(v.is_set("b"));
}

TEST_F(ParserNargsPlus, ng_no_argument1) {
  argparse::Argv seq = {"./test", "-a"};
  EXPECT_THROW(psr->parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserNargsPlus, ng_no_argument2) {
  argparse::Argv seq = {"./test", "-a", "-b", "r1"};
  EXPECT_THROW(psr->parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserNargsPlus, with_default1) {
  arg->set_default("d");
  argparse::Argv seq = {"./test", "-a", "v1", "-b", "r1"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_EQ(1, v.size("a"));
  EXPECT_EQ("v1", v["a"]);
}

TEST_F(ParserNargsPlus, with_default2) {
  arg->set_default("d");
  argparse::Argv seq = {"./test", "-b", "r1"};
  argparse::Values v = psr->parse_args(seq);
  EXPECT_EQ(1, v.size("a"));
  EXPECT_EQ("d", v["a"]);
}



