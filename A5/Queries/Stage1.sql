-- Stage 1 tests: the JOIN ... ON grammar.  Run them after CreateTables.sql.
-- The "-- expect:" line before each query gives the eventual semantic verdict,
-- so a query that parses but breaks a Stage 2 rule is marked ERROR.  In
-- Stage 1 these are run with checkQueries.py --parse-only, which only checks
-- what parses and what does not.

---------------------------------------------------------------------------
-- a) basic joins and left-associated join chains
---------------------------------------------------------------------------

-- S1.  a join of two tables
-- expect: OK
SELECT o.o_orderkey, c.c_name
FROM orders AS o JOIN customer AS c ON o.o_custkey = c.c_custkey;

-- S2.  a chain of three tables groups to the left, so the second ON can
-- see p from the first join
-- expect: OK
SELECT p.p_name, s.s_name
FROM part AS p JOIN partsupp AS ps ON p.p_partkey = ps.ps_partkey
     JOIN supplier AS s ON ps.ps_suppkey = s.s_suppkey AND p.p_retailprice > s.s_acctbal;

-- S3.  a chain of four tables
-- expect: OK
SELECT c.c_name, s.s_name
FROM customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey
     JOIN lineitem AS l ON l.l_orderkey = o.o_orderkey
     JOIN supplier AS s ON l.l_suppkey = s.s_suppkey AND c.c_nationkey = s.s_nationkey;

-- S4.  a join followed by WHERE and GROUP BY
-- expect: OK
SELECT n.n_name, SUM (s.s_acctbal)
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey
WHERE s.s_acctbal > 0.0
GROUP BY n.n_name;

-- S5.  GROUP BY is still only accepted after a WHERE clause, joins or not
-- expect: PARSE ERROR
SELECT n.n_name, SUM (s.s_acctbal)
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey
GROUP BY n.n_name;

---------------------------------------------------------------------------
-- b) parenthesized and nested joins
---------------------------------------------------------------------------

-- S6.  parentheses group the join to the right
-- expect: OK
SELECT r.r_name
FROM region AS r JOIN (nation AS n JOIN supplier AS s ON n.n_nationkey = s.s_nationkey)
     ON r.r_regionkey = n.n_regionkey;

-- S7.  a whole join inside parentheses
-- expect: OK
SELECT p.p_name
FROM (part AS p JOIN partsupp AS ps ON p.p_partkey = ps.ps_partkey);

-- S8.  two parenthesized joins joined to each other
-- expect: OK
SELECT c.c_name, p.p_name
FROM (customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey)
     JOIN (lineitem AS l JOIN part AS p ON l.l_partkey = p.p_partkey)
     ON l.l_orderkey = o.o_orderkey;

-- S9.  unbalanced parentheses
-- expect: PARSE ERROR
SELECT n.n_name
FROM (nation AS n JOIN region AS r ON n.n_regionkey = r.r_regionkey;

-- S10.  the inner join has its ON, but the outer JOIN does not
-- expect: PARSE ERROR
SELECT r.r_name
FROM region AS r JOIN (nation AS n JOIN supplier AS s ON n.n_nationkey = s.s_nationkey);

---------------------------------------------------------------------------
-- c) joins mixed with comma-separated tables
---------------------------------------------------------------------------

-- S11.  a join, then a table
-- expect: OK
SELECT c.c_name
FROM customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey, nation AS n
WHERE c.c_nationkey = n.n_nationkey;

-- S12.  a table, then a join
-- expect: OK
SELECT s.s_name
FROM region AS r, nation AS n JOIN supplier AS s ON n.n_nationkey = s.s_nationkey
WHERE r.r_regionkey = n.n_regionkey;

-- S13.  a join, then another join
-- expect: OK
SELECT p.p_name, s.s_name
FROM part AS p JOIN partsupp AS ps ON p.p_partkey = ps.ps_partkey,
     supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey
WHERE ps.ps_suppkey = s.s_suppkey;

-- S14.  a plain comma list with no joins still parses
-- expect: OK
SELECT c.c_name, n.n_name
FROM customer AS c, orders AS o, nation AS n, region AS r
WHERE c.c_custkey = o.o_custkey AND c.c_nationkey = n.n_nationkey AND
      n.n_regionkey = r.r_regionkey;

-- S15.  ON belongs to a JOIN; it cannot follow a comma
-- expect: PARSE ERROR
SELECT n.n_name
FROM nation AS n, region AS r ON r.r_regionkey = n.n_regionkey;

---------------------------------------------------------------------------
-- d) ON conditions exercising WHERE syntax
---------------------------------------------------------------------------

-- S16.  several ANDs, a parenthesized OR, and NOT
-- expect: OK
SELECT l.l_orderkey
FROM lineitem AS l JOIN partsupp AS ps
     ON l.l_partkey = ps.ps_partkey AND l.l_suppkey = ps.ps_suppkey AND
        (l.l_quantity > ps.ps_availqty OR l.l_quantity = ps.ps_availqty) AND
        NOT l.l_extendedprice < ps.ps_supplycost;

-- S17.  an ON condition that starts with a parenthesized disjunction
-- expect: OK
SELECT o.o_orderkey
FROM orders AS o JOIN customer AS c
     ON (o.o_custkey = c.c_custkey OR o.o_totalprice > c.c_acctbal) AND
        o.o_orderstatus = "F";

-- S18.  an aggregate in an ON condition parses (it is a Stage 2 error)
-- expect: ERROR
SELECT o.o_orderkey
FROM orders AS o JOIN lineitem AS l ON SUM (l.l_extendedprice) > o.o_totalprice;

-- S19.  the inner ON refers to r, which is outside the parentheses; this
-- parses (it is a Stage 2 error)
-- expect: ERROR
SELECT r.r_name
FROM region AS r JOIN (nation AS n JOIN supplier AS s
     ON n.n_nationkey = s.s_nationkey AND r.r_regionkey = n.n_regionkey)
     ON r.r_regionkey = n.n_regionkey;

-- S20.  an ON with no condition
-- expect: PARSE ERROR
SELECT n.n_name
FROM nation AS n JOIN region AS r ON
WHERE n.n_nationkey > 1;

---------------------------------------------------------------------------
-- reserved keywords
---------------------------------------------------------------------------

-- S21.  JOIN is a reserved keyword, so it cannot be used as an alias
-- expect: PARSE ERROR
SELECT n.n_name
FROM nation AS n, region AS join;
