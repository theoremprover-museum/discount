


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_lemma.c                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Lemma-Funktionen                                       */
/*                                                                       */
/*   Aenderungen: <1> 18.10.1991 neu                                     */
/*                                                                       */
/*************************************************************************/

#include "pcl_lemma.h"




/*----------------------------------------------------------------------------*/
/*                       Globale Variable                                     */
/*----------------------------------------------------------------------------*/


long   s_average_size=S_AVERAGE_SIZE,
       s_max_size=S_MAX_SIZE;
double s_min_fak=S_MIN_FAK,
       s_max_fak=S_MAX_FAK;

long o_min_used=O_MIN_USED;

long i_lemma_weight=I_LEMMA_WEIGHT;

double t_weight_factor=T_WEIGHT_FACTOR;
long   t_offset = T_OFFSET;

long c_init_weight=C_INIT_WEIGHT,
     c_hypo_weight=C_HYPO_WEIGHT,
     c_orient_weight=C_ORIENT_WEIGHT,
     c_cp_weight=C_CP_WEIGHT,
     c_redu_weight=C_REDU_WEIGHT,
     c_inst_weight=C_INST_WEIGHT,
     c_quot_weight=C_QUOT_WEIGHT,
     c_lemma_weight=C_LEMMA_WEIGHT;

long p_max_length=P_MAX_LENGTH;

long u_min_length=U_MIN_LENGTH,
     u_min_used=U_MIN_USED;



/*----------------------------------------------------------------------------*/
/*                 Forward-Deklarationen interner Funktionen                  */
/*----------------------------------------------------------------------------*/


NumListList_p convert_pcl_ids(Step_pList_p inlist,long len);



