argparse-cpp
============================

Argument parser for CLI in C++. This is inspired by ArgumentParser in Python.

Example
----------------

```cpp
#include <iostream>

#include "./argparse.hpp"

int main(int argc, char *argv[]) {
  // Create Parser with program name.
  argparse::Parser psr("example");

  // can parse "./example -c my_conf.yml" and "./example --config my_conf.yml
  psr.add_argument("-c", "--config").set_default("conf.yml")
    .metavar("FILE").help("Configuration file");

  // can parse "./example -v" and "./example -vvv"
  psr.add_argument("-v", "--verbose").action("count").set_default("0")
    .dest("verb").help("verbose output");

  // can parse "./example -i file1 -i file2"
  psr.add_argument("-i").action("append").dest("input")
    .help("specify input file(s)");

  // can parse "./example -w" to set 'write'
  psr.add_argument("-w").action("store_const").dest("mode")
    .set_const("write").help("set 'write' mode");
  // can parse "./example -r" to set 'read'
  psr.add_argument("-r").action("store_const").dest("mode")
    .set_const("read").help("set 'read' mode");

  // Parse
  argparse::Values val = psr.parse_args(argc, argv);

  // Exit if help was shown.
  if (val.is_help_mode()) {
    return 0;
  }

  // Get a value as std::string, it's same with val.get("config", 0)
  std::cout << "config:  " << val["config"] << std::endl;
  // Get a value as int.
  std::cout << "verbose: " << val.to_int("verb") << std::endl;

  // Output if a value is set.
  std::cout << "   mode: " << (val.is_set("mode") ? val["mode"] : "(none)")
            << std::endl;

  // Output all appended values by iteration.
  for (size_t i = 0; i < val.size("input"); i++) {
    std::cout << " - input: " << val.get("input", i) << std::endl;
  }

  return 0;
}
```


Author
-----------------

Masayoshi Mizutani <mizutani@sfc.wide.ad.jp>

License
-----------------

The BSD 2-Clause License. [see License](LICENSE.md).
