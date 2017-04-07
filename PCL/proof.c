


/*************************************************************************/
/*                                                                       */
/*   File:        proof.c                                                */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Hauptprogramm zum Generieren von Gleichheitsketten     */
/*                                                                       */
/*   Aenderungen: <1> 7.2.1992 neu                                       */
/*                                                                       */
/*************************************************************************/


#include "pcl_doio.h"
#include "pcl_lemma.h"
#include "pcl_fproof.h"
#include "pcl_printproof.h"


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/

StepCell anchor;

BOOL     NoLemmas = FALSE,
         Latex = FALSE,
         Prolog = FALSE,
         PrSubst = TRUE,
         PrPlace = TRUE;


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
   OutputType output = ascii;

   anchor.pred = &anchor;
   anchor.succ = &anchor;

   InitIo(argc,argv,&anchor);

   if(Prolog)
   {
      AllStepsNewVars(&anchor);
   }

   ExtractProof(&anchor);
   BuildTree(&anchor);

   VERBOSE(fprintf(stderr,"Proof tree completed...\n"));

   if(!NoLemmas)
   {
      FindLemmas(&anchor,CRITERIA,ITERATE);
      VERBOSE(fprintf(stderr,"Lemmata seleted...\n"));
   }
      
   ProofForward(&anchor);
      
   VERBOSE(fprintf(stderr,"Proof generated...\n"));

   if(Latex)
   {
      if(Prolog)
      {
	 fprintf(stderr,"Warning: -latex and -prolog incompatible!\n");
      }
      output = latex;
   }
   if(Prolog)
   {
      output = prolog;
      NoBrackets = TRUE;
   }

   PrintProof(&anchor,output,PrSubst,PrPlace);

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
/* Globale Variable : out, in, WriteToFile, ReadFromFile, Verbose, Help,      */
/*                    printcomment, NoBrackets (aus pcl_terms.c), NoLemmas    */
/*                                                                            */
/* Seiteneffekte    : Lesen der Eingabe, setzen von out.                      */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                    <2> 17.5.1991 Support fuer Luxus-Optionen (-v, -c, -h)  */
/*                                                                            */
/******************************************************************************/


void InitIo(int argc,char* argv[],Step_p anchor)
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
      else if(strcmp(argv[f],"-v")==0)
      {
         Verbose = TRUE;
      }
      else if(strcmp(argv[f],"-h")==0)
      {
         Help = TRUE;
      }
      else if(strcmp(argv[f],"-nobrackets")==0)
      {
         NoBrackets = TRUE;
      }
      else if(strcmp(argv[f],"-nolemmas")==0)
      {
         NoLemmas = TRUE;
      }
      else if(strcmp(argv[f],"-latex")==0)
      {
         Latex = TRUE;
      }
      else if(strcmp(argv[f],"-prolog")==0)
      {
         Prolog = TRUE;
	 PrologVars = TRUE;

      }
            else if(strcmp(argv[f],"-nosubst")==0)
      {
         PrSubst = FALSE;
      }
      else if(strcmp(argv[f],"-noplace")==0)
      {
         PrPlace = FALSE;
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
      if (strcmp(argv[f],"-o")==0)
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
/* Aenderungen      : <1> 18.10.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintInfo()
{
   fprintf(stderr,"\
\n\n   proof 1.1  vom 7.7.1992\
\n\n   Usage: proof [-v] [-h] [-nobrackets] [-nolemmas]\
\n                [-o outfile] [infile1 ... infileN]\
\n\n   Geschrieben von Stephan Schulz\
\n\n   proof generiert aus eine PCL-Eingabe einen Gleichheitsbeweis,\
\n   der in einem an die menschliche Gewohnheit angepassten Format\
\n   ausgegeben wird.\
\n   Wird kein Eingabefile angegeben, so wird die Standard-\
\n   Eingabe gelesen, mehrere Eingabefiles werden wie eine\
\n   zusammenhaengende Datei behandelt.\
\n\n    OPTIONEN:\
\n\n    -v           Ausgabe von Ablaufinformationen.\
\n    -h           Ausgabe der Programminformationen, Abbruch.\
\n    -nobrackets  Unterdrueckt die Ausgabe von Klammern hinter\
\n                 Konstanten.\
\n    -nolemmas    Unterdrueckt die Suche nach Lemmata, nur die\
\n                 in der Eingabe spezifizierten Lemmata werden\
\n                 als solche behandelt.\
\n    -noplace     Keine Ausgabe der Stellen, an denen Gleichungen\
\n                 angewandt wurden.\
\n    -nosubst     Keine Ausgabe der verwendeten Substitututionen.\
\n    -latex       Ausgabe in LaTeX. Es wird kein vollstaendiges\
\n                 Dokument, sondern nur ein zum Einbinden in\
\n                 bestehende Dokumente geeigneter Textbaustein\
\n                 geschrieben.\
\n    -prolog      Ausgabe in einem Prolog-freundlichen Format.\
\n                 Diese Option impliziert -nobrackets, -noplace\
\n                 und -nosubst. Sie ist unvertraeglich mit -latex\
\n                 und ueberschreibt diese.\
\n    -o outfile   Schreibe Ausgabe in outfile. Entfaellt diese\
\n                 Option, so wird auf die Standard-Ausgabe ge-\
\n                 schrieben.\n\
\n\n    BUGS: Da PCL keine Reduktionsordnungen mehr kennt, koennen\
\n          allgemeine Paramodulationsschritte als Reduktionen be-\
\n          schrieben werden. proof kann verwirrt werden, wenn da-\
\n          im ersetzten Term Variable auftauchen, die nicht beim\
\n          Match gebunden wurden.\n\n\n");

}



/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


