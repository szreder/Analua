#include <iostream>

#include "Preprocessor.hpp"

namespace {

namespace Error {
	static const std::string LongComment = "Error parsing long comment";

	static const std::string EofReached = "EOF reached";
	static const std::string UnexpectedSymbol = "Unexpected symbol";
}

struct ErrorStream {
	ErrorStream(std::ostream &os = std::cerr) : m_os{os} {}
	~ErrorStream() { m_os << '\n'; }

	template <typename T>
	inline ErrorStream & operator << (const T &arg)
	{
		m_os << arg;
		return *this;
	}

	std::ostream &m_os;
};


inline ErrorStream error(const std::string &where, const std::string &why)
{
	ErrorStream es{std::cerr};
	es << where << ": " << why;
	return es;
}

}

bool Preprocessor::preprocess()
{
	std::string result;
	char stringChar = 0;

	auto p = m_data.cbegin();

	auto step = [this, &p]
	{
		m_position += 1;
		return *p++;
	};

	auto nextLine = [this, &p]
	{
		m_position.lines(1);
		++p;
	};

	while (p != m_data.cend()) {
		if (stringChar != 0) {
			if (*p == '\\') {
				result.push_back(step());
				if (p != m_data.cend())
					result.push_back(step());
				continue;
			}

			result.push_back(*p);
			if (*p == stringChar)
				stringChar = 0;
			step();
		} else if (*p == '-' && !result.empty() && result.back() == '-') {
			result.pop_back();
			result.append(2, ' ');
			step();

			if (*p == '[' && p != m_data.cend() && (*(p + 1) == '[' || *(p + 1) == '=')) {
				const yy::position commentStart = m_position - 2;
				//long comment
				std::string endMarker = "]";
				auto tmp = p + 1;
				while (tmp != m_data.cend() && *tmp == '=')
					++tmp;
				if (tmp == m_data.cend()) {
					error(Error::LongComment, Error::EofReached) << ", started at: " << commentStart;
					return false;
				}

				if (*tmp != '[') {
					error(Error::LongComment, Error::UnexpectedSymbol) << " (" << *tmp << "), started at: " << commentStart;;
					return false;
				}

				endMarker.append(tmp - p - 1, '=');
				endMarker.push_back(']');
				result.append(endMarker.size(), ' ');

				p = tmp + 1;
				const auto endMarkerIdx = m_data.find(endMarker, p - m_data.cbegin());
				if (endMarkerIdx == std::string::npos) {
					error(Error::LongComment, Error::EofReached) << ", started at: " << commentStart;
					return false;
				}
				const auto endMarkerIter = m_data.cbegin() + endMarkerIdx + endMarker.size();
				while (p != endMarkerIter) {
					if (*p == '\n') {
						result.push_back('\n');
						nextLine();
					} else {
						result.push_back(' ');
						step();
					}
				}
			} else {
				//short comment
				while (p != m_data.end() && step() != '\n')
					result.push_back(' ');
				result.push_back('\n');
				m_position.lines(1);
			}
		} else {
			if (*p == '\'' || *p == '"')
				stringChar = *p;
			result += step();
		}
	}

	m_data = result;
	return true;
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
