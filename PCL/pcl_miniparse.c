

/*************************************************************************/
/*                                                                       */
/*   File:        pcl_miniparse.c                                        */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Parser fuer PCL                                        */
/*                                                                       */
/*   Aenderungen: <1> Uebernahme von pcl_parse.c                         */
/*                    Es werden nur die fuer die extract-Operation not   */
/*                    wendigen Datenstrukturen aufgebaut, sonstige In-   */
/*                    formationen werden in String-Form gespeichert.     */
/*                                                                       */
/*************************************************************************/

#include "pcl_miniparse.h"




/*----------------------------------------------------------------------------*/
/*                 Globale Variable                                           */
/*----------------------------------------------------------------------------*/


static StringCell ErrCell = {NULL,0,0};

StringCell  aktcomment = {NULL,0,0};

BOOL PrintAnnotations = FALSE;


/*----------------------------------------------------------------------------*/
/*           Forward-Deklarationen interner Funktionen                        */
/*----------------------------------------------------------------------------*/

NumList_p parse_numlist();

String_p      parse_term(String_p isparsed);
String_p      parse_arglist(String_p isparsed);
char*         parse_place();

String_p      parse_termpair(String_p isparsed);

miniJust_p    parse_just();

miniJust_p    parse_initial();
miniJust_p    parse_hypothesis();
miniJust_p    parse_orient();
miniJust_p    parse_cp();
miniJust_p    parse_tes_red();
miniJust_p    parse_instance();
miniJust_p    parse_quotation();
void          parse_annotations(miniStep_p handle);

void          AcceptTok(TokenType tok,char* lit);
void          check(TokenType tok, char* lit);
BOOL          test(TokenType tok);

void          AcceptId(TokenType tok,char* id);
void          check_id(TokenType tok,char* id);
BOOL          test_id(TokenType tok,char* id);

/*----------------------------------------------------------------------------*/
/*                 Exportierte Funktionen                                     */
/*----------------------------------------------------------------------------*/

MakeAlloc(miniJustCell);
MakeAlloc(miniStepCell);
MakeFree(miniJustCell);
MakeFree(miniStepCell);

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeminiJust(miniJust_p junk)                      */
/*                    IN     miniJust_p junk                                  */
/*                                                                            */
/* Beschreibung     : Gibt den von der Rechtfertigung belegten Speicherplatz  */
/*                    frei.                                                   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch FreeminiJustCell                                  */
/*                                                                            */
/* Aenderungen      : <1> 05.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeminiJust(miniJust_p junk)
{
   if(junk)
   {
      switch(junk->operation)
      {
         case initial:
         case hypothesis:
	                  break;
         case quotation:  
                          FreeNumList(junk->arg1.targ);
                          break;
         case orientx:
         case orientu:    
                          FreeminiJust(junk->arg1.rarg);
                          break;
         case cp:
         case tes_red:
                          FREE(junk->place1);
                          FreeminiJust(junk->arg1.rarg);
                          FREE(junk->place2);
                          FreeminiJust(junk->arg2.rarg);
                          break;
         case instance:
                          FreeminiJust(junk->arg1.rarg);
                          FreeminiJust(junk->arg2.rarg);
                          break;
         default:         
                          fprintf(stderr,"Warning: Illegal Cell returned to FreeminiJust...\n");
                          break;
      }
      FreeminiJustCell(junk);
   }
   else
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreeminiJust (pcl_mem.c)...\n");
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeminiStep(miniStep_p junk)                      */
/*                    IN     miniStep_p junk                                  */
/*                                                                            */
/* Beschreibung     : Gibt den vom PCL-Schritt belegten Speicherplatz frei.   */
/*                                                                            */
/* Globale Variable : FreeminiStepList                                        */
/*                                                                            */
/* Seiteneffekte    : Durch FreeNumList, FreeminiJust, FreeminiStepCell.      */
/*                                                                            */
/* Aenderungen      : <1> 05.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeminiStep(miniStep_p junk)
{
   if(junk)
   {
      FreeNumList(junk->id);
      FREE(junk->pair);
      FreeminiJust(junk->just);
      FREE(junk->comment);
      FreeminiStepCell(junk);
   }
   else
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreeminiStep (pcl_mem.c)...\n");
   }
}


/*-----------------------------------------------------------------------
//
// Function: IsTrivial()
//
//   "Uberprueft, ob das in einem String kodierte Termpaar trivial
//   ist, i.e. ob die beiden Seiten syntaktisch identisch sind.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

BOOL IsTrivial(char *pair)
{
   char *point;

   while(*pair && iswhite(*pair))
   {
      pair++;
   }

   if((point = strstr(pair, "=")))
   {
      point++;
   }
   else if((point = strstr(pair, "->")))
   {
      point++;
      point++;
   }
   else
   {
      Error("Submitted string contains neiter = nor -> "
	    "(Function IsTrivial() in pcl_miniparse)\n");
   }
   while(*point && iswhite(*point))
   {
      point++;
   }
   if(!*point)
   {
      Error("Submitted string is not a valid termpair"
	    "(Function IsTrivial() in pcl_miniparse)\n");
   }
   return strstr(pair,point)==pair;
}

/*-----------------------------------------------------------------------
//
// Function: IsRedundant()
//
//   "Uberprueft, ob ein PCL-Schritt redundant ist (d.h. ein
//   abgeleitetes Ergebniss mit justifikation "instance" ist.
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

BOOL IsRedundant(miniStep_p step)
{
   if((step->type == tes_lemma) ||
      (step->type == tes_intermed) ||
      (step->type == tes_eqn) ||
      (step->type == tes_rule))
   {
      return step->just->operation == instance;
   }
   return FALSE;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintPlace(char* prt)                              */
