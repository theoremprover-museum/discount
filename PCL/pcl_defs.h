/*-------------------------------------------------------------------------

File        : pcl_defs.h  (Version fuer die urspruenglichen PCL-Programme)       

Autor       : Stephan Schulz

Inhalt      : Standard-Definitionen fuer alle Files. Diese Datei
              existiert noch einmal fuer die Lern-Programme, die
	      ansonsten gro"se Teile des Codes wiederverwenden. Ich
	      hoffe, die Aenderungen auf diese Dateien beschr"anken zu
	      k"onnen.

Aenderungen : <1> 8.2.1991  neu 
              <2> 16.6.1994 Kommentare neu, ErrStr rausgeworfen

-------------------------------------------------------------------------*/


#ifndef _pcl_defs

#define _pcl_defs


#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "systeminclude.h"  


/*-----------------------------------------------------------------------*/
/*                           DEBUG-Parameter                             */
/*-----------------------------------------------------------------------*/

#define DNULL       0
#define DALLOC     64
#define DMAIN      16
#define DPARSETC    2
#define DTOKEN      4 
#define DMPARSE    32
#define DSTRINGS    1
#define DSCAN       8
#define DNEXTCHAR 128
#define DTMEM     256
#define DPROOF    512
#define DTEQ     1024
#define DMATCH   2048
#define DEBUGFLAG (DNULL) 

#define DEBUG(level,com) if(level & DEBUGFLAG){com;}


/*-----------------------------------------------------------------------*/
/*          Komfortable Macros, teileweise redundant                     */
/*-----------------------------------------------------------------------*/



#define iswhite(ch) ((ch==' ')||(ch=='\n')||(ch=='\t'))
#define ischar(ch)  ((ch)!=EOF)

#define FREE(ptr)  {if(ptr) free(ptr);} 

#define min(x,y)  (((x)>(y))?(y):(x))
#define max(x,y)  (((x)<(y))?(y):(x))

/* Used to stiffle warnings about control - volatile seems to have */
/* problems with some versions of GNU C (?)                        */

#define FAKE_RETURN return 0

#define MAX_LONG ((2^31)-1)

/*-----------------------------------------------------------------------*/
/*                       Typen und Verwandtes                            */
/*-----------------------------------------------------------------------*/


#define TRUE 1
#define FALSE 0

typedef int BOOL;

typedef enum 
{
   NoPair,
   tes_eqn,
   tes_rule,
   tes_lemma,
   tes_goal,
   tes_final,   
   tes_final_r,  /* nur fuer trans ... */
   tes_final_e,  /* ...                */
   crit_goal,          /* Neue Typen f"ur die parallele Extraktion und */
   tes_intermed,       /* f"ur die Behandlung von kritischen Zielen...*/
   tes_intermedgoal,   /* */
   crit_intermedgoal   /* */
}StepType;

typedef enum
{
   eqn,
   rule
}PairType;


typedef enum
{
   initial,
   hypothesis,
   orientu,
   orientx,
   cp,
   tes_red,
   instance,
   quotation
}OpType;



/*-----------------------------------------------------------------------*/
/*                Error-Codes fuer exit()                                */
/*-----------------------------------------------------------------------*/


#define NO_ERROR       0
#define OPTION_ERROR   1
#define SYNTAX_ERROR   2
#define IO_ERROR       3
#define OUT_OF_MEM    10
#define OTHER_ERROR  999 



#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/

