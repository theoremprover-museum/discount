
#include "parseexp.h"



/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/

/* Suffix fuer eine Datei einer Expertenspezifikation        */
#define EXPFILESUFFIX "xp"

/*****************************************************************************/
/*                                                                           */
/*                            Loakle Variable                                */
/*                                                                           */
/*****************************************************************************/

static char Filename[MAXPATHLEN];  /* kompletter Pfad der zu lesenden Datei */
static short f_descr; /* Interner Filedescriptor des Scanners */

/*****************************************************************************/
/*                                                                           */
/*                      Lokale Funktionsvereinbarung                         */
/*                                                                           */
/*****************************************************************************/

static void    SyntaxError ( char *error );
static void    CheckError ( bool cond, char *msg );
static int     ReadGanzzahl( void );
static void    ReadZahlPaar( short *zahl1, short *zahl2 );
static void    ReadRealzahl ( double *real, char *id, Symbol *s );

static short   parameter_anz_best ( short fkts_nr );
static short   Read_Param_list ( ParameterElement *paramliste,
				char *id, Symbol *s);
static short   ReadExp ( ObjectInFrameInfo *exp_liste, char *id, Symbol *s );
static short   ReadGa( ObjectInFrameInfo *ga_liste, char *id, Symbol *s );
static short   ReadDom( ObjectInFrameInfo *dom_liste, char *id, Symbol *s );
static short   ReadPolynomExp ( ParameterElement *paramliste,
			       char *id, Symbol *s);
static short   Read_Param_list2( ParameterElement *paramliste,
				char *id, Symbol *s);

/*
//-----------------------------------------------------------------------------
//  Funktion:       SyntaxError
//
//  Parameter:      error       Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung
//-----------------------------------------------------------------------------
*/

