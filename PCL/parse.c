
/*************************************************************************/
/*                                                                       */
/*   File:        parse.c                                                */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Parser fuer Termpaare (wird erweitert zu pcl-Parser)   */
/*                                                                       */
/*   Aenderungen: <1> 11.2.1991  neu                                     */
/*                                                                       */
/*************************************************************************/

#include "parse.h"




/*----------------------------------------------------------------------------*/
/*                 Globale Variable                                           */
/*----------------------------------------------------------------------------*/


static StringCell ErrCell = {NULL,0,0};

tc_StepData aktstep;
StringCell  aktcomment = {NULL,0,0};

StringCell  res_lside = {NULL,0,0},
            res_rside = {NULL,0,0},
            res_place1 = {NULL,0,0},
            res_place2 = {NULL,0,0};

char*       tc_IdPrint[] = {"I","CP","R","E","U","G","F"};
 


/*----------------------------------------------------------------------------*/
/*           Forward-Deklarationen interner Funktionen                        */
/*----------------------------------------------------------------------------*/

void      parse_term(String_p res);
void      parse_arglist(String_p res);
void      parse_place(String_p res);

void      parse_assign();
void      parse_delete();
void      parse_clear();
void      parse_swap();
void      parse_reduce();
void      parse_buildcp();
void      parse_subsum();
void      parse_initial();

void      parse_termpair();
tc_IdType Check_tc_ids();

void      AcceptTok(TokenType tok,char* lit);
void      check(TokenType tok, char* lit);
BOOL      test(TokenType tok);

void      AcceptId(TokenType tok,char* id);
void      check_id(TokenType tok,char* id);
BOOL      test_id(TokenType tok,char* id);

