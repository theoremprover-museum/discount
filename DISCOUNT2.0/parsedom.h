#ifndef __PARSEDOM
#define __PARSEDOM

#include "domain.h"

/*****************************************************************************/
/*                                                                           */
/*                       exportierte Funktionen                              */
/*                                                                           */
/*****************************************************************************/

term  *newterm_dom ( function fcode, short arity );
void  delterm_dom ( term *t );
void  deleteterm_dom ( term *t );
short ParseDom ( char *filename , DomStatusType d_type );

#endif
