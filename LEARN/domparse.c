/*-------------------------------------------------------------------------

File       : domparse.c 

Autor      : Stephan Schulz

Inhalt     : Hauptprogramm, zum Testen von Parser fuer Dom"anen etc.

Aenderungen: <1> 24.8.1994 neu (aus makedom.c)

-------------------------------------------------------------------------*/


#include "pcl_doio.h"
#include "lrn_parse.h"


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/*                       Forward-Deklarationen                                */
/*----------------------------------------------------------------------------*/

void main(int argc,char* argv[]);

void InitIo(int argc,char* argv[]);

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
   Dom_p    dom;

   InitIo(argc,argv);
   InitScanner(in, "Wer weiss");
   NextRealToken();

   dom = ParseDomHeadList();

   PrintDomHeadList(dom);

}



/*-------------------------------------------------------------------------

FUNCTION         : void InitIo(int argc,char* argv[])
                   IN     int argc
                   IN     char* argv[]

Beschreibung     : Setzt die Optionen, Oeffenen der Eingabedatei.

Globale Variable : out, ReadFromFile

Seiteneffekte    : Setzen der Optionen

Aenderungen      : <1> 17.4.1991 neu    
                   <2> 17.5.1991 Support fuer Luxus-Optionen (-v, -c, -h)
		   <3> 09.4.1993 Option -i 
		   <4> 21.8.1994 Umstellung fuer makedom, neue Box
		   <5> 24.8.1994 Umstellung fuer parsedom

-------------------------------------------------------------------------*/


void InitIo(int argc,char* argv[])
{
   int      f;

   for(f = 1; f<argc; f++)
   {
      if(strcmp(argv[f],"-v")==0)
      {
         Verbose = TRUE;
      }
      else if(strcmp(argv[f],"-h")==0)
      {
         Help = TRUE;
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

   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      if(*(argv[f])=='-')
      {   /* Optionen ueberspringen  */
      }
      else
      {
         VERBOSE(fprintf(stderr,"Opening %s...\n",argv[f]));
	 
	 if(!ReadFromFile)
	 {
	    OpenInput(argv[f]);
	 }
	 else
	 {
	    Error("Only one input file possible...\n");
	 }
      }
   }

   if(!ReadFromFile)
   {   /* lese Standard-Eingabe  */
      VERBOSE(fprintf(stderr,"Input stream is stdin...\n"));
   }
}





/*-------------------------------------------------------------------------

FUNCTION         : void PrintInfo()  

Beschreibung     : Gibt Informationen ueber das Programm aus. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 17.5.1991 neu
                   <2> 16.8.1994 Anpassung an makedom
		   <3> 24.8.1994 Anpassung an domparse

-------------------------------------------------------------------------*/


void PrintInfo()
{
   fprintf(stderr,"\n\
\n\
   domparse 0.8  vom 16.8.1999\n\
\n\
   Usage: domparse [-v] [-h] [infile]\n\
\n\
   Geschrieben von Stephan Schulz\n\
\n\
   domparse parst eine Domaene und gibt sie wieder aus...\n\
\n\
   OPTIONEN:\n\
    -v          Ausgabe von Programm- und Ablaufinformationen\n\
    -h          Ausgabe der Programminformationen, Abbruch.\n\
");
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/



