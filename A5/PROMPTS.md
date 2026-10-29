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

Technical Specification:
1. Check each parsed 'SELECT ... FROM ... WHERE' query against the catalog. Preserve Stage 1 parsing, join trees, and verbose output.
2. Verify each FROM table exists. Resolve aliases and check attribute ownership throughout SELECT, WHERE, GROUP BY, and ON.
3. SELECT, WHERE, and GROUP BY see all FROM aliases. Each ON sees only aliases in its join's two subtrees.
4. Check all expression operands, including both OR branches, expressions under NOT, and aggregate arguments.
5. Only allow numeric arithmetic with INT/DOUBLE compatibility and STRING + STRING concatenation.
6. For comparissons we must allow numeric pairs and two strings. Require boolean operands for logical operations, and boolean conditions in WHERE and ON.
7. SUM and AVG require numeric arguments only. We must not allow aggreagtes in ON.
8. Enforce grouping for selected attributes outside aggregates. Allow aggregate-only queries without needing GROUP BY. If you have any questions about expression-level grouping rules, ask me first before implementation.
9. Before implementation, ask me about any unresolved choices. This can include duplicate aliases, aggreagte placement or neting, grouping expressions, boolean compatibility, and numeric results. We should then record these choices in our ParserTypes.h file.
10. Print output per statement. OK, ERROR - with a useful explanation - or PARSE ERROR. Don't stop execution at a PARSE ERROR, we should ocntinue with the next statements.
11. Again, before implementation, discuss any unresolved decisions and propose complete test queries witjh expected outputs for my apporval.
12. Put all new TPC-H tests in Queries/Stage2.sql. Cover references, types, aggregates, grouuping, and ON scope with valid and invalid cases. Avoid submitted tests based on optional judgment calls.
13. Run full checking on Test.sql, JoinExamples.sql, Stage1.sql, and Stage2.sql using CreateTables.sql. Preserve the existing query files and the checking script.

Let me know if you have any questions before implementation.

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

Technical Specification:
1. Preserve the agreed Stage 1 and Stage 2 behaviour.
2. Apply the following ten mutations:
1) FROM: Accept a nonexistent table, using a controlled fallback so the mutation produces a verdict rather than crashing.
2) SELECT: Accept an atribute missing from its aliased table.
3) WHERE: Accept comparisons between numbers and strings.
4) SELECT: Accept SUM of a string.
5) Expressions: Reject valid String + String concatentaions>
6) GROUP BY: Allow an ungrouped selected aattribute alongside an aggregate.
7) ON scope: Allow references to aliases outside the join's two operands.
8) On aggregates: Allow aggregate expressions inside ON.
9) Grammar: Remove the prenthesized-join production, causing valid parenthesized joins to fail parsing.
10) Grammar: Remove support for comma-seperated FROM items, causing valid comma lists to fail parsing.
Each mutation must affect only the named rule.
3. Before any changes are made, I would like for you to explain the concrete change for each mutation and identify existing queries expected to catch it. Ask me about potential ambiguity. In addition, each mutation must change the intended rule without introducing unrelated failures.
4.  For each mutation: Apply it, rebuild, run every query file using the supplied checker and a fresh catalog initialized with CreateTables.sql, and record the results. Never combine mutations.
5. A semantic mutation is caught by a change between OK and ERROR> A grammar mutation is caught by a change between parsing verdict and PARSE ERROR. Compilation failures, crashes, message-only changes, and unrelated failures do not count.|
6. If no existing query catches a mutation, propose a focused TPC-H query and its expected verdict for my approval. Add approved tests only to Queries/Stage3.sql. Use required behavious established by the assignment and examples; avoid optional judgement calls.
7. Verify each added query against the restored correct checker, then reapply the mutation and confirm that the query catches it. Do not change expectations merely to match the implementation.
8. Undo each nutation completely, rebuild, and confirm all query files pass ebfore starting the next. Preserve approved Stage2.sql additions.
9. Report each mutatio, whether existing tests caught it, the file and quiery identifier of a catching test, and its expected and mutated verdicts. Report any uncaught mutation honestly.
10. Finish with all mutations removed and all query files passing. Update the design comment in ParserTypes.h with the STage 3 approach and results.

Let me know if you have any additional questions before we begin with implementation.

----------------------------------------------------------------------

