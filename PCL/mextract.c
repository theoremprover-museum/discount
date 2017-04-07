/*-----------------------------------------------------------------------

File  : mextract.c 

Author: Stephan Schulz

Contents
 
  Hauptprogramm zum Extrahieren von Beweisen  

Changes

<1> 17.4.1991 neu
<2> 17.7.1991 Uebernahme fuer miniextract.
<3> 29.9.1996 Neue Comment-Boxen, Aufbohren fuer die Extraktion mit
    verschiedenen Leveln (nur benoetigte, auch einen Schritt
    entfernte, usw.)
<4> 22.5.1997 Cost function uund cost-abhaengige Extraction

-----------------------------------------------------------------------*/

#include "pcl_minidoio.h"
#include "pcl_minianalyse.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

miniStepCell anchor;
long         MaxRWDistance = MAX_LONG,
             MaxCPDistance = MAX_LONG,
             MinCPCost     = 0;
BOOL         RemoveTrivials = TRUE;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void main(int argc,char* argv[]);

void InitIo(int argc,char* argv[],miniStep_p anchor);

void PrintInfo();

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

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
   long norm1_k = 0, 
        norm2_k = 0, 
	extract1_res = 0,
        extract2_res = 0,
        intermeds1 = 0,
        intermeds2 = 0;

   DEBUG(16,printf("main...\n"));

   anchor.pred = &anchor;
   anchor.succ = &anchor;

   DEBUG(16,printf("...init...\n"));

   InitIo(argc,argv,&anchor);

   DEBUG(16,printf("...InitIo...\n"));

   VERBOSE(fprintf(stderr,"Starting extraction.\n"));

   if(Normalize)
   {
      VERBOSE(fprintf(stderr,"Starting normalization.\n"));
      norm1_k = NormalizeProof(&anchor);
   }
   
   if(PrintAnnotations)
   {
      ExtractProofLevels(&anchor,MaxRWDistance,MaxCPDistance,
			 MinCPCost, RemoveTrivials);
   }
   else
   {
      ExtractProof(&anchor, FALSE);
   }

   if(Statistics)
   {
      extract1_res = ProtocolSize(&anchor);
      intermeds1 = ProtocolIntermeds;
   }
   
   if(Normalize2)
   {
      VERBOSE(fprintf(stderr,"Starting strong normalization.\n"));
      norm2_k = StrongNormalizeProof(&anchor);
   }
  
   if(Normalize || Normalize2)
   {
      VERBOSE(fprintf(stderr,"Starting second extraction.\n"));
      if(PrintAnnotations)
      {
	 ExtractProofLevels(&anchor,MaxRWDistance,MaxCPDistance,
			    MinCPCost, RemoveTrivials);
      }
      else
      {
	 ExtractProof(&anchor, FALSE);
      }
   }

   if(Statistics)
   {
      extract2_res = ProtocolSize(&anchor);
      intermeds2 = ProtocolIntermeds;
   }
   
   VERBOSE(fprintf(stderr,"Done...\n"));

   PrintOut(&anchor);
   
   if(Statistics)
   {
      fprintf(stderr,"\n\
================ mextract statistics block ===================\n\n\
  Original listing: Steps read: %ld tes-intermeds: %ld.\n\n", PclSteps,
	      PclIntermeds);
      if(Normalize)
      {
	 fprintf(stderr,"  Normalization found %ld redundancies.\n",
		 norm1_k);
      }
      fprintf(stderr,"  First extraction: Steps: %ld tes-intermeds: %ld.\n", 
	      extract1_res, intermeds1);
      if(Normalize2)
      {
	 fprintf(stderr,"  Strong Normalization found %ld redundancies.\n\n",
		 norm2_k);
      }
      fprintf(stderr,"\
  Final listing: Steps written: %ld tes-intermeds: %ld.\n\n\
==============================================================\n",
	      extract2_res, intermeds2);
   }

   DEBUG(16,printf("...main\n"));

   EndIo();
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void InitIo(int argc,char* argv[],miniStep_p anchor)    */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                    IN     miniStep_p anchor                                */
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
/*                    <3> 17.7.1991 Aenderungen fuer miniextract              */
/*                    <4> 09.4.1993 Option -i                                 */
/*                                                                            */
/******************************************************************************/


