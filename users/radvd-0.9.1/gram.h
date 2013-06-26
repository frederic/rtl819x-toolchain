/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     T_INTERFACE = 258,
     T_PREFIX = 259,
     T_ROUTE = 260,
     STRING = 261,
     NUMBER = 262,
     SIGNEDNUMBER = 263,
     DECIMAL = 264,
     SWITCH = 265,
     IPV6ADDR = 266,
     INFINITY = 267,
     T_IgnoreIfMissing = 268,
     T_AdvSendAdvert = 269,
     T_MaxRtrAdvInterval = 270,
     T_MinRtrAdvInterval = 271,
     T_MinDelayBetweenRAs = 272,
     T_AdvManagedFlag = 273,
     T_AdvOtherConfigFlag = 274,
     T_AdvLinkMTU = 275,
     T_AdvReachableTime = 276,
     T_AdvRetransTimer = 277,
     T_AdvCurHopLimit = 278,
     T_AdvDefaultLifetime = 279,
     T_AdvDefaultPreference = 280,
     T_AdvSourceLLAddress = 281,
     T_AdvOnLink = 282,
     T_AdvAutonomous = 283,
     T_AdvValidLifetime = 284,
     T_AdvPreferredLifetime = 285,
     T_AdvRouterAddr = 286,
     T_AdvHomeAgentFlag = 287,
     T_AdvIntervalOpt = 288,
     T_AdvHomeAgentInfo = 289,
     T_Base6to4Interface = 290,
     T_UnicastOnly = 291,
     T_HomeAgentPreference = 292,
     T_HomeAgentLifetime = 293,
     T_AdvRoutePreference = 294,
     T_AdvRouteLifetime = 295,
     T_AdvMobRtrSupportFlag = 296,
     T_BAD_TOKEN = 297
   };
#endif
#define T_INTERFACE 258
#define T_PREFIX 259
#define T_ROUTE 260
#define STRING 261
#define NUMBER 262
#define SIGNEDNUMBER 263
#define DECIMAL 264
#define SWITCH 265
#define IPV6ADDR 266
#define INFINITY 267
#define T_IgnoreIfMissing 268
#define T_AdvSendAdvert 269
#define T_MaxRtrAdvInterval 270
#define T_MinRtrAdvInterval 271
#define T_MinDelayBetweenRAs 272
#define T_AdvManagedFlag 273
#define T_AdvOtherConfigFlag 274
#define T_AdvLinkMTU 275
#define T_AdvReachableTime 276
#define T_AdvRetransTimer 277
#define T_AdvCurHopLimit 278
#define T_AdvDefaultLifetime 279
#define T_AdvDefaultPreference 280
#define T_AdvSourceLLAddress 281
#define T_AdvOnLink 282
#define T_AdvAutonomous 283
#define T_AdvValidLifetime 284
#define T_AdvPreferredLifetime 285
#define T_AdvRouterAddr 286
#define T_AdvHomeAgentFlag 287
#define T_AdvIntervalOpt 288
#define T_AdvHomeAgentInfo 289
#define T_Base6to4Interface 290
#define T_UnicastOnly 291
#define T_HomeAgentPreference 292
#define T_HomeAgentLifetime 293
#define T_AdvRoutePreference 294
#define T_AdvRouteLifetime 295
#define T_AdvMobRtrSupportFlag 296
#define T_BAD_TOKEN 297




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 103 "gram.y"
typedef union YYSTYPE {
	unsigned int		num;
	int			snum;
	double			dec;
	int			bool;
	struct in6_addr		*addr;
	char			*str;
	struct AdvPrefix	*pinfo;
	struct AdvRoute		*rinfo;
} YYSTYPE;
/* Line 1240 of yacc.c.  */
#line 131 "y.tab.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



