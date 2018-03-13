#include <iostream>

#include "Preprocessor.hpp"

int main()
{
	Preprocessor pp;

	std::string line;
	while (std::getline(std::cin, line)) {
		pp += line;
		pp += '\n';
	}

	pp.preprocess();
	std::cout << pp.data();

	return 0;
}
