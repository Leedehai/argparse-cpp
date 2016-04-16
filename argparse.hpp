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

#ifndef __ARGPARSE_HPP__
#define __ARGPARSE_HPP__

#include <memory>
#include <vector>
#include <string>

namespace argparse {
  class Values;
  class Values_;

  
  class Argument {
  private:
    std::string name_;
    std::string name2_;
    std::string nargs_;
    std::string const_;
    std::string default_;
    std::string type_;
    std::string choices_;
    bool required_;
    std::string help_;
    std::string metavar_;
    std::string dest_;
    
  public:
    Argument(const std::string& name);
    ~Argument();
    
    Argument& name(const std::string &v_name);
    Argument& action(const std::string &v_action);
    Argument& nargs(const std::string &v_nargs);
    Argument& set_const(const std::string &v_const);
    Argument& set_default(const std::string &v_default);
    Argument& type(const std::string &v_type);
    Argument& choices(const std::string &v_choices);
    Argument& required(bool req);
    Argument& help(const std::string &v_help);
    Argument& metavar(const std::string &v_metavar);
    Argument& dest(const std::string &v_dest);
  };

  
  class Parser {
  private:
    std::string prog_name_;
    
  public:
    Parser(const std::string &prog_name);
    ~Parser();
    Parser& usage(const std::string &s);
    Argument& add_argument(const std::string &name);
    Values parse_args(const std::vector<const std::string> args);
    Values parse_args(int argc, char *argv[]);
  };
  
  
  class Values {
  private:
    std::shared_ptr<Values_> ptr_;
  public:
    Values();
    Values(const Values &obj);
    ~Values();
  };
}

#endif   // __ARGPARSE_HPP__
