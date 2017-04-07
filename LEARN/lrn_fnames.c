/*-------------------------------------------------------------------------

File        : lrn_fnames.c

Autor       : Stephan Schulz

Inhalt      : Funktionen, die mit Filenamen umgehen.

Aenderungen : <1> 

-------------------------------------------------------------------------*/

#include "lrn_fnames.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

char* find_last_pos(char* search, char ch);


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : char* find_last_pos(char* search, char ch)

Beschreibung     : Gibt einen Zeiger auf das letzte Vorkommen von ch in
                   String zurueck - NULL, falls er nicht vorkommt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> .1994 neu

-------------------------------------------------------------------------*/

char* find_last_pos(char* search, char ch)
{
   char *point, *pos;

   pos = NULL;
   point = search;
   
   while(*point)
   {
      if(*point == ch)
      {
	 pos = point;
      }
      point++;
   }
   return pos;
}



/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : String_p FileName(char* name)

Beschreibung     : Sucht zu einer Pfadangabe den tatsaechlichen
                   Filenamen, ohne Direktory-Angabe

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 21.8.1994 neu

-------------------------------------------------------------------------*/

String_p FileName(char* name)
{
   char* point;
   String_p handle;

   handle = AllocString();
      
   if((point = find_last_pos(name,'/')))
   {
      AppendString(handle,point+1);
   }
   else
   {
      AppendString(handle,name);
   }
   
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : char* BaseName(char* name)

Beschreibung     : Sucht zu einem (beliebigen) Filenamen den Basename,
                   also den Namen ohne .xxx

Globale Variable : -

Seiteneffekte    : Speicheroperationen (indirekt). 

Aenderungen      : <1> 21.8.1994 neu

-------------------------------------------------------------------------*/

char* BaseName(char* name)
{
   String_p handle;
   char*    end;
   char*    store;

   handle = FileName(name);
      
   end = find_last_pos(ViewString(handle), '.');

   if(end)
   {
      if(strlen(end)>4)
      {
	 fprintf(stderr,"Warning: Suffix has more than 3 characters!\n");
      }
      *end = '\0';
      store =  secure_strdup(ViewString(handle));
   }
   else
   {
      store = secure_strdup(name);
   }
   FreeString(handle);

   return store;
}
   
/*-------------------------------------------------------------------------

FUNCTION         : String_p NormName(char* name)

Beschreibung     : Berechnet den absoluten, normalisierten Filenamen
                   zu einem  beliebigen Namen.

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 21.8.1994 neu

-------------------------------------------------------------------------*/  

String_p NormName(char* name)
{
   String_p handle;
   static char store[MAXPATHLEN+2];
   char* res;

   handle = AllocString();

   if(!(res = realpath(name, store)))
   {
      fprintf(stderr, "Filename '%s' could not be resolved...\n", name);
      perror("ERROR: realpath");
      exit(SYSTEM_ERROR);
   }

   AppendString(handle,store);
   
   return handle;
}
      

   
/*-------------------------------------------------------------------------

FUNCTION         : String_p DirNormName(char* name)

Beschreibung     : Berechne den absoluten, normalisierten Namen des
                   Directories, in dem der name eine Datei (oder ein
		   Directory) bezeichnet.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 8.9.1994 neu

-------------------------------------------------------------------------*/  

String_p DirNormName(char* name)
{
   String_p handle;
   char* res;

   res = find_last_pos(name, '/');

   if(!res)
   {
      handle = NormName(".");
   }
   else if(res == name)
   {
      handle = NormName("/");
   }
   else
   {
      *res = '\0';
      handle = NormName(name);
      *res = '/';
   }

   return handle;
}
   

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


