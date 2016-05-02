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

#include <assert.h>
#include "argparse.hpp"


namespace argparse {

  // ========================================================
  // argparse::exception::*
  //
  exception::ConfigureError::ConfigureError(const std::string& errmsg,
                                            const std::string& tgt) {
    this->err() << "ConfigureError: " << errmsg << ", '" << tgt << "'";
  }
  
  exception::ParseError::ParseError(const std::string &errmsg) {
    this->err() << "ParseError: " << errmsg;
  }
  
  exception::KeyError::KeyError(const std::string& key,
                                const std::string &errmsg) {
    this->err() << "KeyError '" << key << "': " << errmsg;
  }
  
  exception::TypeError::TypeError(const std::string &errmsg) {
    this->err() << "TypeError: " << errmsg;
  }

  exception::IndexError::IndexError(const std::string &errmsg) {
    this->err() << "IndexError: " << errmsg;
  }

  
  // ========================================================
  // argparse::Values
  //
  Values::Values() : ptr_(new argparse_internal::Values()) {
  }
  
  Values::Values(const Values& obj) : ptr_(obj.ptr_) {
  }
  
  Values::~Values() {
  }
  
  Values& Values::operator=(const Values &obj) {
    this->ptr_ = obj.ptr_;
    return *this;
  }

  const std::string& Values::str(const std::string& key, size_t idx) const {
    return this->ptr_->str(key, idx);
  }
  
  const std::string& Values::to_s(const std::string& key, size_t idx) const {
    return this->ptr_->str(key, idx);
  }

  int Values::get(const std::string& key, size_t idx) const {
    return this->ptr_->get(key, idx);
  }
  
  int Values::to_i(const std::string& key, size_t idx) const {
    return this->ptr_->get(key, idx);
  }

  size_t Values::size(const std::string &key) const {
    return this->ptr_->size(key);
  }

  bool Values::is_true(const std::string &key) const {
    return this->ptr_->is_true(key);
  }
  
  bool Values::is_set(const std::string& dest) const {
    return this->ptr_->is_set(dest);
  }

  
  // ========================================================
  // argparse::Argument
  //
  Argument::Argument(argparse_internal::ArgumentProcessor *proc)
  :  arg_format_(ArgFormat::undef), type_(ArgType::STR), proc_(proc) {
  }
  Argument::~Argument() {
  }
  
  const std::string& Argument::set_name(const std::string &v_name) {
    if (!this->name_.empty()) {
      throw argparse::exception::ConfigureError("can not redefine name", v_name);
    }
    
    if (v_name.substr(0, 3) == "---") {
      throw exception::ConfigureError("too long hyphen", v_name);
    } else if (v_name.substr(0, 2) == "--") {
      this->name_ = v_name.substr(2);
    } else if (v_name.substr(0, 1) == "-") {
      this->name_ = v_name.substr(1);
    }
   
    // this is not an option, sequence
    if (this->name_.empty()) {
      this->name_ = v_name;
      this->arg_format_ = ArgFormat::sequence;
    } else {
      this->arg_format_ = ArgFormat::option;
    }
    
    return this->name_;
  }
  
  ParseResult Argument::parse(std::vector<const std::string> args, size_t idx)
  const {
    argparse_internal::Option *opt = NULL;

    if (this->arg_format_ == ArgFormat::option) {
      
    } else if (this->arg_format_ == ArgFormat::sequence) {
      if (args.size() <= idx) {
        std::stringstream ss;
        ss << "not enough arguments for '" << this->name_ << "' option";
        throw exception::ParseError(ss.str());
      }

      opt = argparse_internal::Option::build_option(args[idx], this->type_);
      idx += 1;
    }
    return ParseResult(0, std::unique_ptr<argparse_internal::Option>(opt));
  }
  
  
  Argument& Argument::action(Action action) {
    this->action_ = action;
    return *this;
  }

  Argument& Argument::name(const std::string &v_name) {
    this->name2_ = v_name;
    return *this;
  }

  Argument& Argument::nargs(const std::string &v_nargs) {
    return *this;
  }
  
  Argument& Argument::nargs(size_t v_nargs) {
    return *this;
  }


  Argument& Argument::set_const(const std::string &v_const) {
    return *this;
  }

  Argument& Argument::set_default(const std::string &v_default) {
    return *this;
  }

