/*-------------------------------------------------------------------------

File        : pcl_strings.c  

Autor       : Stephan Schulz

Inhalt      : Neue Stringfunktionen   

Aenderungen : <1> 15.2.1991 neu 
              <2> 18.2.1991 Allgemeine Stringdeskriptoren
	      <3> 20.8.1994 AllocString() neu, Kommentarbox

-------------------------------------------------------------------------*/


#include "pcl_strings.h"

#define STRSIZE 2000



/*----------------------------------------------------------------------------*/
/*               Funktionen                                                   */
/*----------------------------------------------------------------------------*/

MakeAlloc(StringCell);

MakeFree(StringCell);


/*-------------------------------------------------------------------------

FUNCTION         : String_p AllocString()

Beschreibung     : Gibt einen Pointer auf einen initialisierten,
                   leeren String zurueck.

Globale Variable : -

Seiteneffekte    : Speicher

Aenderungen      : <1> 21.8.1994 neu

-------------------------------------------------------------------------*/

String_p  AllocString()
{
   String_p handle;

   handle = AllocStringCell();
   handle->str = NULL;
   handle->len = handle->mem = 0;
   
   return handle;
}
   

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeString(String_p junk)                          */
/*                    IN     String_p junk                                    */
/*                                                                            */
/* Beschreibung     : Kettet eine Stringzelle in die Liste der freien String- */
/*                    zellen ein. ! Die Zelle darf danach nicht mehr de-      */
/*                    referenziert werden !  Der von dieser Zelle referen-    */
/*                    zierte String-Speicher wird ebenfalls zurueckgegeben!!  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von FreeStringCell (siehe dort)            */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeString(String_p junk)
{
   if(junk)
   {
      FREE(junk->str);
      FreeStringCell(junk);
   }
   else
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreeString (pcl_mem.c)...\n");
   }
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* secure_strdup(char* str)                          */
/*                    IN     char* str                                        */
/*                                                                            */
/* Beschreibung     : Dupliziert den EingabeString unter Verwendung von       */
/*                    secure_malloc.                                          */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch secure_malloc (siehe dort)                        */
/*                                                                            */
/* Aenderungen      : <1> 15.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/


char* secure_strdup(char* str)
{
   char* handle;

   handle = secure_malloc(strlen(str)+1);
   strcpy(handle,str);

   return handle;

}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : String_p AppendString(String_p strdes,char* newpart)    */
/*                    IN    String_p strdes                                   */
/*                    IN    char*    newpart                                  */
/*                                                                            */
/* Beschreibung     : Haengt den String, auf den newpart zeigt, an den durch  */
/*                    strdes beschriebene String an. Rueckgabewert ist der    */
/*                    Pointer auf den Gesamtstring. Der Stringdeskriptor wird */
/*                    angepasst.                                              */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Der beschriebene String wird verlaengert,               */
/*                    eventuell werden Speicherbloecke belegt oder frei-      */
/*                    gegeben.                                                */
/*                                                                            */
/* Aenderungen      : <1> 12.2.1991 neu                                       */
/*                    <2> 18.2.1991 Umstellung auf Stringdeskriptor.          */
/*                                                                            */
/******************************************************************************/


char* AppendString(String_p strdes,char* newpart)
{
   long  insize;
   char* handle;

   insize = strlen(newpart);

   DEBUG(1,printf("...AppendString(vor Schleife)...\n"));
   DEBUG(1,printf("%ld %ld %s\n",strdes->len,strdes->mem,strdes->str));
   while((strdes->len + insize)>=strdes->mem)
   {
      handle = secure_malloc(strdes->mem+STRSIZE);
      if(strdes->str)
      {   /* String hat schon Speicher */
         strcpy(handle,strdes->str);
         free(strdes->str);
         strdes->mem += STRSIZE;
         strdes->str = handle;
      }
      else
      {
         strdes->str = secure_malloc(STRSIZE);
         strdes->mem = STRSIZE;
         *(strdes->str) = '\0';
      }
   }
   DEBUG(1,printf("...AppendString(Schleife beendet)...\n"));
   strcat(strdes->str,newpart);
   DEBUG(1,printf("...AppendString(strcat)...\n"));
   strdes->len += insize;
   return strdes->str;
}



