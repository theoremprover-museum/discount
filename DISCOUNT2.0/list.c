/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          list
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul stellt einfache Listenoperation zur
//                      Verfuegung.
//-----------------------------------------------------------------------------
//      $Log: list.c,v $
//      Revision 0.1  1991/08/19  09:49:49  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>
#include    <string.h>

#ifdef  ATARI
    #include    <stdlib.h>
#endif


#include    "list.h"


/*
//=============================================================================
//      Listenverwaltung
//=============================================================================
*/

static listcell *freelist = NULL;


/*
//-----------------------------------------------------------------------------
//  Funktion:       newlist ( void *pointer )
//
//  Parameter:      pointer     Zeiger auf irgendwas
//
//  Rueckgabewert:  Zeiger auf ein neues Listenelement, das die Information
//                  Pointer enthaelt.
//
//  Beschreibung:   Anlegen einer neuen Listenzelle.
//                  Unbelegte Listenelemente werden aus der Freiliste
//                  freelist gewonnen.
//                  Sind keine freien Listenelemente vorhanden, so werden
//                  LISTALLOC neue Elemente allociert und in die Freiliste
//                  eingekettet.
//-----------------------------------------------------------------------------
*/

listcell     *newlist ( void *pointer )
{
    register listcell   *ptr, *next;
    register short      i;
             long       size;

    if (!freelist)
    {
        size = LISTALLOC * sizeof (listcell);
        ptr  = freelist = Allocate ( size );
        if (!ptr)
            Error ( __FILE__ ": "  "newlist", "Zuwenig Speicher fuer Listeneintrag." );

        for ( i = 1; i < LISTALLOC; i++ )
        {
            next = ptr;
            ptr  = ptr->next = ++next;
        }
        ptr->next = NULL;
    }

    ptr = freelist;
    freelist = freelist->next;
    ptr->info = pointer;
    ptr->next = NULL;
    return ptr;
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       deletelist ( listcell *cell )
//
//  Parameter:      cell    ein Listenelement
//
//  Beschreibung:   Das Listenelement cell wird in die Freiliste freelist
//                  eingekettet.
//-----------------------------------------------------------------------------
*/

void     deletelist ( listcell *cell )
{
    cell->next = freelist;
    freelist = cell;
}


/*-----------------------------------------------------------------------
//
// Function: newdlist()
//
//   Allociere eine neue DListCell und initialisiere "Value"
// geeignet. Analog zu newlist.
//
// Global Variables: -
//
// Side Effects    : Speicheroperationen
//
/----------------------------------------------------------------------*/

DList_p newdlist(double  info)
{
   DList_p handle;
   
   handle       = AllocDListCell();
   handle->info = info;
   handle->next = NULL;
   
   return handle;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       DeleteList ( list alist )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//
//  Beschreibung:   Die Liste alist wird geloescht,
//                  d.h. sie wird in die Freiliste umgekettet.
//-----------------------------------------------------------------------------
*/

void    DeleteList ( list *alist )
{
    if (alist->last)
    {
        alist->last->next = freelist;
        freelist = alist->first;

        alist->first  = alist->last = alist->now = NULL;
        alist->length = 0;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Length ( list alist )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//
//  Rueckgabewert:  Laenge der Liste alist
//
//  Beschreibung:   Die Laenge der Liste alist wird ermittelt.
//-----------------------------------------------------------------------------
*/

short   Length ( list *alist )
{
    return  alist->length;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       AddList ( list alist, void *ptr )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//                  ptr     Einzufuegender Pointer
//
//  Beschreibung:   Ein Element an eine Liste anfuegen.
//-----------------------------------------------------------------------------
*/

void    AddList ( list *alist, void *ptr )
{
    if (alist->last)  alist->last  = alist->last->next = newlist (ptr);
                else  alist->first = alist->last = newlist (ptr);
    alist->length++;
}


/*
//-----------------------------------------------------------------------------
//  Funktionen:     First ( list alist )
//                  Next  ( list alist )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//
//  Rueckgabewert:  die entsprechende Information (Zeiger).
//
//  Beschreibung:   Zugriffsfunktionen
//-----------------------------------------------------------------------------
*/

void    *First ( list *alist )
{
    alist->now = alist->first;
    return (alist->now) ? alist->now->info : NULL;
}

void    *Next ( list *alist )
{
    alist->now = (alist->now) ? alist->now->next : NULL;
    return (alist->now) ? alist->now->info : NULL;
}


