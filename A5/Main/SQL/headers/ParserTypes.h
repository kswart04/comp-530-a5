
/*****************************************************************************
 * DESIGN DECISIONS (updated at each stage)
 *
 * Stage 1: JOIN ... ON in the FROM clause
 *
 *  Lexer
 *   - JOIN is a new keyword token, matched case-insensitively like the other
 *     keywords ([Jj][Oo][Ii][Nn]).  The ON token already existed (it is used
 *     by CREATE TABLE ... AS BPLUSTREE ON att) and is reused.  JOIN and ON are
 *     reserved: neither can be used as a table name or an alias.
 *
 *  Grammar
 *     FromList:    JoinExpr | FromList ',' JoinExpr
 *     JoinExpr:    FromPrimary | JoinExpr JOIN FromPrimary ON CNF
 *     FromPrimary: IDENTIFIER AS IDENTIFIER | '(' JoinExpr ')'
 *   - JoinExpr is left-recursive, so a JOIN chain groups to the left:
 *     a JOIN b ON .. JOIN c ON .. is (a JOIN b ON ..) JOIN c ON ..
 *   - Parentheses override the grouping, nest arbitrarily, and may surround
 *     a single table.  Commas separate complete join expressions; a comma
 *     cannot appear inside parentheses.
 *   - Every JOIN requires ON.  The ON condition is the same CNF nonterminal
 *     as the WHERE clause, so it accepts exactly what WHERE accepts.  No
 *     semantic restrictions are applied in Stage 1.
 *   - Non-JOIN behaviour is unchanged, including GROUP BY requiring WHERE.
 *     Every query that parses is still reported as OK.
 *
 *  Parse tree
 *   - FromTree is a tagged node held by shared_ptr (FromTreePtr): a leaf
 *     (isJoin == false) holds a table name and alias; a join node
 *     (isJoin == true) holds its left and right subtrees and its ON
 *     condition, stored as a vector of disjunctions (the same form as the
 *     WHERE clause).  Parentheses do not get nodes of their own; the shape
 *     of the tree records the grouping.
 *   - FromItem wraps a FromTreePtr and is the %union member (myFromItem)
 *     carried by JoinExpr and FromPrimary, the way Value wraps an ExprTreePtr.
 *   - FromList and SFWQuery store vector <FromTreePtr>, one tree per
 *     comma-separated item, in source order.  No flat alias list is kept;
 *     Stage 2 collects aliases from the trees.
 *   - Helpers: makeFromTable, makeJoin, and makeFromList / appendFromList
 *     (which now take FromItems instead of table and alias names).
 *
 *  Verbose printing
 *   - A plain table prints as before ("\ttable AS alias").  A join prints as
 *     an indented tree: "JOIN", then its left and right sides indented one
 *     level, then "ON: " followed by its disjunctions, one per line.
 *
 *  Tests
 *   - Queries/Stage1.sql.  Each "-- expect:" line gives the eventual semantic
 *     verdict (so a query that parses but breaks a Stage 2 rule is marked
 *     ERROR); Stage 1 runs them with checkQueries.py --parse-only.
 *   - Tests are graded by mutation (a test counts if its verdict flips under
 *     a breaking change), so every proposed test was kept, S1-S38, except
 *     S23-S25.  Those put parentheses around a single table, which the
 *     handout lists as a judgment call, so they were removed (the grammar
 *     still accepts it).  The other identifiers were not renumbered.
 *
 * Stage 2: the semantic checks
 *
 *  Code organization
 *   - Every ExprTree class has a virtual check (CheckContext &, string &err)
 *     that returns the expression's type, or ExprType :: Error with the
 *     reason in err.  The first error is propagated by return value; no
 *     exceptions are used.
 *   - CheckContext.h holds the type enum and the CheckContext: the aliases
 *     visible to the current clause, every alias in the FROM clause, the
 *     clause name, whether aggregates are allowed, whether we are inside an
 *     aggregate, whether an aggregate was seen, and the attributes found
 *     outside aggregates.
 *   - SFWQuery :: check (allTables) returns "" for OK or the error message;
 *     SQLStatement :: checkSFWQuery passes it through, and main.cc prints
 *     "OK" or "ERROR: <message>".  Parsing, the join trees, and verbose
 *     output are unchanged, and checking continues after a PARSE ERROR.
 *
 *  Types (int, double, string, bool)
 *   - Literals have their own type; attributes have their catalog type.
 *   - +: int + int is int; a double operand makes it double; string +
 *     string is string (concatenation).  Anything else is an error.
 *   - - * /: numbers only, with the same result rule (int / int is int).
 *   - > < = <>: two numbers (int and double mix freely) or two strings give
 *     bool.  Two bools may be compared with = or <> only.  Anything else is
 *     an error.
 *   - OR and NOT need bool operands; every WHERE and ON condition must be
 *     bool.  (The grammar already guarantees this for parsed queries.)
 *   - SUM and AVG need a number.  SUM keeps its argument's type; AVG is double.
 *   - BOOL attributes are allowed only in = and <> against another bool.
 *
 *  References and scope
 *   - Every FROM table must be in the catalog.  An alias may appear only
 *     once in a FROM clause (an error otherwise); the same table may appear
 *     under different aliases.  Names are matched case-sensitively.
 *   - Every alias.attribute must name a FROM alias whose table has that
 *     attribute, in SELECT, WHERE, GROUP BY, and ON.
 *   - SELECT, WHERE, and GROUP BY see every FROM alias.  An ON condition
 *     sees only the aliases in its join's two subtrees: not aliases joined
 *     later in a chain, not aliases outside its parentheses, and not aliases
 *     in other comma-separated items.
 *
 *  Aggregates and grouping
 *   - Aggregates are allowed only in SELECT: an aggregate in ON, WHERE, or
 *     GROUP BY is an error, and so is an aggregate nested inside another.
 *   - The grouping rule applies when there is a GROUP BY or the SELECT has
 *     an aggregate.  Then each selected value must either match a GROUP BY
 *     expression exactly, or every attribute it uses outside aggregates must
 *     itself be a GROUP BY item.  Constants and attributes inside aggregates
 *     need no grouping; GROUP BY attributes need not be selected.  An
 *     aggregate-only SELECT needs no GROUP BY.  A GROUP BY with no aggregate
 *     in the SELECT is allowed, under the same rule.
 *
 *  Check order (the first error found is reported)
 *   1. every FROM table exists, in source order;  2. no duplicate aliases;
 *   3. ON conditions, inner joins before outer, left before right;
 *   4. WHERE;  5. GROUP BY;  6. SELECT;  7. the grouping rule.
 *   Within an expression, operands are checked left to right before the
 *   operator itself.
 *
 *  Error messages
 *   - "ERROR: " and then the problem, naming the table, alias, attribute,
 *     or operand types involved.  Expressions are shown with the existing
 *     toString () output (there is no SQL pretty-printer).
 *
 *  Tests
 *   - Queries/Stage2.sql covers references, types, aggregates, grouping, and
 *     ON scope, with valid and invalid cases, T1-T47 (every proposed test
 *     was kept).  Its tests compare verdicts only, and avoid the judgment
 *     calls above (duplicate aliases, BOOL, aggregates in WHERE / GROUP BY,
 *     nested aggregates, expression grouping).  T46-T47 check that the
 *     statement after a PARSE ERROR is still checked.
 *
 * Stage 3: mutation testing
 *
 *  Approach
 *   - Ten breaking changes, applied one at a time as exact text edits.  For
 *     each: back up the touched file(s), apply, rebuild, run checkQueries.py
 *     (fresh catalog, CreateTables.sql first) on Test.sql, JoinExamples.sql,
 *     Stage1.sql, Stage2.sql, and Stage3.sql, then restore from the backup,
 *     confirm the file is byte-identical, rebuild, and confirm every query
 *     file passes before the next mutation.
 *   - Caught means some query's verdict flips: OK <-> ERROR for a semantic
 *     mutation, a parsing verdict <-> PARSE ERROR for a grammar mutation.
 *     Every flip was confirmed to come from the mutated rule only.
 *   - Mutation details: (1) a missing table maps to a local empty-schema
 *     table (the catalog's table map is never modified), so attribute checks
 *     still apply; (2) a missing attribute is accepted as an int in SELECT
 *     only; (3) number/string comparisons are accepted in WHERE only;
 *     (4) SUM (not AVG) accepts a string; (5) + rejects two strings;
 *     (6) the whole grouping rule is skipped; (7) an ON sees every FROM
 *     alias; (8) aggregates are accepted in ON, an ON aggregate does not make
 *     the query an aggregate query, and argument-type and nesting checks
 *     are kept; (9) FromPrimary: '(' JoinExpr ')' removed; (10) FromList:
 *     FromList ',' JoinExpr removed.
 *
 *  Results (expected verdict -> mutated verdict of the catching queries)
 *   1  missing table accepted   Stage3 U1 (ERROR -> OK); nothing existing
 *                               caught it, so U1 was added
 *   2  missing SELECT attribute Test 13, Stage2 T6, T9 (ERROR -> OK)
 *   3  WHERE number vs string   Test 3, 6, 15, Stage2 T21, T23, T41 (ERROR -> OK)
 *   4  SUM of a string          Test 4, Stage2 T42 (ERROR -> OK)
 *   5  string + string rejected Test 5, 12, 14, Stage1 S31, Stage2 T15 (OK -> ERROR)
 *   6  grouping rule off        Test 2, Stage2 T29, T30, T31, T44 (ERROR -> OK)
 *   7  ON scope widened         J6, J7, Stage1 S19, Stage2 T12, T13, T14, T35 (ERROR -> OK)
 *   8  aggregates in ON         J9, Stage1 S18, Stage2 T26, T27, T43 (ERROR -> OK)
 *   9  no parenthesized joins   12 queries, e.g. J5, J12 (OK -> PARSE ERROR),
 *                               J6 (ERROR -> PARSE ERROR)
 *   10 no comma FROM lists      25 queries, e.g. Test 1 (OK -> PARSE ERROR),
 *                               Test 2 (ERROR -> PARSE ERROR)
 *   All ten were caught.  All mutations were removed at the end, and every
 *   query file passes.
 *****************************************************************************/

