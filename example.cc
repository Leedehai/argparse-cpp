#include <iostream>

#include "./argparse.hpp"

int main(int argc, char *argv[]) {
  argparse::Parser psr;
  psr.add_argument("-c").name("--config").set_default("conf.yml")
    .metavar("FILE").help("Configuration file");
  psr.add_argument("-v").name("--verbose").action("store_true")
    .help("verbose output");
  
  argparse::Values val = psr.parse_args(argc, argv);
  std::cout 
    << "config:  " << val["config"] << std::endl 
    << "verbose: " << val.is_true("verbose") << std::endl;

  return 0;
}
