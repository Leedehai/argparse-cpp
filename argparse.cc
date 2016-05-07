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

#include <iostream>
#include <iomanip>
#include <set>

#include <assert.h>
#include "argparse.hpp"


namespace argparse {

  // ========================================================
  // argparse::exception::*
  //
  exception::ConfigureError::ConfigureError(const std::string& errmsg,
                                            const std::string& tgt) {
    std::stringstream ss;
    ss << "ConfigureError: " << errmsg << ", '" << tgt << "'";
    this->err_ = ss.str();
  }
  
  exception::ParseError::ParseError(const std::string &errmsg) {
    std::stringstream ss;
    ss << "ParseError: " << errmsg;
    this->err_ = ss.str();
  }
  
  exception::KeyError::KeyError(const std::string& key,
                                const std::string &errmsg) {
    std::stringstream ss;
    ss << "KeyError '" << key << "': " << errmsg;
    this->err_ = ss.str();
  }
  
  exception::TypeError::TypeError(const std::string &errmsg) {
    std::stringstream ss;
    ss << "TypeError: " << errmsg;
    this->err_ = ss.str();
  }

  exception::IndexError::IndexError(const std::string &errmsg) {
    std::stringstream ss;
    ss << "IndexError: " << errmsg;
    this->err_ = ss.str();
  }


  
  // ========================================================
  // argparse::Argument
  //
  const std::map<const std::string, Action> Argument::ACTION_MAP_ = {
    {"store",        Action::store},
    {"store_const",  Action::store_const},
    {"store_true",   Action::store_true},
    {"store_false",  Action::store_false},
    {"append",       Action::append},
    {"append_const", Action::append_const},
    {"count",        Action::count},
    {"help",         Action::help},
  };
  
  const std::map<const std::string, ArgType> Argument::TYPE_MAP_ = {
    {"str",   ArgType::STR},
    {"int",   ArgType::INT},
    {"bool",  ArgType::BOOL},
  };

  
  Argument::Argument(argparse_internal::ArgumentProcessor *proc)
  : arg_format_(ArgFormat::undef),
    nargs_(Nargs::NUMBER),
    nargs_num_(1),
    type_(ArgType::STR),
    required_(false),
    action_(Action::store),
    proc_(proc) {
  }
  Argument::~Argument() {
  }
  
  std::string Argument::extract_opt_name(const std::string& name) {
    std::string res;
    
    if (name.substr(0, 3) == "---") {
      throw exception::ConfigureError("too long hyphen. Supporting only 1 or 2",
                                      name);
    } else if (name.substr(0, 2) == "--") {
      res = name.substr(2);
      if (res.length() <= 1) {
        throw exception::ConfigureError("option name must be 2 letters and up "
                                        "for --", name);
      }
    } else if (name.substr(0, 1) == "-") {
      res = name.substr(1);
      if (res.length() != 1) {
        throw exception::ConfigureError("option name must be 1 letter for -",
                                        name);
      }
    }

    return res;
  }

  const std::string& Argument::set_name(const std::string &v_name) {
    if (!this->name_.empty()) {
      throw argparse::exception::ConfigureError("can not redefine name", v_name);
    }
    
    const std::string opt_name = Argument::extract_opt_name(v_name);
   
    if (opt_name.empty()) {
      // this is not an option, sequence
      this->name_ = v_name;
      this->arg_format_ = ArgFormat::sequence;
    } else {
      this->name_ = opt_name;
      this->arg_format_ = ArgFormat::option;
    }
    
    switch (this->arg_format_) {
      case argparse::ArgFormat::option:
        this->proc_->insert_option(this->name_, this);
        break;
        
      case argparse::ArgFormat::sequence:
        this->proc_->insert_sequence(this);
        break;
        
      case argparse::ArgFormat::undef:
        assert(0);
        break;
    }
    
    return this->name_;
  }
  
  size_t Argument::parse_append(const Argv& args, size_t idx,
                                std::vector<argparse_internal::Var*>* opt_list)
  const {
    std::vector<argparse_internal::Var*> vars;
  
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
      vars.emplace_back(argparse_internal::Var::build_var(args[i], this->type_));
      i++;
    }
    
    assert(i >= idx);
    size_t argc = i - idx;
    assert(argc == vars.size());
    
    std::stringstream err;
    
