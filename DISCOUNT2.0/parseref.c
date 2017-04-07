#include "termpair.h"
#include "complet.h"
#include "domain.h"
#include "scanner.h"
#include "database.h"
#include "expert.h"
#include "parseexp.h"
#include "referee.h"
#include "parseref.h"
#include "team.h"


/*****************************************************************************/
/*                                                                           */
/*                            Globale Variable                               */
/*                                                                           */
/*****************************************************************************/

static char Filename[MAXPATHLEN];
static short f_descr; /* Interner Filedescriptor des Scanners */

/*****************************************************************************/
/*                                                                           */
/*                     Variablen, die auch extern verwendet werden           */
/*                                                                           */
/*****************************************************************************/

/* Diese Variable ist true, wenn es wenigstens einen Experten gibt, der Lei- */
/* ter werden darf.                                                          */
bool LeiterVorhanden = false;

/*****************************************************************************/
/*                                                                           */
/*                      Lokale Funktionsvereinbarung                         */
/*                                                                           */
/*****************************************************************************/

#ifdef   ANSI
    
    static void    SyntaxError ( char *error );
    static void    CheckError ( bool cond, char *msg );

#endif

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
	getchar();
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
    /* Leerer Schleifenkoerper */
  }
}  /* Ende von ReadRealzahl */


/*****************************************************************************/
/*                                                                           */
/*   Funktion     : Read_Parameter                                           */
/*                                                                           */
/*   Parameter    : - Pointer auf ein Feld vom Typ ParameterElement          */
/*                                                                           */
/*   Returnwert   : keine                                                    */
/*                                                                           */
/*   Beschreibung : Diese Funktion liest eine Intervallangabe und den Default*/
/*                  wert fuer einen Parameter.                               */
/*                                                                           */
/*                  Es gilt folgende Vorbedingung : Beim Start muss das      */
/*                  naechste Symbol nach beliebig vielen Zeilenvorschueben   */
/*                  ( Symbol SCRSYM ) ein Doppelpunkt sein.                  */
/*                                                                           */
/*   Globale Var. : keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void Read_Parameter ( ParameterElement *parameter )
{
  char ident[IDENTLENGTH];
  Symbol sym;

  SkipSymbol( f_descr, ident, sym, SCOLLON, Doppelpunkt);

  /******************/
  /* range einlesen */
  /******************/
  parameter->bereich_anfang = ReadGanzzahl();

  SkipSymbol( f_descr, ident,sym,SCOMMA,Komma);

  parameter->bereich_ende = ReadGanzzahl();

  SkipSymbol( f_descr, ident,sym,SSEMICOLLON,';');

  /*************************/
  /* default-Wert einlesen */
  /*************************/
  parameter->normal = ReadGanzzahl();

} /* Ende von Read_Parameter */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadObjekt                                               */
/*                                                                           */
/*  Parameter    :  - Pointer auf ein Feld, das die Objektinformationen      */
/*                    aufnehmen kann.                                        */
/*                  - ein Pointer auf char; dort wird nach dieser Funktion   */
/*                    der Identifier abgelegt, der zuletzt gelesen wurde und */
/*                    nicht mehr zu der Expertenliste gehoert.               */
/*                  - Pointer auf Symbol, das nach dieser Funktion das Symbol*/
/*                    enthaelt, das zuletzt gelesen wurde und nicht mehr zu  */
/*                    der Expertenliste gehoert.                             */
/*                                                                           */
/*  Returnwert   : Anzahl der eingelesenen Experten;                         */
/*                                                                           */
/*  Beschreibung : Diese Funktion liest eine ganze Liste von Experten zu-    */
/*                 sammen mit deren Informationen ein und speichert sie in   */
/*                 der angegebene Variablen ab.                              */
/*                                                                           */
/*                 Es gilt folgende Vorbedingung : Beim Start muss das       */
/*                 naechste Symbol nach beliebig vielen Zeilenvorschueben    */
/*                 ( Symbol SCRSYM ) eine oeffnende Klammer sein.            */
/*                                                                           */
/*  Globale Var. : keine                                                     */
/*                                                                           */
/*****************************************************************************/

