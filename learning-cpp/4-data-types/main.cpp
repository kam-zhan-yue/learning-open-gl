#include <cstdint>
#include <iostream>
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

    return 0;
}