    if (this->nargs_num_ > 1 && argc != this->nargs_num_) {
      assert(this->nargs_ == Nargs::NUMBER);
      err << "option '" << this->name_ << "' must have " << this->nargs_num_
          << "arguments";
    } else if (argc == 0) { // If no argument,
      std::string value;
      
      if (value.empty()) {
        // No arguments and default values
        if (this->nargs_ == Nargs::PLUS) {
          err << "option '" << this->name_ << "' must have 1 or more arguments";
        } else if (this->nargs_ == Nargs::NUMBER) {
          assert(this->nargs_num_ == 1);
          err << "option '" << this->name_ << "' must have 1 arguments";
        } else if (this->nargs_ == Nargs::QUESTION) {
          if (this->const_.empty()) {
            vars.emplace_back(new argparse_internal::VarNull());
          } else {
            vars.emplace_back(argparse_internal::Var::build_var(this->const_,
                                                                this->type_));
          }
        }
      } else {
        vars.emplace_back(argparse_internal::Var::build_var(value,
                                                            this->type_));
      }
    }
    
    // If error, delete all Option instances and throw exception.
    if (! err.str().empty()) {
      for (auto opt_ptr : vars) {
        delete opt_ptr;
      }
      throw exception::ParseError(err.str());
    }
    
    // Move option pointers to opt_list.
    for (auto opt_ptr : vars) {
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
        handle_count(opt_list);
        break;
        
      case Action::help:
        // skip
        break;
    }

    if (opt) {
      opt_list->push_back(opt);
    }
    
