Schrödinger's pun - it's both funny and unfunny at the same time.

In the current state the code needs to be stripped from comments,
as they confuse the parser. Preprocessing is handled by a separate
program (`preprocessor.cpp`) by replacing comments with spaces.
This is done to preserve the original location information without
introducing wacky adnotations in the resulting files.

# TODO
- Long strings.
- Integrate preprocessor into the parser.
- Recursively reading from load()ed files.
- Better location info (e.g. filenames when load()ing other src files).
- Store location info in AST.

# Not implemented
- Nested long strings.
