/*-------------------------------------------------------------------------

File        : pcl_minidoio.h 

Autor       : Stephan Schulz

Inhalt      : Deklarationen fuer pcl_minidoio.c  

Aenderungen : <1> 16.7.1991 Uebernahme von pcl_doio.h
              <2>  8.3.1994 Neue Funktion: ProtocolSize()

-------------------------------------------------------------------------*/

#ifndef _pcl_minidoio

#define _pcl_minidoio


#include "pcl_miniparse.h"
#include "pcl_printer.h"


/*-----------------------------------------------------------------------*/
/*                Deklaration exportierter Variablen                     */
/*-----------------------------------------------------------------------*/


extern FILE *out,
            *in;

extern BOOL WriteToFile,
            ReadFromFile,
            Help,
            Statistics,
            Verbose,
            printcomment;
/* PrintAnnotations ist in pcl_miniparse.[hc] realisiert *seufz* */

extern long PclSteps;
extern long PclIntermeds;
extern long ProtocolIntermeds;


/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


#define VERBOSE(todo) {if(Verbose){todo;}}
#define VERBOUT(arg)  {if(Verbose){fprintf(stderr,(arg));}}

void EndIo();
void OpenOutput(char* name);
void OpenInput(char* name);
void ReadFile(char* name,miniStep_p anchor);
void ReadIn(miniStep_p anchor);
void PrintOut(miniStep_p anchor);
void PrintBack(miniStep_p anchor);

long ProtocolSize(miniStep_p anchor);


#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/

