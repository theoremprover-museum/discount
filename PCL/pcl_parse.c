/*************************************************************************/
/*                                                                       */
/*   File:        pcl_parse.c                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Parser fuer PCL                                        */
/*                                                                       */
/*   Aenderungen: <1> 08.4.1991 Uebernahme von parse.c                   */
/*                                                                       */
/*************************************************************************/


#include "pcl_parse.h"


/*----------------------------------------------------------------------------*/
/*                 Globale Variable                                           */
/*----------------------------------------------------------------------------*/


StringCell ErrCell = {NULL,0,0};

StringCell  aktcomment = {NULL,0,0};



/*----------------------------------------------------------------------------*/
/*           Forward-Deklarationen interner Funktionen                        */
/*----------------------------------------------------------------------------*/

NumList_p parse_numlist();

Term_p    parse_arglist();
Place_p   parse_place();

Pair_p    parse_termpair();

Just_p    parse_just();

Just_p    parse_initial();
Just_p    parse_hypothesis();
Just_p    parse_orient();
Just_p    parse_cp();
Just_p    parse_tes_red();
Just_p    parse_instance();
Just_p    parse_quotation();
void      parse_annotations(Step_p handle);


/*----------------------------------------------------------------------------*/
/*                 Exportierte Funktionen                                     */
/*----------------------------------------------------------------------------*/

