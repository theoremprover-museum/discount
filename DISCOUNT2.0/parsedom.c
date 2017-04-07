#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include "defines.h"
#include "error.h"
#include "scanner.h"
#include "polynom.h"
#include "vartree.h"
#include "term.h"
#include "termpair.h"
#include "complet.h"
#include "domain.h"
#include "database.h"
#include "expert.h"
#include "parser.h"
#include "parseexp.h"
#include "parsedom.h"
#include "memory.h"
#include "team.h"


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/

/* Suffix fuer eine Datei einer Dom"anenspezifikation        */
#define DOMFILESUFFIX   "dom"

/*****************************************************************************/
/*                                                                           */
/*         interne Speicherverwaltung                                        */
/*       ( Stimmt im wesentlichen mit der aus database.c ueberein            */
/*         und sollte bei Zeit mit dieser vereinheitlicht werden!!!!         */
/*         Die Variablen function        EX_FuncCount    und                 */
/*                       F_Info          EX_Function [MAXFUNCTION]           */
/*         werden bei Domaenen in der Struktur Dom_frame mitverwaltet.)      */
/*                                                                           */
/*****************************************************************************/

/*  Liste von Pointern auf Termen, die freien Speicherplatz fuer Terme bein- */
/*  haltet.                                                                  */
/*  Beachte : Fuer jede Stelligkeit gibt es eine eigene Liste von freien     */
/*            Termen. Die Verkettung erfolgt hierbei ueber die Komponente    */
/*            argument[0] in der Struktur von term.                          */
static  term            *DomFreiliste[MAXARITY] = { NULL };

static char Filename[MAXPATHLEN];

static short    dom_descr;  /* interner Filedescriptor vom Scanner */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  newterm_dom                                              */
/*                                                                           */
/*  Parameter    :  -interne Nummer dees Topfunktionssymbols vom Typ function*/
/*                  -Stelligkeit dieses Funktionssymbols                     */
/*                                                                           */
/*  Returnwert   :  Pointer auf eine Variable vom Typ term mit den ueber-    */
/*                  gebenen Informationen                                    */
/*                                                                           */
/*  Beschreibung :  Zunaechst wird die Stelligkeit ueberprueft, ob sie im    */
/*                  erlaubten Intervall liegt [0..MAXARITY].                 */
/*                  Enthaelt die globale Variable DomFreiliste noch Elemente,*/
/*                  d.h. Speicherplatz, der vergeben werden kann, so wird    */
/*                  dieses Element genommen, ansonsten wird neuer Speicher-  */
/*                  platz allociert.                                         */
/*                  Zum Schluss wird der uebergebene Funktionscode und die   */
/*                  Stelligkeit eingetragen.                                 */
/*                                                                           */
/*  Globale Var. :  DomFreiliste                                             */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

term *newterm_dom ( function fcode, short arity )
{
  term *t_ptr;
  long  size;

  if ((arity < 0) || (arity >= MAXARITY))
  {
    Error ( __FILE__ ": "  "newterm_a", "Fehler in der Stelligkeit." );
  }
  
  if (!DomFreiliste[arity])
  {  /* neuer Speicherplatz muss allociert werden */
    size =   sizeof (term) + max(1, arity) * sizeof (term *);
    t_ptr = (term *)Allocate ( size );
  }
  else
  {  /* Es sind noch freie Terme vorhanden */
    t_ptr = DomFreiliste[arity];
    DomFreiliste[arity] = t_ptr->argument[0];
  }

  t_ptr->fcode = fcode;
  t_ptr->arity = arity;

  return t_ptr;
} /* Ende von newterm_dom */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  delterm_dom                                              */
/*                                                                           */
/*  Parameter    :  Pointer auf Term, dessen Speicherplatz auf Toplevel      */
/*                  wieder freigegeben werden soll.                          */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Der uebergebene Term wird geloescht, indem der Pointer   */
/*                  in die der Stelligkeit entsprechenden Freiliste von Ter- */
/*                  men eingefuegt wird.                                     */
/*                                                                           */
/*  Globale Var. :  DomFreiliste                                             */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void delterm_dom ( term *t )
{
  t->argument[0] = DomFreiliste[t->arity];
  DomFreiliste[t->arity] = t;
} /* Ende von delterm_dom */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  deleteterm_dom                                           */
/*                                                                           */
/*  Parameter    :  Pointer auf Term, der geloescht werden soll              */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird rekursiv der gesamte Term mit Hilfe von          */
/*                  delterm_dom geloescht, d.h. der Speicherplatz wird wieder*/
/*                  freigegeben.                                             */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void deleteterm_dom ( term *t )
{
  short i;

  for (i = 0; i < t->arity; deleteterm_dom ( t->argument[i++] ) );
  /* leerer for-Schleifenkoerper */

  delterm_dom( t );
} /* Ende von deleteterm_dom */


