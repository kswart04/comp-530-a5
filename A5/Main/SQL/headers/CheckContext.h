
#ifndef CHECK_CONTEXT_H
#define CHECK_CONTEXT_H

#include "MyDB_Table.h"
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// the type of a checked expression; Error means that a check failed, and the
// message is in the string passed to check ()
enum class ExprType { Int, Double, String, Bool, Error };

// returns the name of a type, for error messages
inline string typeName (ExprType t) {
	switch (t) {
		case ExprType :: Int: return "int";
		case ExprType :: Double: return "double";
		case ExprType :: String: return "string";
		case ExprType :: Bool: return "bool";
		default: return "error";
	}
}

// returns the name of a type with "a" or "an" in front of it
inline string aTypeName (ExprType t) {
	if (t == ExprType :: Int)
		return "an int";
	return "a " + typeName (t);
}

inline bool isNumeric (ExprType t) {
	return t == ExprType :: Int || t == ExprType :: Double;
}

// this is everything that an expression needs to know to check itself
struct CheckContext {

	// the aliases that the current clause can see, and their tables
	map <string, MyDB_TablePtr> visible;

	// every alias in the FROM clause (used to explain an ON scope error)
	set <string> allAliases;

	// the clause being checked: "SELECT", "WHERE", "GROUP BY", or "ON"
	string clause;

	// true if aggregates may appear in the current clause
	bool aggregatesAllowed = false;

	// true while checking the argument of an aggregate
	bool insideAggregate = false;

	// set to true when an aggregate is found
	bool sawAggregate = false;

	// the (alias, attribute) pairs found outside of any aggregate
	vector <pair <string, string>> attsOutsideAggregates;
};

#endif
