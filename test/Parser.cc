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


TEST(Parser, name2) {
  argparse::Parser *psr = new argparse::Parser("test");
  psr->add_argument("-a").name("--action");
  argparse::Argv ok1 = {"./test", "-a", "v"};
  argparse::Argv ok2 = {"./test", "--action", "v"};
  argparse::Argv ng1 = {"./test", "-a"}; // Not enough arguments
  argparse::Argv ng2 = {"./test", "--action"}; // Same as above

  argparse::Values v1 = psr->parse_args(ok1);
  argparse::Values v2 = psr->parse_args(ok2);
  EXPECT_EQ(v1.get("action"), "v"); // Using "name2" instead of "dest".
  EXPECT_EQ(v2.get("action"), "v"); // Same if the option is set by 2nd name.

  EXPECT_THROW(psr->parse_args(ng1), argparse::exception::ParseError);
  EXPECT_THROW(psr->parse_args(ng1), argparse::exception::ParseError);
}

TEST(Parser, usage1) {
  argparse::Parser psr("test");
  psr.add_argument("-a");
  psr.add_argument("-b").nargs("+");
  psr.add_argument("x");
  psr.add_argument("y");
  std::stringstream out;
  psr.set_output(&out);
  psr.usage();
  EXPECT_EQ("usage: test [-a VAL] [-b VAL [VAL ...]] [-h] x y\n", out.str());
}

TEST(Parser, usage2) {
  argparse::Parser psr("test");
  psr.add_argument("-n").metavar("X").required(true);
  psr.add_argument("-m").nargs(2);
  psr.add_argument("r").nargs(2);
  std::stringstream out;
  psr.set_output(&out);
  psr.usage();
  EXPECT_EQ("usage: test [-h] [-m VAL1 VAL2] -n X r1 r2\n", out.str());
}

TEST(Parser, help) {
  /* Sample
   usage: test.py [-h] [-a ALPHA] -b B [-c] [-d [INT [INT ...]]] [-e E [E ...]]
   [-f [F]]
   XXX XXX
   
   positional arguments:
   XXX                   test
   
   optional arguments:
   -h, --help            show this help message and exit
   -a ALPHA, --alpha ALPHA
   first attack
   -b B
   -c
   -d [INT [INT ...]], --int [INT [INT ...]]
   -e E [E ...]
   -f [F]
   
   */
  
  argparse::Parser psr("test");
  psr.add_argument("p").nargs("+").help("Piano");
  psr.add_argument("-d").name("--drum").metavar("POS")
    .required(true).nargs("+").help("Drum");
  psr.add_argument("-v").nargs(2).help("Vocal");
  psr.add_argument("-z").metavar("ZUN")
    .help("Dream divertisement, a memory of a world of morning mist "
          "in an illusionary.");

  std::vector<std::string> lines;
  
  std::stringstream out;
  psr.set_output(&out);

  psr.help();
  std::string buf = out.str();
  size_t idx = 0, next;
  while((next = buf.find_first_of("\n", idx)) != std::string::npos) {
    lines.emplace_back(buf.substr(idx, next - idx));
    idx = next + 1;
  }
  
  EXPECT_EQ(12, lines.size());
  EXPECT_EQ("usage: test -d POS [POS ...] [-h] [-v VAL1 VAL2] [-z ZUN] "
            "p [p ...]",                     lines[0]);
  EXPECT_EQ("",                              lines[1]);
  EXPECT_EQ("positional arguments:",         lines[2]);
  EXPECT_EQ("  p                     Piano", lines[3]);
  EXPECT_EQ("",                              lines[4]);
  EXPECT_EQ("optional arguments:",           lines[5]);
  EXPECT_EQ("  -d POS [POS ...], --drum POS [POS ...]", lines[6]);
  EXPECT_EQ("                        Drum",  lines[7]);
  EXPECT_EQ("  -h, --help            display help", lines[8]);
  EXPECT_EQ("  -v VAL1 VAL2          Vocal", lines[9]);
  EXPECT_EQ("  -z ZUN                ",      lines[10]);
  EXPECT_EQ("     Dream divertisement, a memory of a world of morning mist "
            "in an illusionary.", lines[11]);
}

TEST(Parser, help_by_option) {
  std::stringstream out;
  argparse::Parser psr("test");
  psr.set_output(&out);
  psr.add_argument("p").nargs("+").help("Piano");
  
  argparse::Argv seq = {"./test", "-h"};
  psr.parse_args(seq);
  EXPECT_FALSE(out.str().empty());
}
