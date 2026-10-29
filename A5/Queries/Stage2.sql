-- Stage 2 tests: the semantic checks.  Run them after CreateTables.sql.
-- The "-- expect:" line before each query gives the verdict a correct
-- checker reports; the comment above it says which rule the query exercises.
-- Every ERROR query breaks exactly one rule.

---------------------------------------------------------------------------
-- references: tables, aliases, attributes
---------------------------------------------------------------------------

-- T1.  WHERE sees an alias inside a join and an alias from a comma item
-- expect: OK
SELECT c.c_name, o.o_totalprice
FROM customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey, nation AS n
WHERE c.c_nationkey = n.n_nationkey AND o.o_orderstatus = "F";

-- T2.  a table that does not exist, on the right of a JOIN
-- expect: ERROR
SELECT n.n_name
FROM nation AS n JOIN regions AS r ON n.n_regionkey = r.r_regionkey;

-- T3.  an alias that is not in the FROM clause, in SELECT
-- expect: ERROR
SELECT x.n_name
FROM nation AS n;

-- T4.  an alias that is not in the FROM clause, in GROUP BY
-- expect: ERROR
SELECT SUM (n.n_nationkey)
FROM nation AS n
WHERE n.n_regionkey > 0
GROUP BY x.n_name;

-- T5.  an alias that is not in the FROM clause, inside SUM
-- expect: ERROR
SELECT SUM (x.l_quantity)
FROM lineitem AS l;

-- T6.  an attribute that exists, but in a different table than the alias's
-- expect: ERROR
SELECT n.r_name
FROM nation AS n, region AS r
WHERE n.n_regionkey = r.r_regionkey;

-- T7.  an attribute the table does not have, in ON
-- expect: ERROR
SELECT o.o_orderkey
FROM orders AS o JOIN customer AS c ON o.c_custkey = c.c_custkey;

-- T8.  an attribute the table does not have, in GROUP BY
-- expect: ERROR
SELECT SUM (s.s_acctbal)
FROM supplier AS s
WHERE s.s_acctbal > 0.0
GROUP BY s.s_nationname;

-- T9.  an attribute the table does not have, inside AVG
-- expect: ERROR
SELECT AVG (ps.ps_cost)
FROM partsupp AS ps;

-- T10.  the same table under two different aliases
-- expect: OK
SELECT n1.n_name, n2.n_name
FROM nation AS n1 JOIN nation AS n2 ON n1.n_regionkey = n2.n_regionkey;

-- T32.  a table that does not exist, inside a parenthesized join
-- expect: ERROR
SELECT c.c_name
FROM customer AS c JOIN (orders AS o JOIN lineitems AS l ON l.l_orderkey = o.o_orderkey)
     ON c.c_custkey = o.o_custkey;

-- T33.  an alias in ON that is nowhere in the FROM clause
-- expect: ERROR
SELECT n.n_name
FROM nation AS n JOIN region AS r ON n.n_regionkey = q.r_regionkey;

-- T34.  an attribute the table does not have, under NOT in WHERE
-- expect: ERROR
SELECT p.p_name
FROM part AS p
WHERE NOT p.p_sizes < 10;


---------------------------------------------------------------------------
-- ON scope
---------------------------------------------------------------------------

-- T11.  the outer ON sees aliases deep in both its left and right subtrees
-- expect: OK
SELECT c.c_name
FROM ((customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey)
      JOIN nation AS n ON c.c_nationkey = n.n_nationkey)
     JOIN (lineitem AS l JOIN supplier AS s ON l.l_suppkey = s.s_suppkey)
     ON l.l_orderkey = o.o_orderkey AND s.s_nationkey = c.c_nationkey;

-- T12.  the first ON of a chain uses a table that is joined later
-- expect: ERROR
SELECT c.c_name
FROM customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey AND l.l_orderkey = o.o_orderkey
     JOIN lineitem AS l ON l.l_orderkey = o.o_orderkey;

-- T13.  an ON uses an alias from an earlier comma-separated item
-- expect: ERROR
SELECT s.s_name
FROM region AS r, supplier AS s JOIN nation AS n
     ON s.s_nationkey = n.n_nationkey AND n.n_regionkey = r.r_regionkey;

