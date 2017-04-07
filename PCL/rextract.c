


/*************************************************************************/
/*                                                                       */
/*   File:        rextract.c                                             */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Fuehrt extract-Operation auf umgedrehten Files         */
/*                durch.                                                 */
/*                                                                       */
/*   Aenderungen: <1> 22.9.1991  neu                                     */
/*                <2> 17.4.1998  Alle Axiome extrahieren                 */
/*                                                                       */
/*************************************************************************/

#include "pcl_minianalyse.h"
#include "pcl_miniprinter.h"
#include "pcl_miniparse.h"



/*----------------------------------------------------------------------------*/
/*               Forward-Deklarationen interner Funktionen                    */
/*----------------------------------------------------------------------------*/

void main(int argc,char* argv[]);
void PrintInfo();
void DoExtract();


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/

FILE *out = stdout,
     *in = stdin;

BOOL WriteToFile = FALSE,
     ReadFromFile = FALSE,
     printcomment = FALSE,
     Verbose = FALSE,
     Help = FALSE,
     Reverse = FALSE;

miniStep_p anchor;
NumListList_p UsedList;


/*----------------------------------------------------------------------------*/
/*                       Funktionen                                           */
/*----------------------------------------------------------------------------*/



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void main(int argc,char* argv[])                        */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                                                                            */
/* Beschreibung     : Oeffnet Files und ruft lesende, extrahierende und       */
/*                    schreibende Funktionen auf.                             */
/*                                                                            */
/* Globale Variable : out, in, WriteToFile, ReadFromFile, printcomment,       */
/*                    Verbose, Help, Reverse                                  */
/*                                                                            */
/* Seiteneffekte    : Oeffnen und Schliessen der Files, Abwicklung des        */
/*                    Programmlaufs.                                          */
/*                                                                            */
/* Aenderungen      : <1> 19.9.1991  neu                                      */
/*                                                                            */
/******************************************************************************/


