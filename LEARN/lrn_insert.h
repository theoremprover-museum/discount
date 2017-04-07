/*-------------------------------------------------------------------------

File        : lrn_insert.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zu Funktionen, die sich mit dem Einfuegen
              von neuen Daten in eine vorhandene Wissensbasis
	      beschaeftigen. 

Aenderungen : <1> 9.9.1994 neu

-------------------------------------------------------------------------*/

#ifndef _lrn_insert

#define _lrn_insert

#include <math.h>
 
#include "lrn_domains.h"
#include "lrn_parse.h"

#include "lrn_eqn.h"
#include "lrn_pcltrans.h"

/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


Brain_p ReadBrain(char* kb_path, char* fname, char* example);
Dom_p   ReadDom(char* kb_path, char* fname);
Dom_p   ReadDomHeadList(char* kb_path, char* fname, char* domname);

Dom_p   FindCompatDom(Dom_p list, Dom_p dom, NormSubst_p *make_equal);
Dom_p   FindNewInsertPlace(Dom_p list, Dom_p new_dom);
BOOL    InsertNewDom(char* kb_path, Dom_p new_dom, Dom_p domlist);

void    WriteExample(Step_p anchor, char* kb_path, char* example);
void    WriteSelectionData(Step_p anchor, char* kb_path, char* example);
void    WriteBrain(Brain_p brain, char* kb_path, char* fname);
void    WriteDom(Dom_p dom, char* kb_path, char* fname);
void    WriteDomHeadList(Dom_p domlist, char* kb_path, char* fname);

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





