
#ifndef SQL_EXPRESSIONS
#define SQL_EXPRESSIONS

#include "CheckContext.h"
#include "MyDB_AttType.h"
#include <string>
#include <vector>

// create a smart pointer for database tables
using namespace std;
class ExprTree;
typedef shared_ptr <ExprTree> ExprTreePtr;

// class ExprTree is a pure virtual class... the various classes that implement it are below
class ExprTree {

public:

	// prints the expression, for debugging
	virtual string toString () = 0;

	// checks the expression, returning its type; on a failed check, returns
	// ExprType :: Error and puts the reason for the first failure in err
	virtual ExprType check (CheckContext &ctx, string &err) = 0;

	// returns true (and the alias and attribute) if this is an identifier
	virtual bool isIdentifier (string &alias, string &att) {
		return false;
	}

	virtual ~ExprTree () {}
};

// checks the operands of an arithmetic operator.  Two numbers give an int if
// both are ints, and a double otherwise; two strings are allowed only when
// allowStrings is true (for +, which is concatenation)
inline ExprType checkArithmetic (ExprTreePtr lhs, ExprTreePtr rhs, string op, ExprTree *me,
	bool allowStrings, CheckContext &ctx, string &err) {

	ExprType l = lhs->check (ctx, err);
	if (l == ExprType :: Error)
		return l;
	ExprType r = rhs->check (ctx, err);
	if (r == ExprType :: Error)
		return r;

	if (isNumeric (l) && isNumeric (r)) {
		if (l == ExprType :: Double || r == ExprType :: Double)
			return ExprType :: Double;
		return ExprType :: Int;
	}
	if (allowStrings && l == ExprType :: String && r == ExprType :: String)
		return ExprType :: String;

	if (l == ExprType :: String && r == ExprType :: String)
		err = "arithmetic operator " + op + " on two strings (only + is allowed on strings) in " + me->toString ();
	else
		err = "arithmetic operator " + op + " on " + aTypeName (l) + " and " + aTypeName (r) + " in " + me->toString ();
	return ExprType :: Error;
}

// checks the operands of a comparison: two numbers or two strings may be
// compared, and two bools may be compared with = or <> (equalityOnly)
inline ExprType checkComparison (ExprTreePtr lhs, ExprTreePtr rhs, string op, ExprTree *me,
	bool equalityOnly, CheckContext &ctx, string &err) {

	ExprType l = lhs->check (ctx, err);
	if (l == ExprType :: Error)
		return l;
	ExprType r = rhs->check (ctx, err);
	if (r == ExprType :: Error)
		return r;

	if ((isNumeric (l) && isNumeric (r)) || (l == ExprType :: String && r == ExprType :: String))
		return ExprType :: Bool;
	if (equalityOnly && l == ExprType :: Bool && r == ExprType :: Bool)
		return ExprType :: Bool;

	err = "comparing " + aTypeName (l) + " with " + aTypeName (r) + " using " + op + " in " + me->toString ();
	return ExprType :: Error;
}

// checks an aggregate: it must be allowed in the current clause, must not be
// inside another aggregate, and its argument must be a number.  SUM keeps the
// type of its argument; AVG is a double
inline ExprType checkAggregate (ExprTreePtr child, string name, ExprTree *me, bool isAvg,
	CheckContext &ctx, string &err) {

	bool wasInside = ctx.insideAggregate;
	ctx.insideAggregate = true;
	ExprType c = child->check (ctx, err);
	ctx.insideAggregate = wasInside;
	if (c == ExprType :: Error)
		return c;

	if (!ctx.aggregatesAllowed) {
		err = "aggregate " + me->toString () + " is not allowed in " +
			(ctx.clause == "ON" ? "an ON condition" : "the " + ctx.clause + " clause");
		return ExprType :: Error;
	}
	if (wasInside) {
		err = "aggregate " + me->toString () + " is nested inside another aggregate";
		return ExprType :: Error;
	}
	if (!isNumeric (c)) {
		err = name + " of " + aTypeName (c) + ": " + child->toString ();
		return ExprType :: Error;
	}

	ctx.sawAggregate = true;
	if (isAvg)
		return ExprType :: Double;
	return c;
}

class BoolLiteral : public ExprTree {

private:
	bool myVal;
public:
	
	BoolLiteral (bool fromMe) {
		myVal = fromMe;
	}

	string toString () {
		if (myVal) {
			return "bool[true]";
		} else {
			return "bool[false]";
		}
	}	

	ExprType check (CheckContext &, string &) {
		return ExprType :: Bool;
	}
};

class DoubleLiteral : public ExprTree {

private:
	double myVal;
public:

	DoubleLiteral (double fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "double[" + to_string (myVal) + "]";
	}	

	ExprType check (CheckContext &, string &) {
		return ExprType :: Double;
	}

	~DoubleLiteral () {}
};

class IntLiteral : public ExprTree {

private:
	int myVal;
public:

	IntLiteral (int fromMe) {
		myVal = fromMe;
	}

	string toString () {
		return "int[" + to_string (myVal) + "]";
	}

	ExprType check (CheckContext &, string &) {
		return ExprType :: Int;
	}

	~IntLiteral () {}
};

class StringLiteral : public ExprTree {

private:
	string myVal;
public:

