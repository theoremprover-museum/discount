
/*-------------------------------------------------------------------------

File       : kb_insert.c 

Autor      : Stephan Schulz

Inhalt     : Hauptprogramm, das ein Beispiel (ein PCL-Listing) in eine
             Wissensbasis einfuehrt.

Aenderungen: <1> 9.9.1994 neu (aus makedom.c)

-------------------------------------------------------------------------*/


#include "pcl_lemma.h"
#include "lrn_fnames.h"
#include "lrn_pcltrans.h"
#include "lrn_insert.h"
#include "lrn_filevars.h"



/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/

#define VERSION "1.2 (Mar 2 1998)"

StepCell anchor;
char     *example = NULL;
char     *specname = NULL;
char     *goalname = NULL;
char     *kb_name = NULL;
char     store[MAXPATHLEN];
char     *infilename = NULL;
String_p kb_path;
BOOL     has_specdoms = TRUE,
         has_goaldoms = TRUE,
         has_seldata  = TRUE;
char*    kb_create_version;

FVarCell file_variables[] =
{
   {"Version"      , NULL,           NULL, &kb_create_version},
   {"PreserveArity", &PreserveArity, NULL, NULL},
   {"HasSpecDoms"  , &has_specdoms,  NULL, NULL},
   {"HasGoalDoms"  , &has_goaldoms,  NULL, NULL},
   {"HasSelData"   , &has_seldata,   NULL, NULL},
   {"PosAndNeg"    , &PosAndNeg,     NULL, NULL},
   {NULL, NULL, NULL, NULL}
};   


/*----------------------------------------------------------------------------*/
/*                       Forward-Deklarationen                                */
/*----------------------------------------------------------------------------*/

int main(int argc,char* argv[]);

void handle_options(int argc,char* argv[], BOOL silent);
void InitIo(int argc,char* argv[],Step_p anchor);

void PrintInfo();

/*----------------------------------------------------------------------------*/
/*                      Funktionen                                            */
/*----------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void main(int argc,char* argv[]) 
                   IN     int argc
                   IN     char* argv[]

Beschreibung     : Steuerung des Programmablaufes

Globale Variable : anchor, example, specname, goalname, kb_name, store,
                   infilename, Verbose, ReadFromFile, WriteToFile...

Seiteneffekte    : Na ja...alles halt

Aenderungen      : <1> 9.9.1994 neu

-------------------------------------------------------------------------*/


int main(int argc,char* argv[])
{
   Brain_p      cbrain,
                pbrain,
                new_cbrain,
                new_pbrain;
   Dom_p        specdoms, 
                goaldoms,
                new_specdom,
                new_goaldom;

   anchor.pred = &anchor;
   anchor.succ = &anchor;

   InitIo(argc,argv,&anchor);

   if(!PosAndNeg)
   {
      ExtractProof(&anchor);
   }
   if(!IsProofProtocol(&anchor))
   {
      fprintf(stderr,"ERROR: Completion protocol not supported...\n");
      exit(IO_ERROR);
   }
   BuildTree(&anchor); 

   FindLemmas(&anchor,CRITERIA,ITERATE);
   
   CalcGoalDistAndWeights(&anchor); 
   
   VERBOUT("Input-handling successful...\n");
   VERBOSE(fprintf(stderr,"\n\
Example name:              %s\n\
Read from:                 %s\n\
Use knowledge base:        %s\n\
Goal domain name:          %s\n\
Specification domain name: %s\n\n",\
example, infilename, ViewString(kb_path),goalname, specname););

   VERBOUT("Reading pruned domain independend database...");
   pbrain = ReadBrain(ViewString(kb_path), "pdata", example);
   VERBOUT("...done\n");
   VERBOUT("Reading complete domain independend database..."); 
   cbrain = ReadBrain(ViewString(kb_path), "cdata", example);
   VERBOUT("...done\n");
   VERBOUT("Example name is not yet used in knowledge base.\n\n");

   VERBOUT("Reading specification domain list...");
   specdoms = ReadDomHeadList(ViewString(kb_path), "specdoms", specname);
   VERBOUT("...done\n");
   VERBOUT("Specification domain name is not yet used in knowledge base.\n\n");

   VERBOUT("Reading goal domain list...");
   goaldoms = ReadDomHeadList(ViewString(kb_path), "goaldoms", goalname);
   VERBOUT("...done\n");
   VERBOUT("Goal domain name is not yet used in knowledge base.\n\n");

   VERBOUT("Writing PCL-file of example...");
   WriteExample(&anchor, ViewString(kb_path), example);
   VERBOUT("...done\n\n");
   
   VERBOUT("Builing, merging and writing pruned domain independend\
 database..."); 
   new_pbrain = BuildBrain(&anchor, example, "tmp_name", Pruned);
   pbrain = MergeBrains(new_pbrain, pbrain);
   WriteBrain(pbrain, ViewString(kb_path), "pdata");
   VERBOUT("...done\n\n");

   VERBOUT("Builing, merging and writing complete domain independend\
 database..."); 
   new_cbrain = BuildBrain(&anchor, example, "tmp_name", Complete);
   cbrain = MergeBrains(new_cbrain, cbrain);
   WriteBrain(cbrain, ViewString(kb_path), "cdata");
   VERBOUT("...done\n\n");

   if(!has_specdoms)
   {
      VERBOUT("Skipping specification domains on user request!\n");
   }
   else
   {
      VERBOUT("Building new specification domain...\n");
      new_specdom = BuildSpecDom(&anchor, example, specname);
      InsertNewDom(ViewString(kb_path), new_specdom, specdoms);
      VERBOUT("...done\n");
      VERBOUT("Writing modified specification domain list...\n");
      WriteDomHeadList(specdoms, ViewString(kb_path), "specdoms");
      VERBOUT("...done\n\n");
   }

   if(!has_goaldoms)
   {
      VERBOUT("Skipping goal domains on user request!\n");
   }
   else
   {
      VERBOUT("Building new goal domain...\n");
      new_goaldom = BuildGoalDom(&anchor, example, goalname);
      InsertNewDom(ViewString(kb_path), new_goaldom, goaldoms);
      VERBOUT("...done\n");
      VERBOUT("Writing modified goal domain list...\n");
      WriteDomHeadList(goaldoms, ViewString(kb_path), "goaldoms");
      VERBOUT("...done\n\n");
   }

   if(!has_seldata)
   {
      VERBOUT("Skipping selection data on user request!\n");
   }
   else
   {
      VERBOUT("Writing Example Selection Data...")
      WriteSelectionData(&anchor, ViewString(kb_path), example);
      VERBOUT("...done\n\n");
   }

   return 0;
}


