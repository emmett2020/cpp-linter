#include "utils/shell.h"
#include "tools/clang_tidy.h"

#include <iostream>

using namespace linter;

int main() {
  std::cout << GetClangToolFullPath("clang-tidy", "20");
}
