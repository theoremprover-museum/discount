

/*************************************************************************/
/*                                                                       */
/*   File:        revert.c                                               */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Dreht Files um.                                        */
/*                                                                       */
/*   Aenderungen: <1> 19.9.1991  neu                                     */
/*                                                                       */
/*************************************************************************/

#include "pcl_defs.h"



/*----------------------------------------------------------------------------*/
/*                Typvereinbarungen                                           */
/*----------------------------------------------------------------------------*/

typedef struct linecell
{
   char*           line;
   struct linecell *pred;
   struct linecell *succ;
} LineCell, *Line_p;



/*----------------------------------------------------------------------------*/
/*               Forward-Deklarationen interner Funktionen                    */
/*----------------------------------------------------------------------------*/

void main(int argc,char* argv[]);
void PrintInfo();
void WritePart();
void DoRead();
char* ReadLine();
char* secure_malloc(long size);


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/



FILE *out = stdout,
     *in = stdin;

BOOL WriteToFile = FALSE,
     ReadFromFile = FALSE,
     DeComposed = FALSE,
     printcomment = FALSE,
     Verbose = FALSE,
     Help = FALSE;

LineCell anchor = {NULL,&anchor,&anchor};

char outname[50];
int  fnum = 0;

char* fplace = NULL;

char* ReserveDuRoi;

