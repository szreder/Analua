#pragma once

#include <string>

class Preprocessor {
public:
	const std::string & data() const { return m_data; }

	Preprocessor & operator += (const std::string &input);
	Preprocessor & operator += (char input);
	void preprocess();
private:
	std::string m_data;
};
