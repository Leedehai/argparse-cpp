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

#include "argparse.hpp"

namespace argparse {
  class Values_ {
  public:
    Values_() {}
    ~Values_() {}
  };
  
  // ========================================================
  // argparse::
  //

  // ========================================================
  // argparse::Values
  //
  Values::Values() : ptr_(new Values_()) {
  }
  
  Values::Values(const Values& obj) : ptr_(obj.ptr_) {
  }
  
  // ========================================================
  // argparse::Argument
  //
  Argument::Argument(const std::string& name) : name_(name) {
  }
  Argument::~Argument() {
  }
  
  Argument& Argument::name(const std::string &v_name) {
    return *this;
  }
  
  Argument& Argument::action(const std::string &v_action) {
    return *this;
  }

  Argument& Argument::nargs(const std::string &v_nargs) {
    return *this;
  }

  Argument& Argument::set_const(const std::string &v_const) {
    return *this;
  }

  Argument& Argument::set_default(const std::string &v_default) {
    return *this;
  }

  Argument& Argument::type(const std::string &v_type) {
    return *this;
  }

  Argument& Argument::choices(const std::string &v_choices) {
    return *this;
  }

  Argument& Argument::required(bool req) {
    this->required_ = req;
    return *this;
  }

  Argument& Argument::help(const std::string &v_help) {
    return *this;
  }

  Argument& Argument::metavar(const std::string &v_metavar) {
    return *this;
  }

  Argument& Argument::dest(const std::string &v_dest) {
    return *this;
  }

  // ========================================================
  // argparse::Parser
  //
  Parser::Parser(const std::string &prog_name) : prog_name_(prog_name) {
  }
  
  Parser::~Parser() {
    
  }
  
  // 
}