/*                    IN     char* prt                                        */
/*                                                                            */
/* Beschreibung     : Gibt den Place, auf den prt zeigt, aus.                 */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                    <2> 10.7.1991 Aenderung fuer miniextract                */
/*                                                                            */
/******************************************************************************/

void PrintPlace(char* prt)
{
   fprintf(out,"%s",prt);
}
 

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintJust(miniJust_p prt)                          */
/*                    IN     miniJust_p prt                                   */
/*                                                                            */
/* Beschreibung     : Gibt die Herleitung, auf die prt zeigt, aus.            */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 16.5.1991 neu                                       */
/*                    <2> 10.7.1991 Aenderung fuer miniextract                */
/*                                                                            */
/******************************************************************************/

void PrintJust(miniJust_p prt)
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
      case quotation:  PrintNumList(prt->arg1.targ);
                       break;
      default:         Error("Unknown operation in PrintJust");
                       break;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintStep(miniStep_p prt)                          */
/*                    IN    miniStep_p prt                                    */
/*                                                                            */
/* Beschreibung     : Gibt einen PCL-Schritt aus.                             */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 16.4.1991 neu                                       */ 
/*                    <2> 10.7.1991 Aenderung fuer miniextract                */
/*                                                                            */
/******************************************************************************/

void PrintStep(miniStep_p prt)
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
   if(PrintAnnotations)
   {
      fprintf(out, ":(%ld,%ld,%c,%ld)",
	      prt->rw_distance,
	      prt->cp_distance,
	      prt->used? 'T' : 'F',
	      prt->cp_cost);
   }
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
/* FUNCTION         : miniStep_p ParseStep()                                  */
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

