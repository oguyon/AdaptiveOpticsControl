/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "calc_bison.y" /* yacc.c:339  */

#include <math.h>  /* For math functions, cos(), sin(), etc. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
  //#include "calc.h"  /* Contains definition of `symrec'        */

#include "CLIcore.h"
#include "00CORE/00CORE.h"
#include "COREMOD_memory/COREMOD_memory.h"
#include "COREMOD_iofits/COREMOD_iofits.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "info/info.h"


int yylex();
int yyerror(char *);


extern DATA data;


// NOTATIONS
// d: double
// im: image

 char calctmpimname[200];

  

#line 97 "calc_bison.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "y.tab.h".  */
#ifndef YY_YY_CALC_BISON_H_INCLUDED
# define YY_YY_CALC_BISON_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TKNUMl = 258,
    TKNUMf = 259,
    TKNUMd = 260,
    TKVAR = 261,
    TKNVAR = 262,
    TKIMAGE = 263,
    TKCOMMAND = 264,
    TKFUNC_d_d = 265,
    TKFUNC_dd_d = 266,
    TKFUNC_ddd_d = 267,
    TKFUNC_im_d = 268,
    TKFUNC_imd_d = 269,
    NEG = 270
  };
#endif
/* Tokens.  */
#define TKNUMl 258
#define TKNUMf 259
#define TKNUMd 260
#define TKVAR 261
#define TKNVAR 262
#define TKIMAGE 263
#define TKCOMMAND 264
#define TKFUNC_d_d 265
#define TKFUNC_dd_d 266
#define TKFUNC_ddd_d 267
#define TKFUNC_im_d 268
#define TKFUNC_imd_d 269
#define NEG 270

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 31 "calc_bison.y" /* yacc.c:355  */

  long     val_l;  /* long */  
  float    val_f;  /* float */
  double   val_d;  /* For returning numbers.     */
  char  *string;   /* For returning strings (variables, images)  */
  double (*fnctptr)();    /* pointer to function -> double */

#line 175 "calc_bison.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_CALC_BISON_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 190 "calc_bison.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   658

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  26
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  6
/* YYNRULES -- Number of rules.  */
#define YYNRULES  86
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  205

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   270

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      22,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      23,    24,    18,    17,    25,    16,     2,    19,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    15,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    21,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      20
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    66,    66,    67,    71,    72,    77,    82,    86,    89,
      90,    91,    92,    93,    94,    95,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TKNUMl", "TKNUMf", "TKNUMd", "TKVAR",
  "TKNVAR", "TKIMAGE", "TKCOMMAND", "TKFUNC_d_d", "TKFUNC_dd_d",
  "TKFUNC_ddd_d", "TKFUNC_im_d", "TKFUNC_imd_d", "'='", "'-'", "'+'",
  "'*'", "'/'", "NEG", "'^'", "'\\n'", "'('", "')'", "','", "$accept",
  "input", "line", "expl", "expd", "exps", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,    61,    45,    43,    42,    47,
     270,    94,    10,    40,    41,    44
};
# endif