#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

#include <iostream>
#include <stdlib.h>
#include "ExprTree.h"
#include "MyDB_Catalog.h"
#include "MyDB_Schema.h"
#include "MyDB_Table.h"
#include <string>
#include <utility>

using namespace std;

/*************************************************/
/** HERE WE DEFINE ALL OF THE STRUCTS THAT ARE **/
/** PASSED AROUND BY THE PARSER                **/
/*************************************************/

// structure that encapsulates a parsed computation that returns a value
struct Value {

private:

        // this points to the expression tree that computes this value
        ExprTreePtr myVal;

public:
        ~Value () {}

        Value (ExprTreePtr useMe) {
                myVal = useMe;
        }

        Value () {
                myVal = nullptr;
        }
	
	friend struct CNF;
	friend struct ValueList;
	friend struct SFWQuery;
	#include "FriendDecls.h"
};

// structure that encapsulates a parsed CNF computation
struct CNF {

private:

        // this points to the expression tree that computes this value
        vector <ExprTreePtr> disjunctions;

public:
        ~CNF () {}

        CNF (struct Value *useMe) {
              	disjunctions.push_back (useMe->myVal); 
        }

        CNF () {}

	friend struct SFWQuery;
	#include "FriendDecls.h"
};

// structure that encapsulates a parsed list of value computations
struct ValueList {

private:

