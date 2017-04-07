/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         list
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul stellt einfache Listenoperation zur 
//                      Verfuegung.
//-----------------------------------------------------------------------------
//      $Log: list.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __LIST
#define __LIST

#include    "error.h"
#include    "memory.h"
#include    "newmem.h"

/*
//-----------------------------------------------------------------------------
//      Datenstrukturen
//-----------------------------------------------------------------------------
*/

typedef struct lc
{
   void      *info;
   struct lc *next;
} listcell;

typedef struct
{
   listcell  *first;
   listcell  *last;
   listcell  *now;
   short     length; 
#ifdef MEMDEBUG
   short   debug;
#endif    
} list;

#define EmptyList   { NULL, NULL, NULL, 0 }

/* List for storing doubles, added by StS */

typedef struct dlc
{
   double info;
   struct dlc *next;
} DListCell,*DList_p;

#define AllocDListCell() (DListCell*)SizeMalloc(sizeof(DListCell))
#define FreeDListCell(junk)          SizeFree(junk, sizeof(DListCell))

/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/


listcell *newlist   ( void     *pointer );
void     deletelist ( listcell *cell );

DList_p  newdlist    (double  info);
#define deletedlist(cell) FreeDListCell(cell)

void     DeleteList ( list *alist );
short    Length     ( list *alist );
void     AddList    ( list *alist, void *ptr );
void     *First     ( list *alist );
void     *Next      ( list *alist );



#endif
