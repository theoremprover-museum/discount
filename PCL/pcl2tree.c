/*-------------------------------------------------------------------------

File        : pcl2tree.c

Autor       : Stephan Schulz

Inhalt      : Baumaufbereitung von PCL-Beweisen

Aenderungen : <1> 25.4.1992 Ubernahme von lemma.c

-------------------------------------------------------------------------*/


#include "pcl_doio.h"
#include "pcl_lemma.h"


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/

StepCell anchor;
BOOL     iterate = ITERATE,
         newlemmas = FALSE;
char*    criteria = CRITERIA; /* Maximal: "sotcpu"  */


/*----------------------------------------------------------------------------*/
/*                       Forward-Deklarationen                                */
/*----------------------------------------------------------------------------*/

void main(int argc,char* argv[]);

void InitIo(int argc,char* argv[],Step_p anchor);

void PrintInfo();


/*----------------------------------------------------------------------------*/
/*                      Funktionen                                            */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void main(int argc,char* argv[])                        */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                                                                            */
/* Beschreibung     : Steuerung des Programmlaufs.                            */
/*                                                                            */
/* Globale Variable : anchor                                                  */
/*                                                                            */
/* Seiteneffekte    : Bearbeitung                                             */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991  neu                                      */
/*                                                                            */
/******************************************************************************/


void main(int argc,char* argv[])
{
   anchor.pred = &anchor;
   anchor.succ = &anchor;

   InitIo(argc,argv,&anchor);

   VERBOSE(fprintf(stderr,"Printing results...\n"));
   TreeOut(&anchor);

   EndIo();
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void InitIo(int argc,char* argv[],Step_p anchor)        */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                    IN     Step_p anchor                                    */
/*                                                                            */
/* Beschreibung     : Liest die in der Kommandozeile angegebenen Files und    */
/*                    setzt das Ausgabefile.                                  */
/*                                                                            */
/* Globale Variable : out, in, WriteToFile, ReadFromFile                      */
/*                                                                            */
/* Seiteneffekte    : Lesen der Eingabe, setzen von out.                      */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                    <2> 17.5.1991 Support fuer Luxus-Optionen (-v, -c, -h)  */
/*                                                                            */
/******************************************************************************/


void InitIo(int argc,char* argv[],Step_p anchor)
{
   long f;

   for(f = 1; f<argc; f++)
   {
      if(strcmp(argv[f],"-o")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -o expects filename... \n");
            exit(OPTION_ERROR);
         }
         else if(WriteToFile)
         {
            fprintf(stderr,"ERROR: No more than one -o Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
	    OpenOutput(argv[++f]);
         }
      }
      else if(strcmp(argv[f],"-v")==0)
      {
         Verbose = TRUE;
      }
      else if(strcmp(argv[f],"-h")==0)
      {
         Help = TRUE;
      }
      else if(strcmp(argv[f],"-unfold")==0)
      {
         GatherNodes = FALSE;
      }
      else if(*(argv[f])=='-')
      {
         fprintf(stderr,"ERROR: unknown option %s...\n",argv[f]);
         exit(OPTION_ERROR);
      }
   }

   VERBOSE(PrintInfo());

   if(Help)
   {
      PrintInfo();
      exit(NO_ERROR);
   }

   InitPrinter(out);

   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      if(strcmp(argv[f],"-o")==0)
      {
         f++;
      }
      else if(*(argv[f])=='-')
      {   /* Optionen ueberspringen  */
      }
      else
      {
         
         VERBOSE(fprintf(stderr,"Reading %s...\n",argv[f]));
         
         ReadFile(argv[f],anchor);
         
         VERBOSE(fprintf(stderr,"...done.\n"));
      }
   }
   
   if(!ReadFromFile)
   {   /* lese Standard-Eingabe  */
      
      VERBOSE(fprintf(stderr,"Reading stdin...\n"));
      
      InitScanner(in,"stdin");
      
      ReadIn(anchor);
      
      VERBOSE(fprintf(stderr,"...done.\n"));
   }
}




/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintInfo()                                        */
/*                                                                            */
/* Beschreibung     : Gibt Informationen ueber das Programm aus.              */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 18.10.1991 neu                                      */
/*                                                                            */
/******************************************************************************/

void PrintInfo()
{
   fprintf(stderr,"\n\n\
  pcl2tree 1.0\n\
\n\
  Usage: pcl2tree [-v] [-h][- o outfile] [infile1 ... infileN]\n\
\n\
  Geschrieben von Stephan Schulz\n\
\n\
  pcl2tree generiert aus einer PCL-Eingabe eine Ausgabe, die fuer tview\n\
  verstaendlich ist und eine graphische Darstellung des Beweises\n\
  ermoeglicht.\n\
\n\
   OPTIONEN:\n\
   -v           Ausgabe von Programm- und Ablaufinformationen\n\
   -h           Ausgabe der Programminformationen, Abbruch.\n\
   -unfold      Entfaltet den Graphen. Doppelt benutzte Knoten werden\n\
                auch mehrfach angezeigt.\n\
   -o outfile   Schreibe Ausgabe in outfile. Entfaellt diese Option, so
                wird auf die Standard-Ausgabe geschrieben.\n\
\n");
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