  Argument& Argument::type(ArgType v_type) {
    this->type_ = v_type;
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
  Parser::Parser(const std::string &prog_name)
  : prog_name_(prog_name), proc_(new argparse_internal::ArgumentProcessor()) {
  }
  Parser::~Parser() {
    delete this->proc_;
  }
  
  Argument& Parser::add_argument(const std::string &name) {
    return this->proc_->add_argument(name);
  }

  Values Parser::parse_args(const std::vector<std::string> &args) const {
    Values val;
    return val;
  }

  Values Parser::parse_args(int argc, char *argv[]) const {
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
      args.emplace_back(argv[i]);
    }
    
    return this->parse_args(args);
  }
}


// ==================================================================
// Internal Classes
//

namespace argparse_internal {
  
  Option* Option::build_option(const std::string& val, argparse::ArgType type) {
    Option *opt = NULL;
    
    switch (type) {
      case argparse::ArgType::INT:
        opt = new OptionInt(val);
        break;
        
      case argparse::ArgType::STR:
        opt = new OptionStr(val);
        break;

      case argparse::ArgType::BOOL:
        opt = new OptionBool(val);
        break;
    }
    
    assert(opt);
    
    if (! opt->is_valid()) {
      const std::string msg = opt->err();
      delete opt;
      throw argparse::exception::ParseError(msg);
    }

    return opt;
  }
  
  
  OptionInt::OptionInt(const std::string& val) {
    char *e;
    
    this->value_ = strtol(val.c_str(), &e, 0);
    if (*e != '\0') {
      this->err_stream() << "Invalid number format: " << val;
    }
  }
  
  OptionBool::OptionBool(const std::string& val) {
    if (val == "true") {
      this->value_ = true;
    } else if (val == "false") {
      this->value_ = false;
    } else {
      this->err_stream() << "Invalid bool format: " << val << ", " <<
        "should be true or false";
    }
  }

  // ------------------------------------------------------------------
  // class Values (argparse_interval)
  //

  Values::~Values() {
    for (auto it : this->optmap_) {
      for (auto vid : it.second) {
        delete vid;
      }
    }
  }
  
  Option* Values::find_option(const std::string& dest, size_t idx) const {
    auto it = this->optmap_.find(dest);
    if (it == this->optmap_.end()) {
      throw argparse::exception::KeyError(dest, "not found in destination");
    }
    
    if (idx >= it->second.size()) {
      std::stringstream ss;
      ss << idx << ": out of range for " << dest << ", " <<
      "except < " << it->second.size();
      throw argparse::exception::IndexError(ss.str());
    }
    
    Option *opt = it->second[idx];
    return opt;
  }
  
  const std::string& Values::str(const std::string& dest, size_t idx) const {
    return this->find_option(dest, idx)->str();
  }
  
  int Values::get(const std::string& dest, size_t idx) const {
    return this->find_option(dest, idx)->get();
  }
  bool Values::is_true(const std::string& dest) const {
    return false;
  }
  
  size_t Values::size(const std::string& dest) const {
    auto it = this->optmap_.find(dest);
    if (it == this->optmap_.end()) {
      throw argparse::exception::KeyError(dest, "not found in destination");
    }
    
    return it->second.size();
  }
  
  bool Values::is_set(const std::string& dest) const {
    auto it = this->optmap_.find(dest);
    if (it == this->optmap_.end()) {
      throw argparse::exception::KeyError(dest, "not found in destination");
    }
    
    return (it->second.size() > 0);
  }
  
  void Values::insert_option(const std::string &dest, Option *opt) {
    auto it = this->optmap_.find(dest);
    if (it == this->optmap_.end()) {
      throw argparse::exception::KeyError(dest, "not found in destination");
    }
    
  }
  
  // ------------------------------------------------------------------
  // class ArgumentProcessor
  //
  argparse::Argument& ArgumentProcessor::add_argument(const std::string &name) {
    auto arg = std::make_shared<argparse::Argument>(this);
    const std::string& key = arg->set_name(name);
    
    switch (arg->arg_format()) {
      case argparse::ArgFormat::option:
        this->argmap_.insert(std::make_pair(key, arg));
        break;
        
      case argparse::ArgFormat::sequence:
        this->argvec_.push_back(arg);
        break;
        
      case argparse::ArgFormat::undef:
        assert(0);
        break;
    }
    return *(arg.get());
  }
  
  argparse::Values ArgumentProcessor::parse_args(const std::vector<const std::string> &args) const {
    argparse::Values vals;
    return vals;
  }
  
  argparse::Values ArgumentProcessor::parse_args(int argc, char *argv[]) const {
    std::vector<const std::string> vec;
    for (int i = 0; i < argc; i++) {
      vec.emplace_back(argv[i]);
    }
    return this->parse_args(vec);
  }

}