	StringLiteral (char *fromMe) {
		fromMe[strlen (fromMe) - 1] = 0;
		myVal = string (fromMe + 1);
	}

	string toString () {
		return "string[" + myVal + "]";
	}

	ExprType check (CheckContext &, string &) {
		return ExprType :: String;
	}

	~StringLiteral () {}
};

class Identifier : public ExprTree {

private:
	string tableName;
	string attName;
public:

	Identifier (char *tableNameIn, char *attNameIn) {
		tableName = string (tableNameIn);
		attName = string (attNameIn);
	}

	Identifier (string tableNameIn, string attNameIn) {
		tableName = tableNameIn;
		attName = attNameIn;
	}

	string toString () {
		return "[" + tableName + "_" + attName + "]";
	}	

	ExprType check (CheckContext &ctx, string &err) {

		string name = tableName + "." + attName;

		// find the table that the alias refers to
		auto found = ctx.visible.find (tableName);
		if (found == ctx.visible.end ()) {
			if (ctx.allAliases.count (tableName) > 0)
				err = "alias " + tableName + " is in the FROM clause but is not on either side of this JOIN, " +
					"so its ON condition cannot use it (in " + name + ")";
			else
				err = "no table in the FROM clause has alias " + tableName + " (in " + name + ")";
			return ExprType :: Error;
		}

		// and find the attribute in that table
		MyDB_AttTypePtr attType = nullptr;
		for (auto &a : found->second->getSchema ()->getAtts ()) {
			if (a.first == attName) {
				attType = a.second;
				break;
			}
		}
		if (attType == nullptr) {
			err = "table " + found->second->getName () + " (alias " + tableName + ") has no attribute " + attName;
			return ExprType :: Error;
		}

		if (!ctx.insideAggregate)
			ctx.attsOutsideAggregates.push_back (make_pair (tableName, attName));

		if (attType->isBool ())
			return ExprType :: Bool;
		if (attType->promotableToInt ())
			return ExprType :: Int;
		if (attType->promotableToDouble ())
			return ExprType :: Double;
		return ExprType :: String;
	}

	bool isIdentifier (string &alias, string &att) {
		alias = tableName;
		att = attName;
		return true;
	}

	~Identifier () {}
};

class MinusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	MinusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "- (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkArithmetic (lhs, rhs, "-", this, false, ctx, err);
	}

	~MinusOp () {}
};

class PlusOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	PlusOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "+ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkArithmetic (lhs, rhs, "+", this, true, ctx, err);
	}

	~PlusOp () {}
};

class TimesOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	TimesOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "* (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkArithmetic (lhs, rhs, "*", this, false, ctx, err);
	}

	~TimesOp () {}
};

class DivideOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	DivideOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "/ (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkArithmetic (lhs, rhs, "/", this, false, ctx, err);
	}

	~DivideOp () {}
};

class GtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	GtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "> (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	

	ExprType check (CheckContext &ctx, string &err) {
		return checkComparison (lhs, rhs, ">", this, false, ctx, err);
	}

	~GtOp () {}
};

class LtOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	LtOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "< (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkComparison (lhs, rhs, "<", this, false, ctx, err);
	}

	~LtOp () {}
};

class NeqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	NeqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "!= (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkComparison (lhs, rhs, "<>", this, true, ctx, err);
	}

	~NeqOp () {}
};

class OrOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	OrOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "|| (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		ExprType l = lhs->check (ctx, err);
		if (l == ExprType :: Error)
			return l;
		ExprType r = rhs->check (ctx, err);
		if (r == ExprType :: Error)
			return r;
		if (l != ExprType :: Bool || r != ExprType :: Bool) {
			err = "OR needs two boolean operands, but got " + aTypeName (l) + " and " + aTypeName (r) +
				" in " + toString ();
			return ExprType :: Error;
		}
		return ExprType :: Bool;
	}

	~OrOp () {}
};

class EqOp : public ExprTree {

private:

	ExprTreePtr lhs;
	ExprTreePtr rhs;
	
public:

	EqOp (ExprTreePtr lhsIn, ExprTreePtr rhsIn) {
		lhs = lhsIn;
		rhs = rhsIn;
	}

	string toString () {
		return "== (" + lhs->toString () + ", " + rhs->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkComparison (lhs, rhs, "=", this, true, ctx, err);
	}

	~EqOp () {}
};

class NotOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	NotOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "!(" + child->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		ExprType c = child->check (ctx, err);
		if (c == ExprType :: Error)
			return c;
		if (c != ExprType :: Bool) {
			err = "NOT needs a boolean operand, but got " + aTypeName (c) + " in " + toString ();
			return ExprType :: Error;
		}
		return ExprType :: Bool;
	}

	~NotOp () {}
};

class SumOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	SumOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "sum(" + child->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkAggregate (child, "SUM", this, false, ctx, err);
	}

	~SumOp () {}
};

class AvgOp : public ExprTree {

private:

	ExprTreePtr child;
	
public:

	AvgOp (ExprTreePtr childIn) {
		child = childIn;
	}

	string toString () {
		return "avg(" + child->toString () + ")";
	}	

	ExprType check (CheckContext &ctx, string &err) {
		return checkAggregate (child, "AVG", this, true, ctx, err);
	}

	~AvgOp () {}
};

#endif
