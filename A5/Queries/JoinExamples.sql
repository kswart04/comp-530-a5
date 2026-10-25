-- Examples of the JOIN ... ON syntax that you are adding to the parser.
-- These are the same examples that appear in the assignment handout.  As in
-- Test.sql, the line before each query says what a correct checker reports.
-- Run them after CreateTables.sql

-- J1.  a join of two tables
-- expect: OK
SELECT n.n_name, r.r_name
FROM nation AS n JOIN region AS r ON n.n_regionkey = r.r_regionkey;

-- J2.  a join and a comma in the same FROM clause; the WHERE clause sees
-- every table in the FROM clause
-- expect: OK
SELECT s.s_name
FROM supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey, region AS r
WHERE n.n_regionkey = r.r_regionkey AND r.r_name = "AMERICA";

-- J3.  an ON condition can be anything a WHERE condition can be
-- expect: OK
SELECT l.l_orderkey
FROM lineitem AS l JOIN orders AS o
     ON l.l_orderkey = o.o_orderkey AND
        (l.l_shipdate > o.o_orderdate OR l.l_shipdate = o.o_orderdate) AND
        NOT l.l_quantity < 10;

-- J4.  three tables.  JOIN groups to the left, so this is
-- (customer JOIN orders ON ...) JOIN lineitem ON ..., and the second ON
-- can see customer
-- expect: OK
SELECT c.c_name
FROM customer AS c JOIN orders AS o ON c.c_custkey = o.o_custkey
     JOIN lineitem AS l ON l.l_orderkey = o.o_orderkey AND c.c_nationkey < l.l_suppkey;

-- J5.  parentheses change the grouping
-- expect: OK
SELECT c.c_name
FROM customer AS c JOIN (orders AS o JOIN lineitem AS l ON l.l_orderkey = o.o_orderkey)
     ON c.c_custkey = o.o_custkey;

-- J6.  an ON condition may refer only to the tables on its two sides.  The
-- inner ON here refers to c, which is outside the parentheses
-- expect: ERROR
SELECT c.c_name
FROM customer AS c JOIN (orders AS o JOIN lineitem AS l
     ON l.l_orderkey = o.o_orderkey AND c.c_custkey = o.o_custkey)
     ON c.c_custkey = o.o_custkey;

-- J7.  the same rule, with a comma: r is not part of the join
-- expect: ERROR
SELECT s.s_name
FROM supplier AS s JOIN nation AS n ON n.n_regionkey = r.r_regionkey, region AS r;

-- J8.  an ON condition is type-checked just like a WHERE condition
-- expect: ERROR
SELECT n.n_name
FROM nation AS n JOIN region AS r ON n.n_name = r.r_regionkey;

-- J9.  no aggregates in an ON condition
-- expect: ERROR
SELECT n.n_name
FROM nation AS n JOIN region AS r ON SUM (n.n_regionkey) = r.r_regionkey;

-- J10.  every JOIN has an ON
-- expect: PARSE ERROR
SELECT n.n_name
FROM nation AS n JOIN region AS r;

-- J11.  ON belongs to a JOIN; it cannot follow a comma
-- expect: PARSE ERROR
SELECT n.n_name
FROM nation AS n, region AS r ON n.n_regionkey = r.r_regionkey;

-- J12.  joins nest as deeply as you like
-- expect: OK
SELECT s.s_name, SUM (ps.ps_supplycost)
FROM ((supplier AS s JOIN nation AS n ON s.s_nationkey = n.n_nationkey)
      JOIN region AS r ON n.n_regionkey = r.r_regionkey)
     JOIN (partsupp AS ps JOIN part AS p ON ps.ps_partkey = p.p_partkey)
     ON s.s_suppkey = ps.ps_suppkey
WHERE p.p_size > 10
GROUP BY s.s_name;
