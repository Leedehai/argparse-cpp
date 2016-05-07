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

class ParserAction : public ::testing::Test {
public:
  argparse::Parser psr;
  argparse::Argument *arg;
};

class ParserActionStore : public ParserAction {
public:
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").action("store"));
  }
};

TEST_F(ParserActionStore, ok1) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("v1", val.get("a", 0));
}

TEST_F(ParserActionStore, ng_duplicated_option) {
  argparse::Argv seq = {"./test", "-a", "v1", "-a", "v2"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

class ParserActionStoreConst : public ParserAction {
public:
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").action("store_const").set_const("c"));
  }
};

TEST_F(ParserActionStoreConst, ok1) {
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("c", val["a"]);
}

TEST_F(ParserActionStoreConst, ng_use_nargs1) {
  arg->nargs("?");
  argparse::Argv seq = {"./test", "-a"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreConst, ng_use_nargs2) {
  arg->nargs("*");
  argparse::Argv seq = {"./test", "-a"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreConst, ng_use_nargs3) {
  arg->nargs("+");
  argparse::Argv seq = {"./test", "-a"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreConst, ng_use_nargs4) {
  arg->nargs(2);
  argparse::Argv seq = {"./test", "-a"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreConst, ng_duplicated_option) {
  argparse::Argv seq = {"./test", "-a", "-a"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}


class ParserActionStoreTrue : public ParserAction {
public:
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").action("store_true"));
  }
};

TEST_F(ParserActionStoreTrue, ok_put_true) {
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("true", val["a"]);
  EXPECT_TRUE(val.is_true("a"));
}


TEST_F(ParserActionStoreTrue, ok_put_false_if_no_arg) {
  argparse::Argv seq = {"./test"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("false", val["a"]);
  EXPECT_FALSE(val.is_true("a"));
}

TEST_F(ParserActionStoreTrue, ok_with_default) {
  arg->set_default("v");
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("true", val["a"]);
  EXPECT_TRUE(val.is_true("a"));
}

TEST_F(ParserActionStoreTrue, ok_with_default_and_no_arg) {
  // use default value when no argument even if it's not boolean.
  arg->set_default("v");
  argparse::Argv seq = {"./test"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("v", val["a"]);
}

TEST_F(ParserActionStoreTrue, ng_with_const) {
  arg->set_const("c");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreTrue, ng_with_modified_nargs1) {
  arg->nargs("*");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreTrue, ng_with_modified_nargs2) {
  arg->nargs(2);
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}



class ParserActionStoreFalse : public ParserAction {
public:
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").action("store_false"));
  }
};

TEST_F(ParserActionStoreFalse, ok_put_false) {
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("false", val["a"]);
  EXPECT_FALSE(val.is_true("a"));
}

TEST_F(ParserActionStoreFalse, ok_put_true_if_no_arg) {
  argparse::Argv seq = {"./test"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("true", val["a"]);
  EXPECT_TRUE(val.is_true("a"));
}

TEST_F(ParserActionStoreFalse, ok_with_default) {
  arg->set_default("v");
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("false", val["a"]);
  EXPECT_FALSE(val.is_true("a"));
}

TEST_F(ParserActionStoreFalse, ok_with_default_and_no_arg) {
  // use default value when no argument even if it's not boolean.
  arg->set_default("v");
  argparse::Argv seq = {"./test"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("v", val["a"]);
}

TEST_F(ParserActionStoreFalse, ng_with_const) {
  arg->set_const("c");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreFalse, ng_with_modified_nargs1) {
  arg->nargs("*");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionStoreFalse, ng_with_modified_nargs2) {
  arg->nargs(2);
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}



class ParserActionAppend : public ParserAction {
public:
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").action("append"));
  }
};

TEST_F(ParserActionAppend, ok1) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("v1", val.get("a", 0));
}

TEST_F(ParserActionAppend, ok2) {
  argparse::Argv seq = {"./test", "-a", "v1", "-a", "v2"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(2, val.size("a"));
  EXPECT_EQ("v1", val.get("a", 0));
  EXPECT_EQ("v2", val.get("a", 1));
}

TEST_F(ParserActionAppend, ok3) {
  arg->nargs("*");
  argparse::Argv seq = {"./test", "-a", "-a", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("v1", val.get("a", 0));
}

TEST_F(ParserActionAppend, ng_no_value) {
  argparse::Argv seq = {"./test", "-a", "-a", "v1"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

class ParserActionAppendConst : public ParserAction {
public:
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").action("append_const").set_const("c"));
  }
};

TEST_F(ParserActionAppendConst, ok1) {
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ("c", val.get("a", 0));
}

TEST_F(ParserActionAppendConst, ok2) {
  argparse::Argv seq = {"./test", "-a", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_EQ(2, val.size("a"));
  EXPECT_EQ("c", val.get("a", 0));
  EXPECT_EQ("c", val.get("a", 1));
}

TEST_F(ParserActionAppendConst, ng_with_argument) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserActionAppendConst, ng_with_modified_nargs1) {
  arg->nargs("*");
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}

TEST_F(ParserActionAppendConst, ng_with_modified_nargs2) {
  arg->nargs(2);
  argparse::Argv seq = {"./test"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ConfigureError);
}


class ParserActionCount : public ParserAction {
public:
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").action("count"));
    psr.add_argument("-b");
  }
};

TEST_F(ParserActionCount, ok1) {
  argparse::Argv seq = {"./test", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ(1, val.to_int("a", 0));
  EXPECT_EQ("1", val.get("a", 0));
}

TEST_F(ParserActionCount, ok2) {
  argparse::Argv seq = {"./test", "-a", "-a", "-b", "v", "-a"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ(3, val.to_int("a", 0));
  EXPECT_EQ("3", val.get("a", 0));
}

TEST_F(ParserActionCount, ng_with_argument) {
  argparse::Argv seq = {"./test", "-a", "r", "-b", "v"};
  EXPECT_THROW(psr.parse_args(seq), argparse::exception::ParseError);
}

TEST_F(ParserActionCount, ok_with_default1) {
  argparse::Argv seq = {"./test"};
  arg->set_default("5");
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ(5, val.to_int("a"));
}

TEST_F(ParserActionCount, ok_with_default2) {
  argparse::Argv seq = {"./test", "-a"};
  arg->set_default("5");
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ(6, val.to_int("a"));
}

TEST_F(ParserActionCount, ok_with_other_option) {
  argparse::Argv seq = {"./test", "-aaa", "-b", "v"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_TRUE(val.is_set("a"));
  EXPECT_EQ(1, val.size("a"));
  EXPECT_EQ(3, val.to_int("a"));
  EXPECT_TRUE(val.is_set("b"));
  EXPECT_EQ("v", val["b"]);
}

