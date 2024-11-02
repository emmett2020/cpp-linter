#include <range/v3/all.hpp>

#include "utils/shell.h"
#include "tools/clang_tidy.h"

#include <cctype>
#include <iostream>

using namespace linter;

int main() {
  auto cmd = GetClangToolFullPath("clang-tidy", "20");
  std::cout << cmd.size() << "\n";
  auto c = cmd
         | ranges::views::reverse
         | ranges::views::drop_while(::isspace)
         | ranges::views::reverse
         | ranges::to<std::string>();
  std::cout << c << std::endl;
  RunClangTidy(c, "/linter/build/t.cpp", "", false);
}