/*----------------------------------------------------------------------------*/
/*                    Interne Funktionen                                      */
/*----------------------------------------------------------------------------*/



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p convertpcl_ids(Step_pList_p inlist,      */
/*                                                  long len)                 */
/*                    IN     Step_pList_p inlist                              */
/*                    IN     long         len                                 */
/*                                                                            */
/* Beschreibung     : Gibt zu einer Liste mit Pointern auf PCL-Schritte eine  */
/*                    Liste mit Kopien von ihren Identifiern zurueck.         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Speicheroperationen                                     */
/*                                                                            */
/* Aenderungen      : 13.5.1992 neu                                           */
/*                                                                            */
/******************************************************************************/

NumListList_p convert_pcl_ids(Step_pList_p inlist,long len)
{
   NumListList_p handle,entry;
   Step_pList_p  help;

   handle = AllocNumListListCell();
   handle->succ = handle;
   handle->pred = handle;
   help = inlist->succ; 
   while(help != inlist)
   {
      entry = InsertNumList(handle,CopyNumList(help->this->id));
      entry->value += len;
      help = help->succ;
   }
   return handle;
}




/*----------------------------------------------------------------------------*/
/*   Exportierte Funktionen - Konfiguration der Lemma-Kriterien               */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void SetSyntacticParams(long average_size,              */
 /*                              long max_size,double min_fak, double max_fak) */
 /*                                                                            */
 /*                                                                            */
 /* Beschreibung     : Setzt die Parameter fuer die Lemmasuche nach            */
 /*                    syntaktischen Kriterien.                                */
 /*                                                                            */
 /* Globale Variable : s_average_size,s_max_size,s_min_fak,s_max_fak           */
 /*                                                                            */
 /* Seiteneffekte    :-                                                        */
 /*                                                                            */
 /* Aenderungen      : <1> 25.10.92 neu                                        */
 /*                                                                            */
 /******************************************************************************/

 void SetSyntacticParams(long average_size, long max_size, 
                         double min_fak, double max_fak)
 {
    s_average_size = average_size;
    s_max_size = max_size;
    s_min_fak = min_fak;
    s_max_fak = max_fak;
 }

 /******************************************************************************/
 /*                                                                            */
 /* FUNCTION         : void SetOftenParams(long min_used    )                  */
 /*                                                                            */
 /*                                                                            */
 /* Beschreibung     : Setzt die Parameter fuer die Lemmasuche nach            */
 /*                    haeufig benutzten Schritten.                            */
 /*                                                                            */
 /* Globale Variable : o_min_used                                              */
 /*                                                                            */
 /* Seiteneffekte    :-                                                        */
 /*                                                                            */
 /* Aenderungen      : <1> 8.10.92 neu                                         */
 /*                                                                            */
 /******************************************************************************/

 void SetOftenParams(long min_used)
 {
    o_min_used = min_used;
 }


 /******************************************************************************/
 /*                                                                            */
 /* FUNCTION         : void SetImportantParams(long lemma_weight)              */
 /*                                                                            */
 /*                                                                            */
 /* Beschreibung     : Setzt die Parameter fuer die Lemmasuche nach            */
 /*                    'wichtigen' Schritten.                                   */
 /*                                                                            */
 /* Globale Variable : i_lemma_weight                                          */
 /*                                                                            */
 /* Seiteneffekte    :-                                                        */
 /*                                                                            */
 /* Aenderungen      : <1> 25.10.92 neu                                        */
 /*                                                                            */
 /******************************************************************************/

 void SetImportantParams(long lemma_weight)
 {
    i_lemma_weight = lemma_weight;
 }

 /******************************************************************************/
 /*                                                                            */
 /* FUNCTION         : void SetTreeParams(double weight_factor,long offset)    */
 /*                                                                            */
 /*                                                                            */
 /* Beschreibung     : Setzt die Parameter fuer die Lemmasuche nach            */
 /*                    isolierten Teilbaeumen.                                 */
 /*                                                                            */
 /* Globale Variable : double t_weight_factor                                  */
 /*                                                                            */
 /* Seiteneffekte    :-                                                        */
 /*                                                                            */
 /* Aenderungen      : <1> 25.10.92 neu                                        */
 /*                                                                            */
 /******************************************************************************/

 void SetTreeParams(double weight_factor,long offset)
 {
    t_weight_factor = weight_factor;
    t_offset = offset;
 }

 /******************************************************************************/
 /*                                                                            */
 /* FUNCTION         : void SetCompletionParams(                               */
 /*                                     long init_weight,   long hypo_weight,  */
 /*                                     long orient_weight, long cp_weight,    */
 /*                                     long redu_weight,   long inst_weight,  */
 /*                                     long quot_weight,   long lemma_weight) */
 /*                                                                            */
 /*                                                                            */
 /* Beschreibung     : Setzt die Parameter fuer die Lemmasuche nach            */
 /*                    den verwendeten Vervollstaendigungsschritten.           */
 /*                                                                            */
 /* Globale Variable : c_init_weight,c_hypo_weight,c_orient_weight             */
 /*                    c_cp_weight,c_redu_weight,c_inst_weight                 */
 /*                    c_quot_weight,c_lemma_weight                            */
 /*                                                                            */
 /* Seiteneffekte    :-                                                        */
 /*                                                                            */
 /* Aenderungen      : <1> 25.10.92 neu                                        */
 /*                                                                            */
 /******************************************************************************/

 void SetCompletionParams(long init_weight,   long hypo_weight,
                          long orient_weight, long cp_weight,
                          long redu_weight,   long inst_weight,
                          long quot_weight,   long lemma_weight)
 {
    c_init_weight   = init_weight;
    c_hypo_weight   = hypo_weight;
    c_orient_weight = orient_weight;
    c_cp_weight     = cp_weight;
    c_redu_weight   = redu_weight;
    c_inst_weight   = inst_weight;
    c_quot_weight   = quot_weight;
    c_lemma_weight  = lemma_weight;
 }


 /******************************************************************************/
 /*                                                                            */
 /* FUNCTION         : void SetPartitionParams(long max_length)                */
 /*                                                                            */
 /*                                                                            */
 /* Beschreibung     : Setzt die Parameter fuer die Lemmasuche nach            */
 /*                    zur Beweisunterteilung                                  */
 /*                                                                            */
 /* Globale Variable : p_max_length                                            */
 /*                                                                            */
 /* Seiteneffekte    :-                                                        */
 /*                                                                            */
 /* Aenderungen      : <1> 25.10.92 neu                                        */
 /*                                                                            */
 /******************************************************************************/

 void SetPartitionParams(long max_length)
 {
    p_max_length = max_length;
 }

 /******************************************************************************/
 /*                                                                            */
 /* FUNCTION         : void SetNegativeParams(long min_length,long min_used)   */
 /*                                                                            */
 /*                                                                            */
 /* Beschreibung     : Setzt die Parameter fuer die abweisenden Lemma-Kriterien*/
 /*                                                                            */
 /* Globale Variable : u_min_length, u_min_used                                */
 /*                                                                            */
 /* Seiteneffekte    :-                                                        */
 /*                                                                            */
 /* Aenderungen      : <1> 25.10.92 neu                                        */
 /*                                                                            */
 /******************************************************************************/

 void SetNegativeParams(long min_length,long min_used)
 {
    u_min_length = min_length;
    u_min_used = min_used;
 }


/*----------------------------------------------------------------------------*/
/*    Exportierte Funktionen: Lemma-Suche                                     */
/*----------------------------------------------------------------------------*/





/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long JustWeight(Just_p just)                            */
/*                                                                            */
/* Beschreibung     : Berechnet Gewicht der Rechtfertigung                    */
/*                                                                            */
/* Globale Variable : InitW,HypoW,OrieW,CpW,RedW,InstW,QuotW                  */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 18.10.1991 NEU                                      */
/*                                                                            */
/******************************************************************************/


long JustWeight(Just_p just)
{
   long weight = 0;

   switch(just->operation)
   {
      case initial:    weight = c_init_weight;
                       break;
      case hypothesis: weight = c_hypo_weight;
                       break;
      case orientx:
      case orientu:
                       weight = c_orient_weight + JustWeight(just->arg1.rarg);
                       break;
      case cp:
                       weight = c_cp_weight+JustWeight(just->arg1.rarg)+
                                            JustWeight(just->arg2.rarg);
                       break;
      case tes_red:
                       weight = c_redu_weight+JustWeight(just->arg1.rarg)+
                                              JustWeight(just->arg2.rarg);
                       break;
      case instance:
                       weight = c_inst_weight + JustWeight(just->arg1.rarg);
                       break;
      case quotation:  if((just->arg1.Targ.parg)->type == tes_lemma)
                       {
                          weight = c_quot_weight;
                       }
                       else
                       {
                          weight = c_quot_weight + (just->arg1.Targ.parg)->op_count;;
                       }
                       break;
      default:         Error("Illegal Operation in PCL-Justification");
                       break;
   }
   return weight;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long ChainLen(Just_p just)                              */
/*                                                                            */
/* Beschreibung     : Berechnet Laenge der Beweiskette fuer das Faktum        */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 11.5.1991 NEU                                       */
/*                                                                            */
/******************************************************************************/


long ChainLen(Just_p just)
{
   long len = 0;

   switch(just->operation)
   {
      case initial:    
      case hypothesis: len = 1;
                       break;
      case orientx:
      case orientu:
                       len = ChainLen(just->arg1.rarg);
                       break;
      case cp:
      case tes_red:
                       len = ChainLen(just->arg1.rarg)+ChainLen(just->arg2.rarg);
                       break;
      case instance:
                       len = ChainLen(just->arg1.rarg);
                       break;
      case quotation:
                       if((just->arg1.Targ.parg)->type == tes_lemma)
                       {
                          len = 1;
                       }
                       else
                       {
                          len = (just->arg1.Targ.parg)->chain_len;
                       }
                       break;
      default:         Error("Illegal Operation in PCL-Justification");
                       break;
   }
   return len;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p GetExits(Just_p just,Step_p step)         */
/*                    IN     Just_p just                                      */
/*                    IN     Step_p step                                      */
/*                                                                            */
/* Beschreibung     : Hole die zu den Eltern gehoerenden Ausgaenge -          */
/*                    einschliesslich der ueber die Eltern fuehrenden         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 11.5.1992 Neu                                       */
/*                                                                            */
/******************************************************************************/

NumListList_p GetExits(Just_p just,Step_p step)
{
   NumListList_p handle;

   switch(just->operation)
   {
      case initial:
      case hypothesis:
                       handle = AllocNumListListCell();
                       handle->pred = handle->succ = handle;
                       break; 
      case orientx:
      case orientu:    handle = GetExits(just->arg1.rarg,step);
                       break;
      case cp:
      case tes_red:    handle = MergeNumListLists(GetExits(just->arg1.rarg,step),
                                                  GetExits(just->arg2.rarg,step));
                       break;
      case instance:   handle = MergeNumListLists(GetExits(just->arg1.rarg,step),
                                                  GetExits(just->arg2.rarg,step));
                       break;
      case quotation:
                       if(just->arg1.Targ.parg->type == tes_lemma)
                       {
                          handle =  AllocNumListListCell();
                          handle->pred = handle->succ = handle;
                       }
                       else
                       {    
                          handle = MergeNumListLists(
                                     convert_pcl_ids(just->arg1.Targ.parg->children,
                                                     just->arg1.Targ.parg->chain_len),
                                     CopyNumListList(just->arg1.Targ.parg->tree_exits));
                       }
                       break;
      default:         handle = 0;
                       Error("Illegal Operation in PCL-Justification");
                       break;
   }
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p GetUsed(Just_p just)                      */
/*                                                                            */
/* Beschreibung     : Hole die zu einem Beweisbaum benutzten Schritte         */
/*                    Der Schritt, fuer dessen just GetUsed aufgerufen wurde, */
/*                    ist NICHT dabei, er muss spaeter eingefuegt werden.     */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Speicher                                                */
/*                                                                            */
/* Aenderungen      : <1> 13.5.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

NumListList_p GetUsed(Just_p just)
{
   NumListList_p handle;

   switch(just->operation)
   {
      case initial:
      case hypothesis: handle = AllocNumListListCell();
                       handle->pred = handle;
                       handle->succ = handle;
                       break;
      case orientx:
      case orientu:    handle = GetUsed(just->arg1.rarg);
                       break;
      case cp:
      case tes_red:    handle = MergeNumListLists(GetUsed(just->arg1.rarg),
                                                  GetUsed(just->arg2.rarg));
                       break;
      case instance:   handle = MergeNumListLists(GetUsed(just->arg1.rarg),
                                                  GetUsed(just->arg2.rarg));
                       break;
      case quotation:  
                       if(just->arg1.Targ.parg->type == tes_lemma)
                       {
                          handle = AllocNumListListCell();
                          handle->pred = handle->succ = handle;
                       }
                       else
                       {
                          handle = CopyNumListList(just->arg1.Targ.parg->used_steps);
                       }
                       break;
      default:         handle = 0;
                       Error("Illegal Operation in PCL-Justification");
                       break;
   }
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long ValueExits(NumListList_p exits)                    */
/*                    IN     NumListList_p exits                              */
/*                                                                            */
/* Beschreibung     : Summiert die in exits stehenden value-Felder auf        */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 14.5.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

long ValueExits(NumListList_p exits)
{
   NumListList_p handle;
   long          res = 0;

   handle = exits->succ;
    
   while(handle!=exits)
   {
      res+=handle->value;
      handle = handle->succ;
   }
   return res;
} 


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : IsNoLemma(Step_p step,String_p comment)                 */
/*                                                                            */
/* Beschreibung     : Entscheidet, ob ein Schritt aus technischnen Gr"unden   */
/*                    oder aufgrund der abweisenden Lemma-Kriterien kein      */
/*                    Lemma sein kann.                                        */
/*                                                                            */
/* Globale Variable : u_min_used, u_min_length                                */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 2.10.92 neu                                         */
/*                                                                            */
/******************************************************************************/

BOOL IsNoLemma(Step_p step,String_p comment)
{
   if((step->type==tes_goal) || (step->type==tes_intermedgoal))
   {
      AppendString(comment,"# Kein Lemma: Schritt ist (Teil-) Ziel.\n");
      return TRUE;
   }
   if((step->type==crit_goal) || (step->type==crit_intermedgoal))
   {
      AppendString(comment,"# Kein Lemma: Schritt ist kritisches (Teil-) Ziel.\n");
      return TRUE;
   }
   if(step->type==tes_final)
   {
      AppendString(comment,"# Kein Lemma: Letzter Schritt im Beweis.\n");
      return TRUE;
   }
   if(step->just->operation == initial)
   {
      AppendString(comment,"# Kein Lemma: Axiom.\n");
      return TRUE;
   }
   if(step->chain_len < u_min_length) /* Even a perfect Lemma should not be trivial */
   {
      sprintf(ErrStr,"# Kein Lemma: Beweiskette zu trivial.\n"
                     "# step->chain_len(%ld)<u_min_length(%ld)\n",
                     step->chain_len,u_min_length);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   if(step->children_no<u_min_used)
   {
      sprintf(ErrStr,"# Kein Lemma: Zu selten verwendet.\n"
                     "# step->children_no(%ld)<u_min_used(%ld)\n",
                     step->children_no,u_min_used);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   return FALSE;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : IsSyntaxLemma(Step_p step,String_p comment)             */
/*                                                                            */
/* Beschreibung     : Entscheidet, ob ein Schritt aus syntaktischen Gr"unden  */
/*                    ein Lemma ist.                                          */
/*                                                                            */
/* Globale Variable : s_average_size,s_max_size,s_min_fak,s_max_fak           */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 6.10.92 neu                                         */
/*                                                                            */
/******************************************************************************/

BOOL IsSyntaxLemma(Step_p step,String_p comment)
{
   long   l_len, r_len;
   double fak;
   
   l_len = NumberOfFuncs(step->pair->lside);
   r_len = NumberOfFuncs(step->pair->rside);
   
   fak = max(l_len,r_len)/min(l_len,r_len);

   if(((r_len + l_len)/2)<=s_average_size)
   {
      sprintf(ErrStr,"# Syntaktisches Lemma: Mittlere Termgroesse ist klein.\n"
                     "# ((r_len(%ld) + l_len(%ld))/2)<=s_average_size(%ld)",
                     r_len,l_len,s_average_size);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   else if(max(r_len,l_len)<=s_max_size)
   {
      sprintf(ErrStr,"# Syntaktisches Lemma: Maximale Termgroesse ist klein.\n"
                     "# max(r_len(%ld),l_len(%ld))<=s_max_size(%ld)",
                     r_len,l_len,s_max_size);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   else if((s_min_fak!=0.0)&&(fak>=s_min_fak))
   {
      sprintf(ErrStr,"# Syntaktisches Lemma: Termgroessen sehr unterschiedlich.\n"
                     "# max(l_len,r_len)/min(l_len,r_len)(%fd)>=s_min_fak(%fd)",
                     fak,s_min_fak);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   else if((s_max_fak!=0.0)&&(fak<=s_max_fak))
   {
      sprintf(ErrStr,"# Syntaktisches Lemma: Termgroessen aehnlich.\n"
                     "# max(l_len,r_len)/min(l_len,r_len)(%fd)<=s_max_fak(%fd)",
                     fak,s_min_fak);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   return FALSE;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : IsOftenLemma(Step_p step,String_p comment)              */
/*                                                                            */
/* Beschreibung     : Entscheidet, ob ein haeufig benutzter Schritt           */
/*                    ein Lemma ist.                                          */
/*                                                                            */
/* Globale Variable : o_min_used                                              */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 6.10.92 neu                                         */
/*                                                                            */
/******************************************************************************/

BOOL IsOftenLemma(Step_p step,String_p comment)
{
   if(step->children_no >= o_min_used)
   {
      sprintf(ErrStr,"# Oft benutztes Lemma:\n"
                     "# step->children_no(%ld) >= o_min_used(%ld)\n",
                     step->children_no,o_min_used);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   return FALSE;
}

   
/******************************************************************************/
/*                                                                            */
/* FUNCTION         : IsImportantLemma(Step_p step,String_p comment)          */
/*                                                                            */
/* Beschreibung     : Entscheidet, ob ein 'wichtiger' Schritt                 */
/*                    ein Lemma ist.                                          */
/*                                                                            */
/* Globale Variable : i_lemma_weight                                          */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 6.10.92 neu                                         */
/*                                                                            */
/******************************************************************************/

BOOL IsImportantLemma(Step_p step,String_p comment)
{
   long used_weight;
 
   used_weight = step->children_no * step->chain_len;

   if(used_weight > i_lemma_weight)
   {
      sprintf(ErrStr,"# Wichtiges Lemma:\n"
                     "# used_weight(%ld) > i_lemma_weight(%ld)\n",
                     used_weight,i_lemma_weight);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   return FALSE;
}

   
/******************************************************************************/
/*                                                                            */
/* FUNCTION         : IsTreeLemma(Step_p step,String_p comment)               */
/*                                                                            */
/* Beschreibung     : Entscheidet, Schritt mit relativ isoliertem Beweisbaum  */
/*                    ein Lemma ist.                                          */
/*                                                                            */
/* Globale Variable : t_weight_factor,t_offset                               */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 6.10.92 neu                                         */
/*                                                                            */
/******************************************************************************/

BOOL IsTreeLemma(Step_p step,String_p comment)
{
   long used_weight,exit_weight;
 
   used_weight = step->children_no * step->chain_len;
   exit_weight = ValueExits(step->tree_exits);
      
   if(used_weight > exit_weight*t_weight_factor+t_offset)
   {
      sprintf(ErrStr,"# Baum Lemma: Isolierter Teilbaum.\n"
              "# used_weight(%ld) > exit_weight(%ld)*t_weight_factor(%f)+t_offset(%ld)\n",
              used_weight,exit_weight,t_weight_factor,t_offset);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   return FALSE;
}

   
/******************************************************************************/
/*                                                                            */
/* FUNCTION         : IsCompletionLemma(Step_p step,String_p comment)         */
/*                                                                            */
/* Beschreibung     : Entscheidet an Hand der verwendeten Vervollstaendigungs-*/
/*                    Schritte, ob der Schritt ein Lemma ist.                 */
/*                                                                            */
/* Globale Variable : c_lemma_weight                                          */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 6.10.92 neu                                         */
/*                                                                            */
/******************************************************************************/

BOOL IsCompletionLemma(Step_p step,String_p comment)
{
   if(step->op_count>=c_lemma_weight)
   {
      sprintf(ErrStr,"# Vervollstaendigungs-Lemma:\n"
              "# op_count(%ld)>=c_lemma_weight(%ld)\n",
              step->op_count,c_lemma_weight);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   return FALSE;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : IsPartitionLemma(Step_p step,String_p comment)          */
/*                                                                            */
/* Beschreibung     : Entscheidet an Hand der Laenge der Beweiskette, ob ein  */
/*                    Schritt ein Lemma ist.                                  */
/*                                                                            */
/* Globale Variable : p_max_length                                            */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 6.10.92 neu                                         */
/*                                                                            */
/******************************************************************************/

BOOL IsPartitionLemma(Step_p step,String_p comment)
{   
  if(step->chain_len > p_max_length) /* We don't want too long partial proofs */
   {
      sprintf(ErrStr,"# Lemma zur Beweisunterteilung:\n"
                     "# step->chain_len(%ld)>p_max_length (%ld)\n",
                     step->chain_len,p_max_length);
      AppendString(comment,ErrStr);
      return TRUE;
   }
   return FALSE;
}
      

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL IsLemma(Step_p step,char* criteria, long criterium)*/
/*                    IN     Step_p step                                      */
/*                    IN     char* criteria                                   */
/*                    IN     long criterium                                   */
/*                                                                            */
/* Beschreibung     : Entscheidet an Hand geeigneter Heuristiken, ob der      */
/*                    PCL-Schritt ein Lemma werden soll, f"uhrt entsprechende */
/*                    Massnahmen durch.                                       */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Step-Informationen koennen geaendert werden.            */
/*                                                                            */
/* Aenderungen      : <1> 18.10.1991 neu                                      */
/*                                                                            */
/******************************************************************************/

BOOL IsLemma(Step_p step,char* criteria, long criterium)
{
   long f;
   BOOL result = FALSE;
   static StringCell comment={NULL,0,0}; 

   AppendString(&comment,step->comment);
   FREE(step->comment);

   if(IsNoLemma(step,&comment))
   {
      result = FALSE;
   }
   else if(criterium==-1)
   {
      for(f=0;(criteria[f]!='\0') && !result;f++)
      {
         switch (criteria[f])
         {
         case 's': result = IsSyntaxLemma(step,&comment);
                   break;
         case 'o': result = IsOftenLemma(step,&comment);
                   break;
         case 'i': result = IsImportantLemma(step,&comment);
                   break;
         case 't': result = IsTreeLemma(step,&comment);
                   break;
         case 'c': result = IsCompletionLemma(step,&comment);
                   break;
         case 'p': result = IsPartitionLemma(step,&comment);
                   break;
         default : Error("Unknown lemma criterium in IsLemma() (pcl_lemma.c)...\n");
                   break;           
         }
      }
   }
   else
   {
      switch (criteria[criterium])
      {
      case 's': result = IsSyntaxLemma(step,&comment);
                break;
      case 'o': result = IsOftenLemma(step,&comment);
                break;
      case 'i': result = IsImportantLemma(step,&comment);
                break;
      case 't': result = IsTreeLemma(step,&comment);
                break;
      case 'c': result = IsCompletionLemma(step,&comment);
                break;
      case 'p': result = IsPartitionLemma(step,&comment);
                break;
      default : Error("Unknown lemma criterium in IsLemma() (pcl_lemma.c)...\n");
                break;
      }
   }
   if(result)
   {
      step->type = tes_lemma;
/*      FreeNumListList(step->used_steps);
      FreeNumListList(step->tree_exits);
      step->used_steps = AllocNumListListCell();
      step->used_steps->pred = step->used_steps;
      step->used_steps->succ = step->used_steps;
      InsertNumList(step->used_steps,CopyNumList(step->id));
      step->tree_exits = AllocNumListListCell();
      step->tree_exits->pred = step->tree_exits;
      step->tree_exits->succ = step->tree_exits; 
      step->tree_exits = convert_pcl_ids(step->children,1); */
   }
   if(printcomment)
   {
      step->comment = GetCopyOfString(&comment);
   }
   else 
   {
       step->comment = secure_strdup(NullStr);
    }
   ResetString(&comment);
   return result;
}        


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void ForgetLemmas(Step_p anchor)                        */
/*                    IN    Step_p anchor                                     */
/*                                                                            */
/* Beschreibung     : Loescht in dem bei anchor verankerten Schritt alle      */
/*                    Lemmata.                                                */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 11.9.92 neu                                         */
/*                                                                            */
/******************************************************************************/

void ForgetLemmas(Step_p anchor)
{
   Step_p handle;

   for(handle = anchor->succ; handle!=anchor; handle = handle->succ)
   {
      if(handle->type == tes_lemma)
      { 
         if(handle->pair->type == rule)
         {
            handle->type = tes_rule;
         }
         else
         {
            handle->type = tes_eqn;
         }
      }
   }
}       


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FindLemmas(Step_p anchor,char* criteria,           */
/*                                    BOOL iterate)                           */
/*                                                                            */
/* Beschreibung     : Durchsucht Beweis und markiert Lemmata, nummeriert      */
/*                    Axiome und Lemmata                                      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Beweis wird entsprechend strukturiert.                  */
/*                                                                            */
/* Aenderungen      : <1> 18.10.1991 neu                                      */
/*                    <2>  7. 2.1992 Nummerierung von Axiomen und Lemmata     */
/*                    <3> 14. 5.1992 Neuer Alg...noch laeufts nicht           */
/*                    <4> 25. 9.1992 Noch neuer..aber nicht fertig            */
/*                                                                            */
/******************************************************************************/

void FindLemmas(Step_p anchor,char* criteria,BOOL iterate)
{
   Step_p handle;
   NumListList_p exits;
   long f;

   if(iterate)
   {
      for(f=0;criteria[f]!='\0';f++)
      {
         for(handle = anchor->succ; handle!=anchor; handle = handle->succ)
         {
	    if(handle->used)
	    {
	       handle->op_count = JustWeight(handle->just);
	       handle->chain_len = ChainLen(handle->just);
	       handle->used_steps = GetUsed(handle->just);
	       InsertNumList(handle->used_steps,CopyNumList(handle->id));
	       exits = GetExits(handle->just,handle);
	       handle->tree_exits = RemoveElems(exits,
						handle->used_steps); 
	       /*  fprintf(out,"\n");
		   PrintIdList(handle->id);
		   fprintf(out," used_steps:\n=========\n");
		   PrintNumListList(handle->used_steps);
		   fprintf(out,"exits:\n");
		   PrintNumListList(exits);
		   fprintf(out,"tree_exits:\n");
		   PrintNumListList(handle->tree_exits);  */
	       
	       IsLemma(handle,criteria,f);
	    }
         }
      }
   }
   else
   {
      for(handle = anchor->succ; handle!=anchor; handle = handle->succ)
      {
	 if(handle->used)
	 {
	    handle->op_count = JustWeight(handle->just);
	    handle->chain_len = ChainLen(handle->just);
	    handle->used_steps = GetUsed(handle->just);
	    InsertNumList(handle->used_steps,CopyNumList(handle->id));
	    exits = GetExits(handle->just,handle);
	    handle->tree_exits = RemoveElems(exits,handle->used_steps); 
	    
            /* fprintf(out,"\n");
	       PrintIdList(handle->id);
	       fprintf(out," used_steps:\n=========\n");
	       PrintNumListList(handle->used_steps);
	       fprintf(out,"exits:\n");
	       PrintNumListList(exits);
	       fprintf(out,"tree_exits:\n");
	       PrintNumListList(handle->tree_exits);  */
	    
	    IsLemma(handle,criteria,-1);
	 }
      }
   }
}



/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


