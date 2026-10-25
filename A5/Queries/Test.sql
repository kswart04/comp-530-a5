-- The seventeen test queries.  The "-- expect:" line before each query says
-- what a correct checker reports for it: OK, ERROR, or PARSE ERROR, and the
-- Queries/checkQueries.py script reads these lines and compares them with
-- what your checker prints.  The "-- a correct checker prints:" line is a
-- sample of the whole verdict line, so you can see what each query is
-- there to show; only the first word is graded, and your wording is yours

-- 1.
-- expect: OK
-- a correct checker prints: OK
SELECT n.n_name, SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l, supplier AS s, nation AS n, region AS r
WHERE c.c_custkey = o.o_custkey AND l.l_orderkey = o.o_orderkey AND
      l.l_suppkey = s.s_suppkey AND c.c_nationkey = s.s_nationkey AND
      s.s_nationkey = n.n_nationkey AND n.n_regionkey = r.r_regionkey AND
      r.r_name = "region" AND
      (o.o_orderdate > "date1" OR o.o_orderdate = "date1") AND
      NOT o.o_orderdate < "date2"
GROUP BY n.n_name;

-- 2.  s.s_suppkey is selected but not grouped on
-- expect: ERROR
-- a correct checker prints: ERROR: s.s_suppkey is selected but is not an aggregate and is not in the GROUP BY
SELECT n.n_name, s.s_suppkey, SUM(l.l_extendedprice)
FROM nation AS n, supplier AS s, lineitem AS l
WHERE l.l_suppkey = s.s_suppkey AND s.s_nationkey = n.n_nationkey
GROUP BY n.n_name;

-- 3.  a number is compared with a string
-- expect: ERROR
-- a correct checker prints: ERROR: comparing a number with a string in l.l_extendedprice * (1 - l.l_discount) > "a string"
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM lineitem AS l
WHERE l.l_extendedprice * (1 - l.l_discount) > "a string";

-- 4.  SUM of a string
-- expect: ERROR
-- a correct checker prints: ERROR: SUM of a string: l.l_returnflag + l.l_shipdate + l.l_commitdate
SELECT SUM(l.l_returnflag + l.l_shipdate + l.l_commitdate)
FROM lineitem AS l;

-- 5.  string concatenation is fine
-- expect: OK
-- a correct checker prints: OK
SELECT l.l_returnflag + l.l_shipdate + l.l_commitdate
FROM lineitem AS l
WHERE l.l_shipdate > "a date";

-- 6.  a number is compared with a string, inside an OR
-- expect: ERROR
-- a correct checker prints: ERROR: comparing a number with a string in l.l_tax + l.l_discount > "date2"
SELECT n.n_name, SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l, supplier AS s, nation AS n, region AS r
WHERE c.c_custkey = o.o_custkey AND l.l_orderkey = o.o_orderkey AND
      l.l_suppkey = s.s_suppkey AND c.c_nationkey = s.s_nationkey AND
      s.s_nationkey = n.n_nationkey AND n.n_regionkey = r.r_regionkey AND
      r.r_name = "region" AND
      (l.l_tax + l.l_discount > "date2" OR o.o_orderdate = "date1") AND
      NOT o.o_orderdate < "date2"
GROUP BY n.n_name;

-- 7.  orders has no attribute k_orderkey
-- expect: ERROR
-- a correct checker prints: ERROR: table orders (alias o) has no attribute k_orderkey
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND l.l_orderkey = o.k_orderkey;

-- 8.  there is no table with alias h
-- expect: ERROR
-- a correct checker prints: ERROR: no table in the FROM clause has alias h (in h.o_custkey)
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = h.o_custkey AND l.l_orderkey = o.o_orderkey;

-- 9.  a string is subtracted from a number
-- expect: ERROR
-- a correct checker prints: ERROR: arithmetic on a number and a string in 15.0 - "here is another"
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND
      "this is a string" + "this is another string" > 15.0 - "here is another" AND
      l.l_orderkey = o.o_orderkey;

-- 10.  there is no table customers
-- expect: ERROR
-- a correct checker prints: ERROR: table customers does not exist
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customers AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND l.l_orderkey = o.o_orderkey;

-- 11.
-- expect: OK
-- a correct checker prints: OK
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND l.l_orderkey = o.o_orderkey AND
      1200 + l.l_quantity / (300.0 + 34) = 33;

-- 12.
-- expect: OK
-- a correct checker prints: OK
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND
      "this is a string" + "this is another string" > "here is another" AND
      l.l_orderkey = o.o_orderkey;

-- 13.  customer has no attribute l_extendedprice
-- expect: ERROR
-- a correct checker prints: ERROR: table customer (alias c) has no attribute l_extendedprice
SELECT SUM(l.l_extendedprice * (1 - l.l_discount)), SUM(c.l_extendedprice * 1.01)
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND l.l_orderkey = o.o_orderkey;

-- 14.
-- expect: OK
-- a correct checker prints: OK
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND
      "1204" + "this is a string" = l.l_returnflag AND
      l.l_tax + l.l_discount + l.l_extendedprice > 3.27 AND
      l.l_orderkey = o.o_orderkey;

-- 15.  a number is compared with a string, in the second half of an OR
-- expect: ERROR
-- a correct checker prints: ERROR: comparing a number with a string in l.l_discount + l.l_extendedprice > "327"
SELECT SUM(l.l_extendedprice * (1 - l.l_discount))
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND
      (l.l_tax + l.l_discount + l.l_extendedprice > 3.27 OR
       l.l_discount + l.l_extendedprice > "327") AND
      l.l_orderkey = o.o_orderkey;

-- 16.
-- expect: OK
-- a correct checker prints: OK
SELECT SUM(l.l_extendedprice * (1 - l.l_discount)), SUM(1 - l.l_discount)
FROM customer AS c, orders AS o, lineitem AS l
WHERE c.c_custkey = o.o_custkey AND
      (l.l_tax + l.l_discount + l.l_extendedprice > 3.27 OR
       "here is a string" > "327") AND
      l.l_orderkey = o.o_orderkey;

-- 17.
-- expect: OK
-- a correct checker prints: OK
SELECT SUM(12 + 13.4 + 19), SUM(l.l_extendedprice)
FROM lineitem AS l;