        // this points to the expression tree that computes this value
        vector <ExprTreePtr> valuesToCompute;

public:
        ~ValueList () {}

        ValueList (struct Value *useMe) {
              	valuesToCompute.push_back (useMe->myVal); 
        }

        ValueList () {}

	friend struct SFWQuery;
	#include "FriendDecls.h"
};


// structure to encapsulate a create table
struct CreateTable {

private:

	// the name of the table to create
	string tableName;

	// the list of atts to create... the string is the att name
	vector <pair <string, MyDB_AttTypePtr>> attsToCreate;

	// true if we create a B+-Tree
	bool isBPlusTree;

	// the attribute to organize the B+-Tree on
	string sortAtt;

public:
	string addToCatalog (string storageDir, MyDB_CatalogPtr addToMe) {

		// make the schema
		MyDB_SchemaPtr mySchema = make_shared <MyDB_Schema>();
		for (auto a : attsToCreate) {
			mySchema->appendAtt (a);
		}

		// now, make the table
		MyDB_TablePtr myTable;

		// just a regular file
		if (!isBPlusTree) {
			myTable =  make_shared <MyDB_Table> (tableName, 
				storageDir + "/" + tableName + ".bin", mySchema);	

		// creating a B+-Tree
		} else {
			
			// make sure that we have the attribute
			if (mySchema->getAttByName (sortAtt).first == -1) {
				cout << "B+-Tree not created.\n";
				return "nothing";
			}
			myTable =  make_shared <MyDB_Table> (tableName, 
				storageDir + "/" + tableName + ".bin", mySchema, "bplustree", sortAtt);	
		}

		// and add to the catalog
		myTable->putInCatalog (addToMe);

		return tableName;
	}

