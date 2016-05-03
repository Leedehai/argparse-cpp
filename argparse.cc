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
  const std::vector<argparse_internal::Var*>&
    Values::get_var_arr(const VarMap& varmap, const std::string& key) {
    auto it = varmap.find(key);
    if (it == varmap.end()) {
      throw argparse::exception::KeyError(key, "not found in options");
    }

    return *(it->second);
  }

  const argparse_internal::Var& Values::get_var(const VarMap& varmap,
                                                const std::string& key,
                                                size_t idx) {
    auto arr = get_var_arr(varmap, key);
    if (arr.size() <= idx) {
      throw argparse::exception::IndexError(key);
    }
    
    return *(arr[idx]);
  }

  
  Values::Values(std::shared_ptr<VarMap> varmap) : varmap_(varmap) {
  }
  
  Values::Values(const Values& obj) : varmap_(obj.varmap_) {
  }
  
  Values::~Values() {
  }
  
  Values& Values::operator=(const Values &obj) {
    this->varmap_ = obj.varmap_;
    return *this;
  }

  const std::string& Values::get(const std::string& key, size_t idx) const {
    return this->to_str(key, idx);
  }
  
  const std::string& Values::to_str(const std::string& key, size_t idx) const {
    const argparse_internal::Var& v = Values::get_var(*(this->varmap_.get()),
                                                      key, idx);
    return v.to_s();
  }

  int Values::to_int(const std::string& key, size_t idx) const {
    const argparse_internal::Var& v = Values::get_var(*(this->varmap_.get()),
                                                      key, idx);
    return v.to_i();
  }
  
  size_t Values::size(const std::string &key) const {
    auto arr = get_var_arr(*(this->varmap_.get()), key);
    return arr.size();
  }

  bool Values::is_true(const std::string &key) const {
    auto arr = get_var_arr(*(this->varmap_.get()), key);
    assert(arr.size() == 1);
    return arr[0]->is_true();
  }
  
  bool Values::is_set(const std::string& dest) const {
    const VarMap& varmap = *(this->varmap_.get());
    auto it = varmap.find(dest);
    return (it != varmap.end());
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
                                std::vector<argparse_internal::Var*>* opt_list)
  const {
    std::vector<argparse_internal::Var*> options;
    argparse_internal::Var *opt; // Just for readability.
  
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
      opt = argparse_internal::Var::build_var(args[i], this->type_);
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
          opt = new argparse_internal::VarNull();
          options.push_back(opt);
        }
      } else {
        opt = argparse_internal::Var::build_var(value, this->type_);
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
                         std::vector<argparse_internal::Var*> *opt_list)
  const {
    size_t r_idx = idx;
    argparse_internal::Var* opt = nullptr; // Just for readability.
    
    switch(this->action_) {
      // Check double store error in Parser, no matter in Argument::parse.
      case Action::store:
      case Action::append:
        r_idx = this->parse_append(args, idx, opt_list);
        break;
        
      // Check double store error in Parser, no matter in Argument::parse.
      case Action::store_const:
      case Action::append_const:
        opt = argparse_internal::Var::build_var(this->const_, this->type_);
        break;
        
      case Action::store_true:
        opt = argparse_internal::Var::build_var("true", ArgType::BOOL);
        break;
        
      case Action::store_false:
        opt = argparse_internal::Var::build_var("false", ArgType::BOOL);
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

  // ------------------------------------------------------------------
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
  
  Var* Var::build_var(const std::string& val, argparse::ArgType type) {
    Var *opt = NULL;
    
    switch (type) {
      case argparse::ArgType::INT:
        opt = new VarInt(val);
        break;
        
      case argparse::ArgType::STR:
        opt = new VarStr(val);
        break;

      case argparse::ArgType::BOOL:
        opt = new VarBool(val);
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
  
  
  VarInt::VarInt(const std::string& val) {
    char *e;
    this->str_ = val;
    this->value_ = strtol(val.c_str(), &e, 0);
    if (*e != '\0') {
      this->err_stream() << "Invalid number format: " << val;
    }
  }
  
  const std::string VarBool::true_("true");
  const std::string VarBool::false_("false");

  VarBool::VarBool(const std::string& val) {
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
  // class ArgumentProcessor
  //
  size_t ArgumentProcessor::parse_option(const argparse::Argv& args,
                                         size_t idx,
                                         const std::string& optkey,
                                         argparse::VarMap *varmap) const {
    auto it = this->argmap_.find(optkey);
    if (it == this->argmap_.end()) {
      throw argparse::exception::ParseError("option not found: " + optkey);
    }

    std::shared_ptr<argparse::Argument> argument = it->second;
    const std::string& dest = argument->get_dest();
    auto vit = varmap->find(dest);

    if (vit != varmap->end() &&
        (argument->get_action() != argparse::Action::append &&
         argument->get_action() != argparse::Action::append_const)) {
          throw argparse::exception::ParseError("duplicated option, " +
                                                optkey);
        }

    auto varvec = new std::vector<Var*>();
    varmap->insert(std::make_pair(dest, varvec));
    idx = argument->parse(args, idx, varvec);
    
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
    std::shared_ptr<argparse::VarMap> ptr =
      std::make_shared<argparse::VarMap>();
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
        std::vector<Var*> *vararr = nullptr;
        auto vit = ptr->find(dest);
        if (vit == ptr->end()) {
          vararr = new std::vector<Var*>();
          ptr->insert(std::make_pair(dest, vararr));
        } else {
          vararr = vit->second;
        }
        assert(vararr != nullptr);
        
        idx = argument->parse(args, idx, vararr);
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