MakeAlloc(StepCell);
MakeAlloc(JustCell);
MakeFree(StepCell);
MakeFree(JustCell);
MakeAlloc(Step_pListCell);
MakeFree(Step_pListCell);

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeStep(Step_p junk)                              */
/*                    IN     Step_p junk                                      */
/*                                                                            */
/* Beschreibung     : Gibt den vom PCL-Schritt belegten Speicherplatz frei.   */
/*                                                                            */
/* Globale Variable : FreeStepList                                            */
/*                                                                            */
/* Seiteneffekte    : Durch FreeNumList, FreePair, FreeJust, FreeStepCell.    */
/*                                                                            */
/* Aenderungen      : <1> 05.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeStep(Step_p junk)
{
   if(junk)
   {
      FreeNumList(junk->id);
      FreePair(junk->pair);
      FreeJust(junk->just);
      FREE(junk->comment);
      FreeStep_pList(junk->children);
   }
   else
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreeStep (pcl_parse.c)...\n");
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeJust(Just_p junk)                              */
/*                    IN     Just_p junk                                      */
/*                                                                            */
/* Beschreibung     : Gibt den von der Rechtfertigung belegten Speicherplatz  */
/*                    frei.                                                   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch FreeJustCell, FreePlace                           */
/*                                                                            */
/* Aenderungen      : <1> 05.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeJust(Just_p junk)
{
   if(junk)
   {
      switch(junk->operation)
      {
         case initial:
         case hypothesis:
	                  break;
         case quotation:  
                          FreeNumList(junk->arg1.Targ.sarg);
                          break;
         case orientx:
         case orientu:    
                          FreeJust(junk->arg1.rarg);
                          break;
         case cp:
         case tes_red:
                          FreePlace(junk->place1);
                          FreeJust(junk->arg1.rarg);
                          FreePlace(junk->place2);
                          FreeJust(junk->arg2.rarg);
                          break;
         case instance:
                          FreeJust(junk->arg1.rarg);
                          FreeJust(junk->arg2.rarg);
                          break;
         default:         
                          printf("Warning: Illegal Cell returned to FreeJust...\n");
                          break;
      }
      FreeJustCell(junk);
   }
   else
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreeJust (pcl_parse.c)...\n");
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeStep_pList(Step_pList_p junk)                  */
/*                    IN     Step_pList_p junk                                */
/*                                                                            */
/* Beschreibung     : Gibt den von einer Liste von PCL-Schritt-Pointern       */
/*                    belegten Platz frei.                                    */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch FreeStep_pListCell()                              */
/*                                                                            */
/* Aenderungen      : <1> 11.5.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeStep_pList(Step_pList_p junk)
{
   Step_pList_p help;

   if(junk)
   {
      while(junk->succ != junk)
      {
         help = junk->succ;
         junk->succ = help->succ;
         FreeStep_pListCell(help);
      }
      FreeStep_pListCell(junk);
   }
   else
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreeStep_pList (pcl_parse.c)...\n");
   }
}
       


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintPlace(Place_p prt)                            */
/*                    IN     Place_p prt                                      */
/*                                                                            */
/* Beschreibung     : Gibt den Place, auf den prt zeigt, aus.                 */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintPlace(Place_p prt)
{
   NumList_p help;

   fprintf(out,"%c",prt->side);
   for(help = prt->rest; help; help = help->rest)
   {
      fprintf(out,".%ld",help->value);
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintJust(Just_p prt)                              */
/*                    IN     Just_p prt                                       */
/*                                                                            */
/* Beschreibung     : Gibt die Herleitung, auf die prt zeigt, aus.            */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 16.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintJust(Just_p prt)
{
   switch(prt->operation)
   {
      case initial:    fprintf(out,"initial");
                       break;
      case hypothesis: fprintf(out,"hypothesis");
                       break;
      case orientu:    fprintf(out,"orient(");
                       PrintJust(prt->arg1.rarg);
                       fprintf(out,",u)");
                       break;
      case orientx:    fprintf(out,"orient(");
                       PrintJust(prt->arg1.rarg);
                       fprintf(out,",x)");
                       break;
       case cp:         fprintf(out,"cp(");
                       PrintJust(prt->arg1.rarg);
                       fprintf(out,",");
                       PrintPlace(prt->place1);
                       fprintf(out,",");
                       PrintJust(prt->arg2.rarg);
                       fprintf(out,",");
                       PrintPlace(prt->place2);
                       fprintf(out,")");
                       break;
      case tes_red:    fprintf(out,"tes-red(");
                       PrintJust(prt->arg1.rarg);
                       fprintf(out,",");
                       PrintPlace(prt->place1);
                       fprintf(out,",");
                       PrintJust(prt->arg2.rarg);
                       fprintf(out,",");
                       PrintPlace(prt->place2);
                       fprintf(out,")");
                       break;
      case instance:   fprintf(out,"instance(");
                       PrintJust(prt->arg1.rarg);
                       fprintf(out,",");
                       PrintJust(prt->arg2.rarg);
                       fprintf(out,")");
                       break;
       case quotation:  PrintNumList(prt->arg1.Targ.sarg);
                       break;
      default:         Error("Unknown operation in PrintJust");
                       break;
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : void PrintStepPure(Step_p prt)
                   IN    Step_p prt       

Beschreibung     : Gibt einen PCL-Schritt aus, ohne Newline und ohne
                   Kommentare 

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 25.4.1994 aus PrintStep

-------------------------------------------------------------------------*/


void PrintStepPure(Step_p prt)
{
   PrintIdList(prt->id);
   fprintf(out," : ");

   switch(prt->type)
   {
      case tes_eqn:           fprintf(out,"tes-eqn");
                              break;
      case tes_rule:          fprintf(out,"tes-rule");
                              break;
      case tes_lemma:         fprintf(out,"tes-lemma");
                              break;
      case tes_goal:          fprintf(out,"tes-goal");
                              break;
      case tes_final:         fprintf(out,"tes-final");
                              break;
      case tes_intermed:      fprintf(out,"tes-intermed");
                              break;
      case tes_intermedgoal:  fprintf(out,"tes-intermedgoal");
                              break;
      case crit_intermedgoal: fprintf(out,"crit-intermedgoal");
                              break;
      case crit_goal:         fprintf(out,"crit-goal");
                              break;
      default:                Error("Unknown StepType in PrintStep");
                              break;
   }
   fprintf(out," : ");
   PrintTermPair(prt->pair);
   fprintf(out," : ");
   PrintJust(prt->just);
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintStep(Step_p prt)                              */
/*                    IN    Step_p prt                                        */
/*                                                                            */
/* Beschreibung     : Gibt einen PCL-Schritt aus.                             */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 16.4.1991 neu                                       */ 
/*                                                                            */
/******************************************************************************/

void PrintStep(Step_p prt)
{
   PrintStepPure(prt);
   fprintf(out,"\n");
   PrintComment(prt->comment);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void NextRealToken()                                    */
/*                                                                            */
/* Beschreibung     : Liest solange Token, bis akttoken.token != comment.     */
/*                    Kommentare werden an aktcomment angehaengt.             */
/*                                                                            */
/* Globale Variable : akttoken, aktcomment                                    */
/*                                                                            */
/* Seiteneffekte    : aktkomment wird geaendert, durch Aufruf von NextToken() */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void NextRealToken()
{
   NextToken();
   while(akttoken.token == comment)
   {
      AppendString(&aktcomment,akttoken.literal);
      NextToken();
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Step_p ParseStep()                                      */
/*                                                                            */
/* Beschreibung     : Parst einen PCL-Beweisschritt, gibt Pointer auf Zelle   */
/*                    zurueck.                                                */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Step_p ParseStep()
{
   Step_p handle;

   handle = AllocStepCell();

   handle ->children = AllocStep_pListCell();
   handle->children->pred = handle->children;
   handle->children->succ = handle->children;
   handle->children_no = 0;
   handle->op_count = 0;
   handle->eqproof = NULL;
   handle->ax_lem_no = 0;


   handle->id = parse_numlist();

   AcceptTok(colon,":");

   if(test_id(ident,"tes"))
   {
      NextRealToken();
      AcceptTok(hyphen,"-");

      if(test_id(ident,"eqn"))
      {
	 handle->type = tes_eqn;
      }
      else if(test_id(ident,"rule"))
      {
	 handle->type = tes_rule;
      }
      else if(test_id(ident,"lemma"))
      {
	 handle->type = tes_lemma;
      }
      else if(test_id(ident,"goal"))
      {
	 handle->type = tes_goal;
      }
      else if(test_id(ident,"final"))
      {
	 handle->type = tes_final;
      }
      else if(test_id(ident,"intermed"))
      {
	 handle->type = tes_intermed;
      }
      else if(test_id(ident,"intermedgoal"))
      {
	 handle->type = tes_intermedgoal;
      }
      else
      {
	 RdErr("tes-type expected");
      }
   }
   else
   {
      AcceptId(ident,"crit");
      NextRealToken();
      AcceptTok(hyphen,"-");

      if(test_id(ident,"goal"))
      {
	 handle->type = crit_goal;
      }
      else if(test_id(ident,"intermedgoal"))
      {
	 handle->type = crit_intermedgoal;
      }
      else
      {
	 RdErr("crit-type expected");
      }
   } 


   NextRealToken();

   AcceptTok(colon,":");

   handle->pair = parse_termpair();

   AcceptTok(colon,":");

   handle->just = parse_just();

   if(test(colon))
   {
      parse_annotations(handle);
   }
   else
   {
      handle->rw_distance = handle->cp_distance = -1;
      handle->cp_cost = 0;
      handle->used = FALSE;
   }

   handle->comment = GetCopyOfString(&aktcomment);
   ResetString(&aktcomment);

   return handle;
}

/*----------------------------------------------------------------------------*/
/*                 Interne Funktionen                                         */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumList_p parse_numlist()                               */
/*                                                                            */
/* Beschreibung     : Parst eine von Punkten getrennte Liste von Zahlen       */
/*                    (etwa fuer Step-Identifier). Rueckgabe ist Pointer      */
/*                    auf den Listenkopf.                                     */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

NumList_p parse_numlist()
{
   NumList_p handle;
   NumList_p *help;

   check(number,"Number");

   handle = AllocNumListCell();

   handle->value = akttoken.numval;
   help = &(handle->rest);

   NextRealToken();
   while(test(fullstop))
   {
       NextRealToken();

      check(number,"Number");

      *help = AllocNumListCell();
      (*help)->value = akttoken.numval;
      help = &((*help)->rest);

      NextRealToken();
   }
   *help = NULL;
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p parse_term()                                     */
/*                                                                            */
/* Beschreibung     : Parst ein Term, gibt Zeiger auf Ergebnis zurueck.       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991 neu                                       */
/*                    <2> 08.4.1991 Aufbau der Termstruktur (statt Syntax-    */
/*                                  Check)                                    */
/*                                                                            */
/******************************************************************************/

Term_p parse_term()
{
   Term_p handle;

   handle = AllocTermCell();

   check(Identifier,"Identifier");
   handle->id = secure_strdup(akttoken.literal);
#ifdef LEARN_VERSION
   if(test(idnum))
   {
      handle->norm_id = akttoken.numval;
   }
#endif
   NextRealToken();

   if(test(openbracket))
   {
     handle->args =  parse_arglist();
     handle->isvar = FALSE;
   }
   else
   {
      handle->args = NULL;
      handle->isvar = TRUE;
   }
#ifdef LEARN_VERSION
   handle->arity = GetTermCellArity(handle);
#endif
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p parse_arglist()                                  */
/*                                                                            */
/* Beschreibung     : Parst Argumentenliste einer Funktion, Ergebnis ist      */
/*                    Zeiger auf Argumentliste.                               */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991                                           */
/*                    <2> 08.4.1991 Aufbau der Termstruktur (statt Syntax-    */
/*                                  Check)                                    */
/*                                                                            */
/******************************************************************************/

Term_p parse_arglist()
{
   Term_p handle = NULL;
   Term_p *help = NULL;

   AcceptTok(openbracket,"(");

   help = &handle; 

   while(!test(closebracket))
   {
      *help = parse_term();

      help = &((*help)->chain);
      if(test(comma))
      {
         AcceptTok(comma,"','");
      }
      else if(!test(closebracket))
      {
         RdErr(") expected");
      }
   }
   *help = NULL;
   AcceptTok(closebracket,")");

   return handle;
}

   

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Place_p parse_place()                                   */
/*                                                                            */
/* Beschreibung     : Parst eine Stellenangabe, Zeiger auf Ergebnis wird      */
/*                    zurueckgegeben.                                         */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991                                           */
/*                    <2> 08.4.1991 Aufbau der Struktur                       */
/*                                                                            */
/******************************************************************************/

Place_p parse_place()
{
   Place_p   handle;
   NumList_p *help;

   if(test_id(ident,"L") || test_id(ident,"R"))
   {
      handle = AllocPlaceCell();

      handle->side = *(akttoken.literal);
      help = &(handle->rest);

      NextRealToken();
      while(test(fullstop))
      {
         NextRealToken();

         check(number,"Number");

         *help = AllocNumListCell();
         (*help)->value = akttoken.numval;
         help = &((*help)->rest);

         NextRealToken();
      }
      *help = NULL;
      return handle;
   }
   else
   {
      RdErr("Place starting with 'L' or 'R' expected");
      FAKE_RETURN;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Pair_p parse_termpair()                                 */
/*                                                                            */
/* Beschreibung     : Parst Termpaar, gibt Pointer auf Ergebnis.              */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von parse_term,AcceptTok.                  */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                    <2> 08.4.1991 Aufbau der Struktur.                      */
/*                                                                            */
/******************************************************************************/

Pair_p parse_termpair()
{
   Pair_p handle;

   handle = AllocPairCell();

   handle->lside = parse_term();

   if(test(equ))
   {
      handle->type = eqn;
      NextRealToken();
   }
   else
   {
      handle->type = rule;
      AcceptTok(r_arrow,"->' or '=");
   }
   handle->rside = parse_term();

   return handle;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_just()                                     */
/*                                                                            */
/* Beschreibung     : Parst einen PCL-Ausdruck, gibt Pointer auf Ausdruck     */
/*                    zurueck.                                                */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_just()
{
   Just_p handle;

   if(test(number))
   {
      handle = parse_quotation();
   }
   else if(test_id(ident,"initial"))
   {
      handle = parse_initial();
   }
   else if(test_id(ident,"hypothesis"))
   {
      handle = parse_hypothesis();
   }
   else if(test_id(ident,"orient"))
   {
      handle = parse_orient();
   }
   else if(test_id(ident,"cp"))
   {
      handle = parse_cp();
   }
   else if(test_id(ident,"tes"))
   {
      handle = parse_tes_red();
   }
   else if(test_id(ident,"instance"))
   {
      handle = parse_instance();
   }
   else
   {
      handle = 0;
      RdErr("PCL-Expression expected");
   }
   return handle;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_initial()                                  */
/*                                                                            */
/* Beschreibung     : Parst einen "initial"-PCL-Ausdruck, gibt Pointer auf    */
/*                   Ausdruck zurueck.                                        */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_initial()
{
   Just_p handle;

   AcceptId(ident,"initial");
   handle = AllocJustCell();
   handle->operation = initial;

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_hypothesis()                               */
/*                                                                            */
/* Beschreibung     : Parst einen "hypothesis"-PCL-Ausdruck, gibt Pointer auf */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_hypothesis()
{
   Just_p handle;

   AcceptId(ident,"hypothesis");
   handle = AllocJustCell();
   handle->operation = hypothesis;

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_orient()                                   */
/*                                                                            */
/* Beschreibung     : Parst einen "orient"-PCL-Ausdruck, gibt Pointer auf     */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_orient()
{
   Just_p handle;

   AcceptId(ident,"orient");
   AcceptTok(openbracket,"(");
   handle = AllocJustCell();
   handle->arg1.rarg = parse_just();
   AcceptTok(comma,",");
   if(test_id(ident,"x"))
   {
      handle->operation = orientx;
   }
   else if(test_id(ident,"u"))
   {
      handle->operation = orientu;
   }
   else 
   {
      RdErr("'x' or 'u' expected");
   }
   NextRealToken();

   AcceptTok(closebracket,")");

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_cp()                                       */
/*                                                                            */
/* Beschreibung     : Parst einen "cp"-PCL-Ausdruck, gibt Pointer auf         */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_cp()
{
   Just_p handle;

   AcceptId(ident,"cp");
   AcceptTok(openbracket,"(");

   handle = AllocJustCell();
   handle->operation = cp;

   handle->arg1.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place1 = parse_place();
   AcceptTok(comma,",");

   handle->arg2.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place2 = parse_place();
   if((handle->place2)->rest)
   {
      RdErr("Fourth Argument in cp should designate Side only");
   }
   AcceptTok(closebracket,")");

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_tes_red()                                  */
/*                                                                            */
/* Beschreibung     : Parst einen "tes_red"-PCL-Ausdruck, gibt Pointer auf    */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_tes_red()
{
   Just_p handle;

   AcceptId(ident,"tes");
   AcceptTok(hyphen,"tes-red");
   AcceptId(ident,"red");
   AcceptTok(openbracket,"(");

   handle = AllocJustCell();
   handle->operation = tes_red;

   handle->arg1.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place1 = parse_place();
   AcceptTok(comma,",");

   handle->arg2.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place2 = parse_place();
   if((handle->place2)->rest)
   {
      RdErr("Fourth Argument in tes_red should designate Side only");
   }
   AcceptTok(closebracket,")");

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_instance()                                 */
/*                                                                            */
/* Beschreibung     : Parst einen "instance"-PCL-Ausdruck, gibt Pointer auf   */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_instance()
{
   Just_p handle;

   AcceptId(ident,"instance");
   AcceptTok(openbracket,"(");

   handle = AllocJustCell();
   handle->operation = instance;

   handle->arg1.rarg = parse_just();
   AcceptTok(comma,",");

   handle->arg2.rarg = parse_just();

   AcceptTok(closebracket,")");

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Just_p parse_quotation()                                */
/*                                                                            */
/* Beschreibung     : Parst einen "quotation"-PCL-Ausdruck, gibt Pointer auf  */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Just_p parse_quotation()
{
   Just_p handle;

   handle = AllocJustCell();
   handle->operation = quotation;

   handle->arg1.Targ.sarg = parse_numlist();

   return handle;
}


/*-------------------------------------------------------------------------
//
// Function: parse_annotations()
//
//   Parst die (optionalen) Anmerkungen zu einem PCL-Schritt. 
//
// Global Variables: -
//
// Side Effect     : Input, setzt Werte in *handle
//
//-----------------------------------------------------------------------*/

void parse_annotations(Step_p handle)
{
   AcceptTok(colon, ":");
   AcceptTok(openbracket, "(");
   handle->rw_distance = ParseInt();
   AcceptTok(comma,",");
   handle->cp_distance = ParseInt();
   AcceptTok(comma,",");
   if(!(test_id(ident,"T") || (test_id(ident,"F"))))
   {
      RdErr("Ident ('T' or 'F') expected");
   }
   handle->used = test_id(ident,"T");
   NextRealToken();
   AcceptTok(comma,",");
   handle->cp_cost = ParseInt();
   AcceptTok(closebracket, ")");
}

/*-------------------------------------------------------------------------
//
// Function: ParseInt()
//
//   Parse an optionally signed integer (well, long) number and return
//   its value.
//
// Global Variables: akttoken
//
// Side Effect     : Input is read
//
//-----------------------------------------------------------------------*/

long ParseInt()
{
   long res;

   if(test(hyphen))
   {
      NextRealToken();
      res = -1 * akttoken.numval;
   }
   else
   {
      res = akttoken.numval;
   }
   AcceptTok(number, 
	     "Integer number, optionally preceded by a '-' expected");
   return res;
}
      

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void AcceptTok(TokenType tok,char* lit)                 */
/*                    IN    TokenType tok                                     */
/*                    IN    char*     lit                                     */
/*                                                                            */
/* Beschreibung     : Ueberprueft, ob akttoken.token vom gewuenschten Typ     */
/*                    ist. Wenn ja: NextRealToken, sonst Fehler, Abbruch.     */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, check                              */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void AcceptTok(TokenType tok,char* lit)
{
   check(tok,lit) ;
   NextRealToken();
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void check(TokenType tok,char* lit)                     */
/*                    IN    TokenType tok                                     */
/*                    IN    char*     lit                                     */
/*                                                                            */
/* Beschreibung     : Ueberprueft, ob akttoken.token vom gewuenschten Typ     */
/*                    ist. Wenn ja: weiter, sonst Fehler, Abbruch.            */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, RdErr                              */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void check(TokenType tok,char* lit)
{
   if(!test(tok))
   {
      AppendString(&ErrCell,lit);
      AppendString(&ErrCell," expected");
      RdErr(ViewString(&ErrCell));
      ResetString(&ErrCell);  /* Nur der Schoenheit wegen - RdErr bricht ab */
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL test(TokenType tok)                                */
/*                    IN    TokenType tok                                     */
/*                                                                            */
/* Beschreibung     : Ueberprueft, ob akttoken.token vom gewuenschten Typ     */
/*                    ist.                                                    */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL test(TokenType tok)
{
   if((tok == Identifier)&&((akttoken.token == idnum)||(akttoken.token == ident)))
   {
      return TRUE;
   }
   else if(tok == number)
   {
      return ((akttoken.token == idnum)&&(!strcmp(akttoken.textval,"")));
   }
   else
   {
      return (tok == akttoken.token);
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void AcceptId(TokenType tok,char* id)                   */
/*                    IN    TokenType tok                                     */
/*                    IN    char*     id                                      */
/*                                                                            */
/* Beschreibung     : Falls akttoken den bei check_id aufgezaehlten Be-       */
/*                    dingungen entspricht, NextRealToken, sonst RdErr        */
/*                                                                            */
/* Globale Variable : akktoken                                                */
/*                                                                            */
/* Seiteneffekte    : Ueber aufruf von check_id, NextRealToken                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void AcceptId(TokenType tok,char* id)
{
   check_id(tok,id);
   NextRealToken();
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void check_id(TokenType tok,char* id)                   */
/*                    IN    TokenType tok                                     */
/*                    IN    char*     id                                      */
/*                                                                            */
/* Beschreibung     : Ueberprueft, ob akttoken.token vom gewuenschten Typ     */
/*                    ist.                                                    */
/*                    - Ist tok = Identifier, so muss akttoken.token entweder */
/*                      ident oder idnum sein, *(aktoken.literal) = *id.      */
/*                    - Ist tok = ident, so muss akkttoken.token ebenfalls    */
/*                      ident sein, *(aktoken.literal) = *id.                 */
/*                    - Ist tok = idnum, so muss akttoken.token ebenfalls     */
/*                      idnum sein, *(akttoken.textval) = *id.                */
/*                    Ok -> Weiter, sonst RdErr                               */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch RdErr                                             */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void check_id(TokenType tok,char* id)
{
   switch(tok)
   {
      case Identifier: if((akttoken.token == ident)||(akttoken.token == idnum))
                       {
                          if(strcmp(akttoken.literal,id))
                          {
                             AppendString(&ErrCell,id);
                             AppendString(&ErrCell," expected");
                             RdErr(ViewString(&ErrCell));
                             ResetString(&ErrCell);
                          }
                       }
                       else
                       {
                          RdErr("Identifier expected");
                       }
                       break;
      case idnum:      if(akttoken.token == idnum)
                       {
                          if(strcmp(akttoken.textval,id))
                          {
                             AppendString(&ErrCell,"idnum starting with '");
                             AppendString(&ErrCell,id);
                             AppendString(&ErrCell,"' expected");
                             RdErr(ViewString(&ErrCell));
                             ResetString(&ErrCell);
                          }
                       }
                       else
                       {
                          AppendString(&ErrCell,"idnum starting with '");
                          AppendString(&ErrCell,id);
                          AppendString(&ErrCell,"' expected");
                          RdErr(ViewString(&ErrCell));
                          ResetString(&ErrCell);
                       }
                       break;
      case ident:      if(akttoken.token == ident)
                       {
                          if(strcmp(akttoken.literal,id))
                          {
                             AppendString(&ErrCell,id);
                             AppendString(&ErrCell," expected");
                             RdErr(ViewString(&ErrCell));
                             ResetString(&ErrCell);
                          }
                       }
                       else
                       {
                          RdErr("ident expected");
                       }
                       break;
#ifdef LEARN_VERSION
      case colonident: if(akttoken.token == colonident)
                       {
                          if(strcmp(akttoken.literal,id))
                          {
                             AppendString(&ErrCell,id);
                             AppendString(&ErrCell," expected");
                             RdErr(ViewString(&ErrCell));
                             ResetString(&ErrCell);
                          }
                       }
                       else
                       {
                          RdErr("colonident expected");
                       }
                       break;
#endif
      default:         RdErr("AcceptId called with unexpected argument");
                       break;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL test_id(TokenType tok,char* id)                    */
/*                    IN    TokenType tok                                     */
/*                    IN    char*     id                                      */
/*                                                                            */
/* Beschreibung     : Falls akttoken den bei check_id aufgezaehlten Be-       */
/*                    dingungen entspricht, Rueckgabe TRUE, sonst FALSE       */
/*                                                                            */
/* Globale Variable : akktoken                                                */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL test_id(TokenType tok,char* id)
{
   switch(tok)
   {
      case Identifier: if((akttoken.token == ident)||(akttoken.token == idnum))
                       {
                          if(strcmp(akttoken.literal,id))
                          {
                             return FALSE;
                          }
                          else
                          {
                             return TRUE;
                          }
                       }
                       else
                       {
                          return FALSE;
                       }
                       break;
      case idnum:      if(akttoken.token == idnum)
                       {
                          if(strcmp(akttoken.textval,id))
                          {
                             return FALSE;
                          }
                          else
                          {
                             return TRUE;
                          }
                       }
                       else
                       {
                          return FALSE;
                       }
                       break;
      case ident:      if(akttoken.token == ident)
                       {
                          if(strcmp(akttoken.literal,id))
                          {
                             return FALSE;
                          }
                          else
                          {
                             return TRUE;
                          }
                       }
                       else
                       {
                          return FALSE;
                       }
                       break;
#ifdef LEARN_VERSION
      case colonident: if(akttoken.token == colonident)
                       {
                          if(strcmp(akttoken.literal,id))
                          {
                             return FALSE;
                          }
                          else
                          {
                             return TRUE;
                          }
                       }
                       else
                       {
                          return FALSE;
                       }
                       break;
#endif
      default:         return FALSE;
                       break;
   }
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


