#include <cstring>

#include "Driver.hpp"

Driver::Driver()
	: m_parser{*this}, m_scanner{*this}, m_filename{"<stdin>"}, m_position{&m_filename, 1, 1}
{
}

void Driver::addChunk(Chunk *chunk)
{
	m_chunks.emplace_back(chunk);
}

std::vector <std::unique_ptr <Chunk> > & Driver::chunks()
{
	return m_chunks;
}

const std::vector <std::unique_ptr <Chunk> > & Driver::chunks() const
{
	return m_chunks;
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

bool Driver::setInputFile(const char *filename)
{
	m_filename = filename;
	m_input.open(m_filename);
	if (m_input.fail()) {
		std::cerr << "Unable to open file for reading: " << m_filename.c_str() << '\n';
		return false;
	}

	m_scanner.switch_streams(&m_input);
	m_position.initialize(&m_filename);
	return true;
}
