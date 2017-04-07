/*-------------------------------------------------------------------------

File        : newmem.c

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

#include "newmem.h"



/*-----------------------------------------------------------------------*/
/*                         Globale Variable                              */
/*-----------------------------------------------------------------------*/


#define MEM_ARR_SIZE 300

static Mem_p free_mem_list[MEM_ARR_SIZE] = {NULL};

/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void* SizeMalloc(int size)

Beschreibung     : Returns a block of memory sized size using the
                   internal free-list. This block is freeable with
		   free(), and in all respects behaves like a normal
		   malloc'ed block.

Globale Variable : free_mem_list[]

Seiteneffekte    : Memory operations

Aenderungen      : <1> 9.12.1994 neu

-------------------------------------------------------------------------*/

void* SizeMalloc(int size)
{
   void* handle;
   int    index;
   
 /*  printf("SizeMalloc(%d) called\n", size); */

   index = size - sizeof(MemCell);

   if((index>=0) && (index<MEM_ARR_SIZE) && free_mem_list[index])
   {
      handle = (void*)(free_mem_list[index]);
      free_mem_list[index] = free_mem_list[index]->next;
   } 
   else
   {
      handle = SecureMalloc(size);
   }
   return handle;
}   



/*-------------------------------------------------------------------------

FUNCTION         : void SizeFree(void* junk, int size)

Beschreibung     : Returns a block sized size. Note: size has to be
                   exact - you should onlny give blocks to SizeFree()
		   that have been allocasted with malloc(size) or
		   SizeMalloc(size). 

Globale Variable : free_mem_list[]

Seiteneffekte    : Memory operations

Aenderungen      : <1> 9.12.1994 neu

-------------------------------------------------------------------------*/

void SizeFree(void* junk, int size)
{
   int    index;
   
 /*  printf("SizeFree(%d) called\n", size);*/

   index = size - sizeof(MemCell);

   if(!junk) 
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to typ_free...");
   }
   else if(index>=0 && index<MEM_ARR_SIZE)
   {
      ((Mem_p)junk)->next = free_mem_list[index];
      free_mem_list[index] = (Mem_p)junk;
   } 
   else
   {
      free(junk);
   }
}   


/*-------------------------------------------------------------------------

FUNCTION         : void FreeListFlush()

Beschreibung     : Returns all memory kept in free_mem_list[] to the
                   operation system. This is useful if a very
		   different memory access pattern is expected (as
		   SizeFree() never reorganizes the memory).

Globale Variable : free_mem_list[]

Seiteneffekte    : Memory operations

Aenderungen      : <1> 11.12.1994 neu

-------------------------------------------------------------------------*/

void FreeListFlush()
{
   int f;
   void* handle;

   for(f = 0;f<MEM_ARR_SIZE;f++) 
   {
      while(free_mem_list[f])
      {
	 handle = (void*)free_mem_list[f];
	 free_mem_list[f] = free_mem_list[f]->next;
	 free(handle);
      }
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : void* SecureMalloc(int size)

Beschreibung     : Allocates a block sized size, checks for
                   availability and will recycle free_mem_list. 

Globale Variable : free_mem_list[]

Seiteneffekte    : Memory operations, Termination when out of memory.

Aenderungen      : <1> 9.12.1994 neu

-------------------------------------------------------------------------*/

void* SecureMalloc(int size)
{
   void* handle;

   handle = (void*)malloc(size);

   if(!handle)
   {    /* malloc has no memory left  */
      FreeListFlush(); /* Return own freelist */
      
      handle = (void*)malloc(size);
      
      if(!handle)
      {   /*  Still nothing...*/
         Error(__FILE__ "SecureMalloc()",
	       "Out of Memory");
      }
   }
   return handle;
}

/*-------------------------------------------------------------------------
//
// Function: SecureRealloc()
//
//   Imitates Realloc, but uses SecureMalloc to get new memory.
//
// Global Variables: -
//
// Side Effect     : Via SecureMalloc()
//
//-----------------------------------------------------------------------*/

void* SecureRealloc(void *ptr, int size)
{
   void* handle;
   
   handle = realloc(ptr,size);
   if(!ptr && size!=0)
   {
      FreeListFlush();
      handle = realloc(ptr,size);
      if(!handle)
      {   /*  Still nothing...*/
         Error(__FILE__ "SecureMalloc()",
	       "Out of Memory");
      }
   }
   return handle;
}

/*-------------------------------------------------------------------------

FUNCTION         : char* SecureStrdup(char* source)

Beschreibung     : Implements the functionality of strdup, but uses
                   SecureMalloc() for the memory handling.

Globale Variable : -

Seiteneffekte    : By SecureMalloc()

Aenderungen      : <1> 15.1994 neu

-------------------------------------------------------------------------*/

char* SecureStrdup(char* source)
{
   char* handle;

   handle = (char*)SecureMalloc(strlen(source)+1);
   strcpy(handle,source);

   return handle;
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


