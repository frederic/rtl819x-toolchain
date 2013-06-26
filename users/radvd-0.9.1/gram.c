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

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



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




/* Copy the first part of user declarations.  */
#line 16 "gram.y"

#include <config.h>
#include <includes.h>
#include <radvd.h>
#include <defaults.h>

extern struct Interface *IfaceList;
struct Interface *iface = NULL;
struct AdvPrefix *prefix = NULL;
struct AdvRoute *route = NULL;

extern char *conf_file;
extern int num_lines;
extern char *yytext;
extern int sock;

static void cleanup(void);
static void yyerror(char *msg);

#if 0 /* no longer necessary? */
#ifndef HAVE_IN6_ADDR_S6_ADDR
# ifdef __FreeBSD__
#  define s6_addr32 __u6_addr.__u6_addr32
#  define s6_addr16 __u6_addr.__u6_addr16
# endif
#endif
#endif

#define ABORT	do { cleanup(); YYABORT; } while (0);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

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
/* Line 191 of yacc.c.  */
#line 202 "y.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 214 "y.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   136

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  47
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  24
/* YYNRULES -- Number of rules. */
#define YYNRULES  65
/* YYNRULES -- Number of states. */
#define YYNSTATES  144

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   297

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    45,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    43,     2,    44,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     6,     8,    14,    17,    19,    23,    24,
      26,    27,    29,    30,    32,    35,    37,    41,    45,    49,
      53,    57,    61,    65,    69,    73,    77,    81,    85,    89,
      93,    97,   101,   105,   109,   113,   117,   121,   125,   129,
     133,   135,   138,   144,   149,   150,   152,   155,   157,   161,
     165,   169,   173,   177,   181,   183,   186,   192,   197,   198,
     200,   203,   205,   209,   213,   215
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      48,     0,    -1,    48,    49,    -1,    49,    -1,    50,    43,
      52,    44,    45,    -1,     3,    51,    -1,     6,    -1,    53,
      54,    55,    -1,    -1,    56,    -1,    -1,    58,    -1,    -1,
      64,    -1,    56,    57,    -1,    57,    -1,    16,     7,    45,
      -1,    15,     7,    45,    -1,    17,     7,    45,    -1,    16,
       9,    45,    -1,    15,     9,    45,    -1,    17,     9,    45,
      -1,    13,    10,    45,    -1,    14,    10,    45,    -1,    18,
      10,    45,    -1,    19,    10,    45,    -1,    20,     7,    45,
      -1,    21,     7,    45,    -1,    22,     7,    45,    -1,    24,
       7,    45,    -1,    25,     8,    45,    -1,    23,     7,    45,
      -1,    26,    10,    45,    -1,    33,    10,    45,    -1,    34,
      10,    45,    -1,    32,    10,    45,    -1,    37,     7,    45,
      -1,    38,     7,    45,    -1,    36,    10,    45,    -1,    41,
      10,    45,    -1,    59,    -1,    58,    59,    -1,    60,    43,
      61,    44,    45,    -1,     4,    11,    46,     7,    -1,    -1,
      62,    -1,    62,    63,    -1,    63,    -1,    27,    10,    45,
      -1,    28,    10,    45,    -1,    31,    10,    45,    -1,    29,
      70,    45,    -1,    30,    70,    45,    -1,    35,    51,    45,
      -1,    65,    -1,    64,    65,    -1,    66,    43,    67,    44,
      45,    -1,     5,    11,    46,     7,    -1,    -1,    68,    -1,
      68,    69,    -1,    69,    -1,    39,     8,    45,    -1,    40,
      70,    45,    -1,     7,    -1,    12,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   116,   116,   117,   120,   164,   179,   186,   193,   194,
     198,   201,   205,   208,   211,   212,   215,   219,   223,   227,
     231,   235,   239,   243,   247,   251,   255,   259,   263,   267,
     271,   275,   279,   283,   287,   291,   295,   299,   303,   307,
     313,   317,   324,   355,   378,   379,   382,   383,   386,   390,
     394,   398,   402,   406,   414,   418,   425,   433,   457,   458,
     461,   462,   466,   470,   477,   481
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_INTERFACE", "T_PREFIX", "T_ROUTE", 
  "STRING", "NUMBER", "SIGNEDNUMBER", "DECIMAL", "SWITCH", "IPV6ADDR", 
  "INFINITY", "T_IgnoreIfMissing", "T_AdvSendAdvert", 
  "T_MaxRtrAdvInterval", "T_MinRtrAdvInterval", "T_MinDelayBetweenRAs", 
  "T_AdvManagedFlag", "T_AdvOtherConfigFlag", "T_AdvLinkMTU", 
  "T_AdvReachableTime", "T_AdvRetransTimer", "T_AdvCurHopLimit", 
  "T_AdvDefaultLifetime", "T_AdvDefaultPreference", 
  "T_AdvSourceLLAddress", "T_AdvOnLink", "T_AdvAutonomous", 
  "T_AdvValidLifetime", "T_AdvPreferredLifetime", "T_AdvRouterAddr", 
  "T_AdvHomeAgentFlag", "T_AdvIntervalOpt", "T_AdvHomeAgentInfo", 
  "T_Base6to4Interface", "T_UnicastOnly", "T_HomeAgentPreference", 
  "T_HomeAgentLifetime", "T_AdvRoutePreference", "T_AdvRouteLifetime", 
  "T_AdvMobRtrSupportFlag", "T_BAD_TOKEN", "'{'", "'}'", "';'", "'/'", 
  "$accept", "grammar", "ifacedef", "ifacehead", "name", "ifaceparams", 
  "optional_ifacevlist", "optional_prefixlist", "optional_routelist", 
  "ifacevlist", "ifaceval", "prefixlist", "prefixdef", "prefixhead", 
  "optional_prefixplist", "prefixplist", "prefixparms", "routelist", 
  "routedef", "routehead", "optional_routeplist", "routeplist", 
  "routeparms", "number_or_infinity", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   123,   125,    59,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    47,    48,    48,    49,    50,    51,    52,    53,    53,
      54,    54,    55,    55,    56,    56,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      57,    57,    57,    57,    57,    57,    57,    57,    57,    57,
      58,    58,    59,    60,    61,    61,    62,    62,    63,    63,
      63,    63,    63,    63,    64,    64,    65,    66,    67,    67,
      68,    68,    69,    69,    70,    70
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     1,     5,     2,     1,     3,     0,     1,
       0,     1,     0,     1,     2,     1,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       1,     2,     5,     4,     0,     1,     2,     1,     3,     3,
       3,     3,     3,     3,     1,     2,     5,     4,     0,     1,
       2,     1,     3,     3,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     0,     0,     3,     0,     6,     5,     1,     2,     8,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    10,     9,    15,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    12,    11,    40,     0,    14,    22,    23,    17,    20,
      16,    19,    18,    21,    24,    25,    26,    27,    28,    31,
      29,    30,    32,    35,    33,    34,    38,    36,    37,    39,
       4,     0,     0,     7,    13,    54,     0,    41,    44,     0,
       0,    55,    58,     0,     0,     0,     0,     0,     0,     0,
      45,    47,    43,     0,     0,     0,     0,    59,    61,     0,
       0,    64,    65,     0,     0,     0,     0,     0,    46,    57,
       0,     0,     0,    60,    48,    49,    51,    52,    50,    53,
      42,    62,    63,    56
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     2,     3,     4,     6,    31,    32,    61,    93,    33,
      34,    62,    63,    64,   109,   110,   111,    94,    95,    96,
     116,   117,   118,   123
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -90
static const yysigned_char yypact[] =
{
      12,    34,    35,   -90,   -25,   -90,   -90,   -90,   -90,   -13,
      36,    37,     7,    27,    32,    38,    39,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    57,    58,
      56,     0,    41,   -13,   -90,    10,    13,    22,    23,    24,
      25,    26,    28,    29,    30,    31,    33,    40,    42,    54,
      55,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    72,    41,   -90,    14,   -90,   -90,   -90,   -90,   -90,
     -90,   -90,   -90,   -90,   -90,   -90,   -90,   -90,   -90,   -90,
     -90,   -90,   -90,   -90,   -90,   -90,   -90,   -90,   -90,   -90,
     -90,    69,    70,   -90,    72,   -90,    71,   -90,     2,    73,
      74,   -90,     3,    76,    78,    15,    15,    79,    34,    75,
       2,   -90,   -90,    77,    82,    15,    80,     3,   -90,    81,
      83,   -90,   -90,    84,    85,    86,    87,    88,   -90,   -90,
      89,    90,    91,   -90,   -90,   -90,   -90,   -90,   -90,   -90,
     -90,   -90,   -90,   -90
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -90,   -90,    92,   -90,   -36,   -90,   -90,   -90,   -90,   -90,
      94,   -90,    20,   -90,   -90,   -90,   -27,   -90,    -3,   -90,
     -90,   -90,   -24,   -89
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    37,     1,    38,   124,     9,    24,
      25,    26,   121,    27,    28,    29,   131,   122,    30,   103,
     104,   105,   106,   107,    39,     7,    40,   108,     1,    41,
       5,    42,   114,   115,    59,    60,    35,    36,    43,    44,
      45,    46,    47,    48,    49,    66,    50,    98,    67,    51,
      52,    53,    54,    55,    56,    57,    58,    68,    69,    70,
      71,    72,   126,    73,    74,    75,    76,    92,    77,    91,
     112,   100,    97,   128,   129,    78,   119,    79,   120,   125,
     130,   101,     0,   133,     8,     0,     0,     0,     0,    80,
      81,     0,     0,     0,    82,    83,    84,    85,    86,    87,
      88,    89,    90,     0,   102,    99,     0,     0,     0,   127,
     113,     0,     0,     0,   132,     0,   134,    65,   135,   136,
     137,   138,   139,   140,   141,   142,   143
};