/*----------------------------------------------------------------------------*/
/*                 Exportierte Funktionen                                     */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void Parse_tc_Step()                                    */
/*                                                                            */
/* Beschreibung     : Parst einen tc-Schritt, Ergebnis in aktstep. Die von    */
/*                    aktstep referenzierten Spicherplaetze werden weiterhin  */
/*                    verwendet !                                             */
/*                                                                            */
/* Globale Variable : aktstep, akttoken, aktcomment                           */
/*                                                                            */
/* Seiteneffekte    : Eingabe wird gelesen, die globalen Variablen ver-       */
/*                    aendert.                                                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void Parse_tc_Step()
{
   DEBUG(2,printf("Parse_tc_Step()...\n"));
   
   ResetString(&aktcomment);
   ResetString(&res_place1);
   ResetString(&res_place2);

   DEBUG(2,printf("...Parse_tc_Step() (Strings reset)...\n"));

   aktstep.tc_id_type = Check_tc_ids();
   if(aktstep.tc_id_type == I)
   {
      RdErr("Initial Termpairs not allowed here");
   }

   DEBUG(2,printf("...Parse_tc_Step() (Type recognised)...\n"));

   aktstep.tc_id_numval = akttoken.numval;

   AcceptTok(idnum,"Identifier designating tc-Termpair");

   DEBUG(2,printf("...Parse_tc_Step() (Identifier accepted)...\n"));

   AcceptTok(colon,":");

   DEBUG(2,printf("...Parse_tc_Step() vor Verzweigung...\n"));

   if(test(assign))
   {
      parse_assign();
   }
   else if(test_id(ident,"reduce"))
   {
      parse_reduce();
   }
   else if(test_id(ident,"buildcp"))
   {
      parse_buildcp();
   }
   else if(test_id(ident,"delete"))
   {
      parse_delete();
   }
   else if(test_id(ident,"clear"))
   {
      parse_clear();
   }
   else if(test_id(ident,"subsum"))
   {
      parse_subsum();
   }
   else if(test_id(ident,"initial"))
   {
      parse_initial();
   }
   else if(test_id(ident,"swap"))
   {
      parse_swap();
   }
   else if(test(NoToken))   /* EOF  */
   {
      aktstep.tc_operation = s_noop;
   }
   else 
   {
      RdErr("tc_Operation expected");
   }
   aktstep.comment = ViewString(&aktcomment);
   DEBUG(2,printf("...Parse_tc_Step()\n"));
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void NextRealToken()                                    */
/*                                                                            */
/* Beschreibung     : Liest solange Token, bis akttoken.token != comment.     */
/*                    Kommentare werden an aktcomment angehaengt.             */
/*                                                                            */
/* Globale Variable : akttoken, aktckomment                                   */
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
   DEBUG(4,PrintToken(&akttoken));
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void Print_tc_Step(tc_Step_p outstep)                   */
/*                    IN    tc_Step_p outstep                                 */
/*                                                                            */
/* Beschreibung     : Gibt Informationen zu outstep aus.                      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 26.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void Print_tc_Step(tc_Step_p outstep)
{
   printf("tc_id: %s%ld:",tc_IdPrint[(long)outstep->tc_id_type],outstep->tc_id_numval);
   switch(outstep->tc_operation)
   {
      case s_noop:    printf("noop: %s\n\n",outstep->comment);
                      break;
      case s_reduce:  printf("reduce: %s%ld, %s, %s, %s\n",
                             tc_IdPrint[(long)outstep->arg1_type],
                             outstep->arg1_numval,
                             outstep->place1,
                             outstep->place2,
                             outstep->comment);
                      printf("Result: %s %ld %s\n\n",outstep->res_lside,
                                                     (long)outstep->eq_or_r,
                                                     outstep->res_rside);
                      break;
      case s_buildcp: printf("buildcp: %s%ld, %s,%s%ld, %s, %s\n",
                             tc_IdPrint[(long)outstep->arg1_type],
                             outstep->arg1_numval,
                             outstep->place1,
                             tc_IdPrint[(long)outstep->arg2_type],
                             outstep->arg2_numval,
                             outstep->place2,
                             outstep->comment);
                      printf("Result: %s %ld %s\n\n",outstep->res_lside,
                                                     (long)outstep->eq_or_r,
                                                     outstep->res_rside);
                      break;
      case s_delete:  printf("delete: %s\n\n",outstep->comment);
                      break;
      case s_clear:   printf("clear: %s\n\n",outstep->comment);
                      break;
      case s_subsum:  printf("subsum: %s\n",outstep->comment);
                      printf("Result: %s %ld %s\n\n",outstep->res_lside,
                                                     (long)outstep->eq_or_r,
                                                     outstep->res_rside);
                      break;
      case s_assign:  printf("assign: %s%ld, %s\n",
                             tc_IdPrint[(long)outstep->arg1_type],
                             outstep->arg1_numval,
                             outstep->comment);
                      printf("Result: %s %ld %s\n\n",outstep->res_lside,
                                                     (long)outstep->eq_or_r,
                                                     outstep->res_rside);
                      break;
      case s_swap:    printf("swap: %s\n",outstep->comment);
                      printf("Result: %s %ld %s\n\n",outstep->res_lside,
                                                     (long)outstep->eq_or_r,
                                                     outstep->res_rside);
                      break;
      default:        RdErr("Undefinierte Eingabe bei Print_tc_Step");
                      break;
   }
}

/*----------------------------------------------------------------------------*/
/*                 Interne Funktionen                                         */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_term(String_p res)                           */
/*                    OUT   String_p res                                      */
/*                                                                            */
/* Beschreibung     : Parst ein Term, (nur zur Syntax-Ueberpruefung), das     */
/*                    Ergebnis wird an res->str angehaengt.                   */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991                                           */
/*                                                                            */
/******************************************************************************/

