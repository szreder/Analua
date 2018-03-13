#include <cstring>

#include "Driver.hpp"

Driver::Driver(const std::string &filename)
	: m_parser{*this}, m_scanner{*this}, m_filename{filename}, m_position{&m_filename, 1, 1}
{
}

void Driver::addChunk(Chunk *chunk)
{
	m_chunks.emplace_back(chunk);
}

int Driver::parse()
{
	return m_parser.parse();
}

yy::location Driver::location(const char *s)
{
	yy::position end = m_position + strlen(s);
	auto result = yy::location{m_position, end};
	m_position = end;
	return result;
}

void Driver::nextLine()
{
	m_position.lines(1);
}

void Driver::step()
{
	m_position += 1;
}