	CreateTable () {}

	CreateTable (string tableNameIn, vector <pair <string, MyDB_AttTypePtr>> atts) {
		tableName = tableNameIn;
		attsToCreate = atts;
		isBPlusTree = false;
	}

	CreateTable (string tableNameIn, vector <pair <string, MyDB_AttTypePtr>> atts, string sortAttIn) {
		tableName = tableNameIn;
		attsToCreate = atts;
		isBPlusTree = true;
		sortAtt = sortAttIn;
	}
	
	~CreateTable () {}

	#include "FriendDecls.h"
};

// structure that stores a list of attributes
struct AttList {

private:

	// the list of attributes
	vector <pair <string, MyDB_AttTypePtr>> atts;

public:
	AttList (string attName, MyDB_AttTypePtr whichType) {
		atts.push_back (make_pair (attName, whichType));
	}

	~AttList () {}

	friend struct SFWQuery;
	#include "FriendDecls.h"
};

// a node in the tree for one FROM item: either a table and its alias, or a
// join of two subtrees with its ON condition
struct FromTree;
typedef shared_ptr <FromTree> FromTreePtr;

struct FromTree {

private:

	// true if this is a join; false if it is a table
	bool isJoin;

	// in case we are a table
	string tableName;
	string alias;

	// in case we are a join
	FromTreePtr left;
	FromTreePtr right;
	vector <ExprTreePtr> onDisjunctions;

public:
	FromTree (string tableNameIn, string aliasIn) {
		isJoin = false;
		tableName = tableNameIn;
		alias = aliasIn;
	}

	FromTree (FromTreePtr leftIn, FromTreePtr rightIn, vector <ExprTreePtr> onIn) {
		isJoin = true;
		left = leftIn;
		right = rightIn;
		onDisjunctions = onIn;
	}

	~FromTree () {}

	// prints the tree, for debugging; each level is indented further
	void print (string indent) {
		if (!isJoin) {
			cout << indent << tableName << " AS " << alias << "\n";
			return;
		}
		cout << indent << "JOIN\n";
		left->print (indent + "    ");
		right->print (indent + "    ");
		string label = "ON: ";
		for (auto a : onDisjunctions) {
			cout << indent << "    " << label << a->toString () << "\n";
			label = "    ";
		}
	}

	// makes sure that every table in this tree is in the catalog; returns an
	// error message, or "" if all of them are
	string checkTables (map <string, MyDB_TablePtr> &allTables) {
		if (isJoin) {
			string err = left->checkTables (allTables);
			if (err != "")
				return err;
			return right->checkTables (allTables);
		}
		if (allTables.count (tableName) == 0)
			return "table " + tableName + " does not exist (alias " + alias + ")";
		return "";
	}

	// adds every alias in this tree, and its table, to aliases; returns an
	// error message if an alias is already there, or "" otherwise
	string collectAliases (map <string, MyDB_TablePtr> &allTables, map <string, MyDB_TablePtr> &aliases) {
		if (isJoin) {
			string err = left->collectAliases (allTables, aliases);
			if (err != "")
				return err;
			return right->collectAliases (allTables, aliases);
		}
		if (aliases.count (alias) > 0)
			return "alias " + alias + " is used for more than one table in the FROM clause";
		aliases[alias] = allTables[tableName];
		return "";
	}

