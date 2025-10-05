#include <iostream>
#include "utils.h"

int main() {
  std::cout << "This is part 1\n";
  doPrint();
  std::cout << "This is part 2\n";

  int num{getValueFromUser()};
  printDouble(num);
  return 0;
}