void main(int argc,char* argv[])
{
   int    f;
   miniStep_p handle;

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
            out = fopen(argv[++f],"w");
            if(!out)
            {
               fprintf(stderr,"ERROR: unable to open file %s...\n",argv[f]);
               exit(IO_ERROR);
            }
            WriteToFile = TRUE;
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
      else if(strcmp(argv[f],"-c")==0)
      {
         printcomment = TRUE;
      }
      else if(strcmp(argv[f],"-r")==0)
      {
         Reverse = TRUE;
      }
      else if(strcmp(argv[f],"-i")==0)
      {
         ExtIntermed = TRUE;
      }
      else if(strcmp(argv[f],"-l")==0)
      {
         ExtLast = TRUE;
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
   UsedList = AllocNumListListCell();
   UsedList->pred = UsedList;
   UsedList->succ = UsedList;
   UsedList->this = NULL;
   anchor = AllocminiStepCell();
   anchor->pred = anchor;
   anchor->succ = anchor;

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
         in = fopen(argv[f],"r");
         if(!in)
         {
            fprintf(stderr,"ERROR: unable to open file %s...\n",argv[f]);
            exit(IO_ERROR);
         }
         ReadFromFile = TRUE;
         InitScanner(in,argv[f]);

         DoExtract();

         fclose(in);
      }
   }

   if(!ReadFromFile)
   {   /* lese Standard-Eingabe  */
      InitScanner(in,"stdin");
      DoExtract();
   }
   if(!Reverse)
   {
      handle = anchor->succ;
      while(handle!=anchor)
      {
         PrintStep(handle);
         handle = handle->succ;
      }
   }  
   if(WriteToFile)
   {
      fclose(out);
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
/* Aenderungen      : <1> 22.9.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintInfo()
{
   fprintf(stderr,"\n\n   rextract 1.1  vom 16.2.1992");
   fprintf(stderr,"\n\n   Usage: rextract [-v] [-h] [-c] [-r] [- o outfile] [infile1 ... infileN]");
   fprintf(stderr,"\n\n   Geschrieben von Stephan Schulz");
   fprintf(stderr,"\n\n   rextract generiert aus einer umgedrehten PCL-Eingabe eine PCL-");
   fprintf(stderr,"\n   Ausgabe, die nur noch die fuer das Ergebnis notwendigen Schritte");
   fprintf(stderr,"\n   enthaelt. Wird kein Eingabefile angegeben, so wird die");
   fprintf(stderr,"\n   Standard-Eingabe gelesen, mehrere Eingabefiles werden wie");
   fprintf(stderr,"\n   eine zusammenhaengende Datei behandelt.");
   fprintf(stderr,"\n\n    OPTIONEN:");
   fprintf(stderr,"\n\n    -v          Ausgabe von Ablaufinformationen");
   fprintf(stderr,"\n    -h          Ausgabe der Programminformationen, Abbruch.");
   fprintf(stderr,"\n    -c          Uebernahme von Kommentaren im Eingabefile.");
   fprintf(stderr,"\n    -r          Ausgabe in umgedrehtem PCL-Format. Nur not-");
   fprintf(stderr,"\n                wendig bei Eingaben, bei denen der virtuelle");
   fprintf(stderr,"\n                Speicher nicht fuer den extrahierten Beweis");
   fprintf(stderr,"\n                ausreicht.");
   fprintf(stderr,"\n    -i          Extraktion auch nach den Schritten mit den Typen");
   fprintf(stderr,"\n                tes-intermed, tes-intermedgoal und");
   fprintf(stderr,"\n                crit-intermedgoal, die als Zwischenergebnisse");
   fprintf(stderr,"\n                beim parallelen Beweisen mit dem DISCOUNT-System");
   fprintf(stderr,"\n                entstehen.");
   fprintf(stderr,"\n    -l          Bedingungslose Extraktion des letzen Schrittes");
   fprintf(stderr,"\n                des PCL-Protokols. Dieser wird exakt so");
   fprintf(stderr,"\n                behandelt, als waere er vom Typ tes-final.");
   fprintf(stderr,"\n    -o outfile  Schreibe Ausgabe in outfile. Entfaellt diese");
   fprintf(stderr,"\n                Option, so wird auf die Standard-Ausgabe ge-");
   fprintf(stderr,"\n                schrieben.");
   fprintf(stderr,"\n\n   BUGS: PCL-Instruktionen, die mehr als eine Zeile benoetigen,");
   fprintf(stderr,"\n         werden bereits beim Umdrehen zerlegt, sie koennen nicht");
   fprintf(stderr,"\n         verarbeitet werden. Aus dem selben Grund werden Kommen-");
   fprintf(stderr,"\n         tare, die in einer anderen Zeile als die PCL-Instruktion");
   fprintf(stderr,"\n         stehen, den falschen Schritten zugeordnet.\n\n\n"); 
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void DoExtract()                                        */
/*                                                                            */
/* Beschreibung     :                                                         */
/*                                                                            */
/* Globale Variable :                                                         */
/*                                                                            */
/* Seiteneffekte    :                                                         */
/*                                                                            */
/* Aenderungen      :                                                         */
/*                                                                            */
/******************************************************************************/

void DoExtract()
{
   miniStep_p help;
   NumListList_p handle;
   BOOL InList,ExtStep;

   NextRealToken();
   while(akttoken.token!=NoToken)
   {
      help = ParseStep();
      InList = !CmpNumList(help->id,(UsedList->succ)->this);
      ExtStep= (help->type == tes_final)
               ||(ExtIntermed &&
                       ((help->type == tes_intermed)||
	               (help->type == tes_intermedgoal)||
	               (help->type == crit_intermedgoal)))
               ||(ExtLast)
               ||(help->just->operation==initial); /* Extraxt all axioms */

      ExtLast = FALSE; /* This is a hack...ExtLast implies an          */
		       /* unconditional extraction of the first        */
		       /* (respectively last) step of the PCL-Listing, */
		       /* thereafter it does not influence the         */
		       /* process. Therefore it serves two purposes,   */
		       /* its BOOLEAN Value is something like          */   
		       /* (Is_Last_Step && -l Flag was set)            */

      if(InList||ExtStep)
      {   /* Schritt wird benoetigt */
         if(!printcomment)
         {
            FREE(help->comment); 
            help->comment = secure_strdup(NullStr);
        } 
         MergeNumListLists(UsedList,GetStepParents(help)); /* Notwendige Schritte merken */

         if(InList)
         {   /* Abgearbeiten Schritt aus UsedList loeschen */
            handle = UsedList->succ;
            UsedList->succ = handle->succ;
            (handle->succ)->pred = UsedList;
            FreeNumList(handle->this);
            FreeNumListListCell(handle);
         }
         if(Reverse)
         {
            PrintStep(help);
            FreeminiStep(help);
         }
         else
         {
            help->pred = anchor;
            help->succ = anchor->succ;
            (anchor->succ)->pred = help;
            anchor->succ = help;
         }
      }
      else
      {
         FreeminiStep(help);
      }
   }
}




/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