/*-------------------------------------------------------------------------

FUNCTION         : char* ViewString(String_p strdes)  

Beschreibung     : Gibt die aktuelle Adresse des von strdes
                   beschriebenen Strings. In diesen Speicher sollte
		   nicht geschrieben werden, da"s die L"ange des
		   Strings veraendert!

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 15.2.1991 neu
                   <2> 18.2.1991 Umstellung auf Stringdeskriptor.
		   <3> 20.8.1994 Praezisierung des Kommentars, neue Box.

-------------------------------------------------------------------------*/


char* ViewString(String_p strdes)
{
   if(!strdes->mem)
   {
      strdes->str = secure_malloc(1);
      strdes->mem = 1;
      strdes->len = 0;
      *(strdes->str) = '\0';
   }
   return strdes->str;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* GetCopyOfString(String_p strdes)                  */
/*                                                                            */
/* Beschreibung     : Gibt Pointer auf eine Kopie des vom Deskriptor be-      */
/*                    schriebenen Strings zurueck. Der Speicherplatz muss     */
/*                    von der rufenden Funktion verwaltet werden.             */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Ein Speicherblock fuer die Kopie wird allokiert.        */
/*                    (siehe secure_malloc()).                                */
/*                                                                            */
/* Aenderungen      : <1> 13.2.1991 neu                                       */
/*                    <2> 18.2.1991 Umstellung auf Stringdeskriptor.          */
/*                                                                            */
/******************************************************************************/


char* GetCopyOfString(String_p strdes)
{
   char* handle;

   if(strdes->str)
   {
      handle = secure_strdup(strdes->str);
   }
   else
   {
      handle = secure_malloc(1);
      *handle = '\0';
   }
   return handle;
}




/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* AppendChar(String_p strdes,char newch)            */
/*                    INOUT String_p strdes                                   */
/*                    IN    char newch                                        */
/*                                                                            */
/* Beschreibung     : Analog AppendString, haengt aber immer nur einzelne     */
/*                    Zeichen an.                                             */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Der intern gespeicherte String wird verlaengert,        */
/*                    eventuell werden Speicherbloecke belegt oder frei-      */
/*                    gegeben.                                                */
/*                                                                            */
/* Aenderungen      : <1> 15.2.1991 neu                                       */
/*                    <2> 18.2.1991 Umstellung auf Stringdeskriptor.          */
/*                                                                            */
/******************************************************************************/


char* AppendChar(String_p strdes,char newch) 
{
   char* handle;

   DEBUG(1,printf("AppendChar %ld %ld %s\n",strdes->len,strdes->mem,strdes->str));
   if((strdes->len + 2)>=strdes->mem)  /* Platz fuer /0 lassen */
   {  /* neuer Speicher  */
      handle = secure_malloc(strdes->mem+STRSIZE);
      if(strdes->str)
      {   /* Funktion hat schon Speicher */
         strcpy(handle,strdes->str);
         free(strdes->str);
         strdes->mem += STRSIZE;
         strdes->str = handle;
      }
      else
      {
         strdes->str = secure_malloc(STRSIZE);
         strdes->mem = STRSIZE;
      }
   }
   strdes->str[strdes->len++] = newch; 
   strdes->str[strdes->len] = '\0';

   return strdes->str;
}


/*-------------------------------------------------------------------------

FUNCTION         : char* SetString(String_p strdes, char* string)

Beschreibung     : Setzt den String auf den angegebenen Wert.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 21.8.1994 neu

-------------------------------------------------------------------------*/

char* SetString(String_p strdes, char* string)
{
   ResetString(strdes);
   return AppendString(strdes,string);
}



/*-------------------------------------------------------------------------

FUNCTION         : long StringLength(String_p strdes)

Beschreibung     : Gibt Laenge des Strings zurueck. Macht Zugriff auf
                   die Laenge transparent.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 21.8.1994 neu

-------------------------------------------------------------------------*/

long StringLength(String_p strdes)
{
   if(strdes->str)
   {
      return strdes->len;
   }
   return 0;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void ResetString(String_p strdes)                       */
/*                    INOUT  String_p strdes                                  */
/*                                                                            */
/* Beschreibung     : Setzt den beschriebenen String auf "".                  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Setzt den beschriebenen String auf "".                  */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/


void ResetString(String_p strdes)
{
   if(strdes->str)
   {
      strdes->len = 0;
      *(strdes->str) = '\0';
   }
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