-- T14.  the inner ON of the right subtree uses an alias from the left subtree
-- expect: ERROR
SELECT c.c_name
FROM (customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey)
     JOIN (lineitem AS l JOIN part AS p ON l.l_partkey = p.p_partkey AND l.l_orderkey = o.o_orderkey)
     ON l.l_orderkey = o.o_orderkey;

-- T35.  an ON scope violation under NOT, in the second branch of an OR
-- expect: ERROR
SELECT p.p_name
FROM part AS p JOIN partsupp AS ps
     ON p.p_partkey = ps.ps_partkey AND (p.p_size > 5 OR NOT s.s_acctbal < 0.0),
     supplier AS s;

-- T36.  WHERE sees aliases inside a parenthesized right subtree
-- expect: OK
SELECT r.r_name
FROM region AS r JOIN (nation AS n JOIN supplier AS s ON n.n_nationkey = s.s_nationkey)
     ON r.r_regionkey = n.n_regionkey
WHERE s.s_acctbal > 100.0 AND r.r_name = "EUROPE";


---------------------------------------------------------------------------
-- arithmetic
---------------------------------------------------------------------------

-- T15.  int/double arithmetic in SELECT, WHERE, and ON; string
-- concatenation in SELECT and in an ON comparison
-- expect: OK
SELECT l.l_quantity * l.l_extendedprice / 2, o.o_orderstatus + o.o_orderpriority
FROM lineitem AS l JOIN orders AS o
     ON l.l_orderkey = o.o_orderkey AND l.l_shipdate + "x" > o.o_orderdate
WHERE (l.l_quantity - 1) > o.o_totalprice / 100;

-- T16.  number + string, in SELECT
-- expect: ERROR
SELECT p.p_size + p.p_name
FROM part AS p;

-- T17.  string * number, in WHERE
-- expect: ERROR
SELECT c.c_name
FROM customer AS c
WHERE c.c_name * 2 > 10;

-- T18.  string / string, in WHERE
-- expect: ERROR
SELECT r.r_name
FROM region AS r
WHERE r.r_name / r.r_comment = "x";

-- T19.  string + number, in GROUP BY
-- expect: ERROR
SELECT SUM (l.l_quantity)
FROM lineitem AS l
WHERE l.l_quantity > 1
GROUP BY l.l_returnflag + l.l_quantity;

-- T37.  string - string, in ON
-- expect: ERROR
SELECT n.n_name
FROM nation AS n JOIN region AS r ON n.n_name - r.r_name = "x";

-- T38.  an aggregate's result is a number, so it cannot be added to a string
-- (the parentheses are needed because the grammar does not allow SUM (...)
-- before +)
-- expect: ERROR
SELECT (SUM (l.l_quantity)) + "units"
FROM lineitem AS l;

-- T39.  a concatenation's result is a string, so it cannot be multiplied
-- expect: ERROR
SELECT (c.c_name + c.c_phone) * 2
FROM customer AS c;


---------------------------------------------------------------------------
-- comparisons
---------------------------------------------------------------------------

-- T20.  int vs double and string vs string, with <>, NOT, and OR
-- expect: OK
SELECT p.p_name
FROM part AS p
WHERE (p.p_size > p.p_retailprice OR p.p_name = p.p_brand) AND
      NOT p.p_type < "STANDARD" AND p.p_size <> 3;

-- T21.  number <> string, in WHERE
-- expect: ERROR
SELECT p.p_name
FROM part AS p
WHERE p.p_size <> "large";

-- T22.  string < number, in ON
-- expect: ERROR
SELECT s.s_name
FROM supplier AS s JOIN nation AS n ON s.s_name < n.n_nationkey;

-- T23.  string > number, in the first branch of an OR
-- expect: ERROR
SELECT l.l_orderkey
FROM lineitem AS l
WHERE (l.l_shipmode > 3 OR l.l_quantity > 3);

