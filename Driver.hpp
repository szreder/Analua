#pragma once

#include <memory>
#include <vector>

#include "AST.hpp"
#include "Scanner.hpp"

class Driver {
	friend class yy::Parser;
public:
	Driver(const std::string &filename = "<stdin>");
	~Driver()
	{
		for (const auto &c : m_chunks)
			c->print();
	}

	void addChunk(Chunk *chunk);
	int parse();

	yy::location location(const char *s);
	void nextLine();
	void step();

private:
	yy::Parser m_parser;
	Scanner m_scanner;

	std::vector <std::unique_ptr <Chunk> > m_chunks;
	std::string m_filename;
	yy::position m_position;
};
