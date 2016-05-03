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
  Values::Values(std::shared_ptr<argparse_internal::Values> ptr)
  : ptr_(ptr) {
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
  : arg_format_(ArgFormat::undef),
    nargs_(Nargs::NUMBER),
    nargs_num_(1),
    type_(ArgType::STR),
    action_(Action::store),
    proc_(proc) {
  }
  Argument::~Argument() {
  }
  
  const std::string& Argument::set_name(const std::string &v_name) {
    if (!this->name_.empty()) {
      throw argparse::exception::ConfigureError("can not redefine name", v_name);
    }
    
    if (v_name.substr(0, 3) == "---") {
      throw exception::ConfigureError("too long hyphen. Supporting only 1 or 2",
                                      v_name);
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
  
  size_t Argument::parse_append(const Argv& args, size_t idx,
                                std::vector<argparse_internal::Option*>* opt_list)
  const {
    std::vector<argparse_internal::Option*> options;
    argparse_internal::Option *opt; // Just for readability.
  
    // Defined argument number.
    size_t i = idx, e;
    if (this->nargs_ == Nargs::NUMBER) {
      e = idx + this->nargs_num_;
    } else if (this->nargs_ == Nargs::QUESTION) {
      e = idx + 1;
    } else {
      e = 0;
    }
    
    // Storing arguments.
    while ((e == 0 || i < e) && i < args.size() &&
           args[i].substr(0, 1) != "-") {
      opt = argparse_internal::Option::build_option(args[i], this->type_);
      options.push_back(opt);
      i++;
    }
    
    assert(i >= idx);
    size_t argc = i - idx;
    assert(argc == options.size());
    
    std::stringstream err;
    
    if (this->nargs_num_ > 1 && argc != this->nargs_num_) {
      assert(this->nargs_ == Nargs::NUMBER);
      err << "option '" << this->name_ << "' must have " << this->nargs_num_
      << "arguments";
    } else if (argc == 0) { // If no argument,
      std::string value;
      
      // Use if default_ or const_ is set
      if (! this->default_.empty()) {
        value = this->default_;
      } else if (! this->const_.empty()) {
        value = this->const_;
      }
      
      if (value.empty()) {
        // No arguments and default values
        if (this->nargs_ == Nargs::PLUS) {
          err << "option '" << this->name_ << "' must have 1 or more arguments";
        } else if (this->nargs_ == Nargs::NUMBER) {
          assert(this->nargs_num_ == 1);
          err << "option '" << this->name_ << "' must have 1 arguments";
        } else if (this->nargs_ == Nargs::QUESTION) {
          opt = new argparse_internal::OptionNull();
          options.push_back(opt);
        }
      } else {
        opt = argparse_internal::Option::build_option(value, this->type_);
        options.push_back(opt);
      }
    }
    
    // If error, delete all Option instances and throw exception.
    if (! err.str().empty()) {
      for (auto opt_ptr : options) {
        delete opt_ptr;
      }
      throw exception::ParseError(err.str());
    }
    
    // Move option pointers to opt_list.
    for (auto opt_ptr : options) {
      opt_list->push_back(opt_ptr);
    }
    
    return i;
  }
  
  size_t Argument::parse(const Argv& args, size_t idx,
                         std::vector<argparse_internal::Option*> *opt_list)
  const {
    size_t r_idx = idx;
    argparse_internal::Option* opt = nullptr; // Just for readability.
    
    switch(this->action_) {
      // Check double store error in Parser, no matter in Argument::parse.
      case Action::store:
      case Action::append:
        r_idx = this->parse_append(args, idx, opt_list);
        break;
        
      // Check double store error in Parser, no matter in Argument::parse.
      case Action::store_const:
      case Action::append_const:
        opt = argparse_internal::Option::build_option(this->const_, this->type_);
        break;
        
      case Action::store_true:
        opt = argparse_internal::Option::build_option("true", ArgType::BOOL);
        break;
        
      case Action::store_false:
        opt = argparse_internal::Option::build_option("false", ArgType::BOOL);
        break;

      case Action::count:
      case Action::help:
      case Action::version:
        assert(0);
        break;
        // TODO:
        // count, help, version
    }

    if (opt) {
      opt_list->push_back(opt);
    }
    
    return r_idx;
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
    if (v_nargs == "?") {
      this->nargs_ = Nargs::QUESTION;
    } else if (v_nargs == "*") {
      this->nargs_ = Nargs::ASTERISK;
    } else if (v_nargs == "+") {
      this->nargs_ = Nargs::PLUS;
    } else {
      throw exception::ParseError("Invalid argument: " + v_nargs);
    }

    this->nargs_num_ = 0;
    
    return *this;
  }
  
  Argument& Argument::nargs(size_t v_nargs) {
    this->nargs_num_ = v_nargs;
    this->nargs_ = Nargs::NUMBER;
    return *this;
  }


  Argument& Argument::set_const(const std::string &v_const) {
    this->const_ = v_const;
    return *this;
  }

  Argument& Argument::set_default(const std::string &v_default) {
    this->default_ = v_default;
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

  Values Parser::parse_args(const Argv& args) const {
    Values val = this->proc_->parse_args(args);
    return val;
  }

  Values Parser::parse_args(int argc, char *argv[]) const {
    Argv args;
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
      for (auto vid : *(it.second)) {
        delete vid;
      }
      delete it.second;
    }
  }
  
  Option* Values::find_option(const std::string& dest, size_t idx) const {
    auto it = this->optmap_.find(dest);
    if (it == this->optmap_.end()) {
      throw argparse::exception::KeyError(dest, "not found in arguments");
    }
    
    if (idx >= it->second->size()) {
      std::stringstream ss;
      ss << idx << ": out of range for " << dest << ", " <<
      "except < " << it->second->size();
      throw argparse::exception::IndexError(ss.str());
    }
    
    Option *opt = (*(it->second))[idx];
    return opt;
  }
  
  std::vector<Option*>* Values::get_optmap(const std::string& dest) {
    auto it = this->optmap_.find(dest);
    std::vector<Option*> *vec = nullptr;
    if (it == this->optmap_.end()) {
      vec = new std::vector<Option*>();
      this->optmap_.insert(std::make_pair(dest, vec));
    } else {
      vec = it->second;
    }
    
    assert(vec != nullptr);
    return vec;
  }

  
  const std::string& Values::str(const std::string& dest, size_t idx) const {
    return this->find_option(dest, idx)->str();
  }
  
  int Values::get(const std::string& dest, size_t idx) const {
    return this->find_option(dest, idx)->get();
  }
  bool Values::is_true(const std::string& dest) const {
    return this->find_option(dest, 0)->is_true();
  }
  
  size_t Values::size(const std::string& dest) const {
    auto it = this->optmap_.find(dest);
    if (it == this->optmap_.end()) {
      throw argparse::exception::KeyError(dest, "not found in destination");
    }
    
    return (it->second)->size();
  }
  
  bool Values::is_set(const std::string& dest) const {
    auto it = this->optmap_.find(dest);
    if (it == this->optmap_.end()) {
      throw argparse::exception::KeyError(dest, "not found in destination");
    }
    
    return ((it->second)->size() > 0);
  }
  
  
  // ------------------------------------------------------------------
  // class ArgumentProcessor
  //
  size_t ArgumentProcessor::parse_option(const argparse::Argv& args, size_t idx,
                                         const std::string& optkey,
                                         Values *vals) const {
    auto it = this->argmap_.find(optkey);
    if (it == this->argmap_.end()) {
      throw argparse::exception::ParseError("option not found: " + optkey);
    }

    std::shared_ptr<argparse::Argument> argument = it->second;
    
    auto optvec = vals->get_optmap(argument->get_dest());
    if (optvec->size() > 0 &&
        (argument->get_action() != argparse::Action::append &&
         argument->get_action() != argparse::Action::append_const)) {
          throw argparse::exception::ParseError("duplicated option: " + optkey);
        }
    
    idx = argument->parse(args, idx, optvec);
    
    return idx;
  }

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
  
  argparse::Values ArgumentProcessor::parse_args(const argparse::Argv& args)
  const {
    std::shared_ptr<Values> ptr = std::make_shared<Values>();
    size_t seq_idx = 0;
    
    for (size_t idx = 1; idx < args.size(); ) {
      const std::string& arg = args[idx];
      
      if (arg.substr(0, 3) == "---") {
        throw argparse::exception::ParseError("too long hyphen. "
                                              "Supporting only 1 or 2: " + arg);
      } else if (arg.substr(0, 2) == "--") {
        const std::string key = arg.substr(2);
        idx = this->parse_option(args, idx, key, ptr.get());
      } else if (arg.substr(0, 1) == "-") {
        for (size_t c = 1; c < arg.length(); c++) {
          const std::string key = arg.substr(c, 1);
          idx = this->parse_option(args, idx + 1, key, ptr.get());
        }
      } else {
        if (this->argvec_.size() <= seq_idx) {
          throw argparse::exception::ParseError("too long arguments after " +
                                                args[idx]);
        }
        
        std::shared_ptr<argparse::Argument> argument = this->argvec_[seq_idx];
        const std::string& dest = argument->get_dest();
        idx = argument->parse(args, idx, ptr->get_optmap(dest));
        seq_idx++;
      }
    }

    argparse::Values vals(ptr);
    return vals;
  }
  
  argparse::Values ArgumentProcessor::parse_args(int argc, char *argv[]) const {
    argparse::Argv vec;
    for (int i = 0; i < argc; i++) {
      vec.emplace_back(argv[i]);
    }
    return this->parse_args(vec);
  }

}
