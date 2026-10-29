-- Stage 3 tests: queries added during mutation testing, each one catching a
-- breaking change that no query in Test.sql, JoinExamples.sql, Stage1.sql,
-- or Stage2.sql catches.  Run them after CreateTables.sql.  The "-- expect:"
-- line before each query gives the verdict a correct checker reports.

-- U1.  catches mutation 1 (a nonexistent table is accepted).  The table
-- regions does not exist, and its alias r is never used, so the error can
-- only come from the table-existence check (in Test.sql query 10, Stage2.sql
-- T2, and Stage2.sql T32, the missing table's alias is also used, so the
-- attribute check reports an error even when the existence check is broken)
-- expect: ERROR
SELECT n.n_name
FROM nation AS n, regions AS r
WHERE n.n_nationkey > 1;
