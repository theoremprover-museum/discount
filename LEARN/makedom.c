
/*-------------------------------------------------------------------------

File       : makedom.c 

Autor      : Stephan Schulz

Inhalt     : Hauptprogramm, das aus einem PCL-Listing eine der 3
             Dom"anen-Formen macht.

Aenderungen: <1> 15.6.1994 neu (aus extract.c)

-------------------------------------------------------------------------*/



#include "pcl_doio.h"
#include "pcl_lemma.h"
#include "lrn_pcltrans.h"
#include "lrn_fnames.h"


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/

StepCell anchor;
char     *example = NULL;
char     *name = NULL;
char     *dir = NULL;
char     store[MAXPATHLEN];
char     *infilename = NULL;

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
   Brain_p  brain;
   Dom_p    dom;
   String_p help;
   char*    basename;

   anchor.pred = &anchor;
   anchor.succ = &anchor;

   InitIo(argc,argv,&anchor);

   ExtractProof(&anchor);
   BuildTree(&anchor); 
   FindLemmas(&anchor,CRITERIA,ITERATE);
   CalcGoalDistAndWeights(&anchor);
   

   if(dir)
   {
      help = NormName(dir);   
      AppendChar(help, '/');
   }
   else
   {
     help = AllocString();
   }
   AppendString(help, BaseName(name));

   basename = GetCopyOfString(help);
  
   VERBOSE(fprintf(stderr,"Building Complete brain...\n"));
   brain = BuildBrain(&anchor, example, name, Complete);
   if(WriteToFile)
   {
      OpenOutput(AppendString(help,".cbr"));
   }
   PrintBrain(brain);
   FreeBrain(brain);
   EndIo();
   VERBOSE(fprintf(stderr,"Complete brain written...\n"));
	   
   SetString(help,basename);

   VERBOSE(fprintf(stderr,"Building pruned brain...\n"));
   brain = BuildBrain(&anchor, example, name, Pruned);
   if(WriteToFile)
   {
      OpenOutput(AppendString(help,".pbr"));
   }
   PrintBrain(brain);
   FreeBrain(brain);
   EndIo();
   VERBOSE(fprintf(stderr,"Pruned brain written...\n"));

   SetString(help,basename);

   VERBOSE(fprintf(stderr,"Building goal domain...\n"));
   dom = BuildGoalDom(&anchor, example, name);

   if(WriteToFile)
   {
      OpenOutput(AppendString(help,".gdm"));
   }
   PrintDom(dom);
   FreeDom(dom); 
   EndIo();
   VERBOSE(fprintf(stderr,"Goal domain written...\n"));

   SetString(help,basename);

   VERBOSE(fprintf(stderr,"Building specification domain...\n"));
   dom = BuildSpecDom(&anchor, example, name);
   if(WriteToFile)
   {
      OpenOutput(AppendString(help,".sdm"));
   }
   PrintDom(dom);
   FreeDom(dom); 
   EndIo();
   VERBOSE(fprintf(stderr,"Specification domain written...\n"));
}



/*-------------------------------------------------------------------------

FUNCTION         : void InitIo(int argc,char* argv[],Step_p anchor)
                   IN     int argc
                   IN     char* argv[]
                   IN     Step_p anchor 

Beschreibung     : Liest die in der Kommandozeile angegebenen Files
                   und setzt die Namen und Optionen.

Globale Variable : out, in, WriteToFile, ReadFromFile, name, dir,
                   example 

Seiteneffekte    : Lesen der Eingabe, setzen der Optionen

Aenderungen      : <1> 17.4.1991 neu    
                   <2> 17.5.1991 Support fuer Luxus-Optionen (-v, -c, -h)
		   <3> 09.4.1993 Option -i 
		   <4> 21.8.1994 Umstellung fuer makedom, neue Box

-------------------------------------------------------------------------*/


void InitIo(int argc,char* argv[],Step_p anchor)
{
   int      f;

   WriteToFile = TRUE;

   for(f = 1; f<argc; f++)
   {
      if(strcmp(argv[f],"-d")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -d expects filename... \n");
            exit(OPTION_ERROR);
         }
         else if(dir)
         {
            fprintf(stderr,"ERROR: No more than one -d Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            dir = argv[++f];
         }
      }
      if(strcmp(argv[f],"-e")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -e expects examplename... \n");
            exit(OPTION_ERROR);
         }
         else if(example)
         {
            fprintf(stderr,"ERROR: No more than one -e Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            example = argv[++f];
         }
	 
      }
      if(strcmp(argv[f],"-n")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -n expects Domain- or Brainname... \n");
            exit(OPTION_ERROR);
         }
         else if(name)
         {
            fprintf(stderr,"ERROR: No more than one -n Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            name = argv[++f];
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
      else if(strcmp(argv[f],"-t")==0)
      {
         WriteToFile = FALSE;
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
      if((strcmp(argv[f],"-d")==0)||(strcmp(argv[f],"-e")==0)||
	 (strcmp(argv[f],"-n")==0))
      {
         f++;
      }
      else if(*(argv[f])=='-')
      {   /* Optionen ueberspringen  */
      }
      else
      {
         
         VERBOSE(fprintf(stderr,"Reading %s...\n",argv[f]));
	 
	 if(!infilename)
	 {
	    infilename = argv[f];
	 }
         ReadFile(argv[f],anchor);
         
         VERBOSE(fprintf(stderr,"...done.\n"));
      }
   }   


   if(!infilename)
   {   /* lese Standard-Eingabe  */
      if((!name)||(!example))
      {
	 fprintf(stderr,"ERROR: If reading stdin the user has to assign names\n\
manually (using -n and -e)...\n");
	 exit(OPTION_ERROR);
      }

      VERBOSE(fprintf(stderr,"Reading stdin...\n"));

      InitScanner(in,"stdin");

      ReadIn(anchor);

      VERBOSE(fprintf(stderr,"...done.\n"));
   }
   if(!example)
   {
      example = BaseName(infilename);
   }
      if(!name)
   {
      name = BaseName(infilename);
   }
}





/*-------------------------------------------------------------------------

FUNCTION         : void PrintInfo()  

Beschreibung     : Gibt Informationen ueber das Programm aus. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 17.5.1991 neu
                   <2> 16.8.1994 Anpassung an makedom

-------------------------------------------------------------------------*/


void PrintInfo()
{
   fprintf(stderr,"\n\n\
   makedom 0.8  vom 16.8.1999\n\
\n\
   Usage: makedom [-v] [-h] [- o outfile] [infile1 ... infileN]\n\
\n\
   Geschrieben von Stephan Schulz\n\
\n\
   makedom generiert aus der PCL-beschreibung eines Beweises die\n\
   verschiedenen Domaenen.\n\
\n\
   OPTIONEN:\n\
    -v          Ausgabe von Programm- und Ablaufinformationen\n\
    -h          Ausgabe der Programminformationen, Abbruch.\n\
    -t          Ausgabe auf dem Terminal - zu Testzwecken sinnvoll.\n\
    -d dir      Erzeuge die Ausgabedateien in Directory <dir>\n\
    -e example  Setze den Beispielnamen auf <example>. Default ist der\n\
                Name der ersten Eingabedatei ohne ihren Suffix.\n\
    -n name     Verwende <name> als den Domain- bzw. Brain-Namen.\n\
");
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/