#define YYPACT_NINF -22

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-22)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -22,   231,   -22,   -21,   -22,   -22,    -8,    -2,     2,   -22,
      21,    21,    21,    21,    21,   102,   -22,    21,   -22,    68,
     178,   612,   -22,   102,    21,    21,   378,   387,   396,   -13,
      49,   119,   202,   249,   259,    85,   619,   405,   278,     5,
     102,   102,   102,   102,   -19,    -6,   414,   423,   432,    21,
      21,    21,    21,   102,   -22,    21,    21,    21,    21,   102,
     -22,    21,    21,    21,    21,   102,   -22,   625,   631,    85,
     619,   637,   637,   -22,   -22,   -22,   102,   102,   102,   102,
     102,   102,   -22,   102,   102,   441,   450,   288,   298,   308,
     318,   459,   468,   102,   102,   102,   102,   102,   102,   102,
     102,   -22,   -22,   -22,     0,   110,   124,     0,   110,   124,
     -19,    -6,     4,   -19,    -6,     4,   -19,    -6,     0,   110,
     124,     0,   110,   124,   -19,    -6,     4,   -19,    -6,     4,
     -19,    -6,     0,   110,   124,     0,   110,   124,   -19,    -6,
       4,   -19,    -6,     4,   -19,    -6,   477,   486,   495,   504,
     625,   513,   328,   338,   348,   358,   368,   522,   625,   631,
     135,   157,   135,   157,   -19,    -6,   -19,    -6,   135,   157,
     135,   157,   -19,    -6,   -19,    -6,   -22,   -22,   -22,   -22,
     -22,   102,   102,   102,   102,   102,   -22,   531,   540,   549,
     558,   567,   576,   585,   594,   603,   -22,   -22,   -22,   -22,
     -22,   -22,   -22,   -22,   -22
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     9,    16,    17,    56,    57,    58,
       0,     0,     0,     0,     0,     0,     4,     0,     3,     0,
       0,     0,     8,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    13,    35,     0,     0,     0,     0,
       0,     0,     0,     0,     6,     0,     0,     0,     0,     0,
       5,     0,     0,     0,     0,     0,     7,    18,    19,    20,
      21,    60,    59,    40,    39,    83,     0,     0,     0,     0,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    15,    55,    86,    11,    25,    70,    10,    22,    65,
      12,    28,    75,    31,    32,    80,    14,    36,    26,    27,
      69,    23,    24,    64,    29,    30,    74,    33,    34,    79,
      37,    38,    68,    67,    66,    63,    62,    61,    73,    72,
      71,    78,    77,    76,    81,    82,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    20,    21,
      11,    25,    10,    22,    12,    28,    31,    32,    26,    27,
      23,    24,    29,    30,    33,    34,    44,    42,    43,    41,
      84,     0,     0,     0,     0,     0,    54,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    52,    48,    50,    46,
      51,    47,    49,    45,    85
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -22,   -22,   -22,   109,    -1,   200
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    18,   150,    36,    21
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      20,    22,    53,    49,    50,    51,    52,    23,    53,    27,
      30,    33,    76,    24,    45,    59,    47,    25,    51,    52,
      84,    53,    68,    70,     4,    65,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,    86,
      88,    90,    92,     0,    17,     0,     0,     0,   105,   108,
     111,   114,   117,     0,   119,   122,   125,   128,   131,     0,
     133,   136,   139,   142,   145,    55,    56,    57,    58,     0,
      59,     0,     0,     0,    77,   147,   149,   151,   153,   155,
     156,     0,   157,   159,    49,    50,    51,    52,     0,    53,
      54,     0,   161,   163,   165,   167,   169,   171,   173,   175,
       0,    49,    50,    51,    52,     4,    53,     5,     6,    39,
      19,     0,    40,    41,    42,    13,    14,     0,    15,    26,
      29,    32,    35,    35,    44,    43,    46,     0,    57,    58,
       0,    59,    67,    69,    35,    61,    62,    63,    64,     0,
      65,     0,    63,    64,    78,    65,     0,     0,     0,    85,
      87,    89,    91,    95,    96,     0,    53,     0,   104,   107,
     110,   113,   116,     0,   118,   121,   124,   127,   130,     0,
     132,   135,   138,   141,   144,    99,   100,     0,    59,     0,
     188,   190,   192,   194,   195,   146,   148,     0,   152,   154,
       0,     0,     0,   158,    55,    56,    57,    58,     0,    59,
      60,     0,   160,   162,   164,   166,   168,   170,   172,   174,
      28,    31,    34,    37,    38,     0,     0,    48,    49,    50,
      51,    52,     0,    53,    71,    72,     0,    79,     0,     0,
       0,     2,     3,     0,     4,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,     0,    15,     0,   106,
     109,   112,   115,    16,    17,   120,   123,   126,   129,     0,
       0,   134,   137,   140,   143,    55,    56,    57,    58,     0,
      59,     0,     0,     0,    80,    61,    62,    63,    64,     0,
      65,     0,     0,     0,    81,     0,     0,     0,     0,     0,
     187,   189,   191,   193,    61,    62,    63,    64,     0,    65,
       0,     0,     0,    83,    93,    94,    95,    96,     0,    53,
       0,     0,     0,    76,    97,    98,    99,   100,     0,    59,
       0,     0,     0,    77,    93,    94,    95,    96,     0,    53,
       0,     0,     0,    79,    97,    98,    99,   100,     0,    59,
       0,     0,     0,    80,    93,    94,    95,    96,     0,    53,
       0,     0,     0,   181,    97,    98,    99,   100,     0,    59,
       0,     0,     0,   182,    93,    94,    95,    96,     0,    53,
       0,     0,     0,   183,    97,    98,    99,   100,     0,    59,
       0,     0,     0,   184,    97,    98,    99,   100,     0,    59,
       0,     0,     0,   185,    49,    50,    51,    52,     0,    53,
       0,     0,    73,    55,    56,    57,    58,     0,    59,     0,
       0,    74,    61,    62,    63,    64,     0,    65,     0,     0,
      75,    61,    62,    63,    64,     0,    65,     0,     0,    82,
      49,    50,    51,    52,     0,    53,     0,     0,   101,    55,
      56,    57,    58,     0,    59,     0,     0,   102,    61,    62,
      63,    64,     0,    65,     0,     0,   103,    93,    94,    95,
      96,     0,    53,     0,     0,    73,    97,    98,    99,   100,
       0,    59,     0,     0,    74,    93,    94,    95,    96,     0,
      53,     0,     0,   101,    97,    98,    99,   100,     0,    59,
       0,     0,   102,    93,    94,    95,    96,     0,    53,     0,
       0,   176,    97,    98,    99,   100,     0,    59,     0,     0,
     177,    93,    94,    95,    96,     0,    53,     0,     0,   178,
      97,    98,    99,   100,     0,    59,     0,     0,   179,    97,
      98,    99,   100,     0,    59,     0,     0,   180,    97,    98,
      99,   100,     0,    59,     0,     0,   186,    93,    94,    95,
      96,     0,    53,     0,     0,   196,    97,    98,    99,   100,
       0,    59,     0,     0,   197,    93,    94,    95,    96,     0,
      53,     0,     0,   198,    97,    98,    99,   100,     0,    59,
       0,     0,   199,    93,    94,    95,    96,     0,    53,     0,
       0,   200,    97,    98,    99,   100,     0,    59,     0,     0,
     201,    93,    94,    95,    96,     0,    53,     0,     0,   202,
      97,    98,    99,   100,     0,    59,     0,     0,   203,    97,
      98,    99,   100,     0,    59,     0,     0,   204,    61,    62,
      63,    64,     0,    65,    66,    55,    56,    57,    58,     0,
      59,    93,    94,    95,    96,     0,    53,    97,    98,    99,
     100,     0,    59,    61,    62,    63,    64,     0,    65
};

