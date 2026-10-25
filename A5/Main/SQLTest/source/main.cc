
#include "Lexer.h"
#include "Parser.h"
#include "ParserTypes.h"
#include "MyDB_Catalog.h"
#include "MyDB_Table.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <unistd.h>

using namespace std;

string toLower (string data) {
	transform (data.begin (), data.end (), data.begin (), ::tolower);
	return data;
}

// strips leading and trailing whitespace
string trim (string s) {
	size_t start = s.find_first_not_of (" \t\r\n");
	if (start == string :: npos)
		return "";
	size_t end = s.find_last_not_of (" \t\r\n");
	return s.substr (start, end - start + 1);
}

// Reads SQL statements from standard input, one after another, until end of
// file.  A statement ends at the first semicolon.  Whole-line comments (lines
// whose first non-blank characters are "--") are ignored.  For every statement,
// exactly one verdict line is printed, beginning with "OK", "ERROR", or
// "PARSE ERROR".
//
// Usage: sqlCheck [catalogFile]
//
// Also understood at the prompt (with or without a trailing semicolon):
//    quit / exit     leave the program
//    verbose on      print every parsed SFW query before checking it
//    verbose off     stop doing that (the default)
int main (int numArgs, char **args) {

	// open up the catalog file
	string catFileName = "catFile";
	if (numArgs > 1)
		catFileName = args[1];
	MyDB_CatalogPtr myCatalog = make_shared <MyDB_Catalog> (catFileName);

	// get all of the tables that are already in the catalog
	map <string, MyDB_TablePtr> allTables = MyDB_Table :: getAllTables (myCatalog);

	// print a prompt only if someone is typing at us
	bool interactive = isatty (0);
	bool verbose = false;

	if (interactive) {
		cout << "\n          Welcome to MyDB v0.1\n\n";
		cout << "\"Not the worst database in the world\" (tm) \n\n";
	}

	// this collects the statement that we are reading
	stringstream ss;
	bool statementStarted = false;

	if (interactive)
		cout << "MyDB> " << flush;

	for (string line; getline (cin, line);) {

		string trimmed = trim (line);

		// skip comment lines and blank lines between statements
		if (trimmed.substr (0, 2) == "--" || (trimmed == "" && !statementStarted)) {
			if (interactive)
				cout << (statementStarted ? "    > " : "MyDB> ") << flush;
			continue;
		}

		// see if we got a command, rather than SQL
		if (!statementStarted) {
			string cmd = trimmed;
			if (cmd.size () > 0 && cmd.back () == ';')
				cmd = trim (cmd.substr (0, cmd.size () - 1));
			cmd = toLower (cmd);
			if (cmd == "quit" || cmd == "exit") {
				if (interactive)
					cout << "OK, goodbye.\n";
				return 0;
			}
			if (cmd == "verbose on") {
				verbose = true;
				if (interactive)
					cout << "MyDB> " << flush;
				continue;
			}
			if (cmd == "verbose off") {
				verbose = false;
				if (interactive)
					cout << "MyDB> " << flush;
				continue;
			}
		}

		// no semicolon yet: keep collecting
		size_t pos = line.find (';');
		if (pos == string :: npos) {
			ss << line << "\n";
			statementStarted = true;
			if (interactive)
				cout << "    > " << flush;
			continue;
		}

		// we have a complete statement
		line.resize (pos);
		ss << line;
		string parseMe = ss.str ();
		ss.str ("");
		ss.clear ();
		statementStarted = false;

		// add an extra zero at the end; needed by lexer
		parseMe.push_back ('\0');

		// now parse it
		yyscan_t scanner;
		LexerExtra extra { "" };
		yylex_init_extra (&extra, &scanner);
		const YY_BUFFER_STATE buffer { yy_scan_string (parseMe.data (), scanner) };
		SQLStatement *final = nullptr;
		const int parseFailed { yyparse (scanner, &final) };
		yy_delete_buffer (buffer, scanner);
		yylex_destroy (scanner);

		// if we did not parse correctly
		if (parseFailed) {

			cout << "PARSE ERROR: " << extra.errorMessage;
			if (final != nullptr)
				delete final;

		// a create table: put it in the catalog
		} else if (final->isCreateTable ()) {

			string tableName = final->addToCatalog ("tables", myCatalog);
			if (tableName == "nothing") {
				cout << "ERROR: could not create the table\n";
			} else {
				MyDB_TablePtr temp = make_shared <MyDB_Table> ();
				temp->fromCatalog (tableName, myCatalog);
				allTables[tableName] = temp;
				cout << "OK: created table " << tableName << "\n";
			}
			delete final;

		// a select-from-where query: this is where the semantic check goes
		} else if (final->isSFWQuery ()) {

			if (verbose)
				final->printSFWQuery ();

			// ****************************************************************
			// right now, every query that parses is reported as OK.  The
			// semantic check of the query against allTables goes here
			// ****************************************************************
			cout << "OK\n";

			delete final;
		}

		if (interactive)
			cout << "MyDB> " << flush;
	}

	if (interactive)
		cout << "\n";
	return 0;
}
