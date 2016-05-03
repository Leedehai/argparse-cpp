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
#include <map>
#include <string>
#include <exception>
#include <sstream>

namespace argparse_internal {
  class Values;
  class Var;
  class ArgumentProcessor;
}

namespace argparse {
  enum class Action {
    store,
    store_const,
    store_true,
    store_false,
    append,
    append_const,
    count,
    help,
    version,
  };
  
  
  class Parser;
  class Values;

  namespace exception {
    class Exception : public std::exception {
    private:
      std::stringstream err_;
    protected:
      std::stringstream& err() { return this->err_; }
    public:
      Exception() {}

      virtual const char* what() const throw() {
        return this->err_.str().c_str();
      }
    };

    class ConfigureError : public Exception {
    public:
      ConfigureError(const std::string &errmsg, const std::string &tgt);
    };

    class ParseError : public Exception {
    public:
      ParseError(const std::string &errmsg);
    };
    
    class KeyError : public Exception {
    public:
      KeyError(const std::string& key, const std::string &errmsg);
    };
    
    class TypeError : public Exception {
    public:
      TypeError(const std::string &errmsg);
    };
    
    class IndexError : public Exception {
    public:
      IndexError(const std::string &errmsg);
    };
  }
  
  enum class ArgFormat {
    undef,
    option,
    sequence,
  };
  
  enum class ArgType {
    STR,
    INT,
    BOOL,
  };
  
  enum class Nargs {
    NUMBER,
    ASTERISK,
    QUESTION,
    PLUS,
  };

  typedef std::vector<const std::string> Argv;
  
  class Argument {
  private:
    ArgFormat arg_format_;
    std::string name_;
    std::string name2_;
    Nargs nargs_;
    size_t nargs_num_;
    std::string const_;
    std::string default_;
    ArgType type_;
    std::string choices_;
    bool required_;
    std::string help_;
    std::string metavar_;
    std::string dest_;
    Action action_;
    argparse_internal::ArgumentProcessor *proc_;
    
    size_t parse_append(const Argv& args, size_t idx,
                        std::vector<argparse_internal::Var*> *opt_list) const;
    
  public:
    Argument(argparse_internal::ArgumentProcessor *proc);
    ~Argument();
    
    // can be called only once and should be called by Parser::AddArgument
    const std::string& set_name(const std::string &v_name);
    ArgFormat arg_format() const { return this->arg_format_; }
    size_t parse(const Argv& args, size_t idx,
                 std::vector<argparse_internal::Var*> *opt_list) const;
    
    // can set secondary option name such as first "-s" and second "--sum"
    Argument& name(const std::string &v_name);
    Argument& action(Action action);
    Argument& nargs(const std::string &v_nargs);
    Argument& nargs(size_t v_nargs);
    Argument& set_const(const std::string &v_const);
    Argument& set_default(const std::string &v_default);
    Argument& type(ArgType v_type);
    Argument& choices(const std::string &v_choices);
    Argument& required(bool req);
    Argument& help(const std::string &v_help);
    Argument& metavar(const std::string &v_metavar);
    Argument& dest(const std::string &v_dest);
    
    const std::string& get_name() const { return this->name_; }
    Action get_action() const { return this->action_; }
    const std::string& get_dest() const {
      return (this->dest_.empty() ? this->name_ : this->dest_);
    }
  };
  
  
  class Parser {
  private:
    std::string prog_name_;
    std::string version_;
    argparse_internal::ArgumentProcessor *proc_;
    
  public:
    Parser(const std::string &prog_name);
    ~Parser();
    Parser(const Parser& obj) = delete;

    Parser& usage(const std::string& s);
    Parser& version(const std::string& version);
    Argument& add_argument(const std::string &name);
    Values parse_args(const Argv& args) const;
    Values parse_args(int argc, char *argv[]) const;
  };

  
  class Values {
  private:
    std::shared_ptr<argparse_internal::Values> ptr_;
    
