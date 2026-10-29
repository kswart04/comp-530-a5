# A5 reflection

Half a page, written after Stage 3.

## What did you have to understand about the parser before you could write the Stage 1 prompt, and how did you find it out?
I needed to understand how FROM lists and WHERE conditions were represented, how grammmar actions built the parse tree, and how left recursion establishes join associations. I worked this out form the assignment specification, as well as discussions with the AI. The JOIN examples helped clarify why we needed to preserve each join's operands and ON condition for later scope checking.

## Which rule was hardest to infer from the seventeen queries, and what settled it?
Grouping was the most difficult rule, especially when an aggregate appears without GROUP BY. Comparing query 2 with the valid aggregate-only queries clarified that attrbutes selected outside aggregates must be grouped, while attributes nside aggregate arguments do not. The assignment also identified expression-level grouping as judgment calls, which I had to discuss with the AI. 

## Which of your ten breaking changes did none of your existing queries catch, and what did that tell you about your Stage 2 tests?
Accepting a nonexstent FROM table was initially unchaught. Every exisiting missing-table query also reference an attribute through that table's alias, so attribute checking still produced ERROR. This showed that my tests did not isolate table existence. I added U1, which includes a nonexistent table ithout using its alias, and it caught the mutation.

## What did the AI get wrong, and how did you find out?
Claude proposed an aggregate-in-ON test using SUM()*2, which the supplied grammr could not parse in that form. Running the proposed tests through the stage 1 Parser exposed the problem before semantic checking was implemented. Parenthesizing the aggregate made the query parse and test the intended semantic error. This reinforced the need to verify generated tests against the actual grammar.