static void    SyntaxError ( char *error )
{
    PrintScanText ( f_descr );
    printf ( "****  Fehler in der eingelesenen Datei : %s.\n", Filename );
    printf ( "****  %s\n", error );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CheckError
//
//  Parameter:      cond    Bedingung fuer Fehlerfall
//                  msg     Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung, falls cond erfuellt.
//-----------------------------------------------------------------------------
*/
static void CheckError ( bool cond, char *msg )
{
    if (cond)
    {
        SyntaxError ( msg );
        CloseInput ( f_descr );
	getchar ();
        exit ( 1 );
    }
}




/*****************************************************************************/
/*                                                                           */
/*  Funktion     : ReadGanzzahl                                              */
/*                                                                           */
/*  Parameter    : keine                                                     */
/*                                                                           */
/*  Returnwert   : Integerzahl                                               */
/*                                                                           */
/*  Beschreibung :  Es gilt folgende Vorbedingung : Beim Start muss das      */
/*                  naechste Symbol nach beliebig vielen Zeilenvorschueben   */
/*                  ( Symbol SCRSYM ) eine Integerzahl sein; ansonsten er-   */
/*                  folgt eine Fehlermeldung.                                */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static int ReadGanzzahl( void )
{
  Symbol sym;
  char   ident[IDENTLENGTH];

  SkipSpace( f_descr, ident, sym);

  CheckError ( ( sym != SIDENT) || !IsNumber (ident), "Ganzahl erwartet." );
  return  atoi (ident);
} /* Ende von ReadGanzzahl */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     : ReadZahlPaar                                              */
/*                                                                           */
/*  Parameter    : 2 Pointer auf Integerzahlen, in die Ganzzahlen geschrieben*/
/*                 werden.                                                   */
/*                                                                           */
/*  Returnwert   : keine                                                     */
/*                                                                           */
/*  Beschreibung : Es gilt folgende Vorbedingung :  Beim Start duerfen be-   */
/*                 liebig viele Zeilenvorschuebe kommen; anschliessend muss  */
/*                 diese Zeichenfolge kommen :                               */
/*                           (  <GANZZAHL>   ,  <GANZZAHL>  )                */
/*                 Ist dies nicht der Fall, erfolgt eine Fehlermeldung.      */
/*                                                                           */
/*  Globale Var. : keine                                                     */
/*                                                                           */
/*****************************************************************************/

static void ReadZahlPaar( short *zahl1, short *zahl2 ) 
{
  Symbol sym;
  char   ident[IDENTLENGTH];

  SkipSymbol( f_descr, ident, sym, SBRACKET_L, '(' );

  *zahl1 = ReadGanzzahl();

  SkipSymbol( f_descr, ident, sym, SCOMMA, Komma);

  *zahl2 = ReadGanzzahl();

  SkipSymbol( f_descr, ident, sym, SBRACKET_R, ')');
} /* Ende von ReadZahlPaar */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_10                                                   */
/*                                                                           */
/*  Parameter    :  Integerzahl                                              */
/*                                                                           */
/*  Returnwert   :  Realzahl                                                 */
/*                                                                           */
/*  Beschreibung :  es wird die durch den Parameter angegebene Zehnerpotenz  */
/*                  berechnet.                                               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

double exp_10 ( int exp )
{
  double erg = 1;
  int i;

  for(i=0;i<abs(exp);i++)
  {
    erg *= 10.0;
  }
  
  return (exp<0 ? 1.0/erg : erg );
} /* Ende von exp_10 */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadRealzahl                                             */
/*                                                                           */
/*  Parameter    :  - Pointer auf eine real-Zahl, in die im positiven Fall   */
/*                    der Wert der gelesenen real-Zahl geschrieben wird.     */
/*                  - ein Pointer auf char; dort wird nach dieser Funktion   */
/*                    der Identifier abgelegt, der zuletzt gelesen wurde und */
/*                    nicht mehr zur Realzahl gehoert und kein CR ist.       */
/*                    Es sei denn, das Dateiende ( EOI( f_descr ) = true ) wird er-     */
/*                    reicht.                                                */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    der Realzahl gehoert und nicht SCRSYM ist, ausser beim */
/*                    Dateiende.                                             */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Voraussetzung ist, dass beim Start das naechste Symbol   */
/*                  nach beliebig vielen Zeilenvorschueben ( Symbol SCRSYM ) */
/*                  eine real-Zahl sein muss, ansonsten erfolgt eine         */
/*                  Fehlermeldung.                                           */
/*                  Es wird zunaechst eine Ganzzahl gelesen; folgt danach    */
/*                  ein Punkt, so wird versucht eine zweite Ganzzahl zu      */
/*                  lesen; dies ist dann die Nachkommastellen.               */
/*                  Beachte wird ein Punkt verwendet, so muss vor und nach   */
/*                  ihm eine Ganzahl stehen. -> 1. und .5 sind ungueltig!    */
/*                  Besondere Massnahmen sind erfordelich, wenn eine Realzahl*/
/*                  als letztes Symbol einer Datei eingelesen werden soll!!  */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void  ReadRealzahl ( double *real, char *id, Symbol *s )
{

  /* In dieser Funktion wird nicht die Funktion ReadGanzzahl aufgerufen,     */
  /* weil die Fehlermeldung hier falsch waere!                               */

  SkipSpace( f_descr, id, *s );
  
  CheckError ( ( *s != SIDENT) || !IsNumber (id), "Realzahl erwartet." );
  *real = atoi(id);    

  *s = GetSymbol( f_descr, id);

  if ( *s != SPUNKT )     /* Zahl hat nur Vorkommastellen */
  {
    while(!EOI( f_descr ) && (*s == SCRSYM ) )
    {
      *s = GetSymbol( f_descr,  id );
    }
  }
  else
  {
    *s = GetSymbol( f_descr, id);
    
    CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
    CheckError ( ( *s != SIDENT) || !IsNumber (id), "Realzahl erwartet." );
    *real += atoi(id)/exp_10(strlen(id));

    while ( !EOI( f_descr ) && ( ( *s = GetSymbol( f_descr,  id ) ) == SCRSYM ));
    /* Leerer while-Schleifenkoerper */
  }
}  /* Ende von ReadRealzahl */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  parameter_anz_best                                       */
/*                                                                           */
/*  Parameter    :  Symbol einer CP/CG-Auswahlfunktion                       */
/*                                                                           */
/*  Returnwert   :  Anzahl der Parameter, die fuer diese Funktion erwartet   */
/*                  werden.                                                  */
/*                                                                           */
/*  Beschreibung :  Bestimmung mit einer Switchanweisung                     */
/*                  Beachte : im Vergleich zur frueheren Parameterbestimmung */
/*                            ist dies einer weniger, da die Ordnung weg-    */
/*                            faellt!                                        */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  FuncCount                                                */
/*                                                                           */
/*****************************************************************************/

static short parameter_anz_best ( short fkts_nr )
{
   switch( fkts_nr )
   {
   case SX_FIFO                 :
   case SX_TEAM_FIFO            : 
      return 0;
   case SX_GLOBAL_LEARN         :  
      return 7;  /* Just temporary - may change... */
   case SX_GOAL_BOUND_LEARN     :
      return 8;
   case SX_SPEC_BOUND_LEARN     :
      return 9;
   case SX_KBO_ADD              :
   case SX_KBO_MAX              :
   case SX_KBO_GT               :
   case SX_DIVERGENCE           : 
      return 1;

   case SX_2TET_LEARN           :
   case SX_MAX_RWEIGHT          :
   case SX_GT_RWEIGHT           :
   case SX_ADD_EWEIGHT          :
   case SX_MAX_EWEIGHT          :
   case SX_GT_EWEIGHT           :
   case SX_ADD_FWEIGHT          :
   case SX_MAX_FWEIGHT          :
   case SX_GT_FWEIGHT           :
   case SX_ADD                  :
   case SX_MAX                  :
   case SX_GT                   :
   case SX_DIFF_GOAL            :
   case SX_UNIF_GOAL            :  
      return 3;
      
   case SX_1TET_LEARN           :
   case SX_OCCNEST              :
   case SX_SPEC_ADD             :
   case SX_SPEC_MAX             :
   case SX_SPEC_GT              :
   case SX_ADD_RWEIGHT          :
   case SX_GOALMATCH            :
   case SX_REDUCE_CP            :
   case SX_REDUCE_CPP           :  
      return 4;  /* alte Parameterzahl */
      
   case SX_DIFF                 :  
      return 5;
      
   case SX_GOALSIM              :  
      return 6;
      
   case SX_GOALTEST             :  
      return 11;
      
   case SX_POLY_ADD             :
   case SX_POLY_MAX             :
   case SX_POLY_GT              :  
      return FuncCount+2;

   default                      :
      Error(" parameter_anz_best ", "Unbekannter Experte" );
      return -1;
   }
   
} /* Ende von parameter_anz_best */


/*****************************************************************************/
/*                                                                           */
/*   Funktion     : Read_Param_list                                          */
/*                                                                           */
/*   Parameter    : - Pointer auf ein Feld von Elementen, die vom Typ        */
/*                    ParameterElement sind.                                 */
/*                  - ein Pointer auf char; dort wird nach dieser Funktion   */
/*                    der Identifier abgelegt, der zuletzt gelesen wurde und */
/*                    nicht mehr zu der Parameterliste gehoert.              */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    der Parameterliste gehoert.                            */
/*                                                                           */
/*   Returnwert   : Anzahl der eingelesenen Parameter; maximal MAXPARAM      */
/*                                                                           */
/*   Beschreibung : Diese Funktion liest eine ganze Liste von Parametern ein,*/
/*                  und speichert sie in der angegebenen Variablen ab, wo-   */
/*                  bei die Zahl MAXPARAM nicht ueberschritten werden darf.  */
/*                                                                           */
/*                  Es gilt folgende Vorbedingung : Beim Start muss das      */
/*                  naechste Symbol nach beliebig vielen Zeilenvorschueben   */
/*                  ( Symbol SCRSYM ) ein Identifier sein und zwar der Name  */
/*                  des ersten Parameters; diese Liste kann auch leer sein.  */
/*                  Da man das naechste Zeichen lesen muss, um zu erkennen,  */
/*                  ob die Parameterliste zu Ende ist, wird dieses ueber die */
/*                  Parameter im Funktionskopf als Identifyer und Symbol     */
/*                  uebergeben.                                              */
/*                                                                           */
/*   Globale Var. : keine                                                    */
/*                                                                           */
/*****************************************************************************/

static short Read_Param_list ( ParameterElement *paramliste,
			       char *id, Symbol *s)
{
  char ident[IDENTLENGTH];
  Symbol sym;
  register int i = 0;

  SkipSpace( f_descr, ident,sym);

  while ( (sym == SIDENT ) && (i < MAXPARAM) )
  {

  /***************************/
  /* Parameternamen einlesen */
  /***************************/
    if (!strncpy( paramliste[i].name, ident, IDENTLENGTH))
    {
      Error ( __FILE__ ": " "Read_Param_list", "Parametername zu lang." );
    }

    SkipSymbol( f_descr, ident,sym,SCOLLON,':');
      
  /******************/
  /* range einlesen */
  /******************/
    paramliste[i].bereich_anfang = ReadGanzzahl();

    SkipSymbol( f_descr, ident,sym,SCOMMA,Komma);

    paramliste[i].bereich_ende = ReadGanzzahl();

    SkipSymbol( f_descr, ident,sym,SSEMICOLLON,';');

  /*************************/
  /* default-Wert einlesen */
  /*************************/
    paramliste[i].normal = ReadGanzzahl();

    SkipSpace( f_descr, ident,sym);

    i++;
  } /* Ende der while-Schleife */

  strncpy( id, ident, IDENTLENGTH );
  *s = sym;

  return i;
} /* Ende von Read_Param_list */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     : ReadExp                                                   */
/*                                                                           */
/*  Parameter    :  - Pointer auf ein Feld, das die Experteninformationen    */
/*                    aufnehmen kann.                                        */
/*                  - ein Pointer auf char; dort wird nach dieser Funktion   */
/*                    der Identifier abgelegt, der zuletzt gelesen wurde und */
/*                    nicht mehr zu der Expertenliste gehoert.               */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    der Expertenliste gehoert.                             */
/*                                                                           */
/*  Returnwert   : Anzahl der eingelesenen Experten; maximal MAXEXP_ANZ      */
/*                                                                           */
/*  Beschreibung : Diese Funktion liest eine ganze Liste von Experten zu-    */
/*                 sammen mit deren Informationen ein und speichert sie in   */
/*                 der angegebene Variablen ab.                              */
/*                                                                           */
/*                 Es gilt folgende Vorbedingung : Beim Start muss das       */
/*                 naechste Symbol nach beliebig vielen Zeilenvorschueben    */
/*                 ( Symbol SCRSYM ) ein Identifier sein und zwar eine       */
/*                 oeffnende Klammer.                                        */
/*                                                                           */
/*  Globale Var. : keine                                                     */
/*                                                                           */
/*****************************************************************************/

static short ReadExp ( ObjectInFrameInfo *exp_liste, char *id, Symbol *s )
{
  char   ident[IDENTLENGTH];
  int    i = 0;
  Symbol sym;

  SkipSpace(f_descr, ident, sym);


  while ( (sym == SBRACKET_L) && (i < MAXEXP_ANZ) )
  {
    /**************************/
    /* Expertennamen einlesen */
    /**************************/
    /* Damit bei den Expertennamen auch die bereits vorbelegten Namen wie z.B. */
    /* MAX_WEIGHT verwendet werden koennen, wird die Symboltabelle in dieser   */
    /* Funktion geloescht.                                                     */
    SetScanMode( f_descr, NULL_MODE );

    SkipSymbol( f_descr, ident, sym, SIDENT, Expertennamen );
    strcpy( exp_liste[i].name, ident );

    SetScanMode( f_descr, EXP_MODE );

    SkipSymbol( f_descr, ident, sym, SCOMMA, Komma);
    exp_liste[i].param_anz = Read_Param_list(exp_liste[i].paramliste, ident, &sym);

    /* Beachte Read_Param_list liest bereits ein Zeichen weiter */
    CheckError( (sym != SCOMMA ), "Komma erwartet.");

    ReadRealzahl(&(exp_liste[i].guete), ident, &sym);

    /* Beachte ReadRealzahl liest bereits ein Zeichen weiter */
    CheckError( (sym != SBRACKET_R), " ')' erwartet.");

    i++;

    SkipSpace(f_descr, ident, sym);
  } /* Ende der while-Schleife */

  strncpy( id, ident, IDENTLENGTH );
  *s = sym;

  return i;
} /* Ende von ReadExp */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     : ReadGa                                                    */
/*                                                                           */
/*  Parameter    :  - Pointer auf ein Feld, das die Gutachterinformationen   */
/*                    aufnehmen kann.                                        */
/*                  - ein Pointer auf char; dort wird nach dieser Funktion   */
/*                    der Identifier abgelegt, der zuletzt gelesen wurde und */
/*                    nicht mehr zu der Gutachterliste gehoert.              */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    der Gutachterliste gehoert.                            */
/*                                                                           */
/*  Returnwert   : Anzahl der eingelesenen Gutachter; maximal MAXGA_ANZ      */
/*                                                                           */
/*  Beschreibung : Diese Funktion liest eine ganze Liste von Gutachtern zu-  */
/*                 sammen mit deren Informationen ein und speichert sie in   */
/*                 der angegebene variablen ab.                              */
/*                                                                           */
/*                 Es gilt folgende Vorbedingung : Beim Start muss das       */
/*                 naechste Symbol nach beliebig vielen Zeilenvorschueben    */
/*                 ( Symbol SCRSYM ) ein Identifier sein und zwar            */
/*                 GUTE_GUTACHTER.                                           */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static short ReadGa( ObjectInFrameInfo *ga_liste, char *id, Symbol *s )
{
  Symbol sym;
  char   ident[IDENTLENGTH];
  int    i = 0;

  SkipSpace( f_descr, ident,sym);

  while ( (sym == SBRACKET_L) && (i < MAXGA_ANZ) )
  {

    /* Gutachternamen einlesen */
    SkipSymbol( f_descr, ident, sym, SIDENT, Gutachternamen );
    strcpy( ga_liste[i].name, ident );

    SkipSymbol( f_descr, ident, sym, SCOMMA, Komma);
    ga_liste[i].param_anz = Read_Param_list(ga_liste[i].paramliste, ident, &sym);

    /* Beachte Read_Param_list liest bereits ein Zeichen weiter */
    CheckError( (sym != SCOMMA ), "Komma erwartet.");

    ReadRealzahl(&(ga_liste[i].guete), ident, &sym );

    /* Beachte ReadRealzahl liest bereits ein Zeichen weiter */
    CheckError( (sym != SBRACKET_R), " ')' erwartet.");

    i++;

    SkipSpace( f_descr, ident,sym);
  } /* Ende der while-Schleife */

  strncpy( id, ident, IDENTLENGTH );
  *s = sym;

  return i;
} /* Ende von ReadGa */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     : ReadDom                                                   */
/*                                                                           */
/*  Parameter    :  - Pointer auf ein Feld, das die Domaeneninformationen    */
/*                    aufnehmen kann.                                        */
/*                  - ein Pointer auf char; dort wird nach dieser Funktion   */
/*                    der Identifier abgelegt, der zuletzt gelesen wurde und */
/*                    nicht mehr zu der Domaenenliste gehoert.               */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    der Domaenenliste gehoert.                             */
/*                                                                           */
/*  Returnwert   : Anzahl der eingelesenen Domaenen; maximal MAXDOM_ANZ      */
/*                                                                           */
/*  Beschreibung : Diese Funktion liest eine ganze Liste von Domaenen zu-    */
/*                 sammen mit deren Informationen ein und speichert sie in   */
/*                 der angegebene Variablen ab.                              */
/*                                                                           */
/*                 Es gilt folgende Vorbedingung : Beim Start muss das       */
/*                 naechste Symbol nach beliebig vielen Zeilenvorschueben    */
/*                 ( Symbol SCRSYM ) ein Identifier sein und zwar eine       */
/*                 GUTE_DOMAENEN.                                            */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*****************************************************************************/


static short ReadDom( ObjectInFrameInfo *dom_liste, char *id, Symbol *s )
{
  Symbol sym;
  char   ident[IDENTLENGTH];
  int    i = 0;

  CheckError( (sym != SGUTE_DOMAENEN), "GUTE_DOMAENEN erwartet.");

  SkipSpace( f_descr, ident,sym);

  while ( (sym == SBRACKET_L) && (i < MAXDOM_ANZ) )
  {

    /**************************/
    /* Domaenennamen einlesen */
    /**************************/
    SkipSymbol( f_descr, ident, sym, SIDENT, Domaenennamen);
    strcpy( dom_liste[i].name, ident );

    SkipSymbol( f_descr, ident, sym, SCOMMA, Komma);

    ReadRealzahl(&(dom_liste[i].guete), ident, &sym );

    /* Beachte ReadRealzahl liest bereits ein Zeichen weiter */
    CheckError( (sym != SBRACKET_R), " ')' erwartet.");

    i++;

    SkipSpace( f_descr, ident,sym);
  } /* Ende der while-Schleife */

  strncpy( id, ident, IDENTLENGTH );
  *s = sym;

  return i;
} /* Ende von ReadDom */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadPolynomExp                                           */
/*                                                                           */
/*  Parameter    :                                                           */
/*                                                                           */
/*  Returnwert   :                                                           */
/*                                                                           */
/*  Beschreibung :  Siehe auch wie das im alten team.c realisiert war.       */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static short ReadPolynomExp ( ParameterElement *paramliste,
			       char *id, Symbol *s)
{
  return 0;
} /* Ende von ReadPolynomExp */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  Read_Param_list2                                         */
/*                                                                           */
/*  Parameter    : - Pointer auf ein Feld von Elementen, die vom Typ         */
/*                   ParameterElement sind.                                  */
/*                 - ein Pointer auf char; dort wird nach dieser Funktion    */
/*                   der Identifier abgelegt, der zuletzt gelesen wurde und  */
/*                   nicht mehr zu der Parameterliste gehoert.               */
/*                 - Pointer auf Symbol, das nach dieser Funktion das Symbol */
/*                   enthaelt, das zuletzt gelesen wurde und nicht mehr zu   */
/*                   der Parameterliste gehoert.                             */
/*                                                                           */
/*  Returnwert   :  Anzahl der eingelesenen Parameter; maximal MAXPARAM      */
/*                                                                           */
/*  Beschreibung :  Es werden zunaechst die Parameter eingelesen, die        */
/*                  RedInst, Func-Weight und Var-Weight festlegen.           */
/*                  Anschliesend werden dann die eventuell angegebenen       */
/*                  speziellen Funktionssymbole und deren Gewichte eingelesen*/
/*                  Dabei muss, wenn das Schluesselwort F_GEW_LIST angegeben */
/*                  wird, auch wenigstens eine Funktionssymbol in der Liste  */
/*                  vorkommen!                                               */
/*                  Die Belegung innerhalb der Parameterliste ist dabei wie  */
/*                  folgt :                                                  */
/*                  Die ersten drei Parameter sind durch die 3 Standard-Werte*/
/*                  belegt( Intervall und default-Wert );                    */
/*                  Die Elemete der Liste mit den speziellen Funktionssymbo- */
/*                  len werden wie folgt abgelegt :                          */
/*                  Zuerst wird die Nummer des Funktionssymbols gespeichert  */
/*                  und im naechsten Parameter das zugehoerige Gewicht.      */
/*                  Die Speicherung erfolgt in der Komponente "normal".      */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static short Read_Param_list2( ParameterElement *paramliste,
			       char *id, Symbol *s)
{
  char ident[IDENTLENGTH];
  Symbol sym;
  register int i = 0;
/* Anzahl der Parameter, die ausser den speziellen Funktionssymb gelesen wurden : */
  function func;

  SkipSpace( f_descr, ident,sym);

  /* Einlesen von RedInst, Func-Weight und Var-Weight */
  while ( (sym == SIDENT ) && (i < MAXPARAM) && strcmp( ident, "F_GEW_LIST" ) )
  {
  /***************************/
  /* Parameternamen einlesen */
  /***************************/
    if (!strncpy( paramliste[i].name, ident, IDENTLENGTH))
    {
      Error ( __FILE__ ": " "Read_Param_list2", "Parametername zu lang." );
    }
  
    SkipSymbol( f_descr, ident,sym,SCOLLON,':');

  /******************/
  /* range einlesen */
  /******************/
    paramliste[i].bereich_anfang = ReadGanzzahl();

    SkipSymbol( f_descr, ident,sym,SCOMMA,Komma);

    paramliste[i].bereich_ende = ReadGanzzahl();

    SkipSymbol( f_descr, ident,sym,SSEMICOLLON,';');

  /*************************/
  /* default-Wert einlesen */
  /*************************/
    paramliste[i].normal = ReadGanzzahl();

    i++;

    SkipSpace( f_descr, ident,sym);
  } /* Ende der while-Schleife */
 
  /* Die Liste mit den Funktionssymbolen mit speziellem Gewicht muss als     */
  /* letzter Parameter angegeben werden.                                     */

  if ( !strcmp( ident, "F_GEW_LIST" ) )
  {  /* Die Liste darf nicht leer sein, sonst darf F_GEW_LIST nicht als Para-*/
     /* metername vorkommen.                                                 */

    SkipSymbol( f_descr, ident,sym,SCOLLON,':');
    do
    {
      strncpy( paramliste[i].name, "F_GEW_LIST", IDENTLENGTH);
      /* Lesen des Identifiers des Funktionssymbols */
      /* Die Nummer des Funktionssymbols wird in "bereich_anfang" eingetragen */
      SkipSpace( f_descr, ident,sym);

      /* Wenn ein Funktionssymbol angegeben wird, das in der Spezifikation   */
      /* nicht vorkommt, wird nur eine Warnung ausgegeben und das angegebene */
      /* Gewicht uebergangen.                                                */
      /* Damit ist es moeglich einen Experten mit in eine Standardkonfigura- */
      /* tionsdatei aufzunehmen, der nicht fuer alle Beispiele geeignet ist, */
      /* er kann Gewichte fuer Funktionssymbole angegeben, die nur in einigen*/
      /* Beispielen vorkommen.                                               */
      /************************************************************/
      /* Testen, ob Funktionssymbol in der Spezifikation vorkommt */
      /************************************************************/
      func = FindFunction (ident);

      if( !func )
      { /* Funktionssymbol ist nicht aufgefuehrt */
	printf("\n%c******  Warnung: Unbekannte Funktion bei Angabe des Gewichts.\n", (char)7 );
	printf("******           Angabe wird ignoriert!\n\n");
	/************************************/
	/* Ueberspringen der Gewichtsangabe */
	/************************************/
	SkipSymbol( f_descr, ident,sym,SCOLLON,':');

	(void)ReadGanzzahl();
      } /* Ende von if */
      else
      { /* Funktionssymbol ist aufgefuehrt */

	paramliste[i++].normal = func;

	SkipSymbol( f_descr, ident,sym,SCOLLON,':');

	/**************************************/
	/* Einlesen des zugeordneten Gewichts */
	/**************************************/
	/* Das Gewicht wird in "bereich_ende" abgespeichert */
	paramliste[i++].normal = ReadGanzzahl();
      } /* Ende von else */

      while ((sym = GetSymbol( f_descr, ident)) == SCRSYM)
      {
	CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
      }

    } while ( sym == SSEMICOLLON );

  }
  strncpy( id, ident, IDENTLENGTH );
  *s = sym;

  return i;

} /* Ende von Read_Param_list2 */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadParameter                                            */
/*                                                                           */
/*  Parameter    : - Symbol einer CP- oder CG- Funktion                      */
/*                 - Pointer auf ein Feld von Elementen, die vom Typ         */
/*                   ParameterElement sind.                                  */
/*                 - ein Pointer auf char; dort wird nach dieser Funktion    */
/*                   der Identifier abgelegt, der zuletzt gelesen wurde und  */
/*                   nicht mehr zu der Parameterliste gehoert.               */
/*                 - Pointer auf Symbol, das nach dieser Funktion das Symbol */
/*                   enthaelt, das zuletzt gelesen wurde und nicht mehr zu   */
/*                   der Parameterliste gehoert.                             */
/*                                                                           */
/*  Returnwert   :  Anzahl der eingelesenen Parameter; maximal MAXPARAM      */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur liest die primaeren Daten eines Experten  */
/*                  aus der angegebenen Datei und speichert sie in dem ange- */
/*                  gebenen Experten.                                        */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static short ReadParameter( short fkt, ParameterElement *paramliste,
			    char *ident, Symbol *sym)
{
   short param_anzahl = -1;
   short korr_param_anz; /* korrekte Parameteranzahl, d.h. die, die fuer   */
   /* die CP/CG-Funktion erwartet wird.              */
   
   korr_param_anz = parameter_anz_best( fkt );
   
   switch( fkt )
   {
   case SX_FIFO         :
   case SX_TEAM_FIFO    :
      param_anzahl = 0;
      SkipSpace( f_descr, ident, *sym);
      break;
   case SX_POLY_ADD     :
   case SX_POLY_MAX     :
   case SX_POLY_GT      :  
      param_anzahl = ReadPolynomExp( paramliste, ident, sym );
      break;
      
   case SX_ADD_RWEIGHT  :
   case SX_MAX_RWEIGHT  :
   case SX_GT_RWEIGHT   :
   case SX_ADD_EWEIGHT  :
   case SX_MAX_EWEIGHT  :
   case SX_GT_EWEIGHT   :
   case SX_ADD_FWEIGHT  :
   case SX_MAX_FWEIGHT  :
   case SX_GT_FWEIGHT   :
   case SX_DIVERGENCE   :  
      param_anzahl = Read_Param_list2( paramliste, ident, sym );
      break;
      
   case SX_GLOBAL_LEARN :
   case SX_GOAL_BOUND_LEARN:
   case SX_SPEC_BOUND_LEARN:
   case SX_1TET_LEARN   :
   case SX_2TET_LEARN   :
   case SX_OCCNEST      :
   case SX_ADD          :
   case SX_MAX          :
   case SX_GT           :
   case SX_DIFF         :
   case SX_SPEC_ADD     :
   case SX_SPEC_MAX     :
   case SX_SPEC_GT      :
   case SX_KBO_ADD      :
   case SX_KBO_MAX      :
   case SX_KBO_GT       :
   case SX_GOALMATCH    :
   case SX_GOALSIM      :
   case SX_GOALTEST     :  
   case SX_UNIF_GOAL    :
   case SX_DIFF_GOAL    : 
      param_anzahl = Read_Param_list( paramliste, ident, sym );
      CheckError( (korr_param_anz != param_anzahl),
		 " Falsche Parameteranzahl! " );
      break;
   default              : Error ( __FILE__ ": " "ParseExpert", "Unbekannter Funktionsname." );
      break;
   } /* Ende von switch */
   
   return param_anzahl;
} /* Ende von ReadParameter */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ParseExpert                                              */
/*                                                                           */
/*  Parameter    :  - Name einer Datei, aus der gelesen werden soll          */
/*                  - Pointer auf Experten, in den die gelesenen Daten ge-   */
/*                    schrieben werden.                                      */
/*                                                                           */
/*  Returnwert   :  Typ des gelesenen Experten                               */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur liest die primaeren Daten eines Experten  */
/*                  aus der angegebenen Datei und speichert sie in dem ange- */
/*                  gebenen Experten.                                        */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
ExpertType ParseExpert ( char *filename , ExpertFrame *exp)
{
  char          ident[IDENTLENGTH];
  Symbol        sym;
  ExpertFrame  *expert; /* Pointer auf die Struktur, in die die Daten des    */
	   	        /* Rahmens geschrieben werden.                       */
  ExpertType    exp_type = EXP;

  /**************************************/
  /* Erzeugen des kompletten Pfadnamens */
  /**************************************/
  sprintf( Filename, "%s/%s.%s", ExpertDir, filename, EXPFILESUFFIX );

  if( !DemoMode )
  {
    printf("ExpertenDatei %s lesen\n", Filename );
  }

  /**********************************************************/
  /* Initialisieren der Symboltabelle und Oeffnen der Datei */
  /**********************************************************/

  f_descr = OpenInput ( Filename );
  SetScanMode ( f_descr, EXP_MODE );

  /*******************************/
  /* Einlesen des Expertennamens */
  /*******************************/

  SkipSymbol( f_descr, ident, sym, SEXPERTENNAME, EXPERTENNAME);
  GetFilename ( f_descr, ident );
  /* Dieser Name mu"s mit dem Namen der Datei "ubereinstimmen, um Verwir-   */
  /* rungen zu vermeiden!                                                   */
  CheckError( strcmp(ident, filename), 
	      "Name des Expertenrahmen stimmt nicht mit dem Namen der Datei ueberein!");
  
  /* "Uberpr"ufen, ob es sich um den Rahmen des Database-Experten handelt */
  if( !strcmp( filename, "DATABASE" ) )
  {
    CheckError( Database.spezialist, "Es darf nur ein Database-Spezialist angegeben werden!");

    /*******************************************************/
    /* Es wird der Rahmen des Database-Experten eingelesen */
    /*******************************************************/
    SkipSymbol( f_descr, ident, sym, SEXP_OR_SPEC, EXP_OR_SPEC );
    sym = GetSymbol( f_descr, ident );
    CheckError( strcmp( ident, "S"), "'S' erwartet.");

    strcpy( Database.exp_name, filename );

    Database.spezialist = true;

    exp_type = DAT;

    SkipSymbol( f_descr, ident, sym, SCP_FUNKTION, CP_FUNKTION);

    SetScanMode( f_descr, DATA_MODE );

    Database.cpfkt_nr = GetSymbol( f_descr, ident );
    CheckError(  (Database.cpfkt_nr != SFIND_ALL) &&
		 (Database.cpfkt_nr != SFIND_FIRST), "FIND_ALL oder FIND_FIRST erwartet\n");

    CloseInput ( f_descr );

    return exp_type;
  } /* Ende von if( !strcmp( filename, "DATABASE" ) ) */
  else
  {
    /*********************************************************************/
    /* Es liegt ein Reduktionsspezialist oder ein "normaler" Experte vor */
    /*********************************************************************/
    expert = exp;

    SkipSymbol( f_descr, ident, sym, SEXP_OR_SPEC, EXP_OR_SPEC );
    sym = GetSymbol( f_descr, ident );
    expert->spezialist = ( strcmp("S", ident) == 0 );

  } /* Ende von else */
  

  /**********************************************************/
  /* Einlesen der CP-Funktion und der angegebenen Parameter */
  /**********************************************************/

  SkipSymbol( f_descr, ident, sym, SCP_FUNKTION, CP_FUNKTION);
  expert->cpfkt_nr = GetSymbol( f_descr, ident );
  CheckError( !Check_Symbol( ident, CHECK_EXP ), "Unbekannte CP-Funktion." );

  CheckError( (expert->cpfkt_nr == SX_REDUCE_CGP || expert->cpfkt_nr == SX_REDUCE_CG) , 
	      "REDUCE_CG oder REDUCE_CGP als CP-Funktion nicht erlaubt!" );
  if( expert->cpfkt_nr == SX_REDUCE_CP || expert->cpfkt_nr == SX_REDUCE_CPP  ||
      expert->cpfkt_nr == SX_NONE )
  {
    CheckError( RedSpecCount >= MAXREDSPEC - 1, "Zu viele Reduktionsspezialisten angegeben!");
    CheckError( exp_type == DAT, "NONE, REDUCE_CP oder REDUCE_CPP nicht beim Database-Experten!");

    /*****************************************/
    /* Es liegt ein Reduktionsspezialist vor */
    /*****************************************/
    CheckError( !expert->spezialist, "REDUCE_CP oder REDUCE_CPP als CP-Funktion, aber nicht als Spezialist gekennzeichnet!");

    strcpy( RedSpecInfo[RedSpecCount].name, filename );
    exp_type = RED;
    RedSpecInfo[RedSpecCount].reduce_cp_fkt = expert->cpfkt_nr;

    /*********************************************************/
    /* Lesen der Parameter, falls nicht NONE angegeben wurde */
    /*********************************************************/
    if( RedSpecInfo[RedSpecCount].reduce_cp_fkt != SX_NONE )
    {
      RedSpecInfo[RedSpecCount].red_typ = CP;

      SetScanMode( f_descr, SPEC_MODE );

      SkipSymbol( f_descr, ident, sym, SSTART, START);
      RedSpecInfo[RedSpecCount].cp_start  = ReadGanzzahl();

      SkipSymbol( f_descr, ident, sym, SREDUCE, REDUCE);
      RedSpecInfo[RedSpecCount].subsumcp  = (ReadGanzzahl() == 1);

      SkipSymbol( f_descr, ident, sym, SSUBSUM, SUBSUM);
      RedSpecInfo[RedSpecCount].reducecp  = (ReadGanzzahl() == 1);

      SkipSymbol( f_descr, ident, sym, SDOUBLE, DOUBLE);
      RedSpecInfo[RedSpecCount].doublecp  = (ReadGanzzahl() == 1);

      SetScanMode ( f_descr, EXP_MODE );
    } /* Ende von if( RedSpecInfo[RedSpecCount].reduce_cp_fkt != SX_NONE ) */

    /*****************************************************/
    /* Lesen der Reduktionsfunktion f"ur kritische Ziele */
    /*****************************************************/

    SkipSymbol( f_descr, ident, sym, SCG_FUNKTION, CG_FUNKTION);

    sym = GetSymbol( f_descr, ident );

    CheckError( ((sym != SX_NONE) && (sym != SX_REDUCE_CG) && (sym != SX_REDUCE_CGP)),
		"Falsche CG-Funktion bei Reduktionsspezialist.");
    CheckError( ((sym == SX_NONE) && (RedSpecInfo[RedSpecCount].reduce_cp_fkt == SX_NONE)),
		"Keine Reduktionsfunktion fuer kritische Paare oder kritische Ziele angegeben!");
    CheckError( (((sym == SX_REDUCE_CG) || (sym == SX_REDUCE_CGP)) && !Paramodulation ),
		"Reduktionsspezialist fuer krit. Ziele, aber keine Paramodulation.");

    RedSpecInfo[RedSpecCount].reduce_cg_fkt = sym;

    if( RedSpecInfo[RedSpecCount].reduce_cg_fkt != SX_NONE )
    {
      CheckError( !Paramodulation, "Kritische Ziel-Behandlung nur bei Paramodulation!");
      CheckError( RedSpecInfo[RedSpecCount].red_typ == CP, 
		  "Entweder Reduktionsspezialist fuer krit. Paare oder krit. Ziele, aber nicht beides.");

      RedSpecInfo[RedSpecCount].red_typ = CG;

      SetScanMode( f_descr, SPEC_MODE );

      SkipSymbol( f_descr, ident, sym, SSTART, START);
      RedSpecInfo[RedSpecCount].cg_start  = ReadGanzzahl();

      SkipSymbol( f_descr, ident, sym, SREDUCE, REDUCE);
      RedSpecInfo[RedSpecCount].subsumcg  = (ReadGanzzahl() == 1);

      SkipSymbol( f_descr, ident, sym, SSUBSUM, SUBSUM);
      RedSpecInfo[RedSpecCount].reducecg  = (ReadGanzzahl() == 1);

      SkipSymbol( f_descr, ident, sym, SDOUBLE, DOUBLE);
      RedSpecInfo[RedSpecCount].doublecg  = (ReadGanzzahl() == 1);
    }

    CloseInput ( f_descr );

    return exp_type;
  } /* Ende von if -> Reduktionsspezialist liegt vor */
  
  /**************************************/
  /* "Normaler" Experte wird eingelesen */
  /**************************************/
  strcpy( expert->exp_name, filename );
  expert->cpparam_anz = ReadParameter( expert->cpfkt_nr, expert->cpparamliste, ident, &sym );

  /**********************************************************/
  /* Einlesen der CG-Funktion und der angegebenen Parameter */
  /**********************************************************/
  /* Beachte Read_Param_list hat das naechste Symbol bereits gelesen */

  while (sym == SCRSYM)
  {
    CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
    sym = GetSymbol( f_descr, ident);
  }
  CheckError ( (sym != SCG_FUNKTION), "CG_FUNKTION erwartet." );

  expert->cgfkt_nr = GetSymbol( f_descr, ident );
  CheckError( !Check_Symbol( ident, CHECK_EXP ), "Unbekannter Experte bei CG-Funktion." );

  expert->cgparam_anz = ReadParameter( expert->cgfkt_nr, expert->cgparamliste, ident, &sym );

  /*************************/
  /* Einlesen der Fairness */
  /*************************/
  /* Beachte Read_Param_list hat das naechste Symbol bereits gelesen */

  while (sym == SCRSYM)
  {
    CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
    sym = GetSymbol( f_descr, ident);
  }
  
  CheckError ( (sym != SFAIRNESS), "FAIRNESS erwartet." );

  SkipSpace( f_descr, ident, sym);
  CheckError( ( (strcmp(ident, "Y") != 0) && (strcmp(ident, "N") != 0)), 
	       "Y oder N erwartet.");
  expert->fair = ( !strcmp(ident, "Y") ? true : false );


  /****************************************/
  /* Einlesen der gutgeeigneten Gutachter */
  /****************************************/

  SkipSymbol( f_descr, ident, sym, SGUTE_GUTACHTER, GUTE_GUTACHTER);

  expert->ga_anz = ReadGa(expert->geeignete_ga, ident, &sym);

  /****************************************/
  /* Einlesen der gutgeeigneten Domaenen  */
  /****************************************/

  expert->dom_anz = ReadDom(expert->domaenen, ident, &sym);

  /***************************/
  /* Einlesen der Robustheit */
  /***************************/

  CheckError( (sym != SROBUSTHEIT), "ROBUSTHEIT erwartet.");
  ReadRealzahl(&(expert->robust), ident, &sym );
  while( sym == SCRSYM )
  {
    sym = GetSymbol( f_descr, ident);
  }

  /******************************/
  /* Einlesen des Wissensanteil */
  /******************************/

  CheckError( (sym != SWISSENSANTEIL), "WISSENSANTEIL erwartet.");
  ReadRealzahl(&(expert->wissen), ident, &sym );

  /****************************************/
  /* Einlesen des Beweisphasenbeurteilung */
  /****************************************/

  CheckError( (sym != SBEWEISPHASE), "BEWEISPHASE erwartet.");

  /**********/
  /* Anfang */
  /**********/

  SkipSymbol( f_descr, ident, sym, SANFANG, ANFANG);
  ReadRealzahl(&(expert->phase_anfang), ident, &sym );
  
  /*********/
  /* Mitte */
  /*********/

  CheckError( (sym != SMITTE), "MITTE erwartet.");
  ReadRealzahl(&(expert->phase_mitte), ident, &sym );

  /********/
  /* Ende */
  /********/

  CheckError( (sym != SENDE), "ENDE erwartet.");
  ReadRealzahl(&(expert->phase_ende), ident, &sym );

  /****************************/
  /* Einlesen des Zyklusdauer */
  /****************************/

  CheckError( (sym != SZYKLUSDAUER), "ZYKLUSDAUER erwartet.");

  /**********/
  /* Anfang */
  /**********/

  SkipSymbol( f_descr, ident, sym, SANFANG, ANFANG);
  ReadZahlPaar(&(expert->zeit_anf_min), &(expert->zeit_anf_max) );
  
  /*********/
  /* Mitte */
  /*********/

  SkipSymbol( f_descr, ident, sym, SMITTE, MITTE);
  ReadZahlPaar(&(expert->zeit_mit_min), &(expert->zeit_mit_max) );

  /********/
  /* Ende */
  /********/

  SkipSymbol( f_descr, ident, sym, SENDE, ENDE);
  ReadZahlPaar(&(expert->zeit_end_min), &(expert->zeit_end_max) );

  /***********************************/
  /* Einlesen der aehnliche Experten */
  /***********************************/
  SkipSymbol( f_descr, ident, sym, SAEHNLICHE_EXP, AEHNLICHE_EXP);
  expert->aehnl_exp_anz = ReadExp(expert->aehnl_exp, ident, &sym);

  /**************************************/
  /* Einlesen der moeglicher Nachfolger */
  /**************************************/
  CheckError ( (sym != SNACHFOLGER), "NACHFOLGER erwartet." );
  expert->nachf_exp_anz = ReadExp(expert->nachf_exp, ident, &sym);

  /******************************************/
  /* Einlesen der unvertraeglicher Experten */
  /******************************************/
  CheckError ( (sym != SUNVERTRAEGL), "UNVERTRAEGL erwartet.");
  expert->unvertraegl_exp_anz = ReadExp(expert->unvertraegl_exp, ident, &sym);

  /****************************************/
  /* Einlesen der moeglicher Teamkollegen */
  /****************************************/
  CheckError ( (sym != SMOEGL_TEAMEXP),
               "MOEGL_TEAMEXP erwartet.");
  expert->team_exp_anz = ReadExp(expert->unvertraegl_exp, ident, &sym);

  /***********************************/
  /* Einlesen der Bewertungsprozedur */
  /***********************************/
  CheckError ( (sym != SPROZEDUR),
               "PROZEDUR erwartet.");
  SkipSymbol( f_descr, ident, sym, SIDENT, Funktionsname);
  if (!strncpy(expert->func_name, ident, IDENTLENGTH) )
  {
    Error ( __FILE__ ": " "ParseExpert", "Funktionsname zu lang." );
  }

  /*****************************/
  /* Einlesen der Beschreibung */
  /*****************************/
  SkipSymbol( f_descr, ident, sym, SBESCHREIBUNG, BESCHREIBUNG);
  expert->Beschreibung = 0;

  CloseInput ( f_descr );

  return exp_type;
} /* Ende von ParseExpert*/
