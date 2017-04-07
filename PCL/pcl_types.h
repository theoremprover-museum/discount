/*-------------------------------------------------------------------------

File        : pcl_types.h  

Autor       : Stephan Schulz

Inhalt      : Sowohl von pcl_parse als auch pcl_miniparse benoetigte
              Typen mit Speicherfunktionen 

Aenderungen : <1> 15.2.1992 neu

-------------------------------------------------------------------------*/


#include "pcl_scan.h"
#include "pcl_printer.h"


#ifndef _pcl_types

#define _pcl_types



/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

typedef struct numlistcell
{
   long                value;
   struct numlistcell *rest;
}NumListCell,*NumList_p;


typedef struct numlistlistcell   /* Liste von NumListen <=> PCL-Identifiern */
{
   struct numlistlistcell *pred;
   struct numlistlistcell *succ;
   long                   value;  /* Store a numeric value (ie weight) */
   NumList_p     this;
}NumListListCell,*NumListList_p;



typedef struct placetype
{
   char      side;
   NumList_p rest; 
}PlaceCell,*Place_p;

typedef enum 
{
   none,
   lemma,
   axiom,
   theorem
}ExStep; /* extraordinary step, d.h. Axiom, Theorem, oder Lemma */



/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

NumList_p      AllocNumListCell();
NumListList_p  AllocNumListListCell();
Place_p        AllocPlaceCell();


void           FreeNumList(NumList_p junk);
void           FreeNumListCell(NumList_p junk);

void           FreeNumListListCell(NumListList_p junk);
void           FreeNumListList(NumListList_p junk);

void           FreePlace(Place_p junk);
void           FreePlaceCell(Place_p junk);


NumList_p      CopyNumList(NumList_p source);
long           CmpNumList(NumList_p id1, NumList_p id2);
NumList_p      ConcatNumList(NumList_p l1, NumList_p l2);


void PrintNumList(NumList_p prt);
void PrintIdList(NumList_p prt);


NumListList_p CopyNumListList(NumListList_p source);
NumListList_p MergeNumListLists(NumListList_p anchor,NumListList_p new);
NumListList_p InsertNumList(NumListList_p list,NumList_p id);
NumListList_p RemoveElems(NumListList_p list,NumListList_p to_rem);
void          PrintNumListList(NumListList_p prt);


void PrintComment(char* prt);



#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/