	// checks every ON condition in this tree, inner joins first and left
	// before right.  Each ON sees only the aliases in its join's two subtrees.
	// Puts this tree's aliases into myAliases; returns an error message, or ""
	string checkOn (CheckContext &ctx, map <string, MyDB_TablePtr> &allAliases,
		map <string, MyDB_TablePtr> &myAliases) {

		if (!isJoin) {
			myAliases[alias] = allAliases[alias];
			return "";
		}

		map <string, MyDB_TablePtr> leftAliases;
		map <string, MyDB_TablePtr> rightAliases;
		string err = left->checkOn (ctx, allAliases, leftAliases);
		if (err != "")
			return err;
		err = right->checkOn (ctx, allAliases, rightAliases);
		if (err != "")
			return err;

		myAliases = leftAliases;
		myAliases.insert (rightAliases.begin (), rightAliases.end ());
		ctx.visible = myAliases;
		for (auto a : onDisjunctions) {
			ExprType t = a->check (ctx, err);
			if (t == ExprType :: Error)
				return err;
			if (t != ExprType :: Bool)
				return "the ON condition " + a->toString () + " is " + aTypeName (t) + ", not a bool";
		}
		return "";
	}

	friend struct SFWQuery;
	#include "FriendDecls.h"
};

// structure that encapsulates one parsed FROM item
struct FromItem {

private:

	// this points to the tree for the FROM item
	FromTreePtr myTree;

public:
	FromItem (FromTreePtr useMe) {
		myTree = useMe;
	}

	~FromItem () {}

	friend struct FromList;
	#include "FriendDecls.h"
};

struct FromList {

private:

	// the list of FROM items, in the order they appear in the query
	vector <FromTreePtr> fromItems;

public:
	FromList (FromTreePtr useMe) {
		fromItems.push_back (useMe);
	}

	FromList () {}

	~FromList () {}
	
	friend struct SFWQuery;
	#include "FriendDecls.h"
};

// structure that stores an entire SFW query
struct SFWQuery {

private:

	// the various parts of the SQL query
	vector <ExprTreePtr> valuesToSelect;
	vector <FromTreePtr> tablesToProcess;
	vector <ExprTreePtr> allDisjunctions;
	vector <ExprTreePtr> groupingClauses;

public:
	SFWQuery () {}

	SFWQuery (struct ValueList *selectClause, struct FromList *fromClause,
		struct CNF *cnf, struct ValueList *grouping) {
		valuesToSelect = selectClause->valuesToCompute;
		tablesToProcess = fromClause->fromItems;
		allDisjunctions = cnf->disjunctions;
		groupingClauses = grouping->valuesToCompute;
	}

	SFWQuery (struct ValueList *selectClause, struct FromList *fromClause,
		struct CNF *cnf) {
		valuesToSelect = selectClause->valuesToCompute;
		tablesToProcess = fromClause->fromItems;
		allDisjunctions = cnf->disjunctions;
	}

	SFWQuery (struct ValueList *selectClause, struct FromList *fromClause) {
		valuesToSelect = selectClause->valuesToCompute;
		tablesToProcess = fromClause->fromItems;
	}

	~SFWQuery () {}

	// prints the parsed query, for debugging
	void print () {
		cout << "Selecting the following:\n";
		for (auto a : valuesToSelect)
			cout << "\t" << a->toString () << "\n";
		cout << "From the following:\n";
		for (auto a : tablesToProcess)
			a->print ("\t");
		cout << "Where the following are true:\n";
		for (auto a : allDisjunctions)
			cout << "\t" << a->toString () << "\n";
		cout << "Group using:\n";
		for (auto a : groupingClauses)
			cout << "\t" << a->toString () << "\n";
	}

