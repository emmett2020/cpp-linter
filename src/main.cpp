#include "utils/shell.h"

#include <iostream>

using namespace linter;

int main() {
  auto [exit_code, std_out, std_err] = Execute("/linter/exe", {});
  std::cout << exit_code << "\n";
  std::cout << std_out;
  std::cout << std_err;

  auto [code1, std_out1, _] = Which("g++");
  std::cout << code1 << "\n";
  std::cout << std_out1;
}
