%{
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "calc.h"
#include "calc_bison.h"

#include "CLIcore.h"
#include "COREMOD_arith/COREMOD_arith.h"
#include "info/info.h"
extern DATA data;

long i;
%}

%option noyywrap

alpha   [A-Za-z?]
digit     [0-9]
string    ({alpha}|{digit}|[_.\$])({alpha}|{digit}|[_.\$])*
numberl [0-9]+
numberd	[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?

%%

{numberl}	 { 
yylval.val_l = atol(yytext); 
if(data.Debug>0){printf("Found long number %s -> %ld\n", yytext, yylval.val_l);} 
return TKNUMl; 
}

{numberd}	 { 
yylval.val_d = atof(yytext); 
if(data.Debug>0){printf("Found float number %s -> %f\n", yytext, yylval.val_d);} 
return TKNUMd; 
}



 

"sin("	{
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = sin;
return TKFUNC_d_d; 
}

"cos("  {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = cos;
return TKFUNC_d_d; 
}

"exp("  {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = exp;
return TKFUNC_d_d; 
}

"ln("  {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = log;
return TKFUNC_d_d; 
}

"log("  {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = log10;
return TKFUNC_d_d; 
}

"tan(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = tan;
return TKFUNC_d_d; 
}

"atan("  {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = atan;
return TKFUNC_d_d; 
}

"sqrt("  {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = sqrt;
return TKFUNC_d_d; 
}

"cbrt(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = cbrt;
return TKFUNC_d_d; 
}

"ceil(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = ceil;
return TKFUNC_d_d; 
}

"floor(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = floor;
return TKFUNC_d_d; 
}

"asin(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = asin;
return TKFUNC_d_d; 
}

"acos(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = acos;
return TKFUNC_d_d; 
}




"atan2(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = atan2;
return TKFUNC_dd_d; 
}


"trunc(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = Ptrunc;
return TKFUNC_ddd_d; 
}





"itot(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = arith_image_total;
return TKFUNC_im_d; 
}

"imin(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = arith_image_min;
return TKFUNC_im_d; 
}

"imax(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = arith_image_max;
return TKFUNC_im_d; 
}

"imean(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = arith_image_mean;
return TKFUNC_im_d; 
}





"perc(" {
if(data.Debug>0){printf("Found function %s\n", yytext);} 
yylval.fnctptr = img_percentile;
return TKFUNC_imd_d; 
}



{string}      {
if(data.Debug>0){printf("Found string %s\n", yytext);}
yylval.string = strdup(yytext);
if(variable_ID(yytext)!=-1) {if(data.Debug>0){printf("THIS IS A VARIABLE\n");} return TKVAR;}
if(image_ID(yytext)!=-1) {if(data.Debug>0){printf("THIS IS AN IMAGE\n");} return TKIMAGE;}
if(data.cmdNBarg==0)
{
data.cmdindex = -1;
 for(i=0;i<data.NBcmd;i++)
  { 
   if(!strcmp(yytext, data.cmd[i].key))
    {
     data.cmdindex = i;
     if(data.Debug>0){printf("THIS IS A COMMAND (%ld)\n",i);}
     return TKCOMMAND;
    }
   }
  }
 if(data.Debug>0){printf("THIS IS A NEW VARIABLE\n");}
 return TKNVAR;
}


"+"|"-"|"*"|"/"|"^"	{ 
if(data.Debug>0){printf("Found operator %s\n", yytext);} 
return ((int) yytext[0]); 
}

"("|")"		{ 
if(data.Debug>0){printf("Found parenthese %s\n", yytext);} 
return ((int) yytext[0]); 
}

","	{
if(data.Debug>0){printf("Found coma %s\n", yytext);} 
return ((int) yytext[0]); 
}


"="	{ 
if(data.Debug>0){printf("Found equal %s\n", yytext);} 
return ((int) yytext[0]); 
}


[ \t]+          /* eat up whitespace */

\n	{return ((int) yytext[0]);}

.	printf("Unrecognized character: %s\n", yytext);
%%


