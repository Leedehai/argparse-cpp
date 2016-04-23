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

TEST(Option, OptionInt) {
  argparse_internal::OptionInt opt(2);
  EXPECT_EQ(2, opt.get());
  EXPECT_THROW(opt.str(), argparse::exception::TypeError);
  EXPECT_THROW(opt.is_true(), argparse::exception::TypeError);
}

TEST(Option, OptionStr) {
  argparse_internal::OptionStr opt("five");
  EXPECT_EQ("five", opt.str());
  EXPECT_THROW(opt.get(), argparse::exception::TypeError);
  EXPECT_THROW(opt.is_true(), argparse::exception::TypeError);
}

TEST(Option, OptionBool) {
  argparse_internal::OptionBool opt(true);
  EXPECT_TRUE(opt.is_true());
  EXPECT_THROW(opt.get(), argparse::exception::TypeError);
  EXPECT_THROW(opt.str(), argparse::exception::TypeError);
}