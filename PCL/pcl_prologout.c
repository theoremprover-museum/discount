/*-------------------------------------------------------------------------

File       : pcl_prologout.c

Autor      : Stephan Schulz

Inhalt     : Beweisausgabe in Prolog-Darstellung
             (f"ur Gruppe Dahn in Berlin)

Aenderungen: <1> 20.4.1992 neu

-------------------------------------------------------------------------*/


#include "pcl_prologout.h"


/*----------------------------------------------------------------------------*/
/*                    Forward-Deklarationen interner Funktionen               */
/*----------------------------------------------------------------------------*/

void P_print_eq_just(EqChain_p prt);


/*----------------------------------------------------------------------------*/
/*                       Interne Funktionen                                   */
/*----------------------------------------------------------------------------*/

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void P_print_eq_just(EqChain_p prt)                     */
/*                    IN     EqChain_p prt                                    */ 
/*                                                                            */
/* Beschreibung     : Gibt Begruendung fuer einen einzelnen Gleichheitschritt */ 
/*                    aus.                                                    */
/*                                                                            */
/* Globale Variable : out (aus pcl_printer)                                   */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 20.4.1992 Aus L_Print_eq_just(EqChain_p prt)        */
/*                                                                            */
/******************************************************************************/

void P_print_eq_just(EqChain_p prt)
{
   if(prt->tp->type == tes_lemma)
   {
      fprintf(out, "lemma(%ld, ",prt->tp->ax_lem_no);
   }
   else if(prt->tp->just->operation == initial)
   {
      fprintf(out,"axiom(%ld, ",prt->tp->ax_lem_no);
   }
   else if(prt->tp->type == tes_final)
   {
      fprintf(out,"theorem(%ld, ",prt->tp->ax_lem_no);
   }
   else
   {
      Error("Non-terminal equation used in P_print_eq_just (pcl_prologout.c)");  
   }
   if(prt->swapped)
   {
      fprintf(out,"rl)");
   }
   else
   {
      fprintf(out,"lr)");
   }
}



/*-----------------------------------------------------------------------*/
/*                    ExportierteFunktionen                              */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void P_PrintEqChain(EqChain_p  prt, ExStep type, 
                                                       long ax_lem_no) 

Beschreibung     : Formatierte Ausgabe einer Gleichheitskette zu einem
                   Theorem oder Lemma fuer Prolog. Achtung: Im Gegensatz
		   zu den anderen Routinen gibt diese die Kette in Form
		   von Einzelschritten aus. Deshalb brucht sie auch
		   Informationen ueber den speziellen Status der Gleichung
		   und ueber ihre Nummer als Lemma oder Axiom.

Globale Variable : out (aus pcl_printer)                                   

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 20.4.1992 aus L_PrintEqChain()

-------------------------------------------------------------------------*/

void P_PrintEqChain(EqChain_p prt, ExStep type, long ax_lem_no)
{
   EqTerm_p termhandle;
   EqChain_p chainhandle;

   termhandle = prt->right;
   chainhandle = termhandle->right;

   if(chainhandle == prt)
   {
      putc('\n',out);
      fprintf(stderr,"Warning: Isolated term in P_PrintEqChain (pcl_eqchains.c)...\n");
      return;
   }
   else
   {
      while(chainhandle != prt)
      {
         termhandle = chainhandle->right;

	 fprintf(out,"   proof(");
	 switch(type)
	 {
	 case lemma: 
	    fprintf(out,"lemma(%ld),",ax_lem_no);
	    break;
	 case theorem:
	    fprintf(out,"theorem(%ld),",ax_lem_no) ;
	    break;
	 case axiom:
	    fprintf(out,"axiom(%ld),",ax_lem_no) ;
	    break;
	 default:
	    Error("Can't proof non-special fact in Prolog-notation (pcl_prologout.c)");
	    break;
	 }
	 
	 PrintTerm(termhandle->term);
	 fprintf(out,",");
	 P_print_eq_just(chainhandle);
	 fprintf(out,").\n");
         chainhandle = termhandle->right;
      }
   }
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


