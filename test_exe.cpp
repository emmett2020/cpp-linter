#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "hello world out 1\n";
  std::cerr << "hello world err 1\n";
  std::cout << "hello world out 2\n";
  std::cerr << "hello world err 2\n";
  std::cout << "hello world out 3\n";
  std::cerr << "hello world err 3\n";
  std::cout << "hello world out 4\n";
  std::cerr << "hello world err 4\n";
  std::cout << "hello world out 5\n";
  std::cerr << "hello world err 5\n";
  auto env = getenv("ADD");
  if (env) {
    std::cout << env;
  }
  return 0;
}
