#include "Preprocessor.hpp"

void Preprocessor::preprocess()
{
	std::string result;
	char stringChar = 0;

	auto p = m_data.begin();
	while (p != m_data.end()) {
		if (stringChar != 0) {
			if (*p == '\\') {
				result.push_back(*p++);
				if (p != m_data.end())
					result.push_back(*p++);
				continue;
			}

			result.push_back(*p);
			if (*p == stringChar)
				stringChar = 0;
			++p;
		}
		else if (*p == '-' && !result.empty() && result.back() == '-') {
			result.pop_back();
			++p;
			result.append(2, ' ');
			if (*p == '[' && p != m_data.end() && *(p + 1) == '[') {
				//long comment
				p += 2;
				result.append(2, ' ');
				bool end = false;
				while (!end) {
					while (p != m_data.end() && *p != ']') {
						if (*p == '\n')
							result.push_back('\n');
						else
							result.push_back(' ');
						++p;
					}

					if (p != m_data.end()) {
						result.push_back(' ');
						++p;
						if (p != m_data.end() && *p == ']') {
							++p;
							result.push_back(' ');
						}
						end = true;
					}
				}
			} else {
				//short comment
				while (p != m_data.end() && *p++ != '\n')
					result.push_back(' ');
				result.push_back('\n');
			}
		} else {
			if (*p == '\'' || *p == '"')
				stringChar = *p;
			result += *p++;
		}
	}

	m_data = result;
}

Preprocessor & Preprocessor::operator += (const std::string &input)
{
	m_data += input;
	return *this;
}

Preprocessor & Preprocessor::operator += (char input)
{
	m_data += input;
	return *this;
}
