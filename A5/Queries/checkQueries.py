#!/usr/bin/env python3
#
# Runs one or more .sql files through the semantic checker and compares what
# it prints with what the files say to expect.
#
#   python3 checkQueries.py ../Build/bin/sqlCheck CreateTables.sql Test.sql
#
# With --parse-only as the first argument, OK and ERROR are treated as the
# same thing ("it parsed"), so only the PARSE ERROR expectations are checked.
# That is the mode for Stage 1, when there is no semantic checker yet:
#
#   python3 checkQueries.py --parse-only ../Build/bin/sqlCheck CreateTables.sql Test.sql
#
# The files are fed to the checker in the order given, in one run, so put
# CreateTables.sql (or whatever creates your tables) first.  The run starts
# with a fresh, empty catalog.
#
# Inside a .sql file, a comment line of the form
#
#   -- expect: OK          (or ERROR, or PARSE ERROR)
#
# applies to the next statement.  Statements with no such line (the CREATE
# TABLEs, usually) are run but not checked.  The checker must print exactly
# one line per statement that begins with OK, ERROR, or PARSE ERROR; every
# other line it prints is ignored.
#
# Prints one line per mismatch, then a summary, and exits with the number of
# mismatches (so 0 means everything matched).

import os
import subprocess
import sys

VERDICTS = ("PARSE ERROR", "ERROR", "OK")
COMMANDS = ("quit", "exit", "verbose on", "verbose off")


def verdictOf (line):
	"""the verdict a line of checker output carries, or None"""
	for v in VERDICTS:
		if line.startswith (v):
			return v
	return None


def readStatements (fileName):
	"""returns a list of (expected, statementText, lineNumber), in file order"""
	statements = []
	expect = None
	buf = []
	startLine = None
	with open (fileName) as f:
		for num, raw in enumerate (f, 1):
			line = raw.rstrip ("\n")
			stripped = line.strip ()
			if stripped.startswith ("--"):
				rest = stripped[2:].strip ()
				if rest.lower ().startswith ("expect:"):
					expect = rest[len ("expect:"):].strip ().upper ()
					if expect not in VERDICTS:
						sys.exit (fileName + ":" + str (num) + ": bad expectation '" + expect + "'")
				continue
			if not buf:
				if stripped == "":
					continue
				if stripped.rstrip (";").strip ().lower () in COMMANDS:
					continue
				startLine = num
			if ";" in line:
				buf.append (line[:line.index (";")])
				statements.append ((expect, "\n".join (buf).strip (), startLine))
				buf = []
				expect = None
			else:
				buf.append (line)
	return statements


def main ():
	args = sys.argv[1:]
	parseOnly = False
	if args and args[0] == "--parse-only":
		parseOnly = True
		args = args[1:]
	if len (args) < 2:
		sys.exit ("usage: checkQueries.py [--parse-only] path/to/sqlCheck file.sql [file.sql ...]")
	checker = args[0]
	files = args[1:]

	# in parse-only mode, OK and ERROR both just mean "it parsed"
	def same (a, b):
		if parseOnly:
			return (a == "PARSE ERROR") == (b == "PARSE ERROR")
		return a == b

	statements = []
	text = ""
	for fileName in files:
		statements += [(e, s, fileName + ":" + str (n)) for (e, s, n) in readStatements (fileName)]
		with open (fileName) as f:
			text += f.read () + "\n"

	# every run starts with an empty catalog
	catFile = "checkQueries.catFile"
	if os.path.exists (catFile):
		os.remove (catFile)
	try:
		result = subprocess.run ([checker, catFile], input = text, capture_output = True, text = True)
	finally:
		if os.path.exists (catFile):
			os.remove (catFile)

	verdicts = [l for l in result.stdout.splitlines () if verdictOf (l) is not None]

	if result.returncode != 0:
		print ("the checker exited with status " + str (result.returncode) + " (crashed?)")
		if result.stderr.strip ():
			print (result.stderr.strip ())
	if len (verdicts) != len (statements):
		print ("expected " + str (len (statements)) + " verdict lines for " + str (len (statements)) +
			" statements, but got " + str (len (verdicts)))
		print ("(every statement must produce exactly one line starting with OK, ERROR, or PARSE ERROR)")

	mismatches = 0
	checked = 0
	for i, (expect, stmt, where) in enumerate (statements):
		got = verdicts[i] if i < len (verdicts) else "<no verdict>"
		if expect is None:
			continue
		checked += 1
		if not same (verdictOf (got), expect):
			mismatches += 1
			print ("MISMATCH at " + where + ": expected " + (("parse" if expect != "PARSE ERROR" else "PARSE ERROR") if parseOnly else expect) + ", got: " + got)
			print ("    " + stmt.replace ("\n", "\n    "))

	print (str (checked - mismatches) + " of " + str (checked) + " checked statements matched" +
		(" (" + str (len (statements) - checked) + " unchecked)" if checked != len (statements) else ""))
	sys.exit (min (mismatches + (1 if len (verdicts) != len (statements) else 0), 255))


if __name__ == "__main__":
	main ()
