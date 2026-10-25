/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTEGER = 258,
     IDENTIFIER = 259,
     DBL = 260,
     STR = 261,
     SELECT = 262,
     FROM = 263,
     JOIN = 264,
     WHERE = 265,
     AS = 266,
     BY = 267,
     AND = 268,
     OR = 269,
     NOT = 270,
     SUM = 271,
     AVG = 272,
     GROUP = 273,
     INT = 274,
     BOOL = 275,
     BPLUSTREE = 276,
     CREATE = 277,
     DOUBLE = 278,
     STRING = 279,
     ON = 280,
     TABLE = 281
   };
#endif
/* Tokens.  */
#define INTEGER 258
#define IDENTIFIER 259
#define DBL 260
#define STR 261
#define SELECT 262
#define FROM 263
#define JOIN 264
#define WHERE 265
#define AS 266
#define BY 267
#define AND 268
#define OR 269
#define NOT 270
#define SUM 271
#define AVG 272
#define GROUP 273
#define INT 274
#define BOOL 275
#define BPLUSTREE 276
#define CREATE 277
#define DOUBLE 278
#define STRING 279
#define ON 280
#define TABLE 281




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 13 "/Users/keanu/Documents/Rice University/COMP 530 - Database System Implementation/Assignment 5/comp-530-a5/A5/Main/SQL/source/Parser.y"
{
	struct SQLStatement *myStatement;
	struct SFWQuery *mySelectQuery;
	struct CreateTable *myCreateTable;
	struct FromList *myFromList;
	struct FromItem *myFromItem;
	struct AttList *myAttList;
	struct Value *myValue;
	struct ValueList *allValues;
	struct CNF *myCNF;	
	int myInt;
	char *myChar;
	double myDouble;
}
/* Line 1529 of yacc.c.  */
#line 116 "generated/Parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