-- T40.  string = number, under NOT in ON
-- expect: ERROR
SELECT c.c_name
FROM customer AS c JOIN orders AS o
     ON c.c_custkey = o.o_custkey AND NOT o.o_orderdate = 1995;

-- T41.  an int attribute compared with a string literal
-- expect: ERROR
SELECT r.r_name
FROM region AS r
WHERE r.r_regionkey = "3";


---------------------------------------------------------------------------
-- aggregates
---------------------------------------------------------------------------

-- T24.  AVG of an int and SUM of a double expression; aggregates only, so
-- no GROUP BY is needed
-- expect: OK
SELECT AVG (l.l_quantity), SUM (l.l_extendedprice * (1 - l.l_discount))
FROM lineitem AS l JOIN orders AS o ON l.l_orderkey = o.o_orderkey
WHERE o.o_orderstatus = "F";

-- T25.  AVG of a string
-- expect: ERROR
SELECT AVG (c.c_name)
FROM customer AS c;

-- T26.  AVG in an ON condition
-- expect: ERROR
SELECT o.o_orderkey
FROM orders AS o JOIN lineitem AS l ON AVG (l.l_quantity) > o.o_totalprice;

-- T27.  SUM nested inside arithmetic in an ON condition (the parentheses
-- are needed because the grammar does not allow SUM (...) before *)
-- expect: ERROR
SELECT o.o_orderkey
FROM orders AS o JOIN lineitem AS l
     ON l.l_orderkey = o.o_orderkey AND o.o_totalprice > (SUM (l.l_extendedprice)) * 2;

-- T42.  SUM of a concatenation, in a correctly grouped query
-- expect: ERROR
SELECT n.n_name, SUM (s.s_name + s.s_phone)
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey
WHERE s.s_acctbal > 0.0
GROUP BY n.n_name;

-- T43.  SUM under NOT in an ON condition
-- expect: ERROR
SELECT c.c_name
FROM customer AS c JOIN orders AS o
     ON c.c_custkey = o.o_custkey AND NOT SUM (o.o_totalprice) < 10.0;


---------------------------------------------------------------------------
-- grouping
---------------------------------------------------------------------------

-- T28.  two grouping attributes, both selected, over joins
-- expect: OK
SELECT n.n_name, r.r_name, SUM (s.s_acctbal)
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey
     JOIN region AS r ON n.n_regionkey = r.r_regionkey
WHERE s.s_acctbal > 0.0
GROUP BY n.n_name, r.r_name;

-- T29.  as T28, but r.r_name is selected and not grouped
-- expect: ERROR
SELECT n.n_name, r.r_name, SUM (s.s_acctbal)
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey
     JOIN region AS r ON n.n_regionkey = r.r_regionkey
WHERE s.s_acctbal > 0.0
GROUP BY n.n_name;

-- T30.  an attribute selected next to an aggregate, with no GROUP BY
-- expect: ERROR
SELECT n.n_name, SUM (s.s_acctbal)
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey;

-- T31.  the ungrouped attribute comes after the aggregate in the SELECT list
-- expect: ERROR
SELECT SUM (l.l_quantity), l.l_linestatus
FROM lineitem AS l
WHERE l.l_quantity > 0
GROUP BY l.l_returnflag;

-- T44.  an ungrouped attribute in arithmetic with an aggregate
-- expect: ERROR
SELECT n.n_name, n.n_nationkey + SUM (s.s_acctbal)
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey
WHERE s.s_acctbal > 0.0
GROUP BY n.n_name;

-- T45.  attributes inside aggregates need no grouping, and the GROUP BY
-- attribute does not have to be selected
-- expect: OK
SELECT SUM (l.l_quantity), AVG (l.l_discount)
FROM lineitem AS l
WHERE l.l_quantity > 0
GROUP BY l.l_returnflag;

---------------------------------------------------------------------------
-- continuing after a parse error
---------------------------------------------------------------------------

-- T46.  a WHERE with no condition
-- expect: PARSE ERROR
SELECT n.n_name
FROM nation AS n
WHERE;

-- T47.  the statement after a parse error is checked normally
-- expect: OK
SELECT n.n_name
FROM nation AS n
WHERE n.n_nationkey > 3;
