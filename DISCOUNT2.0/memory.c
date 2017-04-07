/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         memory
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung der Reports der einzelnen Experten.
//-----------------------------------------------------------------------------
//      $Log: memory.c,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>
#include <signal.h>


#ifdef ATARI
    #include    <stdlib.h>
#endif

#include    "memory.h"


/*
//-----------------------------------------------------------------------------
//      Blockliste
//-----------------------------------------------------------------------------
*/

struct block          { struct block  *next;
                        long          size;
                        char          block;    };

static struct block     *Alloclist      = NULL;

long                    memory_size = 0;
void*                   Reserve;


/*
//-----------------------------------------------------------------------------
//  Funktion:       AllocReserve
//
//  Parameter:      -keine-
//
//  Beschreibung:   allokiert einen 10 kB grossen Speicherblock. Dieser Block
//                  wird freigegeben falls kein Speicher mehr zur Verfuegung
//                  steht. 
//-----------------------------------------------------------------------------
*/
void AllocReserve( void )
{
    
     Reserve = Malloc( 10240 );

}


/*
//-----------------------------------------------------------------------------
//  Funktion:       FreeReserve
//
//  Parameter:      -keine-
//
//  Beschreibung:   gibt den mit AllocReserve besorgten Speicher wieder zurueck
//-----------------------------------------------------------------------------
*/
void FreeReserve( void )
{
    
    Mfree( Reserve );     

}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Allocate
//
//  Parameter:      size        Groesse des angeforderten Speicherblocks
//
//  Rueckgabewert:  Zeiger auf Speicherbereich.
//
//  Beschreibung:   Anfordern von Speicher und einketten des Blocks in
//                  die Alloc-Liste.
//-----------------------------------------------------------------------------
*/

void    *Allocate   ( long size )
{
    struct block    *ptr;

    ptr = Malloc ( size + sizeof (struct block) );
    memory_size += size;
    if (ptr)
    {
        ptr->size = size;
        ptr->next = Alloclist;
        Alloclist = ptr;
        return &(ptr->block);
    }
    else
    {
        printf ( "Kein Speicher mehr.\n" );
	FreeReserve();

	while (1)
	  sleep ( 10000 );
	exit ( 0 );
    }
    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Free
//
//  Parameter:      -keine-
//
//  Beschreibung:   Freigeben des mit Allocate angeforderten Speichers
//-----------------------------------------------------------------------------
*/

void    Free ( void )
{
    struct block    *ptr;

    while (Alloclist)
    {
        ptr       = Alloclist;
        Alloclist = Alloclist->next;
/*        fprintf ( stderr, "Free %ld %ld\n", (long)ptr, ptr->size ); */
        Mfree ( ptr );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Statistic
//
//  Parameter:      -keine-
//
//  Beschreibung:   Ausgabe einer Speicherstatistik
//-----------------------------------------------------------------------------
*/

void    Statistic   ( void )
{
    struct block    *ptr    = Alloclist;
    short           i       = 1;
    long            size    = 0;

    printf ( "\n\n" );
    while (ptr)
    {
        printf ( "%3d.            %7ld Bytes\n", i++, (ptr->size) );
        size += ptr->size;
        ptr = ptr->next;
    }
    printf ( "\nSpeicherbedarf: %7ld Bytes\n\n", size );
}

