#include <cstdint>
#include <iomanip>
#include <ios>
#include <iostream>

void print(int x) {
 std::cout << "Printing: " << x << '\n';
}

int main() {
 std::cout << std::left; // left justify output
 std::cout << "bool: "<< sizeof(bool) << " bytes\n";
 std::cout << "int: "<< sizeof(int) << " bytes\n";

 int x{};
 std::cout << sizeof(x);

 unsigned int z{ 2 };
 unsigned int y{ 3 };

 std::cout << z - y << '\n'; // prints 4294967295 (incorrect!)


 signed int s { -1 };
 unsigned int u { 1 };

 if (s < u) // -1 is implicitly converted to 4294967295, and 4294967295 < 1 is false
  std::cout << "-1 is less than 1\n";
 else
  std::cout << "1 is less than -1\n"; // this statement executes
 std::int8_t t{65};
 std::cout << t << '\n';


 std::cout << "least 8:  " << sizeof(std::int_least8_t)  * 8 << " bits\n";
 std::cout << "least 16: " << sizeof(std::int_least16_t) * 8 << " bits\n";
 std::cout << "least 32: " << sizeof(std::int_least32_t) * 8 << " bits\n";
 std::cout << '\n';
 std::cout << "fast 8:  "  << sizeof(std::int_fast8_t)   * 8 << " bits\n";
 std::cout << "fast 16: "  << sizeof(std::int_fast16_t)  * 8 << " bits\n";
 std::cout << "fast 32: "  << sizeof(std::int_fast32_t)  * 8 << " bits\n";


 double d{0.1};
 std::cout << d << '\n'; // use default cout precision of 6
 std::cout << std::setprecision(17); std::cout << d << '\n';



 double d1{ 1.0 };
 std::cout << d1 << '\n';

 double d2{ 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 + 0.1 }; // should equal 1.0
 std::cout << d2 << '\n';

 std::cout << true << '\n';
 std::cout << false << '\n';

 std::cout << std::boolalpha;

 std::cout << true << '\n';
 std::cout << false << '\n';

 double example{5.5};
 int converted = static_cast<int>(example);
 print(converted);

 return 0;
}