void InitIo(int argc,char* argv[],miniStep_p anchor)
{
   int  f;

   for(f = 1; f<argc; f++)
   {    /* Teste, ob -o Option gesetzt und korrekt, teste, ob illegale Optionen */
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
      else if(strcmp(argv[f],"-r")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -r expects argument... \n");
            exit(OPTION_ERROR);
         }
	 MaxRWDistance = atol(argv[++f]);
	 PrintAnnotations = TRUE;
      }
      else if(strcmp(argv[f],"-p")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -p expects argument... \n");
            exit(OPTION_ERROR);
         }
	 MaxCPDistance = atol(argv[++f]);
	 PrintAnnotations = TRUE;
      }
      else if(strcmp(argv[f],"-m")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -m expects argument... \n");
            exit(OPTION_ERROR);
         }
	 MinCPCost = atol(argv[++f]);
	 PrintAnnotations = TRUE;
      }
      else if(strcmp(argv[f],"-v")==0)
      {
         Verbose = TRUE;
      }
      else if(strcmp(argv[f],"-h")==0)
      {
         Help = TRUE;
      }
      else if(strcmp(argv[f],"-s")==0)
      {
         Statistics = TRUE;
      }
      else if(strcmp(argv[f],"-c")==0)
      {
         printcomment = TRUE;
      }
      else if(strcmp(argv[f],"-i")==0)
      {
         ExtIntermed = TRUE;
      }
      else if(strcmp(argv[f],"-l")==0)
      {
         ExtLast = TRUE;
      }
      else if(strcmp(argv[f],"-n")==0)
      {
         Normalize = TRUE;
      }
      else if(strcmp(argv[f],"-n2")==0)
      {
         Normalize2 = TRUE;
      }
      else if(strcmp(argv[f],"-t")==0)
      {
         RemoveTrivials = FALSE;
      }    
      else if(*(argv[f])=='-')
      {
         fprintf(stderr,"ERROR: unknown option %s...\n",argv[f]);
         exit(OPTION_ERROR);
      }
   }

   if(Help)
   {
      PrintInfo();
      exit(NO_ERROR);
   }

   InitPrinter(out);

   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      if (strcmp(argv[f],"-o")==0 || 
	  strcmp(argv[f],"-r")==0 || 
	  strcmp(argv[f],"-p")==0 || 
	  strcmp(argv[f],"-m")==0)
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
/* Aenderungen      : <1> 17.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintInfo()
{
   fprintf(stderr,"\
\n\n   mextract 1.2  vom 30.9.1992\
\n\n   Usage: mextract [-v] [-h] [-c] [- o outfile] [infile1 ... infileN]\
\n\n   Geschrieben von Stephan Schulz\
\n\n   mextract generiert aus einer PCL-Eingabe eine PCL-Ausgabe,\
\n   die nur noch die fuer das Ergebnis notwendigen Schritte\
\n   enthaelt. Wird kein Eingabefile angegeben, so wird die\
\n   Standard-Eingabe gelesen, mehrere Eingabefiles werden wie\
\n   eine zusammenhaengende Datei behandelt.\
\n\n    OPTIONEN:\
\n\n    -v          Ausgabe von Ablaufinformationen\
\n    -h          Ausgabe der Programminformationen, Abbruch.\
\n    -s          Statistische Ausgaben ueber Schrittzahlen.\
\n    -c          Uebernahme von Kommentaren im Eingabefile.\
\n    -i          Extraktion auch nach den Schritten mit den Typen\
\n                tes-intermed, tes-intermedgoal und\
\n                crit-intermedgoal, die als Zwischenergebnisse\
\n                beim parallelen Beweisen mit dem DISCOUNT-System\
\n                entstehen.\
\n    -l          Bedingungslose Extraktion des letzen Schrittes\
\n                des PCL-Protokols. Dieser wird exakt so\
\n                behandelt, als waere er vom Typ tes-final.\
\n    -n          'Normalisieren' des Beweiseslistings, d.h.\
\n                Zusammenfassen von Schritten, die nur durch\
\n                Zitate begruendet werden, zu einem einzelnen\
\n                Schritt (mit dem aussagekraeftigstem Typ).\
\n    -n2         Starkes Normalisieren: Schritte mit gleichem Fakt\
\n                werden zusammengefasst, wenn ihre Typen vertraeglich\
\n                sind. tes-final ist mit keinem anderen Typ\
\n                vertraeglich!\
\n    -r level    Extraktion auch nicht benutzter Schritte bis zum\
\n                Rewrite-Abstand <level> vom Beweis.\
\n    -p level    Extraktion auch nicht benutzter Schritte bis zum\
\n                CP-Abstand <level> vom Beweis.\
\n    -m cost     Extraction auch nicht benutzter Schritte, die\
\n                mindestens <cost> cp-Inferenzenn zur Folge haben.
\n    -t          Behalte auch Schritte, die im Beweisprozess als\
\n                trivial nachgewiesen werden, bevor sie weitere\
\n                Nachkommen erzeugen (nur sinnvoll bei -r oder -p\
\n                Optionen).\
\n    -o outfile  Schreibe Ausgabe in outfile. Entfaellt diese\
\n                Option, so wird auf die Standard-Ausgabe ge-\
\n                schrieben.\n\n\n");

}



/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