void parse_term(String_p res)
{
   check(Identifier,"Identifier");
   AppendString(res,akttoken.literal);
   NextRealToken();

   if(test(openbracket))
   {
      parse_arglist(res);
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_arglist(String_p res)                        */
/*                    OUT      String_p res                                   */
/*                                                                            */
/* Beschreibung     : Parst Argumentenliste einer Funktion, Ergebnis wird     */
/*                    an res->str angehaengt.                                 */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991                                           */
/*                                                                            */
/******************************************************************************/

void parse_arglist(String_p res)
{
   check(openbracket,"(");
   AppendString(res,akttoken.literal);
   NextRealToken();
   while(!test(closebracket))
   {
      parse_term(res);
      if(test(comma))
      {
         AppendString(res,akttoken.literal);
         AcceptTok(comma,"','");
      }
      else if(!test(closebracket))
      {
         RdErr(") expected");
      }
   }
   AppendString(res,akttoken.literal);
   AcceptTok(closebracket,")");
}

   

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_place(String_p res)                          */
/*                    OUT      String_p res                                   */
/*                                                                            */
/* Beschreibung     : Parst eine Stellenangabe,Ergebnis wird an res->str      */
/*                    angehaengt.                                             */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 20.2.1991                                           */
/*                                                                            */
/******************************************************************************/

void parse_place(String_p res)
{
   if(test_id(ident,"L") || test_id(ident,"R"))
   {
      AppendString(res,akttoken.literal);
      NextRealToken();
      while(test(fullstop))
      {
         AppendString(res,akttoken.literal);
         AcceptTok(fullstop,"'.'");
         check_id(idnum,"");
         AppendString(res,akttoken.literal);
         NextRealToken();
      }
   }
   else
   {
      RdErr("Place starting with 'L' or 'R' expected");
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_assign()                                     */
/*                                                                            */
/* Beschreibung     : Parst eine Zuweisung, Ergebnis wird in aktstep ein-     */
/*                    getragen.                                               */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_assign()
{
   AcceptTok(assign,"<==");
   aktstep.tc_operation = s_assign;

   aktstep.arg1_type = Check_tc_ids();
   aktstep.arg1_numval = akttoken.numval;
   AcceptTok(idnum,"tc_Identifier");
  
   parse_termpair();
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_reduce()                                     */
/*                                                                            */
/* Beschreibung     : Parst eine Reduktion, Ergebnis wird in aktstep ein-     */
/*                    getragen.                                               */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_reduce()
{
   AcceptTok(ident,"reduce");
   aktstep.tc_operation = s_reduce;

   AcceptTok(openbracket,"(");

   aktstep.arg1_type = Check_tc_ids();
   aktstep.arg1_numval = akttoken.numval;
   AcceptTok(idnum,"tc_Identifier");

   AcceptTok(comma,",");

   parse_place(&res_place1);
   aktstep.place1 = ViewString(&res_place1);

   AcceptTok(comma,",");

   parse_place(&res_place2);
   aktstep.place2 = ViewString(&res_place2);

   AcceptTok(closebracket,")");

   parse_termpair();
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_buildcp()                                    */
/*                                                                            */
/* Beschreibung     : Parst eine Kritische-Paar-Bildung, Ergebnis wird in     */
/*                    aktstep eingetragen.                                    */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_buildcp()
{
   AcceptTok(ident,"buildcp");
   aktstep.tc_operation = s_buildcp;

   AcceptTok(openbracket,"(");

   aktstep.arg1_type = Check_tc_ids();
   aktstep.arg1_numval = akttoken.numval;
   AcceptTok(idnum,"tc_Identifier");

   AcceptTok(comma,",");

   parse_place(&res_place1);
   aktstep.place1 = ViewString(&res_place1);

   AcceptTok(comma,",");

   aktstep.arg2_type = Check_tc_ids();
   aktstep.arg2_numval = akttoken.numval;
   AcceptTok(idnum,"tc_Identifier");

   AcceptTok(comma,",");

   parse_place(&res_place2);
   aktstep.place2 = ViewString(&res_place2);

   AcceptTok(closebracket,")");

   AppendString(&aktcomment,"# Ueberlappung war: ");
   parse_term(&aktcomment);

   parse_termpair();
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_subsum()                                     */
/*                                                                            */
/* Beschreibung     : Parst eine Subsum-Operation, Ergebnis wird in aktstep   */
/*                    eingetragen.                                            */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_subsum()
{
   AcceptTok(ident,"subsum");
   aktstep.tc_operation = s_subsum;

   AcceptTok(openbracket,"(");

   aktstep.arg1_type = Check_tc_ids();
   aktstep.arg1_numval = akttoken.numval;
   AcceptTok(idnum,"tc_Identifier");

   AcceptTok(closebracket,")");

   parse_termpair(); 
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_initial()                                    */
/*                                                                            */
/* Beschreibung     : Parst eine Initial-Operation, Ergebnis wird in aktstep  */
/*                    eingetragen.                                            */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_initial()
{
   AcceptTok(ident,"initial");
   aktstep.tc_operation = s_initial;

   parse_termpair(); 
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_delete()                                     */
/*                                                                            */
/* Beschreibung     : Parst eine Delete-Operation, Ergebnis wird in aktstep   */
/*                    eingetragen.                                            */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_delete()
{
   AcceptTok(ident,"delete");
   aktstep.tc_operation = s_delete;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_clear()                                      */
/*                                                                            */
/* Beschreibung     : Parst eine Clear-Operation, Ergebnis wird in aktstep    */
/*                    eingetragen.                                            */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_clear()
{
   AcceptTok(ident,"clear");
   aktstep.tc_operation = s_clear;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void parse_swap()                                       */
/*                                                                            */
/* Beschreibung     : Parst eine Swap-Operation, Ergebnis wird in aktstep     */
/*                    eingetragen.                                            */
/*                                                                            */
/* Globale Variable : akttoken, aktstep                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von NextRealToken,AcceptTok                */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991                                           */
/*                                                                            */
/******************************************************************************/
   
void parse_swap()
{
   AcceptTok(ident,"swap");
   aktstep.tc_operation = s_swap;

   parse_termpair();
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : parse_termpair()                                        */
/*                                                                            */
/* Beschreibung     : Parst Termpaar, traegt Ergebnis in aktstep ein.         */
/*                                                                            */
/* Globale Variable : akttoken,aktstep                                        */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von parse_term,AcceptTok.                  */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void parse_termpair()
{
   ResetString(&res_lside);
   ResetString(&res_rside);

   parse_term(&res_lside);

   if(test(equ))
   {
      aktstep.eq_or_r = akttoken.token;
      NextRealToken();
   }
   else
   {
      AcceptTok(r_arrow,"->' or '=");
      aktstep.eq_or_r = r_arrow;
   }
   parse_term(&res_rside);

   aktstep.res_lside = ViewString(&res_lside);
   aktstep.res_rside = ViewString(&res_rside);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : tc_IdType Check_tc_ids()                                */
/*                                                                            */
/* Beschreibung     : Ueberprueft, ob akttoken ein gueltiger tc-Identifier    */
/*                    ist. Ja -> Rueckgabe des Typs, Nein -> Fehler           */
/*                                                                            */
/* Globale Variable : akttoken                                                */
/*                                                                            */
/* Seiteneffekte    : Abbruch bei Fehler                                      */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

tc_IdType Check_tc_ids()
{
   if(test_id(idnum,"I"))
   {
      return I;
   }
   else if(test_id(idnum,"CP"))
   {
      return CP;
   }
   else if(test_id(idnum,"R"))
   {
      return R;
   }
   else if(test_id(idnum,"E"))
   {
      return E;
   }
   else if(test_id(idnum,"U"))
   {
      return U;
   }
   else if(test_id(idnum,"G"))
   {
      return G;
   }
   else if(test_id(idnum,"F"))
   {
      return F;
   }
   else
   {
      RdErr("Identifier designating StepType expected");
      FAKE_RETURN;
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void AcceptTok(TokenType tok,char* lit)
                   IN    TokenType tok
                   IN    char*     lit    

Beschreibung     : Ueberprueft, ob akttoken.token vom gewuenschten Typ
                   ist. Wenn ja: NextRealToken, sonst Fehler, Abbruch.   

Globale Variable : akttoken

Seiteneffekte    : Durch NextRealToken, check   

Aenderungen      : <1> 20.2.1991 neu
                   <2> 27.8.1994 Umbenennung (alter Name: accept), um 
                                 Aerger mit dem accept in den Netzwerk-
                                 Teilen von DISCOUNT zu vermeiden. 
				 Neue Box! 

-------------------------------------------------------------------------*/


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
      printf("...check (!test)...\n");
      printf("%s\n",lit);
      printf("%ld, %ld, %s\n",ErrCell.mem,ErrCell.len,ErrCell.str);
      printf("%s\n",AppendString(&ErrCell,lit));
      AppendString(&ErrCell," expected");
      printf("%s\n",ViewString(&ErrCell));
      printf("%s\n",ViewString(&ErrCell));
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