    return r_idx;
  }
  
  void Argument::handle_count(std::vector<argparse_internal::Var*> *opt_list) {
    argparse_internal::Var* var = nullptr;
    if (opt_list->size() == 0) {
      var = argparse_internal::Var::build_var("0", ArgType::INT);
      opt_list->push_back(var);
    } else {
      var = (*opt_list)[0];
    }
    assert(var != nullptr);
    assert(opt_list->size() == 1);
 

    argparse_internal::VarInt* varint =
      dynamic_cast<argparse_internal::VarInt*>(var);
    varint->increment();
  }

  Argument& Argument::action(const std::string& action) {
    auto ait = Argument::ACTION_MAP_.find(action);
    if (ait == Argument::ACTION_MAP_.end()) {
      throw exception::ConfigureError(action + " is not matched with keywords",
                                      this->name_);
    }
    this->action_ = ait->second;
    
    if (this->action_ == Action::count) {
      this->type_ = ArgType::INT;
    }
    return *this;
  }

  Argument& Argument::name(const std::string &v_name) {
    if (this->arg_format_ != ArgFormat::option) {
      throw exception::ConfigureError("second name is allowed for only option, "
                                      "not sequence", this->name_);
    }
    const std::string opt_name = Argument::extract_opt_name(v_name);
    
    if (opt_name.empty()) {
      throw exception::ConfigureError("second name must be option format, "
                                      "e.g. -a", v_name);
    }
    
    this->name2_ = opt_name;
    this->proc_->copy_option(this->name_, opt_name);
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
      throw exception::ConfigureError("Invalid argument: " + v_nargs,
                                      this->name_);
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
  
  Argument& Argument::type(const std::string& v_type) {
    auto it = Argument::TYPE_MAP_.find(v_type);
    if (it == Argument::TYPE_MAP_.end()) {
      throw exception::ConfigureError("invalid keyword: " + v_type,
                                      this->name_);
    }
    
    this->type(it->second);
    return *this;
  }


  Argument& Argument::required(bool req) {
    this->required_ = req;
    return *this;
  }

  Argument& Argument::help(const std::string &v_help) {
    this->help_ = v_help;
    return *this;
  }

  Argument& Argument::metavar(const std::string &v_metavar) {
    this->metavar_ = v_metavar;
    return *this;
  }

  Argument& Argument::dest(const std::string &v_dest) {
    this->dest_ = v_dest;
    return *this;
  }

  void Argument::check_consistency() const {
    // Python's argparse call add_argument with all parameter, but argpares-cpp
    // does not call with all parameter by design. Then consistency chcker is
    // required.
    
    if ((this->action_ == Action::store_const ||
         this->action_ == Action::append_const)) {
      // Checking if const parameter is set for store_const or append_const.
      if (this->const_.empty()) {
        std::string msg = "store_const and append_const are "
                          "required 'const' parameter";
        throw argparse::exception::ConfigureError(msg, this->name_);
      }
      
      // Checking if nargs is set for store_const or append_const.
      // nargs should not be modified.
      if (this->nargs_ != Nargs::NUMBER || this->nargs_num_ != 1) {
        std::string msg = "store_const and append_const support only 1 argument ";
        throw argparse::exception::ConfigureError(msg, this->name_);
      }
    }
    
    if (this->action_ == Action::count) {
      if (this->type_ != ArgType::INT) {
        throw argparse::exception::ConfigureError("action 'count' must have "
                                                  "'int' type", this->name_);
      }
    }

    if ((this->action_ == Action::store_true ||
         this->action_ == Action::store_false)) {
      if (! this->const_.empty()) {
        std::string msg = "store_true and store_false do not support 'const'";
        throw argparse::exception::ConfigureError(msg, this->name_);
      }

      if (this->nargs_ != Nargs::NUMBER || this->nargs_num_ != 1) {
        std::string msg = "store_true and store_false support only 1 argument";
        throw argparse::exception::ConfigureError(msg, this->name_);
      }
    }
  }

  std::string Argument::build_usage(const std::string& arg_name) const {
    std::string usage;
    if (this->arg_format_ == ArgFormat::sequence) {
      if (! this->metavar_.empty()) {
        usage = this->metavar_;
      } else if (! this->name2_.empty()) {
        usage = this->name2_;
      } else {
        usage = this->name_;
      }
    }

    std::stringstream ss;
    std::string meta;
    
    if (this->arg_format_ == ArgFormat::option) {
      ss << ((arg_name.length() > 1) ? "--" : "-") << arg_name;
      meta = ((this->metavar_.empty()) ? "VAL" : this->metavar_);
    } else {
      meta = ((this->metavar_.empty()) ? usage : this->metavar_);
    }
    
    if (this->action_ == Action::store || this->action_ == Action::append) {
      if (ss.str().length() > 0) {
        ss << " ";
      }
      
      switch (this->nargs_) {
        case Nargs::ASTERISK:
          ss << "[" << meta << " [" << meta << " ...]]";
          break;
          
        case Nargs::QUESTION:
          ss << "[" << meta << "]";
          break;
          
        case Nargs::PLUS:
          ss << meta << " [" << meta << " ...]";
          break;
          
        case Nargs::NUMBER:
          if (this->nargs_num_ > 1) {
            for (size_t i = 0; i < this->nargs_num_; i++) {
              ss << meta << (i + 1);
              if (i < this->nargs_num_ - 1) {
                ss << " ";
              }
            }
          } else {
            ss << meta;
          }
          break;
      }
    }
    
    return ss.str();
  }

  std::string Argument::usage() const {
    return this->build_usage(this->name_);
  }

  std::string Argument::usage2() const {
    std::string ret;
    if (! this->name2_.empty()) {
      ret = this->build_usage(this->name2_);
    }
    
    return ret;
  }
  
  // ========================================================
  // argparse::Parser
  //
  Parser::Parser(const std::string &prog_name)
  : prog_name_(prog_name), proc_(new argparse_internal::ArgumentProcessor()),
    output_(&std::cout) {
    this->add_argument("-h").name("--help").action("help").help("display help");
  }
  Parser::Parser()
  : prog_name_("(none)"), proc_(new argparse_internal::ArgumentProcessor()),
    output_(&std::cout){
  }
  Parser::~Parser() {
    delete this->proc_;
  }
  
  Argument& Parser::add_argument(const std::string &name) {
    return this->proc_->add_argument(name);
  }

  Values Parser::parse_args(const Argv& args) const {
    Values val = this->proc_->parse_args(args);
    if (val.is_help_mode()) {
      this->help();
    }
    return val;
  }

  Values Parser::parse_args(int argc, char *argv[]) const {
    Argv args;
    for (int i = 0; i < argc; i++) {
      args.emplace_back(argv[i]);
    }
    
    return this->parse_args(args);
  }
  
  void Parser::usage() const {
    this->proc_->usage(this->prog_name_, this->output_);
  }
  
  void Parser::help() const {
    this->usage();
    this->proc_->help(this->output_);
  }
  
  void Parser::set_output(std::ostream *output) {
    this->output_ = output;
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
  
  const std::string& Values::operator[](const std::string& key) const {
    return this->to_str(key, 0);
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
    try {
      auto arr = get_var_arr(*(this->varmap_.get()), key);
      return arr.size();
    } catch (exception::KeyError &e) {
      return 0;
    }
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

  bool Values::is_help_mode() const {
    return this->varmap_->is_help_mode();
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
    if (val == VarBool::true_) {
      this->value_ = true;
    } else if (val == VarBool::false_) {
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
    
    // No parsing option if help
    if (argument->get_action() == argparse::Action::help) {
      varmap->set_help_mode(true);
      return idx;
    }
    
    
    const std::string& dest = argument->get_dest();
    auto vit = varmap->find(dest);

    std::vector<Var*> *vars = nullptr;
    
    if (vit != varmap->end()) {
      if (argument->get_action() != argparse::Action::append &&
          argument->get_action() != argparse::Action::append_const &&
          argument->get_action() != argparse::Action::count) {
          throw argparse::exception::ParseError("duplicated option, " + optkey);
      }
      
      vars = vit->second;
    } else {
      vars = new std::vector<Var*>();
      varmap->insert(std::make_pair(dest, vars));
    }
    
    idx = argument->parse(args, idx, vars);
    
    return idx;
  }

  argparse::Argument& ArgumentProcessor::add_argument(const std::string &name) {
    auto arg = new argparse::Argument(this);
    arg->set_name(name);
    return *arg;
  }
  
  void ArgumentProcessor::insert_option(const std::string &name,
                                        argparse::Argument *arg) {
    if (this->argmap_.find(name) != this->argmap_.end()) {
      throw argparse::exception::ConfigureError("duplicated option name", name);
    }
    
    std::shared_ptr<argparse::Argument> ptr(arg);
    this->argmap_.insert(std::make_pair(name, ptr));
  }

  void ArgumentProcessor::copy_option(const std::string& src,
                                      const std::string& dst) {
    auto it = this->argmap_.find(src);
    if (it == this->argmap_.end()) {
      throw argparse::exception::ConfigureError("can not copy option from " +
                                                src, dst);
    }

    if (this->argmap_.find(dst) != this->argmap_.end()) {
      throw argparse::exception::ConfigureError("duplicated option name", dst);
    }

    this->argmap_.insert(std::make_pair(dst, it->second));
  }
  
  void ArgumentProcessor::insert_sequence(argparse::Argument *arg) {
    // std::unique_ptr<argparse::Argument> ptr(arg);
    this->argvec_.emplace_back(arg);
  }
  
  argparse::Values ArgumentProcessor::parse_args(const argparse::Argv& args)
  const {
    // Checking consistency of Argument instances.
    for (auto it : this->argmap_) {
      (it.second)->check_consistency();
    }
    for (size_t i = 0; i < this->argvec_.size(); i++) {
      this->argvec_[i]->check_consistency();
    }
    
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
        idx = this->parse_option(args, idx + 1, key, ptr.get());
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
        
        const std::string& dest = this->argvec_[seq_idx]->get_dest();
        std::vector<Var*> *vararr = nullptr;
        auto vit = ptr->find(dest);
        if (vit == ptr->end()) {
          vararr = new std::vector<Var*>();
          ptr->insert(std::make_pair(dest, vararr));
        } else {
          vararr = vit->second;
        }
        assert(vararr != nullptr);
        
        idx = this->argvec_[seq_idx]->parse(args, idx, vararr);
        seq_idx++;
      }
    }

    // Setting default value if missing option.
    for (auto it : this->argmap_) {
      auto& arg = (it.second);
      const std::string& dest = arg->get_dest();
      
      if (ptr->find(dest) == ptr->end()) {
        // Value is not set.
        
        if (arg->get_action() == argparse::Action::append ||
            arg->get_action() == argparse::Action::store ||
            arg->get_action() == argparse::Action::store_true ||
            arg->get_action() == argparse::Action::store_false) {
          // 'append' and 'store' can use default value,
          const std::string& v_default = arg->get_default();
      
          if (!v_default.empty()) {
            // The option has default and was not specified in argument.
            auto vars = new std::vector<Var*>();
            vars->emplace_back(Var::build_var(v_default, arg->get_type()));
            ptr->insert(std::make_pair(dest, vars));
          }
        }
      }
      
      // If still values are not set.
      if (ptr->find(dest) == ptr->end()) {
        if (arg->get_action() == argparse::Action::store_true) {
          // put false because of no store_true argument.
          auto vars = new std::vector<Var*>();
          vars->emplace_back(Var::build_var("false", argparse::ArgType::BOOL));
          ptr->insert(std::make_pair(dest, vars));
        }
        
        if (arg->get_action() == argparse::Action::store_false) {
          // put true because of no store_false argument.
          auto vars = new std::vector<Var*>();
          vars->emplace_back(Var::build_var("true", argparse::ArgType::BOOL));
          ptr->insert(std::make_pair(dest, vars));
        }
      }
      
      // Checking required options.
      // This check should be done after setting default values.
      for (auto it : this->argmap_) {
        auto& arg = (it.second);
        auto& dest = arg->get_dest();
        if (ptr->find(dest) == ptr->end() && arg->is_required()) {
          std::stringstream ss;
          ss << "option '" << arg->get_name() << "' is required";
          throw argparse::exception::ParseError(ss.str());
        }
      }
      
    }
    
    argparse::Values vals(ptr);
    return vals;
  }

  void ArgumentProcessor::handle_usage_line(const argparse::Argument& arg,
                                            const std::string& tab,
                                            std::stringstream *buf,
                                            std::ostream *out) {
    std::string usage = arg.usage();
    if (buf->str().length() + usage.length() + 1 > 80) {
      *out << buf->str() << std::endl;
      buf->str(tab);
    }
    
    if (arg.is_required() ||
        arg.get_format() == argparse::ArgFormat::sequence) {
      (*buf) << " " << usage;
    } else {
      (*buf) << " [" << usage << "]";
    }
  }
  
  void ArgumentProcessor::handle_help_line(const argparse::Argument &arg,
                                           std::ostream *out) {
    std::stringstream ss;
    const size_t width = 80;
    const std::string tab("                        ");
    const std::string help = arg.get_help();
    
    if (arg.get_format() == argparse::ArgFormat::sequence) {
      ss << arg.get_name();
    } else if (arg.get_format() == argparse::ArgFormat::option){
      const std::string usage2 = arg.usage2();
      ss << arg.usage() << (! usage2.empty() ? ", " + usage2 : "");
    }
    
    *out << "  " << std::setw(tab.length() - 2) << std::left << ss.str();

    if (ss.str().length() > tab.length()) {
      *out << std::endl;
      if (help.length() < width - tab.length()) {
        *out << tab << help << std::endl;
      } else {
        *out << std::setw(width) << std::right << help << std::endl;
      }
    } else {
      if (help.length() > width - tab.length()) {
        *out << std::endl << std::setw(width) << std::right << help << std::endl;
      } else {
        *out << help << std::endl;
      }
    }
  }
  
  void ArgumentProcessor::usage(const std::string& prog_name,
                                std::ostream *out) const {
    std::stringstream ss, tab;
    std::set<std::string> done_args;
    
    ss << "usage: " << prog_name ;
    for (size_t i = 0; i < ss.str().length() + 1; i++) {
      tab << " ";
    }
    
    for (auto it : this->argmap_) {
      const std::string& name = it.second->get_name();
      if (done_args.find(name) == done_args.end()) {
        done_args.insert(name);
        handle_usage_line(*(it.second), tab.str(), &ss, out);
      }
    }

    for (size_t n = 0; n < this->argvec_.size(); n++) {
      const std::string& name = this->argvec_[n]->get_name();
      if (done_args.find(name) == done_args.end()) {
        done_args.insert(name);
        handle_usage_line(*(this->argvec_[n]), tab.str(), &ss, out);
      }
    }
    
    *out << ss.str() << std::endl;
  }
  
  void ArgumentProcessor::help(std::ostream *out) const {
    std::set<std::string> done_args;

    *out << std::endl << "positional arguments:" << std::endl;
    for (size_t n = 0; n < this->argvec_.size(); n++) {
      const std::string& name = this->argvec_[n]->get_name();
      if (done_args.find(name) == done_args.end()) {
        done_args.insert(name);
        handle_help_line(*(this->argvec_[n]), out);
      }
    }

    *out << std::endl << "optional arguments:" << std::endl;
    for (auto it : this->argmap_) {
      const std::string& name = it.second->get_name();
      if (done_args.find(name) == done_args.end()) {
        done_args.insert(name);
        handle_help_line(*(it.second), out);
      }
    }
  }

}
