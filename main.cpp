#include <fstream>
#include <iostream>

#include "Driver.hpp"

int main(int argc, char **argv)
{
	Driver d;

	if (argc > 1) {
		if (!d.setInputFile(argv[1]))
			return 1;
	}

	d.parse();
	return 0;
}
