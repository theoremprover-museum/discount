/*************************************************************************/
/*                                                                       */
/*   File:        pcl_doio.h                                             */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen fuer pcl_doio.c                          */
/*                                                                       */
/*   Aenderungen: <1> 17.4.1991 neu                                      */
/*                                                                       */
/*************************************************************************/

#ifndef _pcl_doio

#define _pcl_doio


#include "pcl_parse.h"
#include "pcl_printer.h"

#define VERBOSE(todo) {if(Verbose){todo;}}
#define VERBOUT(arg)  {if(Verbose){fprintf(stderr,(arg));}}

/*----------------------------------------------------------------------------*/
/*                   Forward-Deklarationen                                    */
/*----------------------------------------------------------------------------*/

void EndIo();
void OpenOutput(char* name);
void OpenInput(char* name);
void ReadFile(char* name,Step_p anchor);
void ReadIn(Step_p anchor);
void PrintOut(Step_p anchor);
void PrintBack(Step_p anchor);

extern FILE *out,
            *in;

extern BOOL WriteToFile,
            ReadFromFile,
            Help,
            Verbose,
            printcomment;


#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


