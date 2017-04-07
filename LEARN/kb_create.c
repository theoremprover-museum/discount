/*-------------------------------------------------------------------------

File       : kb_create.c 

Autor      : Stephan Schulz

Inhalt     : Erzeugt die Grundstruktur einer Wissensbasis im
             Filesystem. 

Aenderungen: <1> 24.08.1994 neu (aus makedom.c)
             <2> 15.12.1995 v 1.1, introduces kb_variables file.

-------------------------------------------------------------------------*/


#include "pcl_doio.h"
#include "lrn_fnames.h"
#include "lrn_domains.h"
#include <sys/types.h>
#include <sys/stat.h>


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/


#define VERSION "1.2 (Mar 2 1998)"

char* kb_name = NULL;

/* PreserveArity defined in pcl_norm.h, PosAndNeg defined in lrn_eqnoccur.h */
BOOL  HasSpecDoms   = TRUE;
BOOL  HasGoalDoms   = TRUE;
BOOL  HasSelData    = TRUE;

/*----------------------------------------------------------------------------*/
/*                       Forward-Deklarationen                                */
/*----------------------------------------------------------------------------*/

void main(int argc,char* argv[]);

void InitIo(int argc,char* argv[]);

void PrintInfo();

/*----------------------------------------------------------------------------*/
/*                      Funktionen                                            */
/*----------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void main(int argc,char* argv[]) 
                   IN     int argc
                   IN     char* argv[]

Beschreibung     : Steuerung des Programmablaufes

Globale Variable : kb_name

Seiteneffekte    : -

Aenderungen      : <1> 8.9.1994 neu
                   <2> 5.8.1998 SELECTIONDATA/

-------------------------------------------------------------------------*/


void main(int argc,char* argv[])
{
   String_p abs_basename,
            tmp;
   BrainCell empty_brain;

   InitIo(argc,argv);

   abs_basename = DirNormName(kb_name);
   tmp = FileName(kb_name);
   AppendChar(abs_basename,'/');
   AppendString(abs_basename, ViewString(tmp));

   VERBOUT("Creating base directory...\n");
   
   if(mkdir(ViewString(abs_basename),S_IRWXU|S_IRWXG)) 
      /* Da stimmt noch was nicht..kann an der umask liegen, oder */
      /* daran, das ich in C keine Oktalzahlen Screiben kann... */
   {
      perror("ERROR: mkdir (base directory)");
      exit(IO_ERROR);
   }
   VERBOUT("...successful.\nCreating files...\n");
   
   AppendChar(abs_basename,'/');
   kb_name = GetCopyOfString(abs_basename);

   AppendString(abs_basename,"kb_variables");
   OpenOutput(ViewString(abs_basename));
   fprintf(out,"Version = \"" VERSION "\"\n\n");
   fprintf(out,"PreserveArity = %s \n", PreserveArity ? "TRUE" : "FALSE" );
   fprintf(out,"PosAndNeg     = %s \n", PosAndNeg     ? "TRUE" : "FALSE" );
   fprintf(out,"HasSpecDoms   = %s \n", HasSpecDoms   ? "TRUE" : "FALSE" );
   fprintf(out,"HasGoalDoms   = %s \n", HasGoalDoms   ? "TRUE" : "FALSE" );
   fprintf(out,"HasSelData    = %s \n", HasSelData    ? "TRUE" : "FALSE" );
   EndIo();
 

   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "specdoms");
   OpenOutput(ViewString(abs_basename));
   EndIo();

   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "goaldoms");
   OpenOutput(ViewString(abs_basename));
   EndIo();
   
   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "pdata");
   OpenOutput(ViewString(abs_basename));
   empty_brain.name = "pruned_database";
   empty_brain.type = Pruned;
   empty_brain.examples = NULL;
   empty_brain.facts = NULL;
   PrintBrain(&empty_brain);
   EndIo();
   
   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "cdata");
   OpenOutput(ViewString(abs_basename));
   empty_brain.name = "complete_database";
   empty_brain.type = Complete;
   PrintBrain(&empty_brain);
   EndIo();
   
   VERBOUT("...successful.\nCreating subdirectories...\n");

   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "SPECDOMS");
   if(mkdir(ViewString(abs_basename),S_IRWXU|S_IRWXG)) 
   {
      perror("ERROR: mkdir (SPECDOMS)");
      exit(IO_ERROR);
   }

   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "GOALDOMS");
   if(mkdir(ViewString(abs_basename),S_IRWXU|S_IRWXG)) 
   {
      perror("ERROR: mkdir (GOALDOMS)");
      exit(IO_ERROR);
   }

   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "EXAMPLES");
   if(mkdir(ViewString(abs_basename),S_IRWXU|S_IRWXG)) 
   {
      perror("ERROR: mkdir (EXAMPLES)");
      exit(IO_ERROR);
   }

   SetString(abs_basename,kb_name);
   AppendString(abs_basename, "SELECTIONDATA");
   if(mkdir(ViewString(abs_basename),S_IRWXU|S_IRWXG)) 
   {
      perror("ERROR: mkdir (SELECTIONDATA)");
      exit(IO_ERROR);
   }
   VERBOUT("...successful.\nKnowledge base created.\n");
   
   exit(NO_ERROR);
}



