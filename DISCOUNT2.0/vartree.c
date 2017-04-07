/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          vartree
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Funktionen zur Verwaltung von Variablenbaeumen.
//-----------------------------------------------------------------------------
//      $Log: vartree.c,v $
//      Revision 0.1  1991/08/19  09:50:47  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/19  09:32:15  pitz
//      Neue Nummerierung aufgrund eines Datenverlustes durch Stromausfall !!
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>

#include    "memory.h"
#include    "vartree.h"


/*
//-----------------------------------------------------------------------------
//      Lokale Daten und Funktionen
//-----------------------------------------------------------------------------
*/

static vartree *freelist = NULL;


#ifdef ANSI
    static vartree *new ( variable code, variable info );
    static void delete  ( vartree *cell );
    static void rdelete ( vartree *cell );
#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTinit
//
//  Parameter:      -keine-
//
//  Beschreibung:   Loeschen der Freiliste.
//-----------------------------------------------------------------------------
*/

void    VTinit ( void )
{
    freelist = NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       new ( variable code, variable info )
//
//  Parameter:      code        Code fuer die Variable
//                  info        Information ueber Variable
//
//  Rueckgabewert:  Zeiger auf einen neuen Knoten fuer vartree
//
//  Beschreibung:   Anlegen eines neuen Knotenelements.
//                  Unbelegte Knotenelemente werden aus freelist gewonnen.
//                  Sind keine freien Knotenelemente vorhanden, so werden
//                  TREEALLOC neue Elemente allociert.
//-----------------------------------------------------------------------------
*/

static vartree *new ( variable code, variable info )
{
    register vartree    *ptr, *next;
    register short      i;
             long       size;
             short      index;

    if (!freelist)
    {
        size = TREEALLOC * sizeof (vartree);
        ptr = freelist = Allocate ( size );
        if (!ptr)
            Error ( __FILE__ ": "  "new", "Zuwenig Speicher fuer Variablenbaum." );

        for ( i = 1; i < TREEALLOC; i++ )
        {
            next = ptr;
            #ifdef MEMDEBUG
               ptr->debug = 0;
            #endif 
            ptr = ptr->son = ++next;
        }
        ptr->son = NULL;
    }

    ptr      = freelist;
    freelist = freelist->son;

    for ( i = 0; i < VTSIZE; i++ )
    {
        ptr->code[i]  =    0;
        ptr->info[i]  =    0;
        ptr->count[i] =    0;
        ptr->son      = NULL;
    }
    index = code & VTMASK;
    ptr->code[index]  = code;
    ptr->info[index]  = info;
    ptr->count[index] =    1;

    #ifdef MEMDEBUG
       if (ptr->debug)
          Error ( __FILE__ ": "  "vartree:new", "MEMORY-ERROR." );
       ptr->debug++;
    #endif

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       delete ( vartree *knot )
//
//  Parameter:      knot    ein Knoten
//
//  Beschreibung:   Das Knotenelement knot wird in die Freiliste freelist
//                  umgekettet.
//-----------------------------------------------------------------------------
*/

static void delete ( vartree *knot )
{
    #ifdef MEMDEBUG
       knot->debug--;
       if (knot->debug)
           Error ( __FILE__ ": "  "vartee:delete", "MEMORY-ERROR." );
    #endif

    knot->son    = freelist;
    freelist     = knot;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       rdelete ( vartree *knot )
//
//  Parameter:      knot    ein Knoten
//
//  Beschreibung:   Rekursives Freigeben eines Variablenbaums
//-----------------------------------------------------------------------------
*/

static void rdelete ( vartree *cell )
{
    if (cell)
    {
        if (cell->son)
            rdelete (cell->son);

        delete ( cell );
    }
}


/*
//=============================================================================
//      Globale Definitionen fuer Export
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       VTclear ( vartree **root )
//
//  Parameter:      root    Referenz auf Wurzel der Variablenbaums
//
//  Beschreibung:   Loeschen des Variablenbaums
//-----------------------------------------------------------------------------
*/

void    VTclear ( vartree **root )
{
    rdelete ( *root );
    *root = NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTadd   ( vartree **root, variable code, variable info )
//
//  Parameter:      root    Referenz auf Wurzel der Variablenbaums
//                  code    Code fuer die Variable
//                  info    Information zur Variable
//
//  Beschreibung:   Einfuegen in Variablenbaum.
//                  Mehrfacheinfuegen moeglich !
//-----------------------------------------------------------------------------
*/

void    VTadd   ( vartree **root, variable code, variable info )
{
    register vartree    **ptr;
    register short      index;

    ptr = root;
    index = code & VTMASK;
    while (*ptr)
    {
        if (!((*ptr)->code[index]))
        {
            (*ptr)->code[index]  = code;
            (*ptr)->info[index]  = info;
            (*ptr)->count[index] =    1;
            return;
        }
        ptr = &((*ptr)->son);
    }
    *ptr = new ( code, info );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTfind  ( vartree *root, variable code )
//
//  Parameter:      root    Wurzel der Variablenbaums
//                  code    gesuchter Variablencode
//
//  Rueckgabewert:  Info der Variablen code oder 0
//
//  Beschreibung:   Suchen einer Variablen im Variablenbaum.
//-----------------------------------------------------------------------------
*/

variable    VTfind  ( vartree *root, variable code )
{
    register short      index;

    index = code & VTMASK;
    while (root)
    {
        if (code == root->code[index])
            return root->info[index];

        root = root->son;
    }
    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTfindinc  ( vartree *root, variable code )
//
//  Parameter:      root    Wurzel der Variablenbaums
//                  code    gesuchter Variablencode
//
//  Rueckgabewert:  Info der Variablen code oder 0
//
//  Beschreibung:   Suchen einer Variablen im Variablenbaum.
//                  Erhoehen des Zaehlers, falls Variable vorhanden.
//-----------------------------------------------------------------------------
*/

variable    VTfindinc  ( vartree *root, variable code )
{
    register short      index;

    index = code & VTMASK;
    while (root)
    {
        if (code == root->code[index])
        {
            root->count[index]++;
            return root->info[index];
        }

        root = root->son;
    }
    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTfindcount  ( vartree *root, variable code )
//
//  Parameter:      root    Wurzel der Variablenbaums
//                  code    gesuchter Variablencode
//
//  Rueckgabewert:  Count der Variablen code oder 0
//
//  Beschreibung:   Suchen einer Variablen im Variablenbaum.
//-----------------------------------------------------------------------------
*/

short   VTfindcount ( vartree *root, variable code )
{
    register short      index;

    index = code & VTMASK;
    while (root)
    {
        if (code == root->code[index])
            return root->count[index];

        root = root->son;
    }
    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTless    ( vartree *vt1, vartree *vt2 )
//
//  Parameter:      vt1, vt2    zu vergleichende Variablenbaeume
//
//  Rueckgabewert:  true    Alle in vt1 vorkommenden Variablen kommen
//                          mindestens genausooft in vt2 vor.
//                  false   sonst
//
//  Beschreibung:   Vergleich von Vorkommen von Variablen in Variablenbaeumen.
//-----------------------------------------------------------------------------
*/

bool    VTless    ( vartree *vt1, vartree *vt2 )
{
    short       i;
    variable    x, y;

    if (!(vt1))
        return true;

    for ( i = 0 ; i < VTSIZE; i++ )
    {
        if ((x = vt1->code[i]) != 0)
        {
            if ((y = VTfindcount (vt2, x)) <= 0)
                return false;

            if (y < vt1->count[i])
                return false;
        }
        if (!VTless (vt1->son, vt2))
            return false;
    }
    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTpartof ( vartree *vt1, vartree *vt2 );
//
//  Parameter:      vt1, vt2    zu vergleichende Variablenbaeume
//
//  Rueckgabewert:  true    Alle in vt1 vorkommenden Variablen kommen
//                          mindestens einmal in vt2 vor.
//                  false   sonst
//
//  Beschreibung:   Vergleich von Vorkommen von Variablen in Variablenbaeumen.
//-----------------------------------------------------------------------------
*/

bool    VTpartof ( vartree *vt1, vartree *vt2 )
{
    short       i;
    variable    x;

    if (!(vt1))
        return true;

    for ( i = 0 ; i < VTSIZE; i++ )
    {
        if ((x = vt1->code[i]) != 0)
        {
            if (!VTfind (vt2, x))
                return false;
        }

        if (!VTpartof (vt1->son, vt2))
            return false;
    }
    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       VTprint ( vartree *root )
//
//  Parameter:      root    Wurzel der Variablenbaums
//
//  Beschreibung:   Kontrollausgabe des Variablenbaums.
//-----------------------------------------------------------------------------
*/

void VTprint ( vartree *root )
{
   short   i;
   
   if (root)
   {
      for ( i = 0 ; i < VTSIZE ; i++ )
      {
	 if (root->code[i])
	 {
	    printf ( "%4ld %4ld %4ld\n",
		    (long)(-root->code[i]),
		    (long)(root->info[i]),
		    (long)(root->count[i]) );
	 }
      }
      VTprint ( root->son );
   }
}
