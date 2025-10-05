#include <iostream>
#include "utils.h"

void doPrint() { std::cout << "A function\n"; }

int getValueFromUser() {
  std::cout << "Enter an integer: ";
  int input{};
  std::cin >> input;
  return input;
}

void printDouble(int n) {
  std::cout << "Double of your number is " << n * 2 << '\n';
}
