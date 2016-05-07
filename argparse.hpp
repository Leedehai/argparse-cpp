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
    static void handle_count(std::vector<argparse_internal::Var*> *opt_list);
    static std::string extract_opt_name(const std::string& name);
    std::string build_usage(const std::string& arg_name) const;
    
    static const std::map<const std::string, Action> ACTION_MAP_;
    static const std::map<const std::string, ArgType> TYPE_MAP_;
    
  public:
    Argument(argparse_internal::ArgumentProcessor *proc);
    ~Argument();
    
    // can be called only once and should be called by Parser::AddArgument
    const std::string& set_name(const std::string &v_name);
    ArgFormat arg_format() const { return this->arg_format_; }
    size_t parse(const Argv& args, size_t idx,
                 std::vector<argparse_internal::Var*> *opt_list) const;
    
    // can set secondary option name such as first "-s" and second "--sum"
    Argument& name(const std::string& v_name);
    Argument& action(const std::string& action);
    Argument& nargs(const std::string& v_nargs);
    Argument& nargs(size_t v_nargs);
    Argument& set_const(const std::string& v_const);
    Argument& set_default(const std::string& v_default);
    Argument& type(ArgType v_type);
    Argument& type(const std::string& v_type);
    // TODO: implement choices (#1)
    // Argument& choices(const std::string& v_choices);
    Argument& required(bool req);
    Argument& help(const std::string& v_help);
    Argument& metavar(const std::string& v_metavar);
    Argument& dest(const std::string& v_dest);
    
    ArgFormat get_format() const { return this->arg_format_; }
    const std::string& get_name() const { return this->name_; }
    Action get_action() const { return this->action_; }
    const std::string& get_dest() const {
      if (this->dest_.empty()) {
        return (this->name2_.empty() ? this->name_ : this->name2_);
      } else {
        return this->dest_;
      }
    }
    const std::string& get_const() const { return this->const_; }
    const std::string& get_default() const { return this->default_; }
    ArgType get_type() const { return this->type_; }
    bool is_required() const { return this->required_; }
    const std::string& get_help() const { return this->help_; }
    
    void check_consistency() const;
    std::string usage() const;
    std::string usage2() const;
  };
  
  
  class Parser {
  private:
    std::string prog_name_;
    std::string version_;
    argparse_internal::ArgumentProcessor *proc_;
    std::ostream *output_;
    
  public:
    Parser(const std::string &prog_name);
    Parser();
    ~Parser();
    Parser(const Parser& obj) = delete;

    Argument& add_argument(const std::string &name);
    Values parse_args(const Argv& args) const;
    Values parse_args(int argc, char *argv[]) const;
    void usage() const;
    void help() const;
    
    void set_output(std::ostream *output);
  };

  /*
  typedef std::map<const std::string,
                   std::vector<argparse_internal::Var*>*> VarMap;
   */
  // DO NOT Upcast because destructor of std::map is not virtual.
  
  class VarMap : public std::map<const std::string,
  std::vector<argparse_internal::Var*>*> {
  private:
    bool help_;

  public:
    VarMap() : help_(false) {};
    ~VarMap() {}
    void set_help(bool help) { this->help_ = help; }
    bool is_help() const { return this->help_; }
  };
  
  class Values {
  private:
    std::shared_ptr<VarMap> varmap_;
    static const std::vector<argparse_internal::Var*>&
      get_var_arr(const VarMap& varmap, const std::string& key);
    static const argparse_internal::Var& get_var(const VarMap& varmap,
                                                 const std::string& key,
                                                 size_t idx);
    
  public:
    Values(std::shared_ptr<VarMap> varmap);
    Values(const Values& obj);
    ~Values();
    Values& operator=(const Values &obj);
    const std::string& operator[](const std::string& key) const;
    const std::string& get(const std::string& dest, size_t idx=0) const;
    int to_int(const std::string& dest, size_t idx=0) const;
    const std::string& to_str(const std::string& dest, size_t idx=0) const;
    

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
    
    void increment() {
      this->value_++;
      std::stringstream ss;
      ss << this->value_;
      this->str_ = ss.str();
    }
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
  // class ArgumentProcessor
  //
  class ArgumentProcessor {
  private:
    std::map<const std::string, std::shared_ptr<argparse::Argument> > argmap_;
    std::vector<std::unique_ptr<argparse::Argument> > argvec_;
    size_t parse_option(const argparse::Argv& args, size_t idx,
                        const std::string& optkey,
                        argparse::VarMap *varmap) const;
    static void handle_usage_line(const argparse::Argument& arg,
                                  const std::string& tab,
                                  std::stringstream *buf, std::ostream *out);
    static void handle_help_line(const argparse::Argument& arg,
                                 std::ostream *out);

  public:
    ArgumentProcessor() = default;
    ~ArgumentProcessor() = default;
    
    argparse::Argument& add_argument(const std::string &name);
    void insert_option(const std::string& name, argparse::Argument* arg);
    void copy_option(const std::string& src, const std::string& dst);
    void insert_sequence(argparse::Argument *arg);

    argparse::Values parse_args(const argparse::Argv& args) const;
    void usage(const std::string& prog_name, std::ostream *out) const;
    void help(std::ostream *out) const;
  };
  
}

#endif   // __ARGPARSE_HPP__
