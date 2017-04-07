


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_mem.c                                              */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Funktionen zu einer Speicherverwaltung, die mit        */
/*                Bloecken in wenigen festen Groessen effizient arbeiten */
/*                sollte...                                              */
/*                                                                       */
/*   Aenderungen: <1> 8.2.1991 neu                                       */
/*                <2> 15.2.1991 Stringfunktionen ausgelagert             */
/*                <3> 18.2.1991 Stringdeskriptoren als neuer Datentyp    */
/*                <4> 04.4.1991 Erweiterung fuer echtes PCL, Cons-Zellen */
/*                              als Obertyp fuer gleichgrosse Strukturen */
/*                <5> 09.7.1991 Neue Datentypen fuer miniextract         */
/*                <6> 15.2.1992 Allen neu (siehe .h-File)                */
/*                                                                       */
/*************************************************************************/

#include "pcl_mem.h"



/*----------------------------------------------------------------------------*/
/*                        Globale Variable                                    */
/*----------------------------------------------------------------------------*/

/* Speicher fuer sprintf, external-Deklaration in pcl_defs.c */

char ErrStr[150];
char NullStr[] = "";


char* FreeMemList[201]={NULL};




/*----------------------------------------------------------------------------*/
/*                Exportierte Funktionen                                      */
/*----------------------------------------------------------------------------*/

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char*     typ_malloc(int size)                          */
/*                    IN       int size                                       */
/*                                                                            */
/* Beschreibung     : Allokiert unter Verwendung der internen Freispeicher-   */
/*                    Liste und, falls diese leer ist von secure_malloc,      */
/*                    einen Block der gewuenschten Groesse.                   */
/*                                                                            */
/* Globale Variable : FreeMemList                                             */
/*                                                                            */
/* Seiteneffekte    : Alle Eintraege von FreeMemList koennen durch            */
/*                    secure_malloc veraendert werden.                        */
/*                                                                            */
/* Aenderungen      : <1> 15.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

char* typ_malloc(int size)
{
   char* handle;

   DEBUG(DTMEM,fprintf(stderr,"typ_malloc called with size %d...",size));
   if(size>=4 && size<=200 && FreeMemList[size])
   {
      handle = FreeMemList[size];
      FreeMemList[size] = *((char**)FreeMemList[size]);
      DEBUG(DTMEM,fprintf(stderr,"handled internally\n"));
   } 
   else
   {
      handle = secure_malloc(size);
      DEBUG(DTMEM,fprintf(stderr,"called secure_malloc\n"));
   }
   return handle;
}   


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void      typ_free(char* adr,int size)                  */
/*                    IN      char* adr                                       */
/*                    IN      int size                                        */
/*                                                                            */
/* Beschreibung     : Gibt einen Speicherblock der Groesse size an die        */
/*                    interne Speicherverwaltung zurueck.                     */
/*                                                                            */
/* Globale Variable : FreeMemList                                             */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 15.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void typ_free(char* adr,int size)
{
   char **handle;

   DEBUG(DTMEM,fprintf(stderr,"typ_free called with pointer %ld and\
 size %d...\n",(long) adr,size));
   if(!adr)
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to typ_free...");
   }
   if(size>=4 && size<=200)
   {
      handle = (char**)adr;
      *handle = FreeMemList[size];
      FreeMemList[size] = adr;
   }
   else
   {
      free(adr);
   }
} 


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* secure_malloc(int size)                           */
/*                    IN    int size                                          */
/*                                                                            */
/* Beschreibung     : Stellt ueber malloc Pointer auf Speicher der Groesse    */
/*                    size zur Verfuegung. Liefert malloc keinen gueltigen    */
/*                    Pointer, wird der interne freie Speicher zurueckge-     */
/*                    geben. Ist auch der folgende malloc-Aufruf nicht er-    */
/*                    folgreich, so bricht das System mit Fehlermeldung ab.   */
/*                                                                            */
/* Globale Variable : FreeMemList                                             */ 
/*                                                                            */
/* Seiteneffekte    : Der in den Speicherlisten verwaltete Speicherplatz kann */
/*                    zurueckgegeben werden. Bei Speicherplatzmangel bricht   */
/*                    das Programm mit Fehlermeldung ab.                      */
/*                                                                            */
/* Aenderungen      : <1> 15.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

char* secure_malloc(int size)
{
   char* handle;
   long  f;

   handle = malloc(size);

   if(!handle)
   {    /* kein Speicherplatz mehr  */
      for(f = 4;f<=200;f++) 
      {
         while(FreeMemList[f])
         {
            handle = FreeMemList[f];
            FreeMemList[f] = *((char**)FreeMemList[f]);
            free(handle);
         }
      }

      handle = malloc(size);

      if(!handle)
      {   /* es gibt immer noch nichts ! */
         Error("Out of Memory (secure_malloc in pcl_mem.c)");
      }
   }
   
   return handle;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void Error(char* message)                               */
/*                                                                            */
/* Beschreibung     : Gibt Fehlermeldung aus und bricht Programm ab.          */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Programmabbruch                                         */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

#ifdef __GNUC__
volatile
#endif
void Error(char* message)
{
   fprintf(stderr,"ERROR: %s ...\n", message);
   exit(OTHER_ERROR);
}

/*----------------------------------------------------------------------------*/
/*                Ende des Files                                              */
/*----------------------------------------------------------------------------*/