/*----------------------------------------------------------------------------*/
/*                       Funktionen                                           */
/*----------------------------------------------------------------------------*/



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void main(int argc,char* argv[])                        */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                                                                            */
/* Beschreibung     : Liest das File und dreht es, wenn moegich, um.          */
/*                                                                            */
/* Globale Variable : out, in, WriteToFile, ReadFromFile                      */
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
   Line_p handle;

   ReserveDuRoi = malloc(4096); /* Speicherplatz fuer Fileoperationen reservieren */

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
            WriteToFile = TRUE;
            fplace = argv[++f];
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
   if(Verbose)
   {
      PrintInfo();
   }
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

         DoRead();

         fclose(in);
      }
   }

   if(!ReadFromFile)
   {   /* lese Standard-Eingabe  */
      DoRead();
   }

   if(DeComposed)
   {
      WritePart();
      FREE(ReserveDuRoi);
   }
   else
   {
      FREE(ReserveDuRoi);
      if(WriteToFile)
      {
         out = fopen(fplace,"w");
         if(!out)
         {
            fprintf(stderr,"ERROR: unable to open file %s...\n",fplace);
            exit(IO_ERROR);
         }
      }
      while(anchor.pred!=&anchor)
      {
         handle = anchor.pred;
         anchor.pred = handle->pred;
         fprintf(out,"%s\n",handle->line);
         FREE(handle->line);
         FREE(handle);
      }
      anchor.succ = &anchor;
      if(WriteToFile)
      {
         fclose(out);
      }
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
/* Aenderungen      : <1> 19.9.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintInfo()
{
   fprintf(stderr,"\n\n   revert 1.0 vom 19.9.1991");
   fprintf(stderr,"\n\n   Usage: revert [-v] [-h] [- o outfile] [infile1 ... infileN]");
   fprintf(stderr,"\n\n   revert liest eine Textdatei und gibt die Zeilen in umgekehrter");
   fprintf(stderr,"\n   Reihenfolge wieder aus. Wird kein Eingabefile angegeben,");
   fprintf(stderr,"\n   so wird die Standard-Eingabe gelesen, mehrere Eingabefiles");
   fprintf(stderr,"\n   werden wie eine zusammenhaengende Datei behandelt.");
   fprintf(stderr,"\n   Falls der Speicherplatz zur Aufnahme der Datei nicht");
   fprintf(stderr,"\n   ausreicht, wird die Ausgabe in Dateien mit den Namen");
   fprintf(stderr,"\n   outfile.rev* beziehungsweise stdout.rev* geschrieben.");

   fprintf(stderr,"\n\n   Geschrieben von Stephan Schulz");
   fprintf(stderr,"\n\n    -v          Ausgabe der Programminformationen vor");
   fprintf(stderr,"\n                Beginn der Uebersetzung.");
   fprintf(stderr,"\n    -h          Ausgabe der Programminformationen, Abbruch.");
   fprintf(stderr,"\n    -o outfile  Schreibe Ausgabe in outfile. Entfaellt diese");
   fprintf(stderr,"\n                Option, so wird auf die Standard-Ausgabe ge-");
   fprintf(stderr,"\n                schrieben.");
   fprintf(stderr,"\n\n    BUGS: Fuer die generierten Filenamen wird ein interner");
   fprintf(stderr,"\n          Puffer von nur 50 Zeichen Laenge verwendet.\n\n\n");
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void WritePart()                                        */
/*                                                                            */
/* Beschreibung     : Generiert einen Filenamen und schreibt das bisher ge-   */
/*                    lesene File hinein.                                     */
/*                                                                            */
/* Globale Variable : ReserveDuRoi, WriteToFile, out, fplace, fnum, fname,    */
/*                    anchor, DeComposed                                      */
/*                                                                            */
/* Seiteneffekte    : ... (s.o.)                                              */
/*                                                                            */
/* Aenderungen      : <1> 19.9.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void  WritePart()
{
   Line_p handle;

   FREE(ReserveDuRoi);

   if(WriteToFile)
   {
      sprintf(outname,"%s.rev%i",fplace,fnum);
   }
   else
   {
      sprintf(outname,"stdout.rev%d",fnum++);
   }
   out = fopen(outname,"w");
   if(!out)
   {
      fprintf(stderr,"ERROR: unable to open file %s...\n",outname);
      exit(IO_ERROR);
   }
   else
   {
      fprintf(stderr,"Warning: Input exceeding memory - writing to %s\n",outname);
      DeComposed = TRUE;
   }
   while(anchor.pred!=&anchor)
   {
      handle = anchor.pred;
      anchor.pred = handle->pred;
      fprintf(out,"%s\n",handle->line);
      FREE(handle->line);
      FREE(handle);
   }
   anchor.succ = &anchor;
   fclose(out);
   ReserveDuRoi = malloc(4096); /* Speicherplatz fuer Fileoperationen reservieren */
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void DoRead()                                           */
/*                                                                            */
/* Beschreibung     : Liest File, kettet ein.                                 */
/*                                                                            */
/* Globale Variable : in, anchor                                              */
/*                                                                            */
/* Seiteneffekte    : ... (s.o.) Eventuell aufruf von WritePart.              */
/*                                                                            */
/* Aenderungen      : <1> 19.9.1991 neu                                       */
/*                                                                            */
/******************************************************************************/


void DoRead()
{
   Line_p handle;

   handle = (Line_p)secure_malloc(sizeof(LineCell));
   handle->line = ReadLine();

   while(handle->line)
   {
      handle->succ = &anchor;
      handle->pred = anchor.pred;
      (anchor.succ)->pred = handle;
      anchor.pred = handle;

      handle = (Line_p)secure_malloc(sizeof(LineCell));
      handle->line = ReadLine();
   }
   FREE(handle);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* ReadLine()                                        */
/*                                                                            */
/* Beschreibung     : Liest eine Zeile, gibt Pointer auf String zurueck.      */
/*                    EOF => NULL                                             */
/*                                                                            */
/* Globale Variable : in                                                      */
/*                                                                            */
/* Seiteneffekte    :                                                         */
/*                                                                            */
/* Aenderungen      :                                                         */
/*                                                                            */
/******************************************************************************/

char* ReadLine()
{
   static char* aktline = NULL;
   static long  aktmem = 0;
   long         aktsize = 1;
   char*        help;
   char*        handle;
   char         ch;

   ch = getc(in);

   if(ch==EOF)
   {
      return NULL;
   }

   if(!aktline)
   {
      aktline = secure_malloc(200);
      aktmem = 200;
   }

   help = aktline;
   *help = '\0';

   while(ch!=EOF && ch!='\n')
   {
      if(aktsize==aktmem)
      {
         handle = secure_malloc(aktmem+200);
         strcpy(handle,aktline);
         aktmem +=200;
         FREE(aktline);
         aktline = handle;
         help = aktline+aktsize-1;
     } 
     *help=ch;
     help++;
     *help='\0';
     aktsize++;

     ch = getc(in);
   }
   handle = secure_malloc(aktsize);
   strcpy(handle,aktline);

   return handle;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* secure_malloc(long size)                          */
/*                    IN     long size                                        */
/*                                                                            */
/* Beschreibung     : Stellt Speicher zur Verfuegung, bei Speichermangel      */
/*                    Aufruf von WritePart.                                   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Aufruf von WritePart                                    */
/*                                                                            */
/* Aenderungen      : <1> 19.9.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

char* secure_malloc(long size)
{
   char* handle;

   handle = malloc(size);
   if(!handle)
   {
      WritePart();
      handle = malloc(size);
   }
   if(!handle)
   {
      fprintf(stderr,"ERROR: Out of Memory in secure_malloc (revert)...\n");
      exit(OUT_OF_MEM);
   }
   return handle;
}







/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