/*-------------------------------------------------------------------------
//
// Function: handle_options()
//
//   Scans the comandline and sets the options. This is a function of
//   its own because most of this has to happen twice: Once at the
//   very start, to set the variables required for reading the
//   kb_variables file, then after this file has been read, to
//   overwrite the variables set there (options have a higher priority
//   than the description file of the domain!) If silent is set, do
//   not complain about multiple uses of the same option - take the
//   last one. The -h option is handled instantly!
//
// Global Variablee : example, specname, goalname, kb_name, kb_path,
//                    has_specdoms, has_goaldoms, Verbose
//
// Side Effects     : May set all variables influenced by options (all
//                    those listed), may terminate the program
//                    successfully after displaying help text.
//
//-----------------------------------------------------------------------*/

void handle_options(int argc,char* argv[], BOOL silent)
{
   int f;

   for(f = 1; f<argc; f++)
   {
      if(strcmp(argv[f],"-e")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -e expects example name... \n");
            exit(OPTION_ERROR);
         }
         else if(example && !silent)
         {
            fprintf(stderr,"ERROR: No more than one -e Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            example = argv[++f];
         }
      }
      else if(strcmp(argv[f],"-s")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -s expects domain name... \n");
            exit(OPTION_ERROR);
         }
         else if(specname && !silent)
         {
            fprintf(stderr,"ERROR: No more than one -s Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            specname = argv[++f];
         }
	 
      }
      else if(strcmp(argv[f],"-g")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -g expects domain name... \n");
            exit(OPTION_ERROR);
         }
         else if(goalname && !silent)
         {
            fprintf(stderr,"ERROR: No more than one -g Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            goalname = argv[++f];
         }
	 
      }
      else if((strcmp(argv[f],"-k")==0) || (strcmp(argv[f],"-K")==0))
      {
	 if(f==argc-1)
         {
            fprintf(stderr,
		    "ERROR: -k or -K expects knowledge base... \n");
            exit(OPTION_ERROR);
         }
         else if(kb_name && !silent)
         {
            fprintf(stderr,
		    "ERROR: No more than one -k or -K Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            kb_name = argv[++f];
         }
      }      
      else if(strcmp(argv[f],"-v")==0)
      {
         Verbose = TRUE;
      }
      else if(strcmp(argv[f],"-h")==0)
      {
	 /* Help is handled instantly, because it terminates the
	    program and should suceed even if the invocation of the
	    program is incorrect */
	 PrintInfo();
	 exit(NO_ERROR);
      }
      else if(strcmp(argv[f],"-S")==0)
      {
         has_specdoms = FALSE;
      }
      else if(strcmp(argv[f],"-G")==0)
      {
         has_goaldoms = FALSE;
      }
      else if(strcmp(argv[f],"-E")==0)
      {
         has_seldata = FALSE;
      }
      else if(*(argv[f])=='-')
      {
         fprintf(stderr,"ERROR: unknown option %s...\n",argv[f]);
         exit(OPTION_ERROR);
      }
   }
}

/*-------------------------------------------------------------------------

Function         : void InitIo(int argc,char* argv[],Step_p anchor)
                   IN     int argc
                   IN     char* argv[]
                   IN     Step_p anchor 

Beschreibung     : Liest die in der Kommandozeile angegebenen Files
                   und setzt die Namen und Optionen.

Globale Variable : out, in, anchor, example, specname, goalname,
                   kb_name, store, infilename, Verbose, ReadFromFile,
		   WriteToFile... 

Seiteneffekte    : Lesen der Eingabe, setzen der Optionen

Aenderungen      : <1> 17.4.1991 neu    
                   <2> 17.5.1991 Support fuer Luxus-Optionen (-v, -c, -h)
		   <3> 09.4.1993 Option -i 
		   <4> 21.8.1994 Umstellung fuer makedom, neue Box
		   <5>  9.9.1994 Umstellung fuer kb_insert

-------------------------------------------------------------------------*/


void InitIo(int argc,char* argv[],Step_p anchor)
{
   int      f;
   String_p filename;
   FILE*    test_file;
   
   /* Check for options. Only -k and -v are of interest here, but it's
      easier to check them all */

   handle_options(argc,argv, FALSE);

   if(!kb_name)
   {
      fprintf(stderr,
	      "ERROR: Missing -k option - cannot find the knowledge base...\n");
      exit(OPTION_ERROR);
   }
   kb_path = NormName(kb_name);
   filename = AllocString();
   SetString(filename,ViewString(kb_path));
   AppendString(filename, "/kb_variables");
   
   if((test_file = fopen(ViewString(filename),"r")))
   {
      fclose(test_file);
      VERBOUT("Reading file variables from kb_variables...\n");
      ReadFileVariables(ViewString(filename), file_variables);
   }
   else
   {
      VERBOUT("Old style knowledge base without file variables.\n");
   }

   /* Handle options - this time in earnest */
   handle_options(argc,argv, TRUE);

   VERBOSE(fprintf(stderr, "kb_insert running\n"));

   /* Read input */
   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      if((strcmp(argv[f],"-e")==0)||(strcmp(argv[f],"-s")==0)||
	 (strcmp(argv[f],"-g")==0)||(strcmp(argv[f],"-k")==0))
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
      if(!example)
      {
	 fprintf(stderr,
		 "ERROR: If reading stdin the user has to assign an example\
name using -e...\n");
	 exit(OPTION_ERROR);
      }

      VERBOSE(fprintf(stderr,"Reading <stdin>...\n"));

      InitScanner(in,"stdin");

      ReadIn(anchor);

      infilename = "<stdin>";
      VERBOSE(fprintf(stderr,"...done.\n"));
   }
   if(!example)
   {
      example = BaseName(infilename);
   }
   if(!specname)
   {
      specname = example;
   }
   if(!goalname) 
   {
     goalname = example;
   }
}





