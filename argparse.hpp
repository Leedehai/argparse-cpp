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
  class Option;
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

  typedef std::tuple<int, std::unique_ptr<argparse_internal::Option> > ParseResult;
  
  class Argument {
  private:
    ArgFormat arg_format_;
    std::string name_;
    std::string name2_;
    std::string nargs_;
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
    
  public:
    Argument(argparse_internal::ArgumentProcessor *proc);
    ~Argument();
    
    // can be called only once and should be called by Parser::AddArgument
    const std::string& set_name(const std::string &v_name);
    ArgFormat arg_format() const { return this->arg_format_; }
    ParseResult parse(std::vector<const std::string> args, size_t idx) const;
    
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
    Values parse_args(const std::vector<std::string> &args) const;
    Values parse_args(int argc, char *argv[]) const;
  };

  
  class Values {
  private:
    std::shared_ptr<argparse_internal::Values> ptr_;
    
  public:
    Values();
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
  // class Option: Option Values
  //
  class Option {
  private:
    bool valid_;
    std::stringstream err_;
    
  protected:
    std::stringstream& err_stream() {
      this->valid_ = false;
      return this->err_;
    }
    
  public:
    Option() : valid_(true) {}
    virtual ~Option() = default;
    virtual const std::string& str() const {
      throw argparse::exception::TypeError("not has a string value");
    }
    virtual int get() const {
      throw argparse::exception::TypeError("not has an integer value");
    }
    virtual bool is_true() const {
      throw argparse::exception::TypeError("not has a boolean value");
    }
    bool is_valid() const { return this->valid_; }
    const std::string err() const {
      return this->err_.str();
    }
    static Option* build_option(const std::string& val, argparse::ArgType type);
  };
  
  class OptionInt : public Option {
  private:
    int value_;
    
  public:
    OptionInt(const std::string& val);
    ~OptionInt() = default;
    int get() const override { return this->value_; }
  };

  class OptionStr : public Option {
  private:
    std::string value_;
    
  public:
    OptionStr(const std::string& value) : value_(value) {}
    ~OptionStr() = default;
    const std::string& str() const override { return this->value_; }
  };

  class OptionBool : public Option {
  private:
    bool value_;
    
  public:
    OptionBool(const std::string& value);
    ~OptionBool() = default;
    bool is_true() const override { return this->value_; }
  };

  // ------------------------------------------------------------------
  // class Values: Actual Instance of class argparse::Value
  //
  class Values {
  private:
    std::map<std::string, std::vector<Option*> > optmap_;
    Option* find_option(const std::string& dest, size_t idx) const;
    
  public:
    Values() = default;
    ~Values();
    
    const std::string& str(const std::string& dest, size_t idx=0) const;
    int get(const std::string& dest, size_t idx=0) const;
    
    size_t size(const std::string& dest) const;
    bool is_true(const std::string& dest) const;
    bool is_set(const std::string& dest) const;
    void insert_option(const std::string &dest, Option *opt);
  };
  
  class ArgumentProcessor {
  private:
    std::map<const std::string, std::shared_ptr<argparse::Argument> > argmap_;
    std::vector<std::shared_ptr<argparse::Argument> > argvec_;
  public:
    ArgumentProcessor() = default;
    ~ArgumentProcessor() = default;
    
    argparse::Argument& add_argument(const std::string &name);
    argparse::Values parse_args(const std::vector<const std::string> &args) const;
    argparse::Values parse_args(int argc, char *argv[]) const;
  };
  
}

#endif   // __ARGPARSE_HPP__