/*****************************************************************************/
/*                                                                           */
/*             Funktionen zur Fehlermeldung                                  */
/*                                                                           */
/*****************************************************************************/

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
    PrintScanText ( dom_descr );
    printf ( "****  Fehler in der Domaenenspezifikation : %s.\n", Filename );
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
        CloseInput ( dom_descr );
        flush ();
        getchar ();
        exit ( 1 );
    }
}


/*****************************************************************************/
/*                                                                           */
/*             Hilfsvariablen fuer den Domaenen-Parser                       */
/*                                                                           */
/*****************************************************************************/

/* Feld, in dem die Funktionssymbole und deren Stelligkeit der momentan      */
/* gelesenen Signatur stehen                                                 */
/* Beachte dom_fkt_feld[0] enthaelt keine Informationen.                     */
/* Wird in ParseDom auf das entsprechende Feld gesetzt.                      */

static F_Info *dom_fkt_feld = NULL;

/* Anzahl der Funktionssymbole im Feld dom_fkt_feld                          */
/* Wird in ParseDom entsprechend gesetzt.                                    */

static short dom_fkt_feld_laenge;

/*****************************************************************************/
/*                                                                           */
/*             Hilfsfunktionen fuer den Domaenen-Parser                      */
/*                                                                           */
/*****************************************************************************/

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

  SkipSpace( dom_descr, ident, sym);

  CheckError ( ( sym != SIDENT) || !IsNumber (ident), "Ganzahl erwartet." );
  return  atoi (ident);
} /* Ende von ReadGanzzahl */

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

