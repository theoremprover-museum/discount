/*-------------------------------------------------------------------------

File        : pcl_mininorm.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zu Funktionen zum "normalisieren von
              PCL-Listings, also zum Entfernen von reinen
	      "cite"-Expressions und (spaeter) zum Entfernen von
	      duplizierten Ergebnissen.

Aenderungen : <1>  2.3.1994 neu 
              <2> 13.3.1994 Neue Fkt. FindStepByIdBack()

-------------------------------------------------------------------------*/

#ifndef _pcl_mininorm

#define _pcl_mininorm

#include "pcl_miniparse.h"




/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/


typedef struct numlisttreecell   /* Geordnete Baume von               */
				 /* pcl-identifier-Schluesseln mit    */
				 /* PCL-(mini-)Schritten als Wert     */
{
   struct numlisttreecell *right;
   struct numlisttreecell *left;
   NumList_p              key;
   miniStep_p             value;
   long                   balance;  /* Fuer AVL-Baeume */
}NumListTreeCell,*NumListTree_p;


typedef struct stringtreecell   /* Dasselbe fuer Strings = */
				/* minitermpairs           */
{
   struct stringtreecell *right;
   struct stringtreecell *left;
   char*                 key;
   miniStep_p            value;
}StringTreeCell,*StringTree_p;



/*-----------------------------------------------------------------------*/
/*                  Deklaration exportierter Variablen                   */
/*-----------------------------------------------------------------------*/

extern BOOL Normalize;
extern BOOL Normalize2;


/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


NumListTree_p AllocNumListTreeCell();
StringTree_p  AllocStringTreeCell();


void          FreeNumListTreeCell(NumListTree_p junk);
void          FreeStringTreeCell(StringTree_p junk);

void          FreeNumListTree(NumListTree_p junk);
void          FreeStringTree(StringTree_p junk);

BOOL          NumListTreeInsert(NumListTree_p *anchor, NumListTree_p new);
NumListTree_p NumListTreeFind(NumListTree_p anchor, NumList_p search);
void          PrintNumListTree(NumListTree_p anchor);

void          StringTreeInsert(StringTree_p *anchor, StringTree_p new);
StringTree_p  StringTreeFind(StringTree_p anchor, char* search);
void          PrintStringTree(StringTree_p anchor);



miniJust_p    RenumJust(miniJust_p just, NumListTree_p data);

miniStep_p    FindStepById(NumList_p id, miniStep_p anchor, 
			   miniStep_p final);
miniStep_p    FindStepByIdBack(NumList_p id, miniStep_p anchor, 
			       miniStep_p final);
long          NormalizeProof(miniStep_p anchor);
long          StrongNormalizeProof(miniStep_p anchor);


#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





