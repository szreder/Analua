#include <iostream>

#include "Preprocessor.hpp"

int main()
{
	std::ios_base::sync_with_stdio(false);
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
