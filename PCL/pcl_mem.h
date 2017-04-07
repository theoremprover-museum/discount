/*************************************************************************/
/*                                                                       */
/*   File:        pcl_mem.h                                              */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Definitionen zur Speicherverwaltung fuer Termzellen,   */
/*                Identifier.                                            */
/*                                                                       */
/*   Aenderungen: <1> 8.2.1991 neu                                       */
/*                <2> 15.2.1991 Stringfunktionen ausgelagert             */
/*                <3> 18.2.1991 Stringdeskriptoren als neuer Datentyp    */
/*                <4> 19.2.1991 IdCell erweitert.                        */
/*                <5> 15.1.1992 Vollstaendig neu: Typdeklarationen in    */
/*                              passenden Modulen, Macros erlauben die   */
/*                              Allokierung von allen Typen, eine Fun-   */
/*                              ktion verwaltet ueber ein Array alle     */
/*                              Groessen.                                */
/*                                                                       */
/*************************************************************************/

#include "pcl_defs.h"


#ifndef _pcl_mem

#define _pcl_mem




/*----------------------------------------------------------------------------*/
/*                    Macros fuer Allokierung - Deallokierung                 */
/*----------------------------------------------------------------------------*/

#define MakeAlloc(typ) typ* Alloc##typ(){  return (typ*) typ_malloc(sizeof(typ));}  

#define MakeFree(typ)  void Free##typ(typ* junk){  typ_free((char*)junk,sizeof(typ));}



/*-----------------------------------------------------------------------*/
/*                        Exportierte Variable                           */
/*-----------------------------------------------------------------------*/


extern char ErrStr[]; /* Speicherplatz fuer sprintf */
extern char NullStr[];

/*----------------------------------------------------------------------------*/
/*                  Exportierte Funktionen                                    */
/*----------------------------------------------------------------------------*/


extern char*     typ_malloc(int size);
extern void      typ_free(char* adr,int size);

extern char*     secure_malloc(int size);

extern
#ifdef __GNUC__
volatile
#endif
void      Error(char* message);


#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