/*-------------------------------------------------------------------------

FUNCTION         : void PrintInfo()  

Beschreibung     : Gibt Informationen ueber das Programm aus. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 17.5.1991 neu
                   <2>  9.9.1994 Anpassung an kb_insert

-------------------------------------------------------------------------*/


void PrintInfo()
{
   fprintf(stderr,"\n\n\
   kb_insert " VERSION "\n\
\n\
   Usage: kb_insert[-v] [-h] -k <kb> ... [infile1 ... infileN]\n\
\n\
   Written by Stephan Schulz\n\
\n\
   kb_insert will try to insert the data from a PCL listing into the\n\
   knowledge-base denoted with the -k or -K option.\n\
\n\
   OPTIONS:\n\
    -v            Print this information and comments on the\n\
                  operation's progress.\n\
    -h            Prints this information and terminates.\n\
    -k/-K <kb>    The filename of the knowledge base to use. This\n\
                  option is not optional ;-). Both lower case and\n\
                  capital k are allowed for compatibility with\n\
                  discount and older versions of kb_insert.\n\
    -e <example>  Use <example> as the name for the example in the\n\
                  knowledge base. It defaults to the base name of the\n\
                  (first) input file. If neither is given the\n\
                  program will signal an error and terminate.\n\
    -s <dom>      The name to use for a possible new specification\n\
                  domain to accomodate the example. It defaults to the\n\
                  example name.\n\
    -g <dom>      The name to use for a possible new goal domain to\n\
                  accomodate the example. It defaults to the example\n\
                  name.\n\
    -S            Do not insert the example as a specification domain.\n\
    -G            Do not insert the example as a goal domain.\n\
\n\
    The last two options allow for a much faster learning process in\n\
    some instances if the user is interested in the global knowledge\n\
    only. The options will overwrite the variables in kb_variables.\n\
");
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/



