/*-------------------------------------------------------------------------

File        : newmem.h

Autor       : Stephan Schulz

Inhalt      : This implements a simple-to-use, efficient memory
              management for problems with a very regular memory
	      access pattern (like most theorem provers). I did some
	      experiments and found that this is usually faster than
	      DISCOUNT's old memory system. It is also simpler and
	      more transparent. It would probably pay of to replace
	      all the old code, but that's a task for someone else
	      ;-). 

Aenderungen : <1> 9.12.1994

-------------------------------------------------------------------------*/

#ifndef _newmem

#define _newmem

#include <memory.h>
#include "error.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

typedef struct memcell
{
   struct memcell* next;
}MemCell, *Mem_p;



/*-----------------------------------------------------------------------*/
/*      Deklaration exportierter Funktionen und Variablen                */
/*-----------------------------------------------------------------------*/


extern void* SizeMalloc(int size);
extern void  SizeFree(void* junk, int size);
extern void  FreeListFlush();
extern void* SecureMalloc(int size);
extern void* SecureRealloc(void *ptr, int size);
extern char* SecureStrdup(char* source);


/*-------------------------------------------------------------------------
  If you want to have a special Allocator and Deallocator for a
  datatype just copy the following templates to your .h-file and fill
  them in... The allocated structures will not be initialized - you
  need to write a function build on top of the macros if you want more
  functionality in you Allocator.


#define AllocDataCell() (DataCell*)SizeMalloc(sizeof(DataCell))
#define FreeDataCell(junk)         SizeFree(junk, sizeof(DataCell))

-------------------------------------------------------------------------*/

#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





