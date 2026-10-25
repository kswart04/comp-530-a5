
#ifndef SQL_HEADERS_H
#define SQL_HEADERS_H

//#include "Parser.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************/
// VARIOUS STRUCTURES ASSOCIATED WITH PRODCUTION RULES
/******************************************************/

// a "Value" is something that can be computed... example:
// 12.45, or
// tableOne.value + 34.5, or
// (tableOne.value + tableTwo.value) / 34.56) 
struct Value;

// a "ValueList" is a list of these guys... used to hold everything we need
// in a GROUP BY or a SELECT clause
struct ValueList;

// a "CNF" is a list of boolean clauses
struct CNF;

// this is a general SQL statement
struct SQLStatement;

// this is a select-from-where query
struct SFWQuery;

// this is a create table statement
struct CreateTable;

// this is a list of attribute name, type pairs
struct AttList;

// this is a list of FROM items, each a tree of tables and joins
struct FromList;

// this is one FROM item: either a table and its alias, or a join
struct FromItem;

/******************************************************/
// C FUNCTIONS TO MANIPULATE THE VARIOUS STRUCTURES
/******************************************************/

// build a select query
struct SFWQuery *makeQueryWithGroupBy (struct ValueList *selectClause, struct FromList *fromClause, 
	struct CNF *cnf, struct ValueList *grouping);
struct SFWQuery *makeQuery (struct ValueList *selectClause, struct FromList *fromClause, struct CNF *cnf);
struct SFWQuery *makeQueryNoWhere (struct ValueList *selectClause, struct FromList *fromClause);

// builds an SQL statement out of a select query
struct SQLStatement *makeSelectQuery (struct SFWQuery *fromMe);

// builds an SQL statement out of a create table
struct SQLStatement *makeCreateTable (struct CreateTable *fromMe);

// makes a regular database table
struct CreateTable *makeTableRegular (char *tableName, struct AttList *fromMe);

// makes a B+-Tree table
struct CreateTable *makeTableBPlusTree (char *tableName, struct AttList *fromMe, char *attName);

// makes an attribute list out of a single attribute
struct AttList *makeAttList (char *attName, int whichType);

// makes a FROM item out of a table name and its alias
struct FromItem *makeFromTable (char *tableName, char *aliasName);

// makes a FROM item that joins two FROM items using an ON condition
struct FromItem *makeJoin (struct FromItem *left, struct FromItem *right, struct CNF *onClause);

// makes a from list out of a single FROM item
struct FromList *makeFromList (struct FromItem *fromMe);

// appends a new FROM item to the from list
struct FromList *appendFromList (struct FromList *appendToMe, struct FromItem *appendMe);

// appends an attribute to the end of a list
struct AttList *appendAttList (struct AttList *appendToMe, struct AttList *appendMe);

// construct a new value using a boolean operation
struct Value *nott (struct Value *ofMe);
struct Value *orr (struct Value *lhs, struct Value *rhs);

// construct a new value using a binary comparison operation
struct Value *gt (struct Value *lhs, struct Value *rhs);
struct Value *lt (struct Value *lhs, struct Value *rhs);
struct Value *neq (struct Value *lhs, struct Value *rhs);
struct Value *eq (struct Value *lhs, struct Value *rhs);

// construct a new value using an aggregate function
struct Value *sum (struct Value *ofMe);
struct Value *avg (struct Value *ofMe);

// construct a new value using a binary arithmatic operation
struct Value *times (struct Value *lhs, struct Value *rhs);
struct Value *plus (struct Value *lhs, struct Value *rhs);
struct Value *divide (struct Value *lhs, struct Value *rhs);
struct Value *minus (struct Value *lhs, struct Value *rhs);

// construct a new value using a unary arithmatic operation
struct Value *unaryMinus (struct Value *toMe);

// construct a new value from an atomic input
struct Value *makeIdentifier (char *tableName, char *attName);
struct Value *makeDouble (double fromMe);
struct Value *makeInt (int fromMe);
struct Value *makeString (char *fromMe);

// this adds a new value to a value list
struct ValueList *pushBackValue (struct ValueList *addToMe, struct Value *addMe);

// makes a new value list from a value
struct ValueList *makeValueList (struct Value *addMe);

// makes a new CNF from a expression (hopefully a boolean!!)
struct CNF *makeCNF (struct Value *fromMe);

// push a new CNF onto the back
struct CNF *pushBackDisjunction (struct CNF *ontoMe, struct Value *pushMe);

#ifdef __cplusplus
}
#endif

#endif