static const yytype_int16 yycheck[] =
{
       1,    22,    21,    16,    17,    18,    19,    15,    21,    10,
      11,    12,    25,    15,    15,    21,    17,    15,    18,    19,
      15,    21,    23,    24,     3,    21,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    -1,    16,    -1,    40,
      41,    42,    43,    -1,    23,    -1,    -1,    -1,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    -1,
      61,    62,    63,    64,    65,    16,    17,    18,    19,    -1,
      21,    -1,    -1,    -1,    25,    76,    77,    78,    79,    80,
      81,    -1,    83,    84,    16,    17,    18,    19,    -1,    21,
      22,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      -1,    16,    17,    18,    19,     3,    21,     5,     6,     7,
       1,    -1,    10,    11,    12,    13,    14,    -1,    16,    10,
      11,    12,    13,    14,    15,    23,    17,    -1,    18,    19,
      -1,    21,    23,    24,    25,    16,    17,    18,    19,    -1,
      21,    -1,    18,    19,    25,    21,    -1,    -1,    -1,    40,
      41,    42,    43,    18,    19,    -1,    21,    -1,    49,    50,
      51,    52,    53,    -1,    55,    56,    57,    58,    59,    -1,
      61,    62,    63,    64,    65,    18,    19,    -1,    21,    -1,
     181,   182,   183,   184,   185,    76,    77,    -1,    79,    80,
      -1,    -1,    -1,    84,    16,    17,    18,    19,    -1,    21,
      22,    -1,    93,    94,    95,    96,    97,    98,    99,   100,
      10,    11,    12,    13,    14,    -1,    -1,    17,    16,    17,
      18,    19,    -1,    21,    24,    25,    -1,    25,    -1,    -1,
      -1,     0,     1,    -1,     3,    -1,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    -1,    16,    -1,    49,
      50,    51,    52,    22,    23,    55,    56,    57,    58,    -1,
      -1,    61,    62,    63,    64,    16,    17,    18,    19,    -1,
      21,    -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,
      21,    -1,    -1,    -1,    25,    -1,    -1,    -1,    -1,    -1,
     181,   182,   183,   184,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    -1,    25,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    24,    16,    17,    18,    19,    -1,    21,    -1,
      -1,    24,    16,    17,    18,    19,    -1,    21,    -1,    -1,
      24,    16,    17,    18,    19,    -1,    21,    -1,    -1,    24,
      16,    17,    18,    19,    -1,    21,    -1,    -1,    24,    16,
      17,    18,    19,    -1,    21,    -1,    -1,    24,    16,    17,
      18,    19,    -1,    21,    -1,    -1,    24,    16,    17,    18,
      19,    -1,    21,    -1,    -1,    24,    16,    17,    18,    19,
      -1,    21,    -1,    -1,    24,    16,    17,    18,    19,    -1,
      21,    -1,    -1,    24,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    24,    16,    17,    18,    19,    -1,    21,    -1,
      -1,    24,    16,    17,    18,    19,    -1,    21,    -1,    -1,
      24,    16,    17,    18,    19,    -1,    21,    -1,    -1,    24,
      16,    17,    18,    19,    -1,    21,    -1,    -1,    24,    16,
      17,    18,    19,    -1,    21,    -1,    -1,    24,    16,    17,
      18,    19,    -1,    21,    -1,    -1,    24,    16,    17,    18,
      19,    -1,    21,    -1,    -1,    24,    16,    17,    18,    19,
      -1,    21,    -1,    -1,    24,    16,    17,    18,    19,    -1,
      21,    -1,    -1,    24,    16,    17,    18,    19,    -1,    21,
      -1,    -1,    24,    16,    17,    18,    19,    -1,    21,    -1,
      -1,    24,    16,    17,    18,    19,    -1,    21,    -1,    -1,
      24,    16,    17,    18,    19,    -1,    21,    -1,    -1,    24,
      16,    17,    18,    19,    -1,    21,    -1,    -1,    24,    16,
      17,    18,    19,    -1,    21,    -1,    -1,    24,    16,    17,
      18,    19,    -1,    21,    22,    16,    17,    18,    19,    -1,
      21,    16,    17,    18,    19,    -1,    21,    16,    17,    18,
      19,    -1,    21,    16,    17,    18,    19,    -1,    21
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    27,     0,     1,     3,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    16,    22,    23,    28,    29,
      30,    31,    22,    15,    15,    15,    29,    30,    31,    29,
      30,    31,    29,    30,    31,    29,    30,    31,    31,     7,
      10,    11,    12,    23,    29,    30,    29,    30,    31,    16,
      17,    18,    19,    21,    22,    16,    17,    18,    19,    21,
      22,    16,    17,    18,    19,    21,    22,    29,    30,    29,
      30,    31,    31,    24,    24,    24,    25,    25,    25,    25,
      25,    25,    24,    25,    15,    29,    30,    29,    30,    29,
      30,    29,    30,    16,    17,    18,    19,    16,    17,    18,
      19,    24,    24,    24,    29,    30,    31,    29,    30,    31,
      29,    30,    31,    29,    30,    31,    29,    30,    29,    30,
      31,    29,    30,    31,    29,    30,    31,    29,    30,    31,
      29,    30,    29,    30,    31,    29,    30,    31,    29,    30,
      31,    29,    30,    31,    29,    30,    29,    30,    29,    30,
      29,    30,    29,    30,    29,    30,    30,    30,    29,    30,
      29,    30,    29,    30,    29,    30,    29,    30,    29,    30,
      29,    30,    29,    30,    29,    30,    24,    24,    24,    24,
      24,    25,    25,    25,    25,    25,    24,    29,    30,    29,
      30,    29,    30,    29,    30,    30,    24,    24,    24,    24,
      24,    24,    24,    24,    24
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    26,    27,    27,    28,    28,    28,    28,    28,    29,
      29,    29,    29,    29,    29,    29,    30,    30,    30,    30,
      30,    30,    30,    30,    30,    30,    30,    30,    30,    30,
      30,    30,    30,    30,    30,    30,    30,    30,    30,    30,
      30,    30,    30,    30,    30,    30,    30,    30,    30,    30,
      30,    30,    30,    30,    30,    30,    31,    31,    31,    31,
      31,    31,    31,    31,    31,    31,    31,    31,    31,    31,
      31,    31,    31,    31,    31,    31,    31,    31,    31,    31,
      31,    31,    31,    31,    31,    31,    31
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     2,     2,     2,     2,     1,
       3,     3,     3,     2,     3,     3,     1,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     2,     3,     3,     3,     3,
       3,     5,     5,     5,     5,     7,     7,     7,     7,     7,
       7,     7,     7,     3,     5,     3,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     5,     7,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 72 "calc_bison.y" /* yacc.c:1646  */
    { 
printf("\t double: %.10g\n", (yyvsp[-1].val_d)); 
data.cmdargtoken[data.cmdNBarg].type = 1; 
data.cmdargtoken[data.cmdNBarg].val.numf = (yyvsp[-1].val_d);
}
#line 1480 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 6:
#line 77 "calc_bison.y" /* yacc.c:1646  */
    { 
printf("\t long:   %ld\n", (yyvsp[-1].val_l)); 
data.cmdargtoken[data.cmdNBarg].type = 2; 
data.cmdargtoken[data.cmdNBarg].val.numl = (yyvsp[-1].val_l);
}
#line 1490 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 7:
#line 82 "calc_bison.y" /* yacc.c:1646  */
    { if(data.Debug>0) {printf("\t string: %s\n", (yyvsp[-1].string));}
    //data.cmdargtoken[data.cmdNBarg].type = 3;
sprintf(data.cmdargtoken[data.cmdNBarg].val.string, "%s", (yyvsp[-1].string));
}
#line 1499 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 8:
#line 86 "calc_bison.y" /* yacc.c:1646  */
    { yyerrok;                  }
#line 1505 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 9:
#line 89 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_l) = (yyvsp[0].val_l);        if(data.Debug>0){printf("this is a long\n");}}
#line 1511 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 10:
#line 90 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_l) = (yyvsp[-2].val_l) + (yyvsp[0].val_l);   if(data.Debug>0){printf("long + long\n");}}
#line 1517 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 11:
#line 91 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_l) = (yyvsp[-2].val_l) - (yyvsp[0].val_l);   if(data.Debug>0){printf("long - long\n");}}
#line 1523 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 12:
#line 92 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_l) = (yyvsp[-2].val_l) * (yyvsp[0].val_l);   if(data.Debug>0){printf("long * long\n");}}
#line 1529 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 13:
#line 93 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_l) = -(yyvsp[0].val_l);       if(data.Debug>0){printf("-long\n");}}
#line 1535 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 14:
#line 94 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_l) = (long) pow ((yyvsp[-2].val_l),(yyvsp[0].val_l));  if(data.Debug>0){printf("long ^ long\n");}}
#line 1541 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 15:
#line 95 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_l) = (yyvsp[-1].val_l); }
#line 1547 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 16:
#line 98 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[0].val_d);        if(data.Debug>0){printf("this is a double\n");}}
#line 1553 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 17:
#line 99 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = data.variable[variable_ID((yyvsp[0].string))].value.f;   }
#line 1559 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 18:
#line 100 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[0].val_l); create_variable_ID((yyvsp[-2].string), (yyvsp[0].val_l));  }
#line 1565 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 19:
#line 101 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[0].val_d); create_variable_ID((yyvsp[-2].string), (yyvsp[0].val_d));  }
#line 1571 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 20:
#line 102 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[0].val_l); create_variable_ID((yyvsp[-2].string), (yyvsp[0].val_l));  if(data.Debug>0){printf("creating long\n");}}
#line 1577 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 21:
#line 103 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[0].val_d); create_variable_ID((yyvsp[-2].string), (yyvsp[0].val_d));  if(data.Debug>0){printf("creating double\n");}}
#line 1583 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 22:
#line 104 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_l) + (yyvsp[0].val_d);   if(data.Debug>0){printf("long + double\n");}}
#line 1589 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 23:
#line 105 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) + (yyvsp[0].val_l);   if(data.Debug>0){printf("double + long\n");}}
#line 1595 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 24:
#line 106 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) + (yyvsp[0].val_d);   if(data.Debug>0){printf("double + double\n");}}
#line 1601 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 25:
#line 107 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_l) - (yyvsp[0].val_d);   if(data.Debug>0){printf("long - double\n");}}
#line 1607 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 26:
#line 108 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) - (yyvsp[0].val_l);   if(data.Debug>0){printf("double - long\n");}}
#line 1613 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 27:
#line 109 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) - (yyvsp[0].val_d);   if(data.Debug>0){printf("double - double\n");}}
#line 1619 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 28:
#line 110 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (double) (yyvsp[-2].val_l) * (yyvsp[0].val_d);   if(data.Debug>0){printf("long * double\n");}}
#line 1625 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 29:
#line 111 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) * (yyvsp[0].val_l);   if(data.Debug>0){printf("double * long\n");}}
#line 1631 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 30:
#line 112 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) * (yyvsp[0].val_d);   if(data.Debug>0){printf("double * double\n");}}
#line 1637 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 31:
#line 113 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (double) (yyvsp[-2].val_l) / (yyvsp[0].val_l);   if(data.Debug>0){printf("long / long\n");}}
#line 1643 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 32:
#line 114 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (double) (yyvsp[-2].val_l) / (yyvsp[0].val_d);   if(data.Debug>0){printf("long / double\n");}}
#line 1649 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 33:
#line 115 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) / (yyvsp[0].val_l);   if(data.Debug>0){printf("double / long\n");}}
#line 1655 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 34:
#line 116 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].val_d) / (yyvsp[0].val_d);   if(data.Debug>0){printf("double / double\n");}}
#line 1661 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 35:
#line 117 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = -(yyvsp[0].val_d);       if(data.Debug>0){printf("-double\n");}}
#line 1667 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 36:
#line 118 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = pow ((double) (yyvsp[-2].val_l),(yyvsp[0].val_d));  if(data.Debug>0){printf("long ^ double\n");}}
#line 1673 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 37:
#line 119 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = pow ((yyvsp[-2].val_d),(double) (yyvsp[0].val_l));  if(data.Debug>0){printf("double ^ long\n");}}
#line 1679 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 38:
#line 120 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = pow ((yyvsp[-2].val_d),(yyvsp[0].val_d));  if(data.Debug>0){printf("double ^ double\n");}}
#line 1685 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 39:
#line 121 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].fnctptr)((yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(double)\n");}}
#line 1691 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 40:
#line 122 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].fnctptr)((double) (yyvsp[-1].val_l));  if(data.Debug>0){printf("double=func(double)\n");}}
#line 1697 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 41:
#line 123 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-4].fnctptr)((yyvsp[-3].val_d),(yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(double,double)\n");}}
#line 1703 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 42:
#line 124 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-4].fnctptr)((double) (yyvsp[-3].val_l),(yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(long->double,double)\n");}}
#line 1709 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 43:
#line 125 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-4].fnctptr)((yyvsp[-3].val_d),(double) (yyvsp[-1].val_l));  if(data.Debug>0){printf("double=func(double,long->double)\n");}}
#line 1715 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 44:
#line 126 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-4].fnctptr)((double) (yyvsp[-3].val_l),(double) (yyvsp[-1].val_l));  if(data.Debug>0){printf("double=func(long->double,long->double)\n");}}
#line 1721 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 45:
#line 127 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((yyvsp[-5].val_d),(yyvsp[-3].val_d),(yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(double,double,double)\n");}}
#line 1727 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 46:
#line 128 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((double) (yyvsp[-5].val_l),(yyvsp[-3].val_d),(yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(long->double,double,double)\n");}}
#line 1733 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 47:
#line 129 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((yyvsp[-5].val_d),(double) (yyvsp[-3].val_l),(yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(double,long->double,double)\n");}}
#line 1739 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 48:
#line 130 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((double) (yyvsp[-5].val_l),(double) (yyvsp[-3].val_l),(yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(long->double,long->double,double)\n");}}
#line 1745 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 49:
#line 131 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((yyvsp[-5].val_d),(yyvsp[-3].val_d),(double) (yyvsp[-1].val_l));  if(data.Debug>0){printf("double=func(double,double,long->double)\n");}}
#line 1751 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 50:
#line 132 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((double) (yyvsp[-5].val_l),(yyvsp[-3].val_d),(double) (yyvsp[-1].val_l));  if(data.Debug>0){printf("double=func(long->double,double,long->double)\n");}}
#line 1757 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 51:
#line 133 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((yyvsp[-5].val_d),(double) (yyvsp[-3].val_l),(double) (yyvsp[-1].val_l));  if(data.Debug>0){printf("double=func(double,long->double,long->double)\n");}}
#line 1763 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 52:
#line 134 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-6].fnctptr)((double) (yyvsp[-5].val_l),(double) (yyvsp[-3].val_l),(double) (yyvsp[-1].val_l));  if(data.Debug>0){printf("double=func(long->double,long->double,long->double)\n");}}
#line 1769 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 53:
#line 135 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-2].fnctptr)((yyvsp[-1].string));  if(data.Debug>0){printf("double=func(image)\n");}}
#line 1775 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 54:
#line 136 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-4].fnctptr)((yyvsp[-3].string),(yyvsp[-1].val_d));  if(data.Debug>0){printf("double=func(image,double)\n");}}
#line 1781 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 55:
#line 137 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.val_d) = (yyvsp[-1].val_d);                         }
#line 1787 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 56:
#line 141 "calc_bison.y" /* yacc.c:1646  */
    {(yyval.string) = strdup((yyvsp[0].string));        data.cmdargtoken[data.cmdNBarg].type = 3; if(data.Debug>0){printf("this is a string (new variable/image)\n");}}
#line 1793 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 57:
#line 142 "calc_bison.y" /* yacc.c:1646  */
    {(yyval.string) = strdup((yyvsp[0].string));        data.cmdargtoken[data.cmdNBarg].type = 4; if(data.Debug>0){printf("this is a string (existing image)\n");}}
#line 1799 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 58:
#line 143 "calc_bison.y" /* yacc.c:1646  */
    {(yyval.string) = strdup((yyvsp[0].string));        data.cmdargtoken[data.cmdNBarg].type = 5; if(data.Debug>0){printf("this is a string (command)\n");}}
#line 1805 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 59:
#line 144 "calc_bison.y" /* yacc.c:1646  */
    {(yyval.string) = strdup((yyvsp[-2].string));        delete_image_ID((yyvsp[-2].string)); chname_image_ID((yyvsp[0].string),(yyvsp[-2].string)); if(data.Debug>0){printf("changing name\n");}}
#line 1811 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 60:
#line 145 "calc_bison.y" /* yacc.c:1646  */
    {(yyval.string) = strdup((yyvsp[-2].string));        chname_image_ID((yyvsp[0].string),(yyvsp[-2].string)); if(data.Debug>0){printf("changing name\n");}}
#line 1817 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 61:
#line 146 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_add((yyvsp[-2].string), (yyvsp[0].string), calctmpimname); (yyval.string) = strdup(calctmpimname); if(data.Debug>0){printf("image + image\n");}}
#line 1823 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 62:
#line 147 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstadd((yyvsp[-2].string),(double) (yyvsp[0].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname);  if(data.Debug>0){printf("image + double\n");}}
#line 1829 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 63:
#line 148 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstadd((yyvsp[-2].string),(double) (yyvsp[0].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image + long\n");}}
#line 1835 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 64:
#line 149 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstadd((yyvsp[0].string),(double) (yyvsp[-2].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("double + image\n");}}
#line 1841 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 65:
#line 150 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstadd((yyvsp[0].string),(double) (yyvsp[-2].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("long + image\n");}}
#line 1847 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 66:
#line 151 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_sub((yyvsp[-2].string), (yyvsp[0].string), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image + image\n");}}
#line 1853 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 67:
#line 152 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstadd((yyvsp[-2].string),(double) -(yyvsp[0].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image - double\n");}}
#line 1859 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 68:
#line 153 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstadd((yyvsp[-2].string),(double) -(yyvsp[0].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image - long\n");}}
#line 1865 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 69:
#line 154 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstsubm((yyvsp[0].string),(double) (yyvsp[-2].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("double - image\n");}}
#line 1871 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 70:
#line 155 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstsubm((yyvsp[0].string),(double) (yyvsp[-2].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("long - image\n");}}
#line 1877 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 71:
#line 156 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_mult((yyvsp[-2].string), (yyvsp[0].string), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image * image\n");}}
#line 1883 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 72:
#line 157 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstmult((yyvsp[-2].string),(double) (yyvsp[0].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image * double\n");}}
#line 1889 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 73:
#line 158 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstmult((yyvsp[-2].string),(double) (yyvsp[0].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image * long\n");}}
#line 1895 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 74:
#line 159 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstmult((yyvsp[0].string),(double) (yyvsp[-2].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("double * image\n");}}
#line 1901 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 75:
#line 160 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstmult((yyvsp[0].string),(double) (yyvsp[-2].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("long * image\n");}}
#line 1907 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 76:
#line 161 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_div((yyvsp[-2].string), (yyvsp[0].string), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image / image\n");}}
#line 1913 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 77:
#line 162 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstdiv((yyvsp[-2].string),(double) (yyvsp[0].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image - double\n");}}
#line 1919 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 78:
#line 163 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstdiv((yyvsp[-2].string),(double) (yyvsp[0].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image - long\n");}}
#line 1925 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 79:
#line 164 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstdiv((yyvsp[0].string),(double) (yyvsp[-2].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("double - image\n");}}
#line 1931 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 80:
#line 165 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstdiv((yyvsp[0].string),(double) (yyvsp[-2].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("long - image\n");}}
#line 1937 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 81:
#line 166 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstpow((yyvsp[-2].string),(double) (yyvsp[0].val_l), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image^long\n");}}
#line 1943 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 82:
#line 167 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_cstpow((yyvsp[-2].string),(double) (yyvsp[0].val_d), calctmpimname); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("image^double\n");}}
#line 1949 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 83:
#line 168 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_function_im_im__d_d((yyvsp[-1].string), calctmpimname, (yyvsp[-2].fnctptr)); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("double_func(double)\n");}}
#line 1955 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 84:
#line 169 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_function_imd_im__dd_d((yyvsp[-3].string), (double) (yyvsp[-1].val_d), calctmpimname, (yyvsp[-4].fnctptr)); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("double_func(double, double)\n");}}
#line 1961 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 85:
#line 170 "calc_bison.y" /* yacc.c:1646  */
    {sprintf(calctmpimname,"_tmpcalc%ld",data.calctmp_imindex); data.calctmp_imindex++; arith_image_function_imdd_im__ddd_d((yyvsp[-5].string), (double) (yyvsp[-3].val_d), (double) (yyvsp[-1].val_d), calctmpimname, (yyvsp[-6].fnctptr)); (yyval.string)=strdup(calctmpimname); if(data.Debug>0){printf("double_func(double, double, double)\n");}}
#line 1967 "calc_bison.c" /* yacc.c:1646  */
    break;

  case 86:
#line 171 "calc_bison.y" /* yacc.c:1646  */
    { (yyval.string) = strdup((yyvsp[-1].string));                         }
#line 1973 "calc_bison.c" /* yacc.c:1646  */
    break;


#line 1977 "calc_bison.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
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


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

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

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 176 "calc_bison.y" /* yacc.c:1906  */



#include <stdio.h>

int yylex();

int yyerror (s)  /* Called by yyparse on error */
     char *s;
{
  printf ("PARSING ERROR ON COMMAND LINE ARG %ld: %s\n", data.cmdNBarg, s);
  data.parseerror = 1;
	return 0;
}