  public:
    Values(std::shared_ptr<argparse_internal::Values> ptr);
    Values(const Values& obj);
    ~Values();
    Values& operator=(const Values &obj);
    const std::string& str(const std::string& dest, size_t idx=0) const;
    int get(const std::string& dest, size_t idx=0) const;
    const std::string& to_s(const std::string& dest, size_t idx=0) const;
    int to_i(const std::string& dest, size_t idx=0) const;

    size_t size(const std::string& dest) const;
    bool is_true(const std::string& dest) const;
    bool is_set(const std::string& dest) const;
  };
}



// ==================================================================
// Internal Classes
//
namespace argparse_internal {

  // ------------------------------------------------------------------
  // class Var: Var Values
  //
  class Var {
  private:
    bool valid_;
    std::stringstream err_;
    
  protected:
    std::stringstream& err_stream() {
      this->valid_ = false;
      return this->err_;
    }
    
  public:
    Var() : valid_(true) {}
    virtual ~Var() = default;
    virtual const std::string& to_s() const {
      throw argparse::exception::TypeError("not has a string value");
    }
    virtual int to_i() const {
      throw argparse::exception::TypeError("not has an integer value");
    }
    virtual bool is_true() const {
      throw argparse::exception::TypeError("not has a boolean value");
    }
    virtual bool is_null() const {
      return false;
    }
    
    bool is_valid() const { return this->valid_; }
    const std::string err() const {
      return this->err_.str();
    }
    static Var* build_var(const std::string& val, argparse::ArgType type);
  };
  
  class VarInt : public Var {
  private:
    int value_;
    std::string str_;
    
  public:
    VarInt(const std::string& val);
    ~VarInt() = default;
    const std::string& to_s() const override { return this->str_; }
    int to_i() const override { return this->value_; }
  };

  class VarStr : public Var {
  private:
    std::string value_;
    
  public:
    VarStr(const std::string& value) : value_(value) {}
    ~VarStr() = default;
    const std::string& to_s() const override { return this->value_; }
  };

  class VarBool : public Var {
  private:
    bool value_;
    static const std::string true_;
    static const std::string false_;
    
  public:
    VarBool(const std::string& value);
    ~VarBool() = default;
    const std::string& to_s() const override {
      return (this->value_ ? VarBool::true_ : VarBool::false_);
    }
    bool is_true() const override { return this->value_; }
  };
  
  class VarNull : public Var {
  public:
    VarNull() = default;
    ~VarNull() = default;
    bool is_null() const override { return true; }
  };

  // ------------------------------------------------------------------
  // class Values: Actual Instance of class argparse::Value
  //
  class Values {
  private:
    std::map<std::string, std::vector<Var*>* > optmap_;
    Var* find_Var(const std::string& dest, size_t idx) const;
    
  public:
    Values() = default;
    ~Values();
    
    const std::string& str(const std::string& dest, size_t idx=0) const;
    int get(const std::string& dest, size_t idx=0) const;
    
    size_t size(const std::string& dest) const;
    bool is_true(const std::string& dest) const;
    bool is_set(const std::string& dest) const;
    
    std::vector<Var*>* get_optmap(const std::string& dest);
  };
  
  class ArgumentProcessor {
  private:
    std::map<const std::string, std::shared_ptr<argparse::Argument> > argmap_;
    std::vector<std::shared_ptr<argparse::Argument> > argvec_;
    size_t parse_option(const argparse::Argv& args, size_t idx,
                        const std::string& optkey, Values *vals) const;
  public:
    ArgumentProcessor() = default;
    ~ArgumentProcessor() = default;
    
    argparse::Argument& add_argument(const std::string &name);
    argparse::Values parse_args(const argparse::Argv& args) const;
    argparse::Values parse_args(int argc, char *argv[]) const;
  };
  
}

#endif   // __ARGPARSE_HPP__
