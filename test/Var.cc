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

TEST(Var, VarInt) {
  argparse_internal::VarInt opt("2");
  EXPECT_TRUE(opt.is_valid());
  EXPECT_EQ(2, opt.get());
  EXPECT_THROW(opt.str(), argparse::exception::TypeError);
  EXPECT_THROW(opt.is_true(), argparse::exception::TypeError);
  
  // With alphabet character (invalid)
  argparse_internal::VarInt opt2("123c");
  EXPECT_FALSE(opt2.is_valid());
}

TEST(Var, build_var) {
  // Allow using namespace for readability.
  using namespace argparse_internal;
  
  // Building option class from factory method.
  Var* opt_i = Var::build_var("512", argparse::ArgType::INT);
  Var* opt_s = Var::build_var("abc", argparse::ArgType::STR);
  Var* opt_b = Var::build_var("true", argparse::ArgType::BOOL);

  EXPECT_TRUE(opt_i->is_valid());
  EXPECT_TRUE(opt_s->is_valid());
  EXPECT_TRUE(opt_b->is_valid());
  
  // Throw exception if invalid format from build_option.
  EXPECT_THROW(Var::build_var("123e", argparse::ArgType::INT),
               argparse::exception::ParseError);
  EXPECT_THROW(Var::build_var("xxx", argparse::ArgType::BOOL),
               argparse::exception::ParseError);
  
}

TEST(Var, VarStr) {
  argparse_internal::VarStr opt1("five");
  EXPECT_TRUE(opt1.is_valid());
  EXPECT_EQ("five", opt1.str());
  EXPECT_THROW(opt1.get(), argparse::exception::TypeError);
  EXPECT_THROW(opt1.is_true(), argparse::exception::TypeError);
  
  // Accepting number as string.
  argparse_internal::VarStr opt2("1234");
  EXPECT_TRUE(opt2.is_valid());
  EXPECT_EQ("1234", opt2.str());
  EXPECT_THROW(opt2.get(), argparse::exception::TypeError);
  EXPECT_THROW(opt2.is_true(), argparse::exception::TypeError);

}

TEST(Var, VarBool) {
  argparse_internal::VarBool opt1("true");
  EXPECT_TRUE(opt1.is_valid());
  EXPECT_TRUE(opt1.is_true());
  EXPECT_THROW(opt1.get(), argparse::exception::TypeError);
  EXPECT_THROW(opt1.str(), argparse::exception::TypeError);
  
  argparse_internal::VarBool opt2("false");
  EXPECT_TRUE(opt2.is_valid());
  EXPECT_FALSE(opt2.is_true());
  EXPECT_THROW(opt2.get(), argparse::exception::TypeError);
  EXPECT_THROW(opt2.str(), argparse::exception::TypeError);
  
  argparse_internal::VarBool opt3("invalid_phrase");
  EXPECT_FALSE(opt3.is_valid());

}