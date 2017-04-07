/*-------------------------------------------------------------------------

File        : lrn_parse.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zum Parser fuer Domaenen, Brains, und die
              dazugehoerigen Datentypen.

Aenderungen : <1> 25.8.1994 neu

-------------------------------------------------------------------------*/

#ifndef _lrn_parse

#define _lrn_parse

#include "lrn_domains.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/

Term_p     ParseNormTerm();
NormEqn_p  ParseNormEqn();
EqnOccur_p ParseOccurList();
NormEqn_p  ParseNormEqnLine();
long       ParseNormEqnList(NormEqn_p anchor, BOOL add);
long       ParseEqnTree(NormEqn_p *anchor);
Symbol_p   ParseSig();
EqnOccur_p ParseOccurNamesList();
Dom_p      ParseDomHead();
Dom_p      ParseDomHeadList();
void       ParseDomBody(Dom_p dom);
Dom_p      ParseDom();
Brain_p    ParseBrain();

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