/*-------------------------------------------------------------------------

FUNCTION         : void InitIo(int argc,char* argv[])
                   IN     int argc
                   IN     char* argv[]

Beschreibung     : Setzt die Optionen und den Namen fuer die
                   Wissensbasis. 

Globale Variable : kb_name, Verbose, Help

Seiteneffekte    : Setzen der Optionen

Aenderungen      : <1> 17.4.1991 neu    
                   <2> 17.5.1991 Support fuer Luxus-Optionen (-v, -c, -h)
		   <3> 09.4.1993 Option -i 
		   <4> 21.8.1994 Umstellung fuer makedom, neue Box
		   <5> 24.8.1994 Umstellung fuer parsedom
		   <6>  8.9.1994 Anpassung an kb_create
                   <7>  5.8.1998 Anpassung SELECTIONDATA/

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
      else if(strcmp(argv[f],"-a")==0)
      {
         PreserveArity = TRUE;
      }
      else if(strcmp(argv[f],"-n")==0)
      {
	 PosAndNeg= TRUE;
      }
      else if(strcmp(argv[f],"-S")==0)
      {
         HasSpecDoms = FALSE;
      }
       else if(strcmp(argv[f],"-G")==0)
      {
         HasGoalDoms = FALSE;
      }
       else if(strcmp(argv[f],"-E")==0)
      {
         HasSelData = FALSE;
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

   VERBOSE(PrintInfo());


   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      if(*(argv[f])=='-')
      {   /* Optionen ueberspringen  */
      }
      else
      {
         if(kb_name)
	 {
	    fprintf(stderr,"ERROR: Only one name allowed...\n");
	    exit(ARG_ERROR);
	 }
	 kb_name = argv[f];
      }
   }
   if(!kb_name)
   {
       fprintf(stderr,"ERROR: No name given...\n");
       exit(ARG_ERROR);
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
		   <4>  8.9.1994 Anpassung an kb_create
                   <5>  5.8.1998 Anpassung an SELECTIONDATA/

-------------------------------------------------------------------------*/


void PrintInfo()
{
   fprintf(stderr, "\n\
\n\
   kb_create " VERSION "\n\
\n\
   Usage: kb_create [-v] [-h] [-a] [-n] [-S] [-G] [-E] <name>\n\
\n\
   Written by Stephan Schulz\n\
\n\
   kb_create will create the directory designated by <name> and\n\
   initialize it for use as a knowledge base for DISCOUNT's learning\n\
   experts. <name> will be either an absolute or a relative path\n\
   name pointing to the directory to be created.\n\
\n\
   OPTIONEN:\n\
    -v     Print this information and comments on the operation's\n\
           progress. \n\
    -h     Prints this information and terminates.\n\
    -a     Mark the new knowledge base as one where function\n\
           symbols always keep the same arity.\n\
    -n     Mark the new knowledge base as one containing positive\n\
           and negative examples.\n\
    -S     The new knowledge base will not maintain specification\n\
           bound domains (Default positive for compatibility with\n\
           kb_create v. 1.0).\n\
    -G     The new knowledge base will not maintain goal bound\n\
           domains (ditto).\n\
    -E     The new knowledge base will not contain example\n\
           selection data\n\
\n\n\
");
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/