static short Read_Param_list ( ParameterElement *paramliste, char *id, Symbol *s)
{
  char ident[IDENTLENGTH];
  Symbol sym;
  register int i = 0;

  SkipSpace( dom_descr, ident,sym);

  while ( (sym == SIDENT ) && (i < MAXPARAM) )
  {

  /***************************/
  /* Parameternamen einlesen */
  /***************************/
    if (!strncpy( paramliste[i].name, ident, IDENTLENGTH))
    {
      Error ( __FILE__ ": " "Read_Param_list", "Parametername zu lang." );
    }

    SkipSymbol( dom_descr, ident,sym,SCOLLON,':');

  /******************/
  /* range einlesen */
  /******************/
    paramliste[i].bereich_anfang = ReadGanzzahl();

    SkipSymbol( dom_descr, ident,sym,SCOMMA,Komma);

    paramliste[i].bereich_ende = ReadGanzzahl();

    SkipSymbol( dom_descr, ident,sym,SSEMICOLLON,';');

  /*************************/
  /* default-Wert einlesen */
  /*************************/
    paramliste[i].normal = ReadGanzzahl();

    SkipSpace( dom_descr, ident,sym);

    i++;
  } /* Ende der while-Schleife */

  strncpy( id, ident, IDENTLENGTH );
  *s = sym;

  return i;
} /* Ende von Read_Param_list */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadRealzahl                                             */
/*                                                                           */
/*  Parameter    :  - Pointer auf eine real-Zahl, in die im positiven Fall   */
/*                    der Wert der gelesenen real-Zahl geschrieben wird.     */
/*                  - ein Pointer auf char; dort wird nach dieser Funktion   */
/*                    der Identifier abgelegt, der zuletzt gelesen wurde und */
/*                    nicht mehr zur Realzahl gehoert und kein CR ist.       */
/*                    Es sei denn, das Dateiende ( EOI( dom_descr ) = true )   */
/*                    wird erreicht.                                         */
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

  SkipSpace( dom_descr, id, *s );

  CheckError ( ( *s != SIDENT) || !IsNumber (id), "Realzahl erwartet." );
  *real = atoi(id);

  *s = GetSymbol( dom_descr, id);

  if ( *s != SPUNKT )     /* Zahl hat nur Vorkommastellen */
  {
    while(!EOI( dom_descr ) && (*s == SCRSYM ) )
    {
      *s = GetSymbol( dom_descr,  id );
    } /* Ende von while */
  } /* Ende von if */
  else
  {
    *s = GetSymbol( dom_descr, id);

    CheckError ( EOI( dom_descr ), "Unerwartetes Dateiende." );
    CheckError ( ( *s != SIDENT) || !IsNumber (id), "Realzahl erwartet." );
    *real += atoi(id)/exp_10(strlen(id));

    while ( !EOI( dom_descr ) && ( ( *s = GetSymbol( dom_descr,  id ) ) == SCRSYM ));
    /* Leerer while-Schleifenkoerper */
  } /* Ende von else */
}  /* Ende von ReadRealzahl */


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


  SkipSpace( dom_descr, ident,sym);

  while ( (sym == SBRACKET_L) && (i < MAXGA_ANZ) )
  {

    /***************************/
    /* Gutachternamen einlesen */
    /***************************/
    SkipSymbol( dom_descr, ident, sym, SIDENT, Gutachternamen );
    strcpy( ga_liste[i].name, ident );

    SkipSymbol( dom_descr, ident, sym, SCOMMA, Komma);
    ga_liste[i].param_anz = Read_Param_list(ga_liste[i].paramliste, ident, &sym);

    /* Beachte Read_Param_list liest bereits ein Zeichen weiter */
    CheckError( (sym != SCOMMA ), "Komma erwartet.");

    ReadRealzahl(&(ga_liste[i].guete), ident, &sym );

    /* Beachte ReadRealzahl liest bereits ein Zeichen weiter */
    CheckError( (sym != SBRACKET_R), " ')' erwartet.");

    i++;

    SkipSpace( dom_descr, ident,sym);
  } /* Ende der while-Schleife */

  strncpy( id, ident, IDENTLENGTH );
  *s = sym;

  return i;
} /* Ende von ReadGa */

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
  int i = 0;

  SkipSpace(dom_descr, id, *s);

  while ( (*s == SBRACKET_L) && (i < MAXEXP_ANZ) )
  {
    /**************************/
    /* Expertennamen einlesen */
    /**************************/
    SkipSymbol( dom_descr, id, *s, SIDENT, Expertennamen );
    strcpy( exp_liste[i].name, id );

    SkipSymbol( dom_descr, id, *s, SCOMMA, Komma);
    exp_liste[i].param_anz = Read_Param_list(exp_liste[i].paramliste, id, s);

    /* Beachte Read_Param_list liest bereits ein Zeichen weiter */
    CheckError( (*s != SCOMMA ), "Komma erwartet.");

    ReadRealzahl(&(exp_liste[i].guete), id, s);

    /* Beachte ReadRealzahl liest bereits ein Zeichen weiter */
    CheckError( (*s != SBRACKET_R), " ')' erwartet.");

    i++;

    SkipSpace(dom_descr, id,*s);
  } /* Ende der while-Schleife */

  return i;
} /* Ende von ReadExp */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  findfunc                                                 */
/*                                                                           */
/*  Parameter    :  - Identifier, der getestet werden soll                   */
/*                                                                           */
/*  Returnwert   :  - interne Nummer des Funktionssymbols innerhalb diese    */
/*                    Domaenenspezifikation, falls der Identifier ein Funk-  */
/*                    tionssymbol ist (Nummerierung startet bei 1 !!!)       */
/*                  - 0 sonst.                                               */
/*                                                                           */
/*  Beschreibung :  Das angegeben Feld wird durchlaufen                      */
/*                                                                           */
/*  Globale Var. :  dom_fkt_feld                                             */
/*                  dom_fkt_feld_laenge                                      */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static function findfunc ( char *ident )
{
  short   i;

  for (i = 1; i <= dom_fkt_feld_laenge; i++)
  {
    if ( !strcmp (ident, dom_fkt_feld[i].ident) )
    {
      return i;
    } /* Ende von if */
  } /* Ende von if */
  
  return 0;
} /* ende von findfunc */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadTerm                                                 */
/*                                                                           */
/*  Parameter    :  - Startidentifier eines Terms                            */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    dem Term gehoert.                                      */
/*                                                                           */
/*  Returnwert   :  Pointer auf den eingelesenen Term                        */
/*                                                                           */
/*  Beschreibung :  Es wird zuerst getestet, ob das Topsymbol ein Funktions- */
/*                  symbol ist, wenn nicht, so muss es sich um eine Variable */
/*                  handeln, die dann neu angelegt wird.                     */
/*                  Handelt es sich um ein Funktionssymbol aus der Signatur  */
/*                  der momentan betrachteten Domaene, so werden rekursiv    */
/*                  die Teilterme eingelesen.                                */
/*                  Dabei wird geprueft, ob die Anzahl der Teilterme mit der */
/*                  in der Signatur angegebenen Stelligkeit uebereinstimmt.  */
/*                                                                           */
/*  Globale Var. :  dom_fkt_feld                                             */
/*                  dom_fkt_feld_laenge                                      */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static term *ReadTerm ( char *ident, Symbol *sym )
{
  function  func;
  term      *t;
  short     i = 0;

  /*printf("\nIdentifier : %s\n", ident );*/

  if ( !(func = findfunc( ident )) )
  {   /* Es liegt also eine Variable vor */
    *sym = GetSymbol( dom_descr, ident );  /* Beachte: das naechste Symbol darf kein */
				 /*          Identifier sein!!!!           */

    /* Es ist wichtig, dass hier newterm aus parser.c aufgerufen wird */
    return newterm ( CreateVariable (ident) );
  }

  /* ident ist ein Funktionssymbol */
  t = newterm_dom( func, dom_fkt_feld[func].arity );

  /* Einlesen der Teilterme, falls vorhanden */
  /* Zuerst wird die Moeglichkeit einer Konstanten betrachtet */
  if ((*sym = GetSymbol( dom_descr,ident)) != SBRACKET_L)
  {    
    CheckError ( dom_fkt_feld[func].arity != 0,
		 "Falsche Stelligkeit eines Funktionssymbols." );
    /* ansonsten ist das Funktionssymbol eine Konstante */
    return t;
  }

  /* Funktionssymbol ist keine Konstante */
  while ((*sym = GetSymbol( dom_descr,ident)) == SIDENT)
  {
    CheckError ( i >= dom_fkt_feld[func].arity,
		 "Falsche Stelligkeit eines Funktionssymbols." );
    
    t->argument[i++] = ReadTerm ( ident, sym );
    if ( *sym != SCOMMA )
    {
      /* Test, ob nicht zu wenig Teilterme angegeben */
      CheckError ( i != (dom_fkt_feld[func].arity ), 
		   "Falsche Stelligkeit eines Funktionssymbols." );
      break;
    }
  }
  CheckError ( (*sym != SBRACKET_R), "`)` erwartet." );

  *sym = GetSymbol( dom_descr,ident);
  return t;
} /* Ende von ReadTerm */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadTermpair                                             */
/*                                                                           */
/*  Parameter    :  - Startidentifier des linken Termes                      */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    dem Termpaar gehoert.                                  */
/*                  - Trennungssymbol zwischen den beiden Termen, auf das    */
/*                    getestet werden soll.                                  */
/*                                                                           */
/*  Returnwert   :  Pointer auf das eingelesene Termpaar                     */
/*                                                                           */
/*  Beschreibung :  Es wird die Menge der "bekannten" Variablen geloescht.   */
/*                  Dies ist notwendig, weil in ReadTerm die Funktion        */
/*                  CreateVariable aufgerufen wird, die u.a. auch ueber-     */
/*                  prueft, ob die angegeben Variable bereits in dem Term-   */
/*                  paar vorkommt. Daher muss die Liste der bekannten Vari-  */
/*                  ablen bei jedem Termpaar erst einmal geloescht werden.   */
/*                  Dann wird der linke Term eingelesen; dann das Trennungs- */
/*                  zeichen und schliesslich der rechte Term.                */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static termpair *ReadTermpair ( char *ident, Symbol *sym, Symbol tr_sym )
{
  term *left, *right;
  termpair    *pair;

  ClearVariables ();

  getweight( left = ReadTerm (ident, sym) );

  CheckError ( *sym != tr_sym, "Falsches Trennungssymbol.");

  while ((*sym = GetSymbol( dom_descr,ident)) == SCRSYM)
  {
    CheckError ( EOI(dom_descr), "Unerwartetes Dateiende." );
  }
  CheckError ( (*sym != SIDENT), "Term erwartet." );

  getweight( right = ReadTerm (ident, sym) );

  pair = newpair (left, right, NULL, NULL );
  if ( tr_sym == SEQUAL )
  {
    pair->type = EQUATION;
  }
  else if ( tr_sym == SGREATER )
  {
    pair->type = RULE;
  }
  
  return pair;
} /* Ende von ReadTermpair */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadSignatur                                             */
/*                                                                           */
/*  Parameter    :  - Startidentifier des ersten Funktionssymbols            */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zur */
/*                    Signatur gehoert.                                      */
/*                    Beim Start sollte dieses Variable den Wert SIDENT ha-  */
/*                    ben, sonst wird die leere Signatur akzeptiert.         */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden alle in der Signatur angegebenen Funktions-    */
/*                  symbole und deren Stelligkeit eingelesen, wobei ueber-   */
/*                  prueft wird, ob ein Funktionssymbol doppelt deklariert   */
/*                  wird. Die Stelligkeit muss eine positive Ganzahl sein.   */
/*                  Jedes Funktionssymbol erhaelt bzgl. der momentan be-     */
/*                  trachteten Domaene eine eindeutige Nummer zugeordnet an- */
/*                  hand des Feldindizes. Die Nummerierung beginnt bei 1 !!  */
/*                                                                           */
/*  Globale Var. :  dom_fkt_feld                                             */
/*                  dom_fkt_feld_laenge                                      */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static void ReadSignatur ( char *ident, Symbol *sym )
{
  while (*sym == SIDENT && *sym != SDEF_GLEICH)
  {
    /*****************************************************/
    /* externe Darstellung des Funktionssymbols einlesen */
    /*****************************************************/
    CheckError (findfunc (ident),"Funktionsymbol mehrfach definiert ?" );

    dom_fkt_feld_laenge++;
    strcpy( dom_fkt_feld[dom_fkt_feld_laenge].ident, ident );

    CheckError ( GetSymbol( dom_descr,ident) != SCOLLON, ": erwartet." );

    /************************/
    /* Stelligkeit einlesen */
    /************************/
    CheckError (    (GetSymbol( dom_descr,ident) != SIDENT)
		 || !IsNumber (ident)
		 || (atoi (ident) < 0),
		 "positive Ganzzahl erwartet." );
    dom_fkt_feld[dom_fkt_feld_laenge].arity = atoi (ident);

    SkipSpace( dom_descr, ident, *sym );
  }
} /* Ende von ReadSignatur */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadStartteam                                            */
/*                                                                           */
/*  Parameter    :  - Identifier ( muss beim Start = "STARTTEAM" sein, sonst */
/*                    stimmt etwas nicht )                                   */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zur */
/*                    Startteamdeklartion gehoert und != SCRSYM ist!!        */
/*                    Beim Start sollte dieses Variable den Wert SCRSYM, wenn*/
/*                    kein Startteam angegeben wird, ansonsten den Wert      */
/*                    SBRACKET_L haben.                                      */
/*                  - Pointer auf ein Feld, in das die Experten des Start-   */
/*                    teams eingetragen werden.                              */
/*                  - Pointer auf ein Feld, in das die Gutachter des Start-  */
/*                    teams eingetragen werden.                              */
/*                                                                           */
/*  Returnwert   :  Anzahl der Experten/Gutachter-Paare im Startteam         */
/*                                                                           */
/*  Beschreibung :  Es werden die Experten/Gutachter-Paare des Startteams    */
/*                  eingelesen.                                              */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static short ReadStartteam( char *ident, Symbol *sym,
			    ObjectInFrameInfo *exp_feld, 
			    ObjectInFrameInfo *ga_feld )
{
  int i = 0;
  int exp_nr;

  while( (*sym == SBRACKET_L ) && (i < MAXEXP_ANZ) )
  {
    /*********************/
    /* Experten einlesen */
    /*********************/
    SkipSymbol( dom_descr,ident, *sym, SIDENT, Expertennamen );
    strcpy(exp_feld[i].name, ident );
    CheckError( (exp_nr = get_exp_nr( ident ) == NOEXPERT), "Unbekannter Experte.");
    exp_feld[i].nr = exp_nr;

    
    SkipSymbol( dom_descr,ident, *sym, SCOMMA, Komma);
    exp_feld[i].param_anz = Read_Param_list(exp_feld[i].paramliste, ident, sym);

    /* Beachte Read_Param_list liest bereits ein Zeichen weiter */
    CheckError( (*sym != SSEMICOLLON ), "Semikolon erwartet.");

    /**********************/
    /* Gutachter einlesen */
    /**********************/
    SkipSymbol( dom_descr,ident, *sym, SIDENT, Gutachternamen );
    strcpy( ga_feld[i].name, ident );

    SkipSymbol( dom_descr,ident, *sym, SCOMMA, Komma);
    ga_feld[i].param_anz = Read_Param_list(ga_feld[i].paramliste, ident, sym);

    CheckError( *sym != SBRACKET_R, "`)` erwartet." );

    SkipSpace( dom_descr, ident, *sym );

    i++;
  } /* Ende von while( (*sym == SBRACKET_L ) && (i < MAXEXP_ANZ) ) */

  return i;
} /* Ende von ReadStartteam */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadZyklenInfo                                           */
/*                                                                           */
/*  Parameter    :  - Identifier ( muss beim Start = "ZYKLENDAUER" sein,     */
/*                    sonst stimmt etwas nicht ).                            */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zur */
/*                    Zyklendeklaration gehoert und != SCRSYM ist!!          */
/*                    Beim Start sollte dieses Variable den Wert SCRSYM, wenn*/
/*                    keine Zyklendauer angegeben wird, ansonsten den Wert   */
/*                    SBRACKET_L haben.                                      */
/*                  - Pointer auf ein Feld, in das die Daten ueber die Start-*/
/*                    zyklen eingetragen werden.                             */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden die angegebenen Zyklendauern eingelesen.       */
/*                  Wird keine Zyklendauer angegeben, so wird in das erste   */
/*                  Feld des Arrays der Wert NO_CYCLE_TIME eingetragen und   */
/*                  zwar in die Komponente "normal".                         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static void ReadZyklenInfo( char *ident, Symbol *sym, Zyklen_Info *liste )
 {
   int i = 0;

  while( (*sym == SBRACKET_L ) && (i < MAX_ZYKLEN_ANZ) )
  {
    liste[i].bereich_anfang = ReadGanzzahl();

    SkipSymbol( dom_descr,ident, *sym, SCOMMA, Komma);

    liste[i].bereich_ende = ReadGanzzahl();

    SkipSymbol( dom_descr,ident, *sym, SSEMICOLLON, ;);

    liste[i].normal = ReadGanzzahl();

    SkipSymbol( dom_descr,ident, *sym, SBRACKET_R, Schliessende_Klammer );

    SkipSpace( dom_descr,ident, *sym);

    i++;
  } /* Ende von while */

  /* Ueberpruefen, ob mindestens eine Zyklendauer angegeben wurde */
  if( !i )
  {
    liste[0].normal = NO_CYCLE_TIME;
  } /* Ende von if */
} /* Ende von ReadZyklenInfo */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ParseDom                                                 */
/*                                                                           */
/*  Parameter    :  - Name einer Datei, aus der gelesen werden soll          */
/*                  - StatusTyp der Domaene anhand der Konfigurationsdatei   */
/*                                                                           */
/*  Returnwert   :  Nummer der Domaene, d.h. der Index in DoaminGlobalInfo   */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur liest die fuer den Leiter zur Teamzusam-  */
/*                  menstellung erforderlichen Daten einer Domaene aus der   */
/*                  angegebenen Datei ein und speichert sie in der externen  */
/*                  Variablen DomainGlobalInfo ab. Es wird dabei ueberprueft,*/
/*                  ob die Domaene bereits eingelesen wurde ( z.B. wenn      */
/*                  sie sowohl als kekannte als auch vermutete Domaene an-   */
/*                  gegeben wurde); wenn sie noch nicht gelesen wurde, wird  */
/*                  sie in das naechste freie Feld von DomainGlobalInfo      */
/*                  hineingeschrieben.                                       */
/*                                                                           */
/*  Globale Var. :  dom_fkt_feld                                             */
/*                  dom_fkt_feld_laenge                                      */
/*                                                                           */
/*****************************************************************************/
short ParseDom ( char *filename , DomStatusType  d_type )
{
  char         ident[IDENTLENGTH];
  Symbol       sym;
  short        i;
  DomainFrame *domaene;
  char         dom_name[IDENTLENGTH];
  short        dom_nr;

  /**************************************/
  /* Erzeugen des kompletten Pfadnamens */
  /**************************************/
  sprintf( Filename, "%s/%s.%s", DomainDir, filename, DOMFILESUFFIX );

  if( !DemoMode )
  {
    printf("DomaenenDatei %s lesen\n", Filename );
  }


  /**********************************************************/
  /* Initialisieren der Symboltabelle und Oeffnen der Datei */
  /**********************************************************/

  dom_descr = OpenInput ( Filename );
  SetScanMode ( dom_descr, DOM_MODE );

  /*******************************/
  /* Einlesen des Domenennamens  */
  /*******************************/

  SkipSymbol( dom_descr,ident, sym, SDOMAENENNAME, DOMAENENNAME);
  GetFilename ( dom_descr, dom_name );

  /* Dieser Name mu"s mit dem Namen der Datei "ubereinstimmen, um Verwir-   */
  /* rungen zu vermeiden!                                                   */
  CheckError( strcmp(dom_name, filename),
              "Name des Domaenenrahmen stimmt nicht mit dem Namen der Datei ueberein!");

  /********************************************/
  /* Testen, ob die Doamene bereits vorkommt  */
  /* i hat nach dem for den korrekten Index   */
  /********************************************/
  for ( i=0; i<DGICount; i++ )
  {
    if ( !strcmp(DomainGlobalInfo[i].dom_name, dom_name) )
    {
      break;
    } /* Ende von if */
  } /* Ende von for */
  
  dom_nr = i;

  domaene = &(DomainGlobalInfo[dom_nr]);

  /*************************************************************/
  /* kann stets auf true gesetzt werden, wenn vom Typ SUPPOSED */
  /*************************************************************/
  if ( d_type == SUPPOSED )
  {
    domaene->supposed = true;
  } /* Ende von if */

  /************************************/
  /* Testen, on neue Domaene vorliegt */
  /************************************/
  if ( i != DGICount )  /* Domaene bereits gelesen */
  {
    CloseInput( dom_descr );
    return dom_nr;
  }

  /**************************/
  /* Neue Domaene angegeben */
  /**************************/
  strcpy( domaene->dom_name, dom_name );
  DGICount++;

  /*****************************************/
  /* Initialisieren der globalen Variablen */
  /*****************************************/

  dom_fkt_feld_laenge = 0;
  dom_fkt_feld = domaene->dom_funktion;

  /*************************/
  /* Einlesen der Signatur */
  /*************************/
  SkipSymbol( dom_descr,ident, sym, SSIGNATUR, SIGNATUR);
  SkipSpace( dom_descr, ident, sym );

  ReadSignatur( ident, &sym );
  domaene->dom_fkt_anz = dom_fkt_feld_laenge;

  /******************************************/
  /* Einlesen der definierenden Gleichungen */
  /* Eine mu"s stets angegeben sein.        */
  /******************************************/
  i = 0;
  do
  {
    CheckError( sym != SDEF_GLEICH, "DEF_GLEICH erwartet." );
    SkipSpace( dom_descr, ident, sym );

    do
    {
      Add( &(domaene->def_gleichungen[i]), ReadTermpair( ident, &sym, SEQUAL));
      CheckError( sym != SCRSYM, "Neue Zeile erwartet.");
      SkipSpace( dom_descr, ident, sym );
    } while ( sym != SSTARTTEAM && sym != SDEF_GLEICH );
     i++;
  } while ( sym != SSTARTTEAM );
  
  domaene->def_anz = i;

  /***************************/
  /* Einlesen des Startteams */
  /***************************/

  CheckError( sym != SSTARTTEAM, "STARTTEAM erwartet." );
  SkipSpace( dom_descr, ident, sym );
  /* Startteamspezifikation muss in derselben Zeile beginnen, in der auch  */
  /* STARTTEAM steht!!!!!!                                                 */

  domaene->start_team_groesse = ReadStartteam(ident, &sym, domaene->start_team_exp,
				     domaene->start_team_ga );

  /***************************/
  /* Einlesen der Zykluszahl */
  /***************************/
  CheckError ( sym != SZYKLENZAHL, " ZYKLENZAHL erwartet.");

  domaene->startzyklen_anz = ReadGanzzahl();

  /****************************/
  /* Einlesen der Zyklusdauer */
  /****************************/
  SkipSymbol( dom_descr, ident, sym, SZYKLENDAUER, ZYKLENDAUER );
  sym = GetSymbol( dom_descr, ident );
  /* Spezifikation der Zyklendauer muss in derselben Zeile beginnen, in der*/
  /* auch ZYKLENDAUER steht!!!!!!                                          */

  ReadZyklenInfo( ident, &sym, domaene->zyklen_info );

  /************************************/
  /* Einlesen der geeigneten Experten */
  /************************************/
  while( sym == SCRSYM && !EOI(dom_descr) )
  {
    sym = GetSymbol( dom_descr, ident );
  }
  CheckError ( sym != SGUTE_EXP, "GUTE_EXP erwartet.");
  domaene->geeignete_exp_anz = ReadExp(domaene->geeignete_exp, ident, &sym );

  /**************************************/
  /* Einlesen der ungeeigneten Experten */
  /**************************************/
  CheckError ( sym != SUNGEEIGN_EXP, "UNGEEIGN_EXP erwartet.");
  domaene->ungeeignete_exp_anz = ReadExp(domaene->ungeeignete_exp, ident, &sym );

  /*************************************/
  /* Einlesen der geeigneten Gutachter */
  /*************************************/
  CheckError ( sym != SGUTE_GA, "GUTE_GA erwartet.");
  domaene->geeignete_ga_anz = ReadGa( domaene->geeignete_ga, ident, &sym );

  /****************************************/
  /* Einlesen der uebergeordneten Domaene */
  /****************************************/
  CheckError ( sym != SUEBER_DOM, "UEBER_DOM erwartet.");
  SkipSpace( dom_descr, ident, sym );

  /********************************************************************/
  /* Beachte es darf nur eine uebergeordnete Domaene angegeben werden */
  /********************************************************************/
  if ( sym == SIDENT )
  {
    strcpy( domaene->ueber_dom, ident );
    SkipSpace( dom_descr, ident, sym );
  }

  /**********************************/
  /* Einlesen speziellerer Domaenen */
  /**********************************/
  CheckError( sym != SSPEZIAL_DOM, "SPEZIAL_DOM erwartet." );

  domaene->spezielle_dom_anz = 0;

  SkipSpace( dom_descr, ident, sym );
  while ( sym == SIDENT )
  {
    CheckError ( ( domaene->spezielle_dom_anz == MAXDOM_ANZ ), 
	         "Zu viele spezielle Domaenen.");
      
    strcpy( domaene->spezielle_dom[domaene->spezielle_dom_anz], ident);

    domaene->spezielle_dom_anz++;
    SkipSpace( dom_descr, ident, sym );
  }

  /********************************/
  /* Einlesen verwandter Domaenen */
  /********************************/
  while ( sym == SCRSYM && !EOI(dom_descr) )
  {
    sym = GetSymbol( dom_descr, ident );
  }
  CheckError ( sym != SVERWANDT_DOM, "VERWANDT_DOM erwartet.");

  domaene->verwandte_dom_anz = 0;

  SkipSpace( dom_descr, ident, sym );
  while ( sym == SIDENT )
  {
    CheckError ( ( domaene->verwandte_dom_anz == MAXDOM_ANZ ), 
	         "Zu viele verwandte Domaenen.");
      
    strcpy( domaene->verwandte_dom[domaene->verwandte_dom_anz], ident );

    (domaene->verwandte_dom_anz)++;
    SkipSpace( dom_descr, ident, sym );
  }

  /********************************/
  /* Einlesen bekannter Resultate */
  /********************************/
  CheckError ( sym != SBEKANNTE_RES, "BEKANNTE_RES erwartet.");
  
  SkipSpace( dom_descr, ident, sym );

  while ( ( sym != SBESCHREIBUNG ) && ( sym != SLITERATUR ) && ( sym != SBEISPIELE ) &&
	  !EOI( dom_descr ) )
  {  
    Add ( &(domaene->bekannte_res),
	  ReadTermpair( ident, &sym, SEQUAL ));
    while ( sym == SCRSYM )
    {
      sym = GetSymbol( dom_descr, ident );
    }
  }

  CloseInput( dom_descr );

  return dom_nr;
} /* Ende von ParseDom */