	// checks the query against the tables in the catalog; returns "" if the
	// query is OK, and otherwise the reason (the first error found) it is not
	string check (map <string, MyDB_TablePtr> &allTables) {

		string err;

		// every table in the FROM clause must exist...
		for (auto a : tablesToProcess) {
			err = a->checkTables (allTables);
			if (err != "")
				return err;
		}

		// ...and no alias can be used twice
		map <string, MyDB_TablePtr> aliases;
		for (auto a : tablesToProcess) {
			err = a->collectAliases (allTables, aliases);
			if (err != "")
				return err;
		}

		CheckContext ctx;
		for (auto &a : aliases)
			ctx.allAliases.insert (a.first);

		// the ON conditions, each of which sees only its own join's aliases
		ctx.clause = "ON";
		ctx.aggregatesAllowed = false;
		for (auto a : tablesToProcess) {
			map <string, MyDB_TablePtr> treeAliases;
			err = a->checkOn (ctx, aliases, treeAliases);
			if (err != "")
				return err;
		}

		// from here on, every alias in the FROM clause is visible
		ctx.visible = aliases;

		ctx.clause = "WHERE";
		for (auto a : allDisjunctions) {
			ExprType t = a->check (ctx, err);
			if (t == ExprType :: Error)
				return err;
			if (t != ExprType :: Bool)
				return "the WHERE condition " + a->toString () + " is " + aTypeName (t) + ", not a bool";
		}

		ctx.clause = "GROUP BY";
		for (auto a : groupingClauses) {
			if (a->check (ctx, err) == ExprType :: Error)
				return err;
		}

		// for each selected value, remember the attributes outside aggregates
		ctx.clause = "SELECT";
		ctx.aggregatesAllowed = true;
		vector <vector <pair <string, string>>> outside;
		for (auto a : valuesToSelect) {
			ctx.attsOutsideAggregates.clear ();
			if (a->check (ctx, err) == ExprType :: Error)
				return err;
			outside.push_back (ctx.attsOutsideAggregates);
		}

		// the grouping rule applies if there is a GROUP BY or an aggregate
		if (groupingClauses.empty () && !ctx.sawAggregate)
			return "";

		// a selected value is grouped if it is exactly a GROUP BY expression,
		// or if every attribute in it outside an aggregate is a GROUP BY attribute
		set <string> groupExprs;
		set <pair <string, string>> groupAtts;
		for (auto g : groupingClauses) {
			groupExprs.insert (g->toString ());
			string alias, att;
			if (g->isIdentifier (alias, att))
				groupAtts.insert (make_pair (alias, att));
		}
		for (size_t i = 0; i < valuesToSelect.size (); i++) {
			if (groupExprs.count (valuesToSelect[i]->toString ()) > 0)
				continue;
			for (auto &att : outside[i]) {
				if (groupAtts.count (att) > 0)
					continue;
				string name = att.first + "." + att.second;
				if (groupingClauses.empty ())
					return name + " is selected but is not an aggregate, and the query has aggregates but no GROUP BY";
				return name + " is selected but is not an aggregate and is not in the GROUP BY";
			}
		}
		return "";
	}

	#include "FriendDecls.h"
};

// structure that stores an entire SQL statement
struct SQLStatement {

private:

	// in case we are a SFW query
	SFWQuery myQuery;
	bool isQuery;

	// in case we a re a create table
	CreateTable myTableToCreate;
	bool isCreate;

public:
	SQLStatement (struct SFWQuery* useMe) {
		myQuery = *useMe;
		isQuery = true;
		isCreate = false;
	}

	SQLStatement (struct CreateTable *useMe) {
		myTableToCreate = *useMe;
		isQuery = false;
		isCreate = true;
	}

	bool isCreateTable () {
		return isCreate;
	}

	bool isSFWQuery () {
		return isQuery;
	}

	string addToCatalog (string storageDir, MyDB_CatalogPtr addToMe) {
		return myTableToCreate.addToCatalog (storageDir, addToMe);
	}		
	
	void printSFWQuery () {
		myQuery.print ();
	}

	// checks the SFW query against the catalog's tables; returns "" if it is
	// OK, and otherwise the reason it is not
	string checkSFWQuery (map <string, MyDB_TablePtr> &allTables) {
		return myQuery.check (allTables);
	}

	#include "FriendDecls.h"
};

#endif
