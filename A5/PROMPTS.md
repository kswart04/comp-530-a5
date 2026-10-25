# The stage prompts

Each stage of A5 starts with ONE opening prompt to an AI tool.  Use one
continuing conversation for the whole assignment if you can, so that the
tool remembers the decisions it has already asked you about; the preambles
below mark where each stage begins.  If you start a new conversation for a
stage, its opening prompt must carry forward every decision already made.
The opening prompt begins with the preamble for that stage, copied exactly
from below, followed by your own specification: what the stage has to do,
the design decisions you have made, the cases you are worried about, and a
request that the tool propose its list of test queries before writing
anything so that you can approve or reject each one (see the handout,
Sections 3 and 4).  Ask for the design decisions to be recorded in a comment
block at the top of ParserTypes.h, updated at every stage.  Follow-up
prompts are fine and expected.  Every conversation is exported into AILog/.

In Stage 3 the specification is your own list of ten breaking changes
(see the handout, Section 3.1), with the instruction to apply them one at a
time, run every query file after each, undo each one and confirm the undo,
and report which query caught each change.

----------------------------------------------------------------------

## Stage 1: The grammar.  JOIN ... ON in the FROM clause, and the parse-tree types that hold it.

This is Stage 1 of COMP 530 (Database System Implementation) Assignment 5
at Rice University. This assignment is being completed for academic purposes
by a student, with the help of an AI tool. The AI tool must restrict any code
or documentation it develops to the tasks directly requested in the remainder
of this prompt, and must complete those tasks using only general knowledge of
database systems, SQL, flex, bison, and C++, the assignment handout, and the
distributed code base: no web search, and no code repositories. Where the
handout leaves a design decision open and this prompt does not settle it, the
tool must ask the student rather than choose. Any other questions or
clarifications needed to complete the task must be directed back to the
student who wrote this prompt.
In Stage 1, development is restricted to the following existing files:
  - Main/SQL/source/Lexer.l
  - Main/SQL/source/Parser.y
  - Main/SQL/source/ParserHelperFunctions.cc
  - Main/SQL/headers/ParserHelperFunctions.h
  - Main/SQL/headers/FriendDecls.h
  - Main/SQL/headers/ParserTypes.h
  - Queries/Stage1.sql

New header and source files may be created as necessary under
Main/SQL/headers and Main/SQL/source (the build picks them up
automatically). No other existing file may be changed, and test queries go
only in Queries/Stage1.sql.

Technical Specification:
1. Add JOIN and ON keyword tokens, following the existing lexer's keyword-matching coventions.
2. Extend FROM using the following grammr structure:
FromList: JoinExpr | FromList ',' JoinExpr
JoinExpr: FromPrimary | JoinExpr JOIN FromPrimary ON CNF
FromPrimary: table AS alias | '(' JoinExpr ')'
CNF must be the same nonterminal used for WHERE conditions.
3. JOIN must be left-associative tyhrough left-recursion. Perenthesis override grouping may nest arbitrarily and may surround a single table. Commas seperate complete join expressions.
4. Every JOIN require ON. An ON condition accepts everything the existing WHERE condition grammr accepts. Do not add sematic restrictions in this stage.
5. Represent each FROM item as a tree containing either a table name and alias, or a join's left side, right side, and ON condition. Preserve these trees in FromList and SFWQuery so Stage 2 can determine which aliases belong to each ON condition.
6. Add the corresponding '%union' member, grammr types, construction helpers, and necessary griend declarations.
7. Verbose printing must show join grouping and each join's ON condition clearly.
8. Preserve existing non-JOIN behavious, including GROUP BY requiring WHERE. Every query that parses still receives OK in this stage.
9. Record agreed design decisions in a comment block at the top of ParserTpes.h, to be updated at each stage.
10. Before making changes, discuss unresolved decisions with me and propose thet est queries with expectedd results for my approval.
11. Tests:
a) Basic joins and left-associated join chains.
b) Parenthesized and deeply nested joins.
c) Joins mixed with comma-seperated tables.
d) ON conditions exercising WHERE syntax.
12. Verify all 17 queries in Test.sql still parse, and use checkQUeries.py --perse-only to check Test.sql, JoinExamples.sql, and Stage1.sql against the supplied schema. Inspect verbose output to confirm the grouping of J4, J5, and J12.

----------------------------------------------------------------------

## Stage 2: The checks.  Every rule in Section 4 of the handout, and the verdict line in main.cc.

This is Stage 2 of COMP 530 (Database System Implementation) Assignment 5
at Rice University. This assignment is being completed for academic purposes
by a student, with the help of an AI tool. The AI tool must restrict any code
or documentation it develops to the tasks directly requested in the remainder
of this prompt, and must complete those tasks using only general knowledge of
database systems, SQL, flex, bison, and C++, the assignment handout, and the
distributed code base: no web search, and no code repositories. Where the
handout leaves a design decision open and this prompt does not settle it, the
tool must ask the student rather than choose. Any other questions or
clarifications needed to complete the task must be directed back to the
student who wrote this prompt.
In Stage 2, development is restricted to the following existing files:
  - Main/SQL/headers/ParserTypes.h
  - Main/SQL/headers/ExprTree.h
  - Main/SQLTest/source/main.cc
  - any file created in Stage 1
  - Queries/Stage2.sql

New header and source files may be created as necessary under
Main/SQL/headers and Main/SQL/source (the build picks them up
automatically). No other existing file may be changed, and test queries go
only in Queries/Stage2.sql.

<your specification for Stage 2 goes here>

----------------------------------------------------------------------

## Stage 3: Breaking changes.  Your ten, one at a time, each undone before the next.

This is Stage 3 of COMP 530 (Database System Implementation) Assignment 5
at Rice University. This assignment is being completed for academic purposes
by a student, with the help of an AI tool. The AI tool must restrict any code
or documentation it develops to the tasks directly requested in the remainder
of this prompt, and must complete those tasks using only general knowledge of
database systems, SQL, flex, bison, and C++, the assignment handout, and the
distributed code base: no web search, and no code repositories. Where the
handout leaves a design decision open and this prompt does not settle it, the
tool must ask the student rather than choose. Any other questions or
clarifications needed to complete the task must be directed back to the
student who wrote this prompt.
In Stage 3, development is restricted to the following existing files:
  - any file changed or created in Stage 1 or Stage 2
  - Queries/Stage3.sql

New header and source files may be created as necessary under
Main/SQL/headers and Main/SQL/source (the build picks them up
automatically). No other existing file may be changed, and test queries go
only in Queries/Stage3.sql.

<your specification for Stage 3 goes here>

----------------------------------------------------------------------