miniStep_p ParseStep()
{
   miniStep_p handle;
   static StringCell tpair = {NULL,0,0};

   DEBUG(32,printf("ParseStep...\n"));

   handle = AllocminiStepCell();
   ResetString(&tpair);

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

   DEBUG(32,printf("...Typ erkannt...\n"));

   AcceptTok(colon,":");

   handle->pair = GetCopyOfString(parse_termpair(&tpair));

   DEBUG(32,printf("...Termpaar geparst...\n"));

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
   handle->generated_cps = FALSE;
   handle->trivial = -1;

   handle->comment = GetCopyOfString(&aktcomment);
   ResetString(&aktcomment);
   

   DEBUG(32,printf("...ParseStep\n"));

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
/* FUNCTION         : String_p parse_term(String_p isparsed)                  */
/*                                                                            */
/* Beschreibung     : Parst ein Term, haengt ihn als String an isparsed an.   */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991 neu                                       */
/*                    <2> 08.4.1991 Aufbau der Termstruktur (statt Syntax-    */
/*                                  Check)                                    */
/*                    <3> 5.7.1991  Wieder zurueck zu den Strings (fuer Mini- */
/*                                  extract)                                  */
/*                                                                            */
/******************************************************************************/

String_p parse_term(String_p isparsed)
{
   DEBUG(32,printf("parse_term...\n"));
   check(Identifier,"Identifier");
   AppendString(isparsed,akttoken.literal);
   NextRealToken();

   if(test(openbracket))
   {
     parse_arglist(isparsed);
   }
   DEBUG(32,printf("...parse_term\n"));
   return isparsed;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : String_p parse_arglist(String_p isparsed)               */
/*                                                                            */
/* Beschreibung     : Parst Argumentenliste einer Funktion,                   */
/*                    haengt Ergebnis an.                                     */
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

String_p parse_arglist(String_p isparsed)
{
   DEBUG(32,printf("parse_arglist...\n"));

   AcceptTok(openbracket,"(");

   DEBUG(32,printf("parse_arglist(vor AppendChar)...\n"));

   AppendChar(isparsed,'(');

   DEBUG(32,printf("...parse_arglist(vor while)...\n"));

   while(!test(closebracket))
   {
      parse_term(isparsed);
      if(test(comma))
      {
         AcceptTok(comma,"','");
         AppendChar(isparsed,',');
      }
      else if(!test(closebracket))
      {
         RdErr(") expected");
      }
   }
   AcceptTok(closebracket,")");
   AppendChar(isparsed,')');

   DEBUG(32,printf("...parse_arglist\n"));

   return isparsed;
}

   

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* parse_place()                                     */
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
/*                    <3> 08.7.1991 Zurueck zu Strings                        */
/*                                                                            */
/******************************************************************************/

char* parse_place()
{
   static StringCell handle  = {NULL,0,0};

   ResetString(&handle);

   if(test_id(ident,"L") || test_id(ident,"R"))
   {
      AppendString(&handle,akttoken.literal);

      NextRealToken();
      while(test(fullstop))
      {
         AppendChar(&handle,'.');
         NextRealToken();

         check(number,"Number");
         AppendString(&handle,akttoken.literal);

         NextRealToken();
      }
   }
   else
   {
      RdErr("Place starting with 'L' or 'R' expected");
   }
   return GetCopyOfString(&handle);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : String_p parse_termpair(String_p isparsed)              */
/*                                                                            */
/* Beschreibung     : Parst Termpaar, haengt Ergebnis an.                     */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von parse_term,AcceptTok.                  */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                    <2> 08.4.1991 Aufbau der Struktur.                      */
/*                    <3> 08.7.1991 Zurueck zu Strings                        */
/*                                                                            */
/******************************************************************************/

String_p parse_termpair(String_p isparsed)
{
   parse_term(isparsed);

   if(test(equ))
   {
      AppendString(isparsed," = ");
      NextRealToken();
   }
   else
   {
      AppendString(isparsed," -> ");
      AcceptTok(r_arrow,"-> or =");
   }
   parse_term(isparsed);

   return isparsed;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : miniJust_p parse_just()                                 */
/*                                                                            */
/* Beschreibung     : Parst einen PCL-Ausdruck, gibt Pointer auf Ausdruck     */
/*                    zurueck.                                                */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_just()
{
   miniJust_p handle;

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
/* FUNCTION         : miniJust_p parse_initial()                              */
/*                                                                            */
/* Beschreibung     : Parst einen "initial"-PCL-Ausdruck, gibt Pointer auf    */
/*                   Ausdruck zurueck.                                        */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_initial()
{
   miniJust_p handle;

   AcceptId(ident,"initial");
   handle = AllocminiJustCell();
   handle->operation = initial;

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : miniJust_p parse_hypothesis()                           */
/*                                                                            */
/* Beschreibung     : Parst einen "hypothesis"-PCL-Ausdruck, gibt Pointer auf */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_hypothesis()
{
   miniJust_p handle;

   AcceptId(ident,"hypothesis");
   handle = AllocminiJustCell();
   handle->operation = hypothesis;

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : miniJust_p parse_orient()                               */
/*                                                                            */
/* Beschreibung     : Parst einen "orient"-PCL-Ausdruck, gibt Pointer auf     */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 09.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_orient()
{
   miniJust_p handle;

   AcceptId(ident,"orient");
   AcceptTok(openbracket,"(");
   handle = AllocminiJustCell();
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
/* FUNCTION         : miniJust_p parse_cp()                                   */
/*                                                                            */
/* Beschreibung     : Parst einen "cp"-PCL-Ausdruck, gibt Pointer auf         */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_cp()
{
   miniJust_p handle;

   AcceptId(ident,"cp");
   AcceptTok(openbracket,"(");

   handle = AllocminiJustCell();
   handle->operation = cp;

   handle->arg1.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place1 = parse_place();
   AcceptTok(comma,",");

   handle->arg2.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place2 = parse_place();
   if((handle->place2)[1]!='\0')
   {
      RdErr("Fourth Argument in cp should designate Side only");
   }
   AcceptTok(closebracket,")");

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : miniJust_p parse_tes_red()                              */
/*                                                                            */
/* Beschreibung     : Parst einen "tes_red"-PCL-Ausdruck, gibt Pointer auf    */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_tes_red()
{
   miniJust_p handle;

   AcceptId(ident,"tes");
   AcceptTok(hyphen,"tes-red");
   AcceptId(ident,"red");
   AcceptTok(openbracket,"(");

   handle = AllocminiJustCell();
   handle->operation = tes_red;

   handle->arg1.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place1 = parse_place();
   AcceptTok(comma,",");

   handle->arg2.rarg = parse_just();
   AcceptTok(comma,",");

   handle->place2 = parse_place();
   if((handle->place2)[1]!='\0')
   {
      RdErr("Fourth Argument in tes_red should designate Side only");
   }
   AcceptTok(closebracket,")");

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : miniJust_p parse_instance()                             */
/*                                                                            */
/* Beschreibung     : Parst einen "instance"-PCL-Ausdruck, gibt Pointer auf   */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_instance()
{
   miniJust_p handle;

   AcceptId(ident,"instance");
   AcceptTok(openbracket,"(");

   handle = AllocminiJustCell();
   handle->operation = instance;

   handle->arg1.rarg = parse_just();
   AcceptTok(comma,",");

   handle->arg2.rarg = parse_just();

   AcceptTok(closebracket,")");

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : miniJust_p parse_quotation()                            */
/*                                                                            */
/* Beschreibung     : Parst einen "quotation"-PCL-Ausdruck, gibt Pointer auf  */
/*                    Ausdruck zurueck.                                       */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch NextRealToken, AcceptTok, ...                     */
/*                                                                            */
/* Aenderungen      : <1> 11.4.1991 neu                                       */
/*                    <2> 07.9.1991 Aenderung fuer miniparse                  */
/*                                                                            */
/******************************************************************************/

miniJust_p parse_quotation()
{
   miniJust_p handle;

   handle = AllocminiJustCell();
   handle->operation = quotation;

   handle->arg1.targ = parse_numlist();

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

void parse_annotations(miniStep_p handle)
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
   handle->used = akttoken.textval[0]=='T' ? TRUE : FALSE;
   NextRealToken();
   AcceptTok(comma,",");
   handle->cp_cost = ParseInt();
   AcceptTok(closebracket, ")");
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
      default:         return FALSE;
                       break;
   }
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


