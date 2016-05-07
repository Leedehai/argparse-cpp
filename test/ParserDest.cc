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

class ParserDest : public ::testing::Test {
public:
  argparse::Parser psr;
  argparse::Argument *arg;
  virtual void SetUp() {
    arg = &(psr.add_argument("-a").dest("beta"));
  }
};

TEST_F(ParserDest, ok1) {
  argparse::Argv seq = {"./test", "-a", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_FALSE(val.is_set("a"));
  EXPECT_TRUE(val.is_set("beta"));
  EXPECT_EQ(1, val.size("beta"));
  EXPECT_EQ("v1", val.get("beta", 0));
}

TEST_F(ParserDest, ok2) {
  // Use 2nd name if no dest
  psr.add_argument("-b").name("--gamma");
  argparse::Argv seq = {"./test", "-b", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_FALSE(val.is_set("b"));
  EXPECT_TRUE(val.is_set("gamma"));
  EXPECT_EQ(1, val.size("gamma"));
  EXPECT_EQ("v1", val.get("gamma", 0));
}

TEST_F(ParserDest, ok3) {
  // Use dest if both of 2nd name and dest are set.
  psr.add_argument("-b").name("--gamma").dest("omega");
  argparse::Argv seq = {"./test", "-b", "v1"};
  argparse::Values val = psr.parse_args(seq);
  EXPECT_FALSE(val.is_set("b"));
  EXPECT_FALSE(val.is_set("gamma"));
  EXPECT_TRUE(val.is_set("omega"));
  EXPECT_EQ(1, val.size("omega"));
  EXPECT_EQ("v1", val.get("omega", 0));
}


