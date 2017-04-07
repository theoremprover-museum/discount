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
//      $Log: memory.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __MEMORY
#define     __MEMORY

#include "defines.h"


/*
//-----------------------------------------------------------------------------
//      Groesse des belegten Speichers
//-----------------------------------------------------------------------------
*/

extern long     memory_size;


/*
//-----------------------------------------------------------------------------
//      Makro fuer Speicherreorganisation
//-----------------------------------------------------------------------------
*/


#define LOWBITS         10      /* Diese Bits werden nicht geprueft     */
#define STARTSHIFT      24      /* Soviel Bits wird zuerst geschoben    */
#define DIFFSHIFT        7      /* Differenz zwischen zwei Phasen       */
#define ARRAYSIZE      128      /* Groesse des Feldes = 2^DIFFSHIFT     */
#define BITMASK       0x7f   
#define TESTMASK   0x7ffffc00


#define REORG_PROTOTYPE(name,type)                                      \
static type *name ( type *ptr, short shift );

#define REORG_FUNCTION(name,type,next)                                  \
static type *name ( type *ptr, short shift )                            \
{                                                                       \
    type    *array[ARRAYSIZE] = { NULL };                               \
    type    *hptr;                                                      \
    short   i;                                                          \
    static  last = 100;                                                 \
    static  phase;                                                      \
                                                                        \
    if (shift < LOWBITS)                                                \
        return ptr;                                                     \
                                                                        \
    if (shift == STARTSHIFT)                                            \
    {                                                                   \
        last = 100;                                                     \
        phase = 1;                                                      \
    }                                                                   \
                                                                        \
    if (shift < last)                                                   \
    {                                                                   \
        last = shift;                                                   \
        phase++;                                                        \
    }                                                                   \
                                                                        \
    while (ptr)                                                         \
    {                                                                   \
        hptr = ptr;                                                     \
        ptr  = ptr->next;                                               \
        i    = ((long)(hptr) >> shift) & BITMASK;                       \
        hptr->next = array[i];                                          \
        array[i] = hptr;                                                \
    }                                                                   \
                                                                        \
    for (i = 0; i < ARRAYSIZE; i++)                                     \
    if (array[i])                                                       \
        array[i] = name ( array[i], shift-DIFFSHIFT );                  \
                                                                        \
    hptr = NULL;                                                        \
    for (i = ARRAYSIZE-1; i >= 0 ; i--)                                 \
    if (array[i])                                                       \
    {                                                                   \
        ptr = array[i];                                                 \
        while (ptr->next)                                               \
            ptr = ptr->next;                                            \
        ptr->next = hptr;                                               \
        hptr = array[i];                                                \
    }                                                                   \
    return hptr;                                                        \
}


#define TEST_REORG(name,type,next)                                      \
static void name (type *ptr)                                            \
{                                                                       \
    if (!ptr)                                                           \
        return;                                                         \
    while (ptr->next)                                                   \
    {                                                                   \
        if (((long)(ptr) & TESTMASK) > ((long)(ptr->next) & TESTMASK))  \
            printf ( __FILE__ " error in sortlist %lx %lx.\n",          \
                               (long)(ptr) & TESTMASK, (long)(ptr->next) & TESTMASK );  \
                                                                        \
        ptr = ptr->next;                                                \
    }                                                                   \
}



/*
//-----------------------------------------------------------------------------
//      Funktionsdeklarationen
//-----------------------------------------------------------------------------
*/


void    AllocReserve( void );

void    *Allocate   ( long size );
void    Free        ( void );

void    Statistic   ( void );

#endif

