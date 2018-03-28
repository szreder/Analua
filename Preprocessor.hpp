#pragma once

#include <string>

#include "position.hh"

class Preprocessor {
public:
	const std::string & data() const { return m_data; }

	Preprocessor & operator += (const std::string &input);
	Preprocessor & operator += (char input);
	bool preprocess();
private:
	std::string m_data;
	yy::position m_position;
};
