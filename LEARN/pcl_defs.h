/*-------------------------------------------------------------------------

File        : pcl_defs.h  (Version fuer die Lernprogramme)       

Autor       : Stephan Schulz

Inhalt      : Standard-Definitionen fuer alle Files. Dies ist die neue
              Version fuer die Lern-Programme. Sie definiert das
	      Symbol LEARN_VERSION und wird (potentiell) noch weitere
	      Aenderungen gegen"uber der selben Datei f"ur die
	      PCL-Programme enthalten. ich hoffe, da"s diese Date die
	      einzige bleibt, die wirklich doppelt vorhanden ist.

Aenderungen : <1> 16.6.1994 neu (von pcl_defs.h fuer die pcl-Programme)

-------------------------------------------------------------------------*/


#ifndef _pcl_defs

#define _pcl_defs


#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>

#include "systeminclude.h" 

#define LEARN_VERSION    __is_learn__

#define NAMES_ARE_STRINGS /* Needed for adapting parse routines. If
			     this Macro is defined, the programs will
			     accept both identifiers and "" delimited
			     strings as names in the knowledge
			     base. They will always print names as
			     strings. Otherwise, only identifiers
			     (consisting of idchars as defined in
			     pcl_scan.h) are allowed, and names will
			     not be delimited by quotes. */

#ifdef NAMES_ARE_STRINGS
#define NAME_FORMAT      "\"%s\""  /* Output names as strings */
#else
#define NAME_FORMAT      "%s"
#endif /* NAME_FORMAT */


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
/*          Komfortable Macros, teilweise redundant                     */
/*-----------------------------------------------------------------------*/



#define ischar(ch)  ((ch)!=EOF)

#define FREE(ptr) {if(ptr){free(ptr);ptr = NULL;}}

#define min(x,y)  (((x)>(y))?(y):(x))
#define max(x,y)  (((x)<(y))?(y):(x))

/* Used to stiffle warnings about control - volatile seems to have */
/* problems with some versions of GNU C (?)                        */

#define FAKE_RETURN return 0

/* Irgendwie entspricht ctype.h nicht der man-Page...deshalb: */

#ifndef tolower
#define tolower(c)      ((c) - 'A' + 'a')
#endif


/*-----------------------------------------------------------------------*/
/*                       Typen und Verwandtes                            */
/*-----------------------------------------------------------------------*/


#define TRUE 1
#define FALSE 0

typedef int BOOL;

#define XOR(x1,x2) (((x1)&&!(x2))||(!(x1)&&(x2)))
#define EQUIV(x1,x2) (((x1)&&(x2))||(!(x1)&&!(x2)))


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
#define ARG_ERROR      2
#define SYNTAX_ERROR   3
#define IO_ERROR       4
#define SYSTEM_ERROR   5
#define OUT_OF_MEM    10
#define OTHER_ERROR  999 

/*-----------------------------------------------------------------------*/
/*                Konstanten                                             */
/*-----------------------------------------------------------------------*/

#define MAXAXIOMS    512


#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/

