#include "utils/shell.h"

#include <iostream>

using namespace linter;

int main() {
  auto [exit_code, std_out, std_err] = Exec("/linter/exe", {});
  std::cout << exit_code << "\n";
  std::cout << std_out << "\n";
  std::cout << std_err << "\n";
}
