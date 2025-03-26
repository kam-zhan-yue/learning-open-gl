#include <iostream>
int main() {

  int number{};
  std::cout << "Enter an integer: ";
  std::cin >> number;
  std::cout << "\nDouble that number is: " << number * 2;

  int age{23};

  std::cout << "\nHello!" << std::endl;
  std::cout << "my name is Alex and I am " << age << " years old!\n";

  std::cout << "Enter a number: ";
  int x{};
  std::cin >> x;
  std::cout << "\nYou entered " << x << '\n';

  int y{};
  int z{};
  std::cout << "Enter two numbers separated by a space\n";
  std::cin >> y >> z;
  std::cout << "You entered " << y << " and " << z << '\n';
}