static short ReadObjekt ( ObjectInFrameInfo *objekt, char *id, Symbol *s )
{
  int i = 0;

  SkipSpace(f_descr, id, *s);

  while ( *s == SBRACKET_L )
  {
    /* Namen einlesen */
    SkipSymbol( f_descr, id, *s, SIDENT, Identifier );
    strcpy( objekt[i].name, id );

    SkipSymbol( f_descr, id, *s, SCOMMA, Komma);
    
    ReadRealzahl(&(objekt[i].guete), id, s);

    /* Beachte ReadRealzahl liest bereits ein Zeichen weiter */
    CheckError( (*s != SBRACKET_R), " ')' erwartet.");

    i++;

    SkipSpace(f_descr, id,*s);
  } /* Ende der while-Schleife */

  return i;
} /* Ende von ReadObjekt */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ParseRef                                                 */
/*                                                                           */
/*  Parameter    :  - Name einer Datei, aus der gelesen werden soll          */
/*                  - Pointer auf Gutachter, in den die gelesenen Daten ge-  */
/*                    schrieben werden.                                      */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur liest die Daten des Gutachters aus der    */
/*                  angegebenen Datei.                                       */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
void ParseRef( char *filename , RefFrame *ref)
{
  char     ident[IDENTLENGTH];
  Symbol   sym;
  
  sprintf(Filename,"%s/%s.%s", RefereeDir, filename, REFFILESUFFIX );

  if( !DemoMode )
  {
    printf("GutachterDatei %s lesen\n", Filename );
  }


  /**********************************************************/
  /* Initialisieren der Symboltabelle und Oeffnen der Datei */
  /**********************************************************/

  f_descr = OpenInput ( Filename );
  SetScanMode ( f_descr, REF_MODE );

  /********************************/
  /* Einlesen des Gutachternamens */
  /********************************/

  SkipSymbol( f_descr, ident, sym, SGUTACHTERNAME, GUTACHTERNAME);
  GetFilename ( f_descr, ref->ga_name );
  /* Dieser Name mu"s mit dem Namen der Datei "ubereinstimmen, um Verwir-   */
  /* rungen zu vermeiden!                                                   */
  CheckError( strcmp(ref->ga_name, filename),
	      "Name des Gutachterrahmen stimmt nicht mit dem Namen der Datei ueberein!");

  /*****************************************************************/
  /* Einlesen der Beurteile-Funktion und der angegebenen Parameter */
  /*****************************************************************/

  SkipSymbol( f_descr, ident, sym, SBEURTEILE_FKT, BEURTEILE_FKT);
  ref->beurteile_ga = GetSymbol( f_descr, ident );
  CheckError( ( (ref->beurteile_ga != SR_STATISTIC) && 
	        (ref->beurteile_ga != SR_EXTENDED) &&
		(ref->beurteile_ga != SR_FEELGOOD) ),
	      "Unbekannte Beurteile-Experten-Funktion.");
  
  switch( ref->beurteile_ga )
  {
    
    case SR_STATISTIC  : SkipSymbol( f_descr, ident, sym, SGROESSE_REGELN, GROESSE_REGELN);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_R]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_GLEICH, GROESSE_GLEICH);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_E]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_CP, GROESSE_CP);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_CP]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_ZIELE, GROESSE_ZIELE);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_G]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_CG, GROESSE_CG);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_CG]) );
			 SkipSymbol( f_descr, ident, sym, SGESAMT_RED, GESAMT_RED);
			 Read_Parameter( &(ref->beurt_paramliste[REF_REDCOUNT]) );
			 break;
    case SR_EXTENDED   : SkipSymbol( f_descr, ident, sym, SGROESSE_REGELN, GROESSE_REGELN);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_R]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_GLEICH, GROESSE_GLEICH);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_E]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_CP, GROESSE_CP);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_CP]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_ZIELE, GROESSE_ZIELE);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_G]) );
			 SkipSymbol( f_descr, ident, sym, SGROESSE_CG, GROESSE_CG);
			 Read_Parameter( &(ref->beurt_paramliste[REF_SIZE_CG]) );

			 SkipSymbol( f_descr, ident, sym, SNEUE_REGELN, NEUE_REGELN);
			 Read_Parameter( &(ref->beurt_paramliste[REF_NEW_R]) );
			 SkipSymbol( f_descr, ident, sym, SRED_REGELN, RED_REGELN);
			 Read_Parameter( &(ref->beurt_paramliste[REF_RED_R]) );
			 SkipSymbol( f_descr, ident, sym, SLOESCH_REGELN, LOESCH_REGELN);
			 Read_Parameter( &(ref->beurt_paramliste[REF_DEL_R]) );
			 SkipSymbol( f_descr, ident, sym, SNEUE_GLEICH, NEUE_GLEICH);
			 Read_Parameter( &(ref->beurt_paramliste[REF_NEW_E]) );
			 SkipSymbol( f_descr, ident, sym, SRED_GLEICH, RED_GLEICH);
			 Read_Parameter( &(ref->beurt_paramliste[REF_RED_E]) );
			 SkipSymbol( f_descr, ident, sym, SLOESCH_GLEICH, LOESCH_GLEICH);
			 Read_Parameter( &(ref->beurt_paramliste[REF_DEL_E]) );
			 SkipSymbol( f_descr, ident, sym, SNEUE_CP, NEUE_CP);
			 Read_Parameter( &(ref->beurt_paramliste[REF_NEW_CP]) );
			 SkipSymbol( f_descr, ident, sym, SLOESCH_CP, LOESCH_CP);
			 Read_Parameter( &(ref->beurt_paramliste[REF_DEL_CP]) );
			 SkipSymbol( f_descr, ident, sym, SNEUE_ZIELE, NEUE_ZIELE);
			 Read_Parameter( &(ref->beurt_paramliste[REF_NEW_G]) );
			 SkipSymbol( f_descr, ident, sym, SRED_ZIELE, RED_ZIELE);
			 Read_Parameter( &(ref->beurt_paramliste[REF_RED_G]) );
			 SkipSymbol( f_descr, ident, sym, SNEUE_CG, NEUE_CG);
			 Read_Parameter( &(ref->beurt_paramliste[REF_NEW_CG]) );
			 SkipSymbol( f_descr, ident, sym, SGESAMT_RED, GESAMT_RED);
			 Read_Parameter( &(ref->beurt_paramliste[REF_REDCOUNT]) );
			 break;
    case SR_FEELGOOD   : SkipSymbol( f_descr, ident, sym, SLETZTE_CP, LETZTE_CP);
			 Read_Parameter( &(ref->beurt_paramliste[REF_LAST_CP]) );
			 SkipSymbol( f_descr, ident, sym, SLETZTE_CG, LETZTE_CG);
			 Read_Parameter( &(ref->beurt_paramliste[REF_LAST_CG]) );
			 SkipSymbol( f_descr, ident, sym, SVERGANGEN_FAKTOR, VERGANGEN_FAKTOR);
			 Read_Parameter( &(ref->beurt_paramliste[REF_PAST_FAC]) );
			 SkipSymbol( f_descr, ident, sym, SNAECHSTE_CP, NAECHSTE_CP);
			 Read_Parameter( &(ref->beurt_paramliste[REF_NEXT_CP]) );
			 SkipSymbol( f_descr, ident, sym, SNAECHSTE_CG, NAECHSTE_CG);
			 Read_Parameter( &(ref->beurt_paramliste[REF_NEXT_CG]) );
			 SkipSymbol( f_descr, ident, sym, SZUKUNFTS_FAKTOR, ZUKUNFTS_FAKTOR);
			 Read_Parameter( &(ref->beurt_paramliste[REF_FUTURE_FAC]) );
			 SkipSymbol( f_descr, ident, sym, SKORREKTUR_FAKTOR, KORREKTUR_FAKTOR);
			 Read_Parameter( &(ref->beurt_paramliste[REF_CORRECCT_FAC]) );
			 break;
    default            : CheckError( true, "Unbekannte Beurteile-Experten-Funktion.");
			 break;
  } /* Ende von switch */

  /**********************************************/
  /* Einlesen der Finde-Gute-Resultate-Funktion */
  /**********************************************/
  SkipSymbol( f_descr, ident, sym, SRESULTATE_FKT, RESULTATE_FKT );
  ref->resultate_ga = GetSymbol( f_descr, ident );
  CheckError( ( (ref->resultate_ga != SR_STATISTIC) &&
                (ref->resultate_ga != SS_LAST) ),
              "Unbekannte Finde-Gute-Resultate-Funktion.");

  switch ( ref->resultate_ga )
  {
    case SS_LAST      : SkipSymbol( f_descr, ident, sym, SMAX_REGELN, MAX_REGELN);
		        Read_Parameter( &(ref->res_paramliste[REF_MAX_RULE]) );
		        SkipSymbol( f_descr, ident, sym, SMAX_GLEICH, MAX_GLEICH);
		        Read_Parameter( &(ref->res_paramliste[REF_MAX_EQU]) );
		        SkipSymbol( f_descr, ident, sym, SMAX_ZIELE, MAX_ZIELE);
		        Read_Parameter( &(ref->res_paramliste[REF_MAX_GOAL]) );
		        SkipSymbol( f_descr, ident, sym, SSCHWELLE_REGELN, SCHWELLE_REGELN);
		        Read_Parameter( &(ref->res_paramliste[REF_RULE_LOW]) );
		        SkipSymbol( f_descr, ident, sym, SSCHWELLE_GLEICH, SCHWELLE_GLEICH);
		        Read_Parameter( &(ref->res_paramliste[REF_EQU_LOW]) );
		        SkipSymbol( f_descr, ident, sym, SSCHWELLE_ZIELE, SCHWELLE_ZIELE);
		        Read_Parameter( &(ref->res_paramliste[REF_GOAL_LOW]) );
			break;

    case SR_STATISTIC : SkipSymbol( f_descr, ident, sym, SMAX_REGELN, MAX_REGELN);
		        Read_Parameter( &(ref->res_paramliste[REF_MAX_RULE]) );
		        SkipSymbol( f_descr, ident, sym, SMAX_GLEICH, MAX_GLEICH);
		        Read_Parameter( &(ref->res_paramliste[REF_MAX_EQU]) );
		        SkipSymbol( f_descr, ident, sym, SMAX_ZIELE, MAX_ZIELE);
		        Read_Parameter( &(ref->res_paramliste[REF_MAX_GOAL]) );
		        SkipSymbol( f_descr, ident, sym, SSCHWELLE_REGELN, SCHWELLE_REGELN);
		        Read_Parameter( &(ref->res_paramliste[REF_RULE_LOW]) );
		        SkipSymbol( f_descr, ident, sym, SSCHWELLE_GLEICH, SCHWELLE_GLEICH);
		        Read_Parameter( &(ref->res_paramliste[REF_EQU_LOW]) );
		        SkipSymbol( f_descr, ident, sym, SSCHWELLE_ZIELE, SCHWELLE_ZIELE);
		        Read_Parameter( &(ref->res_paramliste[REF_GOAL_LOW]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_RED, ANZAHL_RED);
		        Read_Parameter( &(ref->res_paramliste[REF_RED_COUNT]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_RED_R, ANZAHL_RED_R);
		        Read_Parameter( &(ref->res_paramliste[REF_RED_RIGHT]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_RED_L, ANZAHL_RED_L);
		        Read_Parameter( &(ref->res_paramliste[REF_RED_LEFT]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_RED_GLEICH, ANZAHL_RED_GLEICH);
		        Read_Parameter( &(ref->res_paramliste[REF_RED_EQU]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_RED_ZIELE, ANZAHL_RED_ZIELE);
		        Read_Parameter( &(ref->res_paramliste[REF_RED_GOAL]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_SUBSUM, ANZAHL_SUBSUM);
		        Read_Parameter( &(ref->res_paramliste[REF_SUBSUM]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_CP, ANZAHL_CP);
		        Read_Parameter( &(ref->res_paramliste[REF_CP_COUNT]) );
		        SkipSymbol( f_descr, ident, sym, SANZAHL_CG, ANZAHL_CG);
		        Read_Parameter( &(ref->res_paramliste[REF_CG_COUNT]) );
			break;

    default          : CheckError( true, "Unbekannte Finde-Gute-Resultate-Funktion.");
		       break;
  } /* Ende des switch */

  SkipSymbol( f_descr, ident, sym, SGEEIGNETE_EXP, GEEIGNETE_EXP);
  ref->exp_anz = ReadObjekt( ref->geeignete_exp, ident, &sym );

  CheckError( (sym != SGEEIGNETE_DOM), "GEEIGNETE_DOM erwartet.");
  ref->dom_anz = ReadObjekt( ref->geeignete_dom, ident, &sym );

  CheckError( (sym != SROBUSTHEIT), "ROBUSTHEIT erwartet.");
  ReadRealzahl(&(ref->robust), ident, &sym );

  while( sym == SCRSYM )
  {
    sym = GetSymbol( f_descr, ident);
  }
  CheckError( (sym != SWISSENSANTEIL), "WISSENSANTEIL erwartet.");
  ReadRealzahl(&(ref->wissen), ident, &sym );

  while( sym == SCRSYM )
  {
    sym = GetSymbol( f_descr, ident);
  }
  CheckError( (sym != SAEHNLICHE_GA), "AEHNLICHE_GA erwartet.");
  ref->aehnl_ga_anz = ReadObjekt( ref->aehnl_ga, ident, &sym );

  while( sym == SCRSYM )
  {
    sym = GetSymbol( f_descr, ident);
  }
  CheckError( (sym != SLEITER_MOEGLICH), "LEITER_MOEGLICH erwartet.");
  ref->beurt_paramliste[REF_NOMASTER].normal = ( ReadGanzzahl() == 1 );
  if ( ref->beurt_paramliste[REF_NOMASTER].normal )
  {
    LeiterVorhanden = true;
  }

  CloseInput ( f_descr );
} /* Ende von ParseRef*/