static const yysigned_char yycheck[] =
{
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,     7,     3,     9,   106,    43,    32,
      33,    34,     7,    36,    37,    38,   115,    12,    41,    27,
      28,    29,    30,    31,     7,     0,     9,    35,     3,     7,
       6,     9,    39,    40,    44,     4,    10,    10,    10,    10,
       7,     7,     7,     7,     7,    45,     8,    43,    45,    10,
      10,    10,    10,    10,     7,     7,    10,    45,    45,    45,
      45,    45,   108,    45,    45,    45,    45,     5,    45,    11,
       7,    11,    62,   110,     7,    45,    10,    45,    10,    10,
       8,    94,    -1,   117,     2,    -1,    -1,    -1,    -1,    45,
      45,    -1,    -1,    -1,    45,    45,    45,    45,    45,    45,
      45,    45,    45,    -1,    43,    46,    -1,    -1,    -1,    44,
      46,    -1,    -1,    -1,    44,    -1,    45,    33,    45,    45,
      45,    45,    45,    45,    45,    45,    45
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,    48,    49,    50,     6,    51,     0,    49,    43,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    32,    33,    34,    36,    37,    38,
      41,    52,    53,    56,    57,    10,    10,     7,     9,     7,
       9,     7,     9,    10,    10,     7,     7,     7,     7,     7,
       8,    10,    10,    10,    10,    10,     7,     7,    10,    44,
       4,    54,    58,    59,    60,    57,    45,    45,    45,    45,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    11,     5,    55,    64,    65,    66,    59,    43,    46,
      11,    65,    43,    27,    28,    29,    30,    31,    35,    61,
      62,    63,     7,    46,    39,    40,    67,    68,    69,    10,
      10,     7,    12,    70,    70,    10,    51,    44,    63,     7,
       8,    70,    44,    69,    45,    45,    45,    45,    45,    45,
      45,    45,    45,    45
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 121 "gram.y"
    {
			struct Interface *iface2;

			iface2 = IfaceList;
			while (iface2)
			{
				if (!strcmp(iface2->Name, iface->Name))
				{
					flog(LOG_ERR, "duplicate interface "
						"definition for %s", iface->Name);
					ABORT;
				}
				iface2 = iface2->next;
			}			

			if (check_device(sock, iface) < 0) {
				if (iface->IgnoreIfMissing) {
					dlog(LOG_DEBUG, 4, "interface %s did not exist, ignoring the interface", iface->Name);
					goto skip_interface;
				}
				else {
					flog(LOG_ERR, "interface %s does not exist", iface->Name);
					ABORT;
				}
			}
			if (setup_deviceinfo(sock, iface) < 0)
				ABORT;
			if (check_iface(iface) < 0)
				ABORT;
			if (setup_linklocal_addr(sock, iface) < 0)
				ABORT;
			if (setup_allrouters_membership(sock, iface) < 0)
				ABORT;

			iface->next = IfaceList;
			IfaceList = iface;

			dlog(LOG_DEBUG, 4, "interface definition for %s is ok", iface->Name);

skip_interface:
			iface = NULL;
		}
    break;

  case 5:
#line 165 "gram.y"
    {
			iface = malloc(sizeof(struct Interface));

			if (iface == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			iface_init_defaults(iface);
			strncpy(iface->Name, yyvsp[0].str, IFNAMSIZ-1);
			iface->Name[IFNAMSIZ-1] = '\0';
		}
    break;

  case 6:
#line 180 "gram.y"
    {
			/* check vality */
			yyval.str = yyvsp[0].str;
		}
    break;

  case 7:
#line 187 "gram.y"
    {
			iface->AdvPrefixList = yyvsp[-1].pinfo;
			iface->AdvRouteList = yyvsp[0].rinfo;
		}
    break;

  case 10:
#line 198 "gram.y"
    {
			yyval.pinfo = NULL;
		}
    break;

  case 12:
#line 205 "gram.y"
    {
			yyval.rinfo = NULL;
		}
    break;

  case 16:
#line 216 "gram.y"
    {
			iface->MinRtrAdvInterval = yyvsp[-1].num;
		}
    break;

  case 17:
#line 220 "gram.y"
    {
			iface->MaxRtrAdvInterval = yyvsp[-1].num;
		}
    break;

  case 18:
#line 224 "gram.y"
    {
			iface->MinDelayBetweenRAs = yyvsp[-1].num;
		}
    break;

  case 19:
#line 228 "gram.y"
    {
			iface->MinRtrAdvInterval = yyvsp[-1].dec;
		}
    break;

  case 20:
#line 232 "gram.y"
    {
			iface->MaxRtrAdvInterval = yyvsp[-1].dec;
		}
    break;

  case 21:
#line 236 "gram.y"
    {
			iface->MinDelayBetweenRAs = yyvsp[-1].dec;
		}
    break;

  case 22:
#line 240 "gram.y"
    {
			iface->IgnoreIfMissing = yyvsp[-1].bool;
		}
    break;

  case 23:
#line 244 "gram.y"
    {
			iface->AdvSendAdvert = yyvsp[-1].bool;
		}
    break;

  case 24:
#line 248 "gram.y"
    {
			iface->AdvManagedFlag = yyvsp[-1].bool;
		}
    break;

  case 25:
#line 252 "gram.y"
    {
			iface->AdvOtherConfigFlag = yyvsp[-1].bool;
		}
    break;

  case 26:
#line 256 "gram.y"
    {
			iface->AdvLinkMTU = yyvsp[-1].num;
		}
    break;

  case 27:
#line 260 "gram.y"
    {
			iface->AdvReachableTime = yyvsp[-1].num;
		}
    break;

  case 28:
#line 264 "gram.y"
    {
			iface->AdvRetransTimer = yyvsp[-1].num;
		}
    break;

  case 29:
#line 268 "gram.y"
    {
			iface->AdvDefaultLifetime = yyvsp[-1].num;
		}
    break;

  case 30:
#line 272 "gram.y"
    {
			iface->AdvDefaultPreference = yyvsp[-1].snum;
		}
    break;

  case 31:
#line 276 "gram.y"
    {
			iface->AdvCurHopLimit = yyvsp[-1].num;
		}
    break;

  case 32:
#line 280 "gram.y"
    {
			iface->AdvSourceLLAddress = yyvsp[-1].bool;
		}
    break;

  case 33:
#line 284 "gram.y"
    {
			iface->AdvIntervalOpt = yyvsp[-1].bool;
		}
    break;

  case 34:
#line 288 "gram.y"
    {
			iface->AdvHomeAgentInfo = yyvsp[-1].bool;
		}
    break;

  case 35:
#line 292 "gram.y"
    {
			iface->AdvHomeAgentFlag = yyvsp[-1].bool;
		}
    break;

  case 36:
#line 296 "gram.y"
    {
			iface->HomeAgentPreference = yyvsp[-1].num;
		}
    break;

  case 37:
#line 300 "gram.y"
    {
			iface->HomeAgentLifetime = yyvsp[-1].num;
		}
    break;

  case 38:
#line 304 "gram.y"
    {
			iface->UnicastOnly = yyvsp[-1].bool;
		}
    break;

  case 39:
#line 308 "gram.y"
    {
			iface->AdvMobRtrSupportFlag = yyvsp[-1].bool;
		}
    break;

  case 40:
#line 314 "gram.y"
    {
			yyval.pinfo = yyvsp[0].pinfo;
		}
    break;

  case 41:
#line 318 "gram.y"
    {
			yyvsp[0].pinfo->next = yyvsp[-1].pinfo;
			yyval.pinfo = yyvsp[0].pinfo;
		}
    break;

  case 42:
#line 325 "gram.y"
    {
			unsigned int dst;

			if (prefix->AdvPreferredLifetime >
			    prefix->AdvValidLifetime)
			{
				flog(LOG_ERR, "AdvValidLifeTime must be "
					"greater than AdvPreferredLifetime in %s, line %d", 
					conf_file, num_lines);
				ABORT;
			}

			if( prefix->if6to4[0] )
			{
				if (get_v4addr(prefix->if6to4, &dst) < 0)
				{
					flog(LOG_ERR, "interface %s has no IPv4 addresses, disabling 6to4 prefix", prefix->if6to4 );
					prefix->enabled = 0;
				} else
				{
					*((uint16_t *)(prefix->Prefix.s6_addr)) = htons(0x2002);
					memcpy( prefix->Prefix.s6_addr + 2, &dst, sizeof( dst ) );
				}
			}

			yyval.pinfo = prefix;
			prefix = NULL;
		}
    break;

  case 43:
#line 356 "gram.y"
    {
			prefix = malloc(sizeof(struct AdvPrefix));
			
			if (prefix == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			prefix_init_defaults(prefix);

			if (yyvsp[0].num > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			prefix->PrefixLen = yyvsp[0].num;

			memcpy(&prefix->Prefix, yyvsp[-2].addr, sizeof(struct in6_addr));
		}
    break;

  case 48:
#line 387 "gram.y"
    {
			prefix->AdvOnLinkFlag = yyvsp[-1].bool;
		}
    break;

  case 49:
#line 391 "gram.y"
    {
			prefix->AdvAutonomousFlag = yyvsp[-1].bool;
		}
    break;

  case 50:
#line 395 "gram.y"
    {
			prefix->AdvRouterAddr = yyvsp[-1].bool;
		}
    break;

  case 51:
#line 399 "gram.y"
    {
			prefix->AdvValidLifetime = yyvsp[-1].num;
		}
    break;

  case 52:
#line 403 "gram.y"
    {
			prefix->AdvPreferredLifetime = yyvsp[-1].num;
		}
    break;

  case 53:
#line 407 "gram.y"
    {
			dlog(LOG_DEBUG, 4, "using interface %s for 6to4", yyvsp[-1].str);
			strncpy(prefix->if6to4, yyvsp[-1].str, IFNAMSIZ-1);
			prefix->if6to4[IFNAMSIZ-1] = '\0';
		}
    break;

  case 54:
#line 415 "gram.y"
    {
			yyval.rinfo = yyvsp[0].rinfo;
		}
    break;

  case 55:
#line 419 "gram.y"
    {
			yyvsp[0].rinfo->next = yyvsp[-1].rinfo;
			yyval.rinfo = yyvsp[0].rinfo;
		}
    break;

  case 56:
#line 426 "gram.y"
    {
			yyval.rinfo = route;
			route = NULL;
		}
    break;

  case 57:
#line 434 "gram.y"
    {
			route = malloc(sizeof(struct AdvRoute));
			
			if (route == NULL) {
				flog(LOG_CRIT, "malloc failed: %s", strerror(errno));
				ABORT;
			}

			route_init_defaults(route, iface);

			if (yyvsp[0].num > MAX_PrefixLen)
			{
				flog(LOG_ERR, "invalid route prefix length in %s, line %d", conf_file, num_lines);
				ABORT;
			}

			route->PrefixLen = yyvsp[0].num;

			memcpy(&route->Prefix, yyvsp[-2].addr, sizeof(struct in6_addr));
		}
    break;

  case 62:
#line 467 "gram.y"
    {
			route->AdvRoutePreference = yyvsp[-1].snum;
		}
    break;

  case 63:
#line 471 "gram.y"
    {
			route->AdvRouteLifetime = yyvsp[-1].num;
		}
    break;

  case 64:
#line 478 "gram.y"
    {
                                yyval.num = yyvsp[0].num; 
                        }
    break;

  case 65:
#line 482 "gram.y"
    {
                                yyval.num = (uint32_t)~0;
                        }
    break;


    }

/* Line 999 of yacc.c.  */
#line 1667 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 487 "gram.y"


static
void cleanup(void)
{
	if (iface)
		free(iface);
	
	if (prefix)
		free(prefix);
}

static void
yyerror(char *msg)
{
	cleanup();
	flog(LOG_ERR, "%s in %s, line %d: %s", msg, conf_file, num_lines, yytext);
}

