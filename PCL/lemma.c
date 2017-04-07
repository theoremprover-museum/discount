


/*************************************************************************/
/*                                                                       */
/*   File:        lemma.c                                                */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Hauptprogramm zum Extrahieren von Beweisen             */
/*                und finden von Lemmata                                 */
/*                                                                       */
/*   Aenderungen: <1> 17.4.1991 neu                                      */
/*                <2> 18.10.1991 Uebernahme von extract.c                */
/*                <3> 21.9.1992 Umstellung auf neue Optionen,            */
/*                              neue Verfahren                           */
/*                                                                       */
/*************************************************************************/


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

   if(newlemmas)
   {
      VERBOSE(fprintf(stderr,"Dropping old lemmas...\n"));
      ForgetLemmas(&anchor);
   }

   VERBOSE(fprintf(stderr,"Building proof tree...\n"));
   ExtractProof(&anchor);
   BuildTree(&anchor);

   VERBOSE(fprintf(stderr,"Searching for lemmata...\n"));
   FindLemmas(&anchor,criteria,iterate);

   VERBOSE(fprintf(stderr,"Printing results...\n"));
   PrintOut(&anchor);

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
#define INT_OPTS 15
#define REAL_OPTS 2
/* Clumsy, but... */
   int  f,g;
   char* int_opts[] = {"-s_average_size",
                       "-s_max_size",
                       "-o_min_used",
                       "-i_lemma_weight",
                       "-t_offset",
                       "-c_init_weight",
                       "-c_hypo_weight",
                       "-c_orient_weight",
                       "-c_cp_weight",
                       "-c_redu_weight",
                       "-c_inst_weight",
                       "-c_quot_weight",
                       "-c_lemma_weight",
                       "-p_max_length",
                       "-u_min_length",
                       "-u_min_used"};

   long int_results[]= {S_AVERAGE_SIZE,
                        S_MAX_SIZE,
                        O_MIN_USED,
                        I_LEMMA_WEIGHT,
                        T_OFFSET,
                        C_INIT_WEIGHT,
                        C_HYPO_WEIGHT,
                        C_ORIENT_WEIGHT,
                        C_CP_WEIGHT,
                        C_REDU_WEIGHT,
                        C_INST_WEIGHT,
                        C_QUOT_WEIGHT,
                        C_LEMMA_WEIGHT,
                        P_MAX_LENGTH,
                        U_MIN_LENGTH,
                        U_MIN_USED};

   char* real_opts[] = {"-s_min_fak",
                        "-s_max_fak",
                        "-t_weight_factor"};

   double real_results[] = {S_MIN_FAK,
                            S_MAX_FAK,
                            T_WEIGHT_FACTOR};                   

   BOOL found;

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
      else if(strcmp(argv[f],"-criteria")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -criteria expects additional argument... \n");
            exit(OPTION_ERROR);
         }
         criteria = argv[++f];
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
      else if(strcmp(argv[f],"-iterate")==0)
      {
         iterate = TRUE;
      }
      else if(strcmp(argv[f],"-newlemmas")==0)
      {
         newlemmas = TRUE;
      }
      else if(*(argv[f])=='-')
      {
         found=FALSE;
         for(g=0;(g<=INT_OPTS)&&!found;g++)
         {
            if(strcmp(argv[f],int_opts[g])==0)
            {
               if(f==argc-1)
               {
                 fprintf(stderr,"ERROR: %s expects integer value... \n",argv[f]);
                 exit(OPTION_ERROR);
               }
               int_results[g]=atol(argv[++f]);
               found = TRUE;
            }
         }
         for(g=0;(g<=REAL_OPTS)&&!found;g++)
         {
            if(strcmp(argv[f],real_opts[g])==0)
            {
               if(f==argc-1)
               {
                 fprintf(stderr,"ERROR: %s expects real value... \n",argv[f]);
                 exit(OPTION_ERROR);
               }
               real_results[g]=atof(argv[++f]);
               found = TRUE;
            }
         }
         if(!found)
         {
            fprintf(stderr,"ERROR: unknown option %s...\n",argv[f]);
            exit(OPTION_ERROR);
         }
      }
   }

   SetSyntacticParams(int_results[0],int_results[1],real_results[0],real_results[1]);
   SetOftenParams(int_results[2]);
   SetImportantParams(int_results[3]); 
   SetTreeParams(real_results[2],int_results[4]);
   SetCompletionParams(int_results[5],int_results[6],int_results[7],int_results[8],
                       int_results[9],int_results[10],int_results[11],int_results[12]);
   SetPartitionParams(int_results[13]);
   SetNegativeParams(int_results[14],int_results[15]);

   if(Help)
   {
      PrintInfo();
      exit(NO_ERROR);
   }

   InitPrinter(out);

   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      found=FALSE;  /* Optionen ueberspringen */
      
      for(g=0;(g<=INT_OPTS)&&!found;g++)
      {
         if(strcmp(argv[f],int_opts[g])==0)
         {
            found = TRUE;
         }
      }
      for(g=0;(g<=REAL_OPTS)&&!found;g++)
      {
         if(strcmp(argv[f],real_opts[g])==0)
         {
            found = TRUE;
         }
      }   
      if(found||(strcmp(argv[f],"-o")==0)||(strcmp(argv[f],"-criteria")==0))
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
   fprintf(stderr,"\n\n   lemma 2.01 vom 13.10.1992");
   fprintf(stderr,"\n\n   Usage: lemma [-v] [-h] [-c] [- o outfile] ... [infile1 ... infileN]");
   fprintf(stderr,"\n\n   Geschrieben von Stephan Schulz");
   fprintf(stderr,
   "\n\n   lemma generiert aus einer PCL-Eingabe eine PCL-Ausgabe,"
     "\n   die nur noch die fuer das Ergebnis notwendigen Schritte"
     "\n   enthaelt. Dabei wird der Beweis durch Lemmata gegliedert."
     "\n   Wird kein Eingabefile angegeben, so wird die Standard-"
     "\n   Eingabe gelesen, mehrere Eingabefiles werden wie eine"
     "\n   zusammenhaengende Datei behandelt."
   "\n\n    OPTIONEN:"
     "\n    In der neuesten Version erlaubt lemma die Nutzung"
     "\n    verschiedener Kriterien zur Lemma-Erzeugung. Dieses bedingt eine"
     "\n    Vielzahl von Optionen, die hier in Gruppen zusammengefasst werden." 
   "\n\n    (1) Optionen, die das Ein- und Ausgabeverhalten beeinflussen."
   "\n\n    -v                Ausgabe von Ablaufinformationen"
     "\n    -h                Ausgabe der Programminformationen, Abbruch."
     "\n    -c                Uebernahme von Kommentaren im Eingabefile,"
     "\n                      Generierung von neuen Kommentaren, die die ver-"
     "\n                      wendeten Kriterien erlaeutern."
     "\n    -o outfile        Schreibe Ausgabe in outfile. Entfaellt diese"
     "\n                      Option, so wird auf die Standard-Ausgabe ge-"
     "\n                      schrieben."
   "\n\n   (2) Optionen zur allgemeinen Steuerung"
   "\n\n   -criteria w        Gibt die fuer die Suche zu verwendenden Kriterien"
     "\n                      an. w ist ein Wort, das nur die Buchstaben"
     "\n                      'soitcp' enthalten darf. Sie stehen fuer:"
     "\n                      's': Syntaktische Kriterien"
     "\n                      'o': Oft benutzte Schritte"
     "\n                      'i': Wichtige Schritte"
     "\n                      't': Isolierte Teilbaeume"
     "\n                      'c': Verwendete Vervollstaendigungsschritte"
     "\n                      'p': Beweisunterteilung"
     "\n                      Default ist \""CRITERIA"\"."
     "\n   -iterate           Suche Lemmata, indem die zu verwendenden"
     "\n                      Kriterien nacheinander auf den kompletten"
     "\n                      Beweis angewendet werden. Ansonsten werden"
     "\n                      die Kriterien auf einzelne Schritte angewendet."
   "\n\n   (3) Optionen zu abweisenden Lemma-Kriterien"
   "\n\n   -u_min_length n    Mindestlaenge fuer die Beweiskette eines"
     "\n                      potentiellen Lemmas. Default ist %ld, bei n=0"
     "\n                      kommt das Kriterium nicht zur Wirkung."
     "\n   -u_min_used n      Ein Schritt wird nur zu einem Lemma, wenn er"
     "\n                      mindestens n mal im weiteren Beweis verwendet"
     "\n                      wird. Default ist %ld, bei n=0 kommt das"
     "\n                      Kriterium nicht zur Wirkung."
   "\n\n   (4) Optionen zu syntaktischen Kriterien"
   "\n\n    -s_average_size n Ein Schritt wird Lemma, falls die"
     "\n                      durchschnittliche Termgroesse kleiner oder gleich"
     "\n                      n ist. Default ist %ld, bei n=0 kommt das"
     "\n                      Kriterium nicht zur Wirkung."
     "\n   -s_max_size n      Ein Schritt wird Lemma, falls die"
     "\n                      maximale Termgroesse kleiner oder gleich"
     "\n                      n ist. Default ist %ld, bei n=0 kommt das"
     "\n                      Kriterium nicht zur Wirkung."
     "\n   -s_min_fak r       Ein Schritt wird Lemma, falls das Verhaeltnis der"
     "\n                      Termgroessen groesser oder gleich r ist. Default"
     "\n                      ist %f, bei r=0.0 wird das Kriterium nicht"
     "\n                      verwendet."
     "\n   -s_max_fak r       Ein Schritt wird Lemma, falls das Verhaeltnis der"
     "\n                      Termgroessen kleiner oder gleich r ist. Default"
     "\n                      ist %f, bei r=0.0 wird das Kriterium nicht"
     "\n                      verwendet."
   "\n\n   (5) Optionen zur Suche nach haeufig verwendeten Lemmata"
   "\n\n   -o_min_used n      Ein Schritt wird zum Lemma, wenn er"
     "\n                      oefter als n mal im weiteren Beweis"
     "\n                      verwendet wird. Default ist %ld." 
   "\n\n   (6) Optionen zur Suche nach wichtigen Lemmata"
   "\n\n   -i_lemma_weight n  Ein Schritt wird Lemma, falls das Produkt aus der"
     "\n                      Laenge der Beweiskette und der Zahl der"
     "\n                      Verwendungen groesser oder gleich n ist. Default"
     "\n                      ist %ld."
   "\n\n   (7) Optionen zur Suche nach isolierten Teilbeweisen"
   "\n\n   -t_weight_factor r Setzt t_weight_factor, Default ist %f."
     "\n   -t_offset n        Setzt t_offset. Default ist %ld. Die Verwendung"
     "\n                      der Daten wird in der ausfuehrlichen Dokumentation"
     "\n                      erlaeutert."
   "\n\n   (8) Optionen zur Lemmasuche nach verwendeten Inferenzschritten"
   "\n\n   -c_init_weight n   Setzt den Wert, mit dem der PCL-Ausdruck"
     "\n                      \"initial\" fuer die Lemma-Suche gewichtet wird,"
     "\n                      auf den Wert n. Default ist %ld."
     "\n   -c_hypo_weight n   Ditto fuer \"hypothesis\". Default ist %ld."
     "\n   -c_orient_weight n Setzt den Wert, mit dem ein PCL-Ausdruck mit"
     "\n                      Operation \"orient\" zum Gewicht des Ausdruckes"
     "\n                      fuer die Lemma-Suche beitraegt auf den Wert n."
     "\n                      Default ist %ld."
     "\n   -c_cp_weight n     Ditto fuer \"cp\". Default ist %ld."
     "\n   -c_redu_weight n   Ditto fuer \"tes-red\". Default ist %ld."
     "\n   -c_inst_weight n   Ditto fuer \"instance\". Default ist %ld."
     "\n   -c_quot_weight n   Ditto fuer Ausdruecke, die nur Verweise auf"
     "\n                      andere Schritte sind. Default ist %ld."
     "\n   -c_lemma_weight n  Setzt Schwellwert fuer Lemma-Generierung. Ein"
     "\n                      PCL-Schritt, dessen Gewicht ueber diesem Wert"
     "\n                      liegt, wird ein Lemma. Default ist %ld."
   "\n\n   (9) Optionen, die die Unterteilung langer Beweisketten steuern"
   "\n\n   -p_max_length n    Ein Schritt, dessen Beweislaenge groesser als n"
     "\n                      ist, wird automatisch zum Lemma. Default ist %ld."
 "\n\n\n   BUGS: - lemma testet nur, ob Argumente vorhanden sind, der Typ wird"
     "\n           nicht ueberprueft."
     "\n         - Jede Option muss fuer sich stehen."
     "\n         - Die Laenge der Beweiskette beruecksichtigt nicht die von proof"
     "\n           vorgenommenen Verkuerzungen, wenn Beweisschritte durch"
     "\n           Instanziierung ueberfluessig werden." 
     "\n\n",
     U_MIN_LENGTH, U_MIN_USED, S_AVERAGE_SIZE, S_MAX_SIZE, S_MIN_FAK, S_MAX_FAK,
     O_MIN_USED,I_LEMMA_WEIGHT, T_WEIGHT_FACTOR, T_OFFSET,C_INIT_WEIGHT, 
     C_HYPO_WEIGHT, C_ORIENT_WEIGHT, C_CP_WEIGHT, C_REDU_WEIGHT,
     C_INST_WEIGHT, C_QUOT_WEIGHT, C_LEMMA_WEIGHT, P_MAX_LENGTH);
}



/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


