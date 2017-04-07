/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : domain                                                    */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : Verarbeiten der Informationen ueber Domaenen waehrend     */
/*                 der Planungsphase des Leiters.                            */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <string.h>
#include "domain.h"
#include "error.h"
#include "polynom.h"
#include "vartree.h"
#include "database.h"
#include "exp_form_next_t.h"
#include "team.h"
#include "parsedom.h"

/*****************************************************************************/
/*                                                                           */
/*                            Lokale Typdefinitionen                         */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* Moegliche Beziehungsarten zwischen zwei      */
               /* Domaenen                                     */
               /************************************************/
typedef enum { ALLGEMEINER, NEUTRAL, SPEZIELLER } relation_typ;

/*****************************************************************************/
/*                                                                           */
/*                            Lokale Variablen                               */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* true, falls alle Globalen Felder f"ur die Do-*/
	       /* m"anen initialisiert sind.                   */
               /************************************************/
static bool               DomainInitialzed = false;

               /************************************************/
               /* Die folgenden 4 Variablen werden nur nach    */
               /* dem Eionsatz eines Database-Experten ver-    */
               /* sorgt.                                       */
               /************************************************/
               /************************************************/
	       /* Indizes der Dom"anen aus dem Feld            */
	       /* FoundDomain, die als interessant angesehen   */
	       /* werden.                                      */
               /************************************************/
static short           IntDomains[MAXDOM_ANZ];

               /************************************************/
	       /* Anzahl dieser interessanten Dom"anen         */
               /************************************************/
static short           IntCount;

               /************************************************/
	       /* Indizes der Dom"anen aus InterestDomain, die */
	       /* aufgrund einer neu gefundenen Dom"ane nicht  */
	       /* mehr interessant sind, weil diese neue Do-   */
	       /* m"ane spezieller ist.                        */
               /************************************************/
static short           NotIntDomains[MAXDOM_ANZ];

               /************************************************/
	       /* Anzahl dieser nicht mehr interessanten Do-   */
	       /* m"anen                                       */
               /************************************************/
static short           NotIntCount;


/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* Infos ueber alle Domaenen                    */
               /************************************************/
DomainFrame        DomainGlobalInfo[MAXDOM_ANZ];

               /************************************************/
               /* Anzahl der verschiedenen Domaenen, die in    */
	       /* DomainGlobalInfo abgelegt wurden; d.h. Anzahl*/
	       /* der Plaetze, die in DomainGlobalInfo belegt  */
	       /* sind                                         */
               /************************************************/
short              DGICount = 0;

               /************************************************/
               /* Anzahl der relevanten Domaenen fuer den      */
	       /* naechsten Zyklus                             */
               /************************************************/
short              TeamDomCount = 0;

               /************************************************/
               /* relevanten Domaenen des naechsten Zyklus     */
               /************************************************/
DomMatchSpec       DomainTeamInfo[MAXRELDOMAIN];

               /************************************************/
               /* Anzahl der zur Zeit interessanten Domaenen   */
               /************************************************/
short              DomainCount = 0;

               /************************************************/
               /* Domaenennummern der interessanten Domaenen   */
               /************************************************/
DomMatchSpec       InterestDomain[MAXDOM_ANZ];

               /************************************************/
               /* Liste der vom Databaseexperten erkannten Do- */
	       /* maenen. Dabei kann eine Domaene auch oefters */
	       /* vorkommen.                                   */
               /************************************************/
short              FoundDomain[MAXDOM_ANZ];

               /************************************************/
               /* Anzahl der vom Databaseexperten erkannten    */
	       /* Domaenen.                                    */
               /************************************************/
short              FoundDomainCount = 0;

               /************************************************/
               /* Zuordnung der Funktionssymbole zwischen Pro- */
	       /* blemspezifikation und der vom Database-      */
	       /* experten erkannten Domaenen                  */
               /************************************************/
short              FoundDomainMatchFunc[MAXDOM_ANZ][MAXFUNCTION];

               /************************************************/
               /* Wurde ein gefundener Match in der globalen   */
               /* Variablen DomainGlobalInfo eingetragen, so   */
               /* enth"alt das folgende Feld als wievielter    */
               /* Match er in der Komponente DomMatches einge- */
               /* tragen wurde.                                */
               /* Die Eintr"age sind nur an den Stellen sinn-  */
               /* voll, an denen in FoundDomain eine Dom"ane   */
               /* steht, die neu eingetragen wurde.            */
               /************************************************/
short              MatchIdx[MAXDOM_ANZ];

               /************************************************/
               /* Liste der in der Konfigurationsdatei angege- */
	       /* benen Domaenen, nach denen der Database-Ex-  */
	       /* perte suchen soll. Es werden die Indizes     */
	       /* gemaess DomainGlobalinfo abgelegt.           */
               /************************************************/
short              SupposedDomains[MAXDOM_ANZ];

               /************************************************/
               /* Anzahl der Domaenen, die in der Konfigura-   */
	       /* tionsdatei angegeben wurden, nach denen ge-  */
	       /* sucht werden soll.                           */
               /************************************************/
short              SupposedDomainsCount = 0;

               /************************************************/
               /* Die folgenden 3 Variablen sind die Faktoren, */
	       /* mit denen die 3 verschiedenen Kriterien bei  */
	       /* der Expertenbeurteilung multipliziert werden.*/
	       /* Sie stehen in diesem Modul, weil ihre Bele-  */
	       /* gung in Abhaengigkeit davon erfolgt, ob eine */
               /* Domaene bekannt ist oder nicht.              */
               /************************************************/
               /* fuer den domaenenspezifischen Anteil         */
               /************************************************/
double             ExpDomFaktor;

               /************************************************/
               /* fuer den Anteil der Robustheit               */
               /************************************************/
double             ExpRobFaktor;

               /************************************************/
               /* fuer den beweisphasenspezifischen Anteil     */
               /************************************************/
double             ExpBewFaktor;

               /************************************************/
               /* fuer den Anteil des KZG                      */
               /************************************************/
double             ExpKZGFaktor;

               /************************************************/
               /* Die folgenden Variablen dienen zur Aufnahme  */
	       /* bestimmter Werte, die bei der Planungsphase  */
	       /* fuer Domaene benoetigt werden. Diese koennen */
	       /* in der Planungskonfigurationsdatei           */
	       /* angegeben werden. Siehe die Beschreibung auch*/
	       /* dort! In der Bezeichnung stimmen sie mit den */
	       /* Konstanten in domain.h ueberein.             */
               /************************************************/
int                maxreldomain;
int                nodomexp;
double             d_expdomfaktor;
double             d_exprobfaktor;
double             d_expbewfaktor;
double             d_expkzgfaktor;
double             nod_expdomfaktor;
double             nod_exprobfaktor;
double             nod_expbewfaktor;
double             nod_expkzgfaktor;


/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static bool          neues_match ( short idx );

static void          match_eintragen ( short idx );

static relation_typ  neu_dom_gef_dom_bez ( short idx1, short idx2 );

static relation_typ  neu_dom_alt_dom_bez ( short idx1, short idx2 );

static relation_typ  Relation ( short idx );

static void          AnalyseFoundDomains ( void );

static void          DefineKnownDomains ( void );

static long          berechne_akt_dom_bewertung ( short dom_nr, int faktor );

static void          JudgeDomains ( void );

static short         beste_dom_best ( short *laenge, short *feld );

static void          SelectDomains ( void );

static void          DefineWeights ( void );


/*****************************************************************************/
/*                                                                           */
/*            Elementare Funktionen fuer Domaenenspezifikationen             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  initialize_dom                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  notwendigen Initialisierungen im Bereich der Domaenen    */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                  DomainInitialzed                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void initialize_dom ( void )
{
  int i;
  int j;

  for ( i=0; i<MAXDOM_ANZ; i++ )
  {
    for ( j=0; j<MAXFUNCMATCH; j++ )
    {
      DomainGlobalInfo[i].DomMatches[j].erkannt = false;
      DomainGlobalInfo[i].DomMatches[j].relevant = false;
      DomainGlobalInfo[i].DomMatches[j].anz_einsaetze = 0;
      DomainGlobalInfo[i].DomMatches[j].bewertung = 0;
    }
    DomainGlobalInfo[i].anz_gef_matches        = 0;
    DomainGlobalInfo[i].supposed               = false;
    DomainGlobalInfo[i].bekannte_res.first     =  NULL;
    DomainGlobalInfo[i].bekannte_res.last      =  NULL;
    DomainGlobalInfo[i].bekannte_res.count     =  0;
    DomainGlobalInfo[i].bekannte_res.setcount  =  0;
    DomainGlobalInfo[i].schlechte_res.first    = NULL;
    DomainGlobalInfo[i].schlechte_res.last     = NULL;
    DomainGlobalInfo[i].schlechte_res.count    = 0;
    DomainGlobalInfo[i].schlechte_res.setcount = 0;
    DomainGlobalInfo[i].startphase = 0;
  }

  DomainInitialzed = true;
} /* Ende von initialize_dom */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_bei_dom_geeignet                                     */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                  Domaenennummer (es genuegt die Domaenennummer, das Funk- */
/* 		    tionssymbolmatch muss nicht angegeben werden.)           */
/*		    Pointer auf double                                       */
/*                                                                           */
/*  Returnwert   :  true, falls der angegebene Experte bei der angegebenen   */
/*		          Domaene als geeignet eingestuft ist                */
/*                  false sonst                                              */
/*                  Im positiven Fall wird in dem dritten Parameter die Guete*/
/*                  des Experten fuer diese Domaene zurueckgegeben.          */
/*                                                                           */
/*  Beschreibung :  Die Liste der gutgeeigneten Experten bei dieser Domaene  */
/*                  wird durchsucht.                                         */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool exp_bei_dom_geeignet( short exp_nr, short dom_nr, double *guete )
{
  int i;

  for ( i=0; i<DomainGlobalInfo[dom_nr].geeignete_exp_anz; i++ )
  {
    if ( !strcmp( DomainGlobalInfo[dom_nr].geeignete_exp[i].name,
		  ExpertGlobalInfo[exp_nr].exp_name ) )
    {
      *guete = DomainGlobalInfo[dom_nr].geeignete_exp[i].guete;
      return true;
    }
  } /* Ende von for */
  
  return false;
} /* Ende von exp_bei_dom_geeignet */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_bei_dom_ungeeignet                                   */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                  Domaenennummer (es genuegt die Domaenennummer, das Funk- */
/* 		    tionssymbolmatch muss nicht angegeben werden.)           */
/*		    Pointer auf double                                       */
/*                                                                           */
/*  Returnwert   :  true, falls der angegebene Experte bei der angegebenen   */
/*		          Domaene als ungeeignet eingestuft ist              */
/*                  false sonst                                              */
/*                  Im positiven Fall wird in dem dritten Parameter die Guete*/
/*                  des Experten fuer diese Domaene zurueckgegeben.          */
/*                                                                           */
/*  Beschreibung :  Die Liste der ungeeigneten Experten bei dieser Domaene   */
/*                  wird durchsucht.                                         */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool exp_bei_dom_ungeeignet( short exp_nr, short dom_nr, double *guete )
{
  int i;

  for ( i=0; i<DomainGlobalInfo[dom_nr].ungeeignete_exp_anz; i++ )
  {
    if ( !strcmp( DomainGlobalInfo[dom_nr].ungeeignete_exp[i].name,
		  ExpertGlobalInfo[exp_nr].exp_name ) )
    {
      *guete = DomainGlobalInfo[dom_nr].ungeeignete_exp[i].guete;
      return true;
    }
  } /* Ende von for */
  
  return false;
} /* Ende von exp_bei_dom_ungeeignet */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  FindFuncInDom                                            */
/*                                                                           */
/*  Parameter    :  Identifier des Funktionssymbols                          */
/*                  Nummer der Domaene, in dessen Sigantur das Funktions-    */
/*                  symbol vorkommen soll.                                   */
/*                                                                           */
/*  Returnwert   :  Falls das Symbol in der angegbenen Signatur vorkommt,    */
/*                  wird dessen Nummer in dieser Signatur zurueckgegeben,    */
/*                  ansonsten wird der Wert 0 zurueckgeliefert ( Die Num-    */
/*                  merierung der Funktionssymbole startet stets bei 1 !!)   */
/*                                                                           */
/*  Beschreibung :  Die Signatur der angegebenen Domaene wird durchsucht.    */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

function FindFuncInDom ( char *ident, short dom_nr )
{
  short i;

  for ( i=1; i<=DomainGlobalInfo[dom_nr].dom_fkt_anz; i++)
  {
    if ( !strcmp(DomainGlobalInfo[dom_nr].dom_funktion[i].ident, ident) )
    {
      return i;
    }
  } /* Ende von for */
  
  return 0;
} /* Ende von FindFuncInDom */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  new_rel_dom                                              */
/*                                                                           */
/*  Parameter    :  Pointer auf DomMatchSpec                                 */
/*                                                                           */
/*  Returnwert   :  true, wenn die angegebene Domaene (d.h. das angegebene   */
/*                        Match) in diesem Zyklus relevant ist und frueher   */
/*                        noch nicht eingesetzt wurde.                       */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es wird ueberprueft, ob bei dem angegebenen Match das    */
/*                  relevant-Flag gesetzt ist und ob es vorher schon mal zum */
/*                  Einsatz kam.                                             */
/*                  BEACHTE : Diese Funktion sollte nur aufgerufen werden,   */
/*                            wenn die Behandlung der Domaenen bereits abge- */
/*                            schlossen ist, denn es wird ueberprueft, ob    */
/*                            der Einsatzzaehler des Matchs auf 1 steht.     */
/*                            Das Erhoehen des Einsatzzaehlers erfolgt       */
/*                            naemlich dann, wenn die Domaene als relevant   */
/*                            eingestuft wird.                               */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */ 
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool new_rel_dom ( DomMatchSpec *domspec )
{
  /**********************/
  /* Testen ob relevant */
  /**********************/
  if( !domaene_relevant( domspec ) )
  {
    return false;
  }
  
  /***********************************************************/
  /* Testen auf fruehere Einsaetze, siehe obige Beschreibung */
  /***********************************************************/
  if( DomainGlobalInfo[domspec->dom_nr].DomMatches[domspec->match_nr].anz_einsaetze == 1 )
  {
    return true;
  }
  else
  {
    return false;
  }
} /* Ende von new_rel_dom */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  neues_match                                              */
/*                                                                           */
/*  Parameter    :  Index aus FoundDomain, er gibt an welches Match unter-   */
/*                  sucht werden soll.                                       */
/*                                                                           */
/*  Returnwert   :  true, falls das Match neu ist                            */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Diese Funktion testet, ob der Match, der durch den Index */
/*                  und die globalen Variablen FoundDomain und               */
/*                  FoundDomainMatchFunc gegeben ist, bereits bekannt ist.   */
/*                  Daf"ur werden bei der entsprechenden Dom"ane alle bisher */
/*                  gefundenen Matches durchlaufen.                          */
/*                                                                           */
/*  Globale Var. :  FoundDomainMatchFunc                                     */
/*                  FoundDomain                                              */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/
static bool neues_match ( short idx )
{
  short i, j;
  DomainFrame *dom_ptr; /* Dom"ane, die betrachtet wird */

	/*****************************************************/
	/* true, falls der Match mit einem bereits bekannten */
	/*       "ubereinstimmt.                             */
	/*****************************************************/
  bool match_gleich = false;
   
  dom_ptr = &(DomainGlobalInfo[FoundDomain[idx]]);

  if( !dom_ptr->anz_gef_matches )
  {
    return true;
  }

  /*************************************************/
  /* Es liegen bei der Dom"ane bereits Matches vor */
  /*************************************************/
  for( i=0; (i<dom_ptr->anz_gef_matches) && !match_gleich; i++ )
  {
    match_gleich = true;

    for(j=1; (j<=dom_ptr->dom_fkt_anz) && match_gleich; j++ )
    {
      if( dom_ptr->DomMatches[i].fkt_symb_zuord[j] != FoundDomainMatchFunc[idx][j] )
      {
        match_gleich = false;	
      }
    } /* Ende des inneren for */
  } /* Ende des "au"seren for */

  return !match_gleich;

} /* Ende von neues_match */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  match_eintragen                                          */
/*                                                                           */
/*  Parameter    :  Index aus FoundDomain, er gibt an welches Match einge-   */
/*                  tragen werden soll.                                      */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur tr"agt den Match, der durch den Index     */
/*                  und die globalen Variablen FoundDomain und               */
/*                  FoundDomainMatchFunc gegeben ist, in der externen Vari-  */
/*                  ablen DomainGlobalInfo ein.                              */
/*                                                                           */
/*  Globale Var. :  FoundDomainMatchFunc                                     */
/*                  FoundDomain                                              */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/
static void match_eintragen ( short idx )
{
  short i;
	   /***********************************************/
	   /* Anzahl der bisher gefundenen Matche bei der */
	   /* betarchteten Dom"ane.                       */
	   /***********************************************/
  short a_g_m;

  DomainFrame *dom_ptr; /* Dom"ane, die betrachtet wird */ 

  dom_ptr = &(DomainGlobalInfo[FoundDomain[idx]]);

  if ( (a_g_m = dom_ptr->anz_gef_matches ) < MAXFUNCMATCH )
  {
    MatchIdx[idx] = a_g_m;

    dom_ptr->DomMatches[a_g_m].erkannt = true;

    for( i=1; i<=dom_ptr->dom_fkt_anz; i++ )
    {
      dom_ptr->DomMatches[dom_ptr->anz_gef_matches].fkt_symb_zuord[i] = FoundDomainMatchFunc[idx][i];
    } /* Ende von for */
  } /* Ende von if */
  else
  {
    printf("     ************* Warnung *************\n");
    printf("     * Gefundenes Match konnte wegen   *\n");
    printf("     * Platzmangels nicht eingetragen  *\n");
    printf("     * werden.                         *\n");
    printf("     * ==> Konstante MAXFUNCMATCH      *\n");
    printf("     *     hochsetzen.                 *\n");
    printf("     ***********************************\n");
  } /* Ende von else */

  (dom_ptr->anz_gef_matches)++;
} /* Ende von match_eintragen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  neu_dom_gef_dom_bez                                      */
/*                                                                           */
/*  Parameter    :  Indexnummer des globalen Feldes FoundDomain              */
/*                  Indexnummer des globalen Feldes IntDomains               */
/*                                                                           */
/*  Returnwert   :  Beziehung des durch den ersten Index gegebenen Match zu  */
/*                  dem durch den zweiten Index gegebenen Match.             */
/*                  ALLGEMEINER : wenn der erste Match allgemeiner ist       */
/*                  SPEZIELLER  : wenn der erste Match spezieller ist        */
/*                  NEUTRAL     : wenn der erste Match in keiner Beziehung   */
/*                                zu dem zweiten Match steht                 */
/*                                                                           */
/*  Beschreibung :  Es wird untersucht, in welcher Beziehung der durch den   */
/*                  ersten Index und die globalen Variablen FoundDomain      */
/*                  und FoundDomainMatchFunc gegebene Match zu dem durch     */
/*                  den zweiten Index und IntDomains gegebene Match steht.   */
/*                                                                           */
/*                  @@@@@ Zur Zeit wird stets NEUTRAL zurueckgeliefert @@@@  */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static relation_typ neu_dom_gef_dom_bez ( short idx1, short idx2 )
{
  return NEUTRAL;
} /* Ende von neu_dom_gef_dom_bez */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  neu_dom_alt_dom_bez                                      */
/*                                                                           */
/*  Parameter    :  Indexnummer des globalen Feldes FoundDomain              */
/*                  Indexnummer des globalen Feldes InterestDomain           */
/*                                                                           */
/*  Returnwert   :  Beziehung des durch den ersten Index gegebenen Match zu  */
/*                  dem durch den zweiten Index gegebenen Match.             */
/*                  ALLGEMEINER : wenn der erste Match allgemeiner ist       */
/*                  SPEZIELLER  : wenn der erste Match spezieller ist        */
/*                  NEUTRAL     : wenn der erste Match in keiner Beziehung   */
/*                                zu dem zweiten Match steht                 */
/*                                                                           */
/*  Beschreibung :  Es wird untersucht, in welcher Beziehung der durch den   */
/*                  ersten Index und die globalen Variablen FoundDomain      */
/*                  und FoundDomainMatchFunc gegebene Match zu dem durch     */
/*                  den zweiten Index und InterestDomain gegebenen Match     */
/*                  steht.                                                   */
/*                                                                           */
/*                  @@@@@ Zur Zeit wird stets NEUTRAL zurueckgeliefert @@@@  */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static relation_typ neu_dom_alt_dom_bez ( short idx1, short idx2 )
{
  return NEUTRAL;
} /* Ende von neu_dom_alt_dom_bez */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  Relation                                                 */
/*                                                                           */
/*  Parameter    :  Index aus FoundDomain, er gibt an welches Match getestet */
/*                  werden soll.                                             */
/*                                                                           */
/*  Returnwert   :  Beziehung des Matches zu den bisher bekannten Matches    */
/*                  ALLGEMEINER : sie ist allgemeiner als eine bereits       */
/*                                bekannte Domaene                           */
/*                  SPEZIELLER : sie ist spezieller als eine bereits bekannte*/
/*                  NEUTRAL : sie steht in keiner Beziehung zu einer bekannte*/
/*                                                                           */
/*  Beschreibung :  Der durch den Index und die globalen Variablen           */
/*                  FoundDomainMatchFunc und FoundDomain gegebene Match wird */
/*                  darauf hin getestet, in welcher Beziehung es zu den be-  */
/*                  reits bekannten steht.                                   */
/*                  Ist er spezieller als ein bereits vor diesem Zyklus be-  */
/*                  kanntes Match, wird dessen Index bzgl. der externen Va-  */
/*                  riablen InterestDomain in der Variablen NotIntDomains    */
/*                  abgelegt;                                                */
/*                  ist er spezieller als ein vom Database im letzten Zyklus */
/*                  gefundener Match, der schon bearbeitet wurde und in der  */
/*                  Variablen IntDomains steht, so wird dessen Index aus dem */
/*                  Feld IntDomains wieder geloescht und die anderen Ein-    */
/*                  traege verschoben.                                       */
/*                                                                           */
/*  Globale Var. :  IntCount                                                 */
/*                  IntDomains                                               */
/*                  NotIntCount                                              */
/*                  NotIntDomains                                            */
/*                                                                           */
/*  Externe Var. :  DomainCount                                              */
/*                                                                           */
/*****************************************************************************/
static relation_typ Relation ( short idx )
{
  short i, j, k;
  relation_typ beziehung = NEUTRAL;

  /***************************************************************************/
  /* Zunaechst werden die Domaenen betrachtet, die bereits vor diesem Zyklus */
  /* bekannt waren :                                                         */
  /***************************************************************************/
  for ( i=0; i<DomainCount; i++ )
  {
    switch ( neu_dom_alt_dom_bez( idx, i ) )
    {
      case ALLGEMEINER : return ALLGEMEINER;

      case NEUTRAL     : break;

      case SPEZIELLER  : /* Die Indizes in NotIntDomains werden in aufstei-  */
                         /* gender Reihenfolge abgelegt; insbesondere wird   */
                         /* kein Index mehrmals abgespeichert.               */
                         for( j=0; j<NotIntCount; j++ )
                         {
                           if ( NotIntDomains[j] == i )
			   { /* Dom"ane bereits als uninteressant erkannt */
                             break;
			   }
                           if ( NotIntDomains[j] > i )
                           { /* Stelle zum Einfuegen gefunden */
                             for ( k=NotIntCount; k>j; k-- )
			     {
                               NotIntDomains[k] = NotIntDomains[k-1];
			     }
                             NotIntDomains[j] = i;
                             NotIntCount++;
                             break;
                           }
                         }

                         if ( j == NotIntCount )
			 { /* Dom"ane wird hinten angef"ugt */
                           NotIntDomains[NotIntCount++] = i;
			 }
                         beziehung = SPEZIELLER;
    }/* Ende von switch */
  } /* Ende von for */

  /***************************************************************************/
  /* Jetzt werden die vom Database-Experten im letzten Zyklus gefundenen und */
  /* bereits als interessant eingestuften Dom"anen betrachtet                */
  /***************************************************************************/
  for ( i=0; i<IntCount; i++ )
  {
    switch ( neu_dom_gef_dom_bez( idx, i ) )
    {
      case ALLGEMEINER : return ALLGEMEINER;

      case NEUTRAL     : break;

      case SPEZIELLER  : /* Loeschen der allgemeineren Domaene aus der Liste */
                         /* IntDomains durch Verschieben                     */
                         IntCount--;
                         for ( j=i; j<IntCount; j++ )
                         {
                           IntDomains[j] = IntDomains[j+1];
                         }

                         beziehung = SPEZIELLER;
    } /* Ende von switch */
  } /* Ende des 2. for */

  return beziehung;

} /* Ende von Relation */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  AnalyseFoundDomains                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur verarbeitet die Informationen, die ein    */
/*                  eventuell eingesetzter Database-Experte geliefert hat,   */
/*                  indem sie die Variablen IntDomains und NotIntDomains     */
/*                  versorgt, also festlegt, welche der neu gefundenen Do-   */
/*                  m"anen interessant sind und welche der bisher bekannten  */
/*                  uninteressant geworden sind.                             */
/*                  Die Variable NotIntDomains wid dabei von der Funktion    */
/*                  Relation versorgt.                                       */
/*                                                                           */
/*                  Die Inmformationen des Database-Experten stehen in den   */
/*                  Variablen FoundDomainCount, FoundDomain und              */
/*                  FoundDomainMatchFunc.                                    */
/*                                                                           */
/*  Globale Var. :  FoundDomainCount                                         */ 
/*                  FoundDomain                                              */
/*                  FoundDomainMatchFunc                                     */ 
/*                  IntDomains                                               */
/*                  IntCount                                                 */
/*                  NotIntCount                                              */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static void AnalyseFoundDomains ( void )
{
  short i;

  /*******************/
  /* Initialisierung */
  /*******************/
  NotIntCount = 0;
  IntCount = 0;

  /*****************************************/
  /* Durchlaufen aller gefundenen Dom"anen */
  /*****************************************/
  for( i=0; i<FoundDomainCount; i++ )
  {
    if( neues_match( i ) )
    {
      match_eintragen( i );

      switch( Relation( i ) )
      {
	case ALLGEMEINER : break;

	case NEUTRAL     :
	case SPEZIELLER  : IntDomains[IntCount++] = i;
			   break;
      } /* Ende von switch */
    } /* Ende von if */
  } /* Ende von for */
} /* Ende von AnalyseFoundDomains */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DefineKnownDomains                                       */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur verarbeitet die Informationen, die ein    */
/*                  eventuell eingesetzter Database-Experte geliefert hat.   */
/*                  Sie sollte erst nach demm ersten Zyklus aufgerufen       */
/*                  werden, da vor dem ersten Zyklus keine Dom"anen bekannt  */
/*                  sein k"onnen, und nur wenn der Database-Experte im       */
/*                  Team gewesen ist, denn nur dann k"onnen neue Dom"anen    */
/*                  erkannt worden sein.                                     */
/*                                                                           */
/*                  Vorher mu"s eine Funktion aufgerufen werden, die die     */
/*                  gefundenen Dom"anen des Database-Experten analysiert     */
/*                  und die Variablen IntDomains und NotIntDomains versorgt. */
/*                                                                           */
/*                  Nach dem Festlegen der interessanten Dom"anen f"ur den   */
/*                  n"achsten Zyklus, wird festgelegt, wann der Database-    */ 
/*                  Experte das n"achste Mal fr"uhestens eingesetzt werden   */ 
/*                  kann.                                                    */ 
/*                                                                           */
/*  Globale Var. :  FoundDomainCount                                         */ 
/*                  FoundDomain                                              */
/*                  FoundDomainMatchFunc                                     */ 
/*                  IntDomains                                               */
/*                  IntCount                                                 */
/*                  NotIntDomains                                            */
/*                  NotIntCount                                              */
/*                                                                           */
/*  Externe Var. :  PlanDocum                                                */
/*                  plan_doc                                                 */
/*                                                                           */
/*****************************************************************************/

static void DefineKnownDomains ( void )
{
  short i;
  short j;

  short naechster_einsatz_db;
  short verschiebung;

  DomainFrame *dom_ptr;
  
  /***********************************************************************************/
  /* Ausgabe der Ergebnisse des Database-Experten, falls Planungsdokumentation aktiv */
  /***********************************************************************************/
  if ( PlanDocum )
  {
    fprintf( plan_doc, "%2d Domaenen wurden als nicht mehr interessant erkannt\n",NotIntCount);
    for ( i=0; i<NotIntCount; i++ )
    {
      dom_ptr = &(DomainGlobalInfo[InterestDomain[NotIntDomains[i]].dom_nr]);
      fprintf( plan_doc, "Domaene %s mit der Substitution :\n", dom_ptr->dom_name );
      for ( j=1; j<=dom_ptr->dom_fkt_anz; j++ )
      {
	fprintf( plan_doc, "%10s  -->  %10s\n", dom_ptr->dom_funktion[j].ident,
		Function[dom_ptr->DomMatches[InterestDomain[NotIntDomains[i]].match_nr].fkt_symb_zuord[j]].ident );
      } /* Ende von for */

      fprintf(plan_doc, "\n");
    }

    fprintf( plan_doc, "%2d Domaenen wurden neu als interessant erkannt \n", FoundDomainCount );
    for ( i=0; i<IntCount; i++ )
    {
      dom_ptr = &(DomainGlobalInfo[FoundDomain[IntDomains[i]]]);
      fprintf( plan_doc, "Domaene %s mit der Substitution :\n", dom_ptr->dom_name );flush();
      for ( j=1; j<=dom_ptr->dom_fkt_anz; j++ )
      {
	fprintf( plan_doc, "%10s  -->  %10s\n", dom_ptr->dom_funktion[j].ident,
			   Function[FoundDomainMatchFunc[IntDomains[i]][j]].ident );
      }

      fprintf(plan_doc, "\n");
    }
  } /* Ende von if ( PlanDocum ) */

  /**************************************************************************/
  /* Zunaechst werden die Matches aus der Liste InterestDomain eliminiert,  */
  /* die der Database-Experte als uninteressant erkannt hat.                */
  /* Dabei wird ausgenutzt, dass die Eintraege in NotIntDomains aufsteigend */
  /* abgelegt sind.                                                         */
  /**************************************************************************/
  j = 0;
  verschiebung = 0;
  if ( NotIntCount )
  {
    for ( i=0; i<DomainCount-verschiebung; i++ )
    {
      if ( i == NotIntDomains[j] )
      {
	verschiebung++;
	j++;
      }
      InterestDomain[i] = InterestDomain[i+verschiebung];
    }
  } /* Ende des aeusseren if */
  
  DomainCount -= NotIntCount;

  /*********************************************************************/
  /* Es werden nun die (als interessant) erkannten Matches eingetragen */
  /*********************************************************************/
  j = 0; /* j dient zum Durchlaufen von IntDomains */
  for ( i=0; i<FoundDomainCount; i++ )
  {
    if ( IntDomains[j] == i ) /* Das Match ist auch interessant */
    {
      InterestDomain[DomainCount].dom_nr = FoundDomain[i];
      InterestDomain[DomainCount++].match_nr = MatchIdx[i];
      DomainGlobalInfo[FoundDomain[i]].DomMatches[MatchIdx[i]].interessant = true;
      j++;
    }
  } /* Ende von for */

  /*******************************************************************************/
  /* Ausgabe der neuen interessanten Dom"anen, falls Planungsdokumentation aktiv */
  /*******************************************************************************/
  if ( PlanDocum )
  {
    fprintf( plan_doc, "%2d interessante Domaenen fuer den %d. Zyklus : \n", DomainCount, CycleCount+1 ); 
    for ( i=0; i<DomainCount; i++ )
    {
      dom_ptr = &(DomainGlobalInfo[InterestDomain[i].dom_nr]);
      fprintf( plan_doc, "Domaene %s mit der Substitution :\n", dom_ptr->dom_name );
      for ( j=1; j<=dom_ptr->dom_fkt_anz; j++ )
      {
         fprintf( plan_doc, "%10s  -->  %10s\n", dom_ptr->dom_funktion[j].ident,
                Function[dom_ptr->DomMatches[InterestDomain[i].match_nr].fkt_symb_zuord[j]].ident );
      }

      fprintf(plan_doc, "\n");
    }
    fprintf(plan_doc, "\n");
  }

  /*********************************************************************/
  /* Festlegen des fr"uhesten n"achsten Einsates des Database-Experten */
  /*********************************************************************/
  naechster_einsatz_db = cycle_diff_db * Database.anz_einsaetze;
  if( HostCount < host_limit_db )
  {
    naechster_einsatz_db *= 2;
  }
  if( DomainCount > dom_limit_db )
  {
    naechster_einsatz_db *= 2;
  }
  Database.wartekonto = CycleCount + naechster_einsatz_db;

  /****************************************************************/
  /* Dokumentation der Bestimmung von naechster_einsatz_db, falls */
  /* Planungsdokumentation aktiv                                  */
  /****************************************************************/
  if( PlanDocum )
  {
    fprintf( plan_doc, "      Naechster Einsatz des Database-Experten fruehestens\n");
    fprintf( plan_doc, "      im %d.ten Zyklus.\n", Database.wartekonto );
    fprintf( plan_doc, "      Mindestabstand ist : %d(cycle_diff_db) * %d(Anzahl bisheriger Einsaetze) = %d Zyklen\n",
	     cycle_diff_db, Database.anz_einsaetze, cycle_diff_db * Database.anz_einsaetze);
    if( HostCount < host_limit_db )
    {
      fprintf( plan_doc,  "      Es stehen weniger als %d Rechner zur Verfuegung,\n", host_limit_db);
      fprintf( plan_doc,  "      deswegen verdoppelt sich der Mindestabstand.\n");
    }
    if( DomainCount > dom_limit_db )
    {
      fprintf( plan_doc,  "      Es sind mehr als %d Domaenen als interessant eingestuft\n", dom_limit_db);
      fprintf( plan_doc,  "      deswegen verdoppelt sich der Mindestabstand.\n");
    }
  } /* Ende von if( PlanDocum ) */

} /* Ende von DefineKnownDomains */

/*****************************************************************************/
/*                                                                           */
/*                 Hilfsfunktionen fuer JudgeDomains                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  berechne_akt_dom_bewertung                               */
/*                                                                           */
/*  Parameter    :  Domaenennummer                                           */
/*                  Faktor, mit dem die Beurteilung modifiziert werden kann. */
/*                                                                           */
/*  Returnwert   :  Die fuer die Domaene berechnete Bewertung                */
/*                                                                           */
/*  Beschreibung :  Es werden all die Experten betrachtet, die bei der an-   */
/*                  gegebenen Domaene als geeignet eingestuft sind und im    */
/*                  letzten Zyklus im Team gewesen sind. Der Mittelwert aus  */
/*                  deren relativen Bewertungen ergibt die Beurteilung der   */
/*                  Domaene. Diese wird durch den Faktor gewichtet, so dass  */
/*                  bestimmte Domaenen anderen bevorzugt werden koennen.     */
/*                  Bis jetzt erfolgt diese Berechnung nur Domaenenabhaengig */
/*                  ohne Beruecksichtigung des speziellen Funktionssymbol-   */
/*                  Matches; falls moeglich sollte dies auch mitberuecksich- */
/*                  tigt werden.                                             */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                  nodomexp                                                 */
/*                                                                           */
/*  Externe Var. :  ExpertGlobalInfo                                         */
/*                  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static long berechne_akt_dom_bewertung ( short dom_nr, int faktor )
{
  short exp_anzahl = 0;
  long  summe      = 0;
  int   exp_nr;

  int i;

  for ( i=0; i<DomainGlobalInfo[dom_nr].geeignete_exp_anz; i++ )
  {
    if ( ( exp_nr = get_exp_nr( DomainGlobalInfo[dom_nr].geeignete_exp[i].name ) ) != NOEXPERT )
    {
      if ( exp_im_alten_team ( exp_nr ) )
      {
	exp_anzahl++;
	summe += ExpertGlobalInfo[exp_nr].rel_bewertung[CycleCount];
      }
    }
  }/* Ende von for */
  
    return ( exp_anzahl ? ( summe*faktor/exp_anzahl) : nodomexp );
} /* Ende von berechne_akt_dom_bewertung */  

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  JudgeDomains                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Alle als interessant eingestuften Domaenen werden hier   */
/*                  beurteilt.                                               */
/*                  Dazu wird die bisherige Gesamtbewertung und die aktuelle */
/*                  aufaddiert und durch 2 geteilt. Dadurch bekommt die ak-  */
/*                  tuelle, d.h. die des letzten Zyklus ein groesseres Ge-   */
/*                  wicht im Vergleich zu den vorigen.                       */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                  DomainCount                                              */
/*                  InterestDomain                                           */
/*                  nodomexp                                                 */
/*                                                                           */
/*  Externe Var. :  PlanDocum                                                */
/*                  plan_doc                                                 */
/*                                                                           */
/*****************************************************************************/

static void JudgeDomains ( void )
{
  int i;
  int bew;

  long *dom_bew;

  if ( PlanDocum )
  {
    fprintf( plan_doc, "Bewertungen der zur Zeit interessanten Domaenen (%d Stueck) : \n",
	    DomainCount);
  }

  for ( i=0; i<DomainCount; i++ )
  {
    dom_bew = &(DomainGlobalInfo[InterestDomain[i].dom_nr].DomMatches[InterestDomain[i].match_nr].bewertung);

    if ( (bew = berechne_akt_dom_bewertung( InterestDomain[i].dom_nr, 1 )) == nodomexp )
    {
      if ( PlanDocum )
      {
	fprintf( plan_doc, "Kein Experte fuer die %d. interessante Domaene %s eingesetzt.\n",
		 i, DomainGlobalInfo[InterestDomain[i].dom_nr].dom_name);
      }
    }
    
    *dom_bew += bew;

    *dom_bew /= 2;
    
    if ( PlanDocum )
    {
      fprintf( plan_doc, "%2d. interessante Domaene %s : aktuelle Bewertung : %3d\n", 
	       i+1, DomainGlobalInfo[InterestDomain[i].dom_nr].dom_name, bew );
      fprintf( plan_doc, "                            Gesamtbewertung    : %3d\n", *dom_bew );
    }
  } /* Ende von for */
} /* Ende von JudgeDomains */


/*****************************************************************************/
/*                                                                           */
/*                 Hilfsfunktionen fuer SelectDomains                        */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  domaene_in_startphase                                    */
/*                                                                           */
/*  Parameter    :  Pointer auf eine Domaenenspezifikation, d.h. sowohl Do-  */
/*                  maenennummer als auch die Nummer der Funktionssymbolzu-  */
/*                  ordnung muss bekannt sein.                               */
/*                                                                           */
/*  Returnwert   :  true, falls sich die angegebene Domaenespezifikation in  */
/*                        der Startphase befindet                            */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es wird getestet, ob die Domaene neu ist, oder ob die    */
/*                  Anzahl der bisherigen Einsaetze kleiner ist als die fuer */
/*                  die Startphase angegebenen.                              */
/*                  BEACHTE : Diese Funktion sollte nur aufgerufen werden,   */
/*                            bevor die Anzahl der Einsaetze der Domaenen    */
/*                            um 1 erhoeht wird; ansonsten muss aus dem      */
/*                            "kleiner" ein kleiner-gleich" werden!!         */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool domaene_in_startphase ( DomMatchSpec *domspec )
{
  if ( domaene_neu( domspec ) || 
       ( DomainGlobalInfo[domspec->dom_nr].
	 DomMatches[domspec->match_nr].anz_einsaetze <
	 DomainGlobalInfo[domspec->dom_nr].startzyklen_anz ) )
  {
    return true;
  }
  
  return false;
} /* Ende von domaene_in_startphase */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  beste_dom_best                                           */
/*                                                                           */
/*  Parameter    :  2 Pointer auf short-Variablen                            */
/*                  Der erste Pointer zeigt auf eine short-Variable, die an- */
/*                  gibt, wie gross das Feld ist, in dem gesucht werden muss.*/
/*                  Der zweite Pointer zeigt auf den Anfang dieses Feldes.   */
/*                                                                           */
/*  Returnwert   :  Index im globalen Feld InterestDomain der Domaene, die   */
/*                  die beste Beurteilung von den Domaenen hat, die in dem   */
/*                  angegebenen Feld vorkommen.                              */ 
/*                                                                           */
/*  Beschreibung :  Die Eintragungen in dem uebergebenen Feld sind die In-   */
/*                  dizes der Domaene in dem Feld InterestDomain.            */
/*                  Der Index der am besten beurteilten Domaene wird durch   */
/*                  den Index der am weitesten rechts steht ueberschrieben   */
/*                  und die uebergebene Laenge des Feldes um 1 verringert, so*/
/*                  dass auf den durch die Laenge angegebenen ersten Plaetzen*/
/*                  immer die Indizes stehen, die eventuell noch betrachtet  */
/*                  werden muessen.                                          */
/*                  Es wird vorausgesetzt, dass wenigstens ein "sinnvoller"  */
/*                  Eintrag in dem uebergebenen Feld vorkommt und dass also  */
/*                  auch die angegebene Laenge wenigstens 1 betraegt; anson- */
/*                  sten koennen Fehler auftreten.                           */
/*                                                                           */
/*  Globale Var. :  InterestDomain                                           */
/*                  DomainGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static short beste_dom_best ( short *laenge, short *feld )
{
  int    i;
  short  index_von_max;         /* Dies ist der Index in InterestDomain    */ 
  short  index_in_feld = 0;     /* Dies ist der Index im uebergebenen Feld */

  index_von_max = feld[0];  

  for ( i=1; i<*laenge; i++ )
  {
    if ( DomainGlobalInfo[InterestDomain[index_von_max].dom_nr].
	     DomMatches[InterestDomain[index_von_max].match_nr].bewertung >
	 DomainGlobalInfo[InterestDomain[feld[i]].dom_nr].
	     DomMatches[InterestDomain[feld[i]].match_nr].bewertung )
    {
      index_von_max = feld[i];
      index_in_feld = i;
    }
  } /* Ende von for */

  --(*laenge);

  if ( *laenge ) /* das bisherige Maximum muss durch den am weitesten rechts */
  {              /* stehenden Eintrag ersetzt werden.                        */
    feld[index_in_feld] = feld[*laenge]  ;
  }
  
  return index_von_max;
} /* Ende von beste_dom_best */
  

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  dom_spezieller                                           */
/*                                                                           */
/*  Parameter    :  Index des globalen Feldes InterestDomain, d.h. es wird   */
/*                  eine Dom"ane angegeben.                                  */
/*            Auf die eigentliche Dom"ane kann also wie folgt zugegriffen    */
/*            werden:                                                        */
/*              DomainGlobalInfo[InterestDomain[dom_idx].dom_nr].***         */
/*            werden:                                                        */
/*                                                                           */
/*  Returnwert   :  true, falls die angegebene Dom"ane spezieller ist als    */
/*                        eine Dom'ane, die im letzten Zyklus relevant war   */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Momentan wird stets true zur"uckgeliefert.               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static bool dom_spezieller ( short dom_idx )
{
  return true;
} /* Ende von dom_spezieller */
  
/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SelectDomains                                            */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden die relevanten Domaenen fuer den naechsten     */
/*                  Zyklus bestimmt. Dabei gilt folgende Abstufung :         */
/*                  1) Domaenen, die relevant waren und noch in der Start-   */
/*                     phase sind, bleiben relevant.                         */
/*                  2) Dann kommen die Domaenen, die bisher noch gar nicht   */
/*                     eingesetzt wurden.                                    */
/*                     Dabei werden die Dom"anen, die spezieller sind als    */
/*                     eine relevante Dom"ane des letzten Zyklus als erstes  */
/*                     als relevant erkl"art                                 */
/*                  3) Zum Schluss entscheidet die Domaenenbeurteilung, falls*/
/*                     nach Betrachten der ersten zwei Kriterien immer noch  */
/*                     nicht die maximale Anzahl relevanter Domaenen erreicht*/
/*                     ist.                                                  */
/*                  Alle als interessant eingestuften Domaenen werden nach   */
/*                  diesen 3 Kriterien untersucht. Zur Verwaltung dieser 3   */
/*                  Mengen werden 3 Felder von Domaenenspezifikationen lokal */
/*                  angelegt. Die Mengen werden anhand der Indizierung in    */
/*                  InterestDomain gebildet.                                 */
/*                                                                           */
/*                  Zum Schluss werden bei den Domaenen, die als relevant    */
/*                  eingestuft sind, der naechste Zyklus als Einsatz einge-  */
/*                  tragen; d.h. eine Domaene gilt als in einem Zyklus ein-  */
/*                  gesetzt, wenn sie als relevant gekennzeichnet ist.       */
/*                                                                           */
/*                  Zu ueberlegen ist, ob evtl. bei den Domaenen analog zu   */
/*                  den Experten ein Wartekonto miteingebaut wird.           */
/*                                                                           */
/*  Globale Var. :  DomainGlobalInfo                                         */
/*                  InterestDomain                                           */
/*                  DomainCount                                              */
/*                  maxreldomain                                             */
/*                  TeamDomCount                                             */
/*                  DomainTeamInfo                                           */
/*                                                                           */
/*  Externe Var. :  PlanDocum                                                */
/*                  plan_doc                                                 */
/*                  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static void SelectDomains ( void )
{
  int i, j;

  short neue_domaenen[MAXDOM_ANZ];
  short anz_neue_domaenen = 0;

  short beurteilte_domaenen[MAXDOM_ANZ];
  short anz_beurteilte_domaenen = 0;

  short relevante_domaenen[MAXDOM_ANZ];
  short anz_relevante_domaenen = 0;

  /***************************************************************************/
  /* Alle interessanten Domaenen werden anhand der 3 Kriterien betrachtet.   */
  /***************************************************************************/
  for ( i=0; i<DomainCount; i++ )
  {
    /************************************************/
    /* Festlegen, ob Domaene noch in der Startphase */
    /************************************************/
    if ( domaene_in_startphase( &(InterestDomain[i]) ) )
    {
      DomainGlobalInfo[InterestDomain[i].dom_nr].startphase = true;
    }
    else
    {
      DomainGlobalInfo[InterestDomain[i].dom_nr].startphase = false;
    }

    if ( domaene_relevant ( &(InterestDomain[i]) ) &&
	 DomainGlobalInfo[InterestDomain[i].dom_nr].startphase )
    {
      relevante_domaenen[anz_relevante_domaenen++] = i;
    }
    else if ( domaene_neu( &(InterestDomain[i]) ) )
    {
      neue_domaenen[anz_neue_domaenen++] = i;
    }
    else
    {
      beurteilte_domaenen[anz_beurteilte_domaenen++] = i;
    }
  } /* Ende von for */

  /***************************************************************************/
  /* Jetzt werden eventuell noch freie Domaenenplaetze fuer den naechsten    */
  /* Zyklus vergeben, falls noch potentielle Kandidaten da sind.             */
  /***************************************************************************/
  /* Zuerst werden die neuen Dom"anen betrachtet, die spezieller sind als    */
  /* eine relevante Dom"ane des letzten Zyklus.                              */
  /***************************************************************************/

  i = 0;
  while ( ( maxreldomain > anz_relevante_domaenen ) && anz_neue_domaenen )
  {
    if( dom_spezieller( neue_domaenen[i] ) )
    {
      relevante_domaenen[anz_relevante_domaenen++] = neue_domaenen[i];
      anz_neue_domaenen--;
      /*********************************************/
      /* Verschieben der restlichen neuen Dom"anen */
      /*********************************************/
      for(j=i; j<anz_neue_domaenen; j++)
      {
	neue_domaenen[j] = neue_domaenen[j+1];
      }
    }
  } /* Ende von while */
	
  /**********************************************/
  /* Jetzt werden die neuen Dom"anen betrachtet */
  /**********************************************/
  while ( ( maxreldomain > anz_relevante_domaenen ) && anz_neue_domaenen )
  {
    relevante_domaenen[anz_relevante_domaenen++] = 
				    neue_domaenen[--anz_neue_domaenen];
  } /* Ende von while */

  /*****************************************************************/
  /* Jetzt werden die restlichen interessanten Dom"anen betrachtet */
  /*****************************************************************/
  while ( ( maxreldomain > anz_relevante_domaenen ) &&  anz_beurteilte_domaenen )
  {
    relevante_domaenen[anz_relevante_domaenen++] =
	      beste_dom_best(&anz_beurteilte_domaenen, beurteilte_domaenen);
  } /* Ende von while */

  /****************************************************************/
  /* Die relevanten Domaenen des letzten Zyklus werden geloescht. */
  /****************************************************************/
  for ( i=0; i<TeamDomCount;i++ )
  {
    DomainGlobalInfo[DomainTeamInfo[i].dom_nr].
	      DomMatches[DomainTeamInfo[i].match_nr].relevant = false;
  }
  
  /***************************************************************************/
  /* Die relevanten Domaenen des naechsten Zyklus werden als solche abgelegt */
  /***************************************************************************/
  TeamDomCount = anz_relevante_domaenen;

  if ( PlanDocum )
  {
    fprintf( plan_doc, "\n\n Hieraus folgt fuer die relevanten Domaenen : \n");
    fprintf( plan_doc, "%d relevante Domaenen fuer den %d. Zyklus : \n", TeamDomCount, CycleCount + 1);
  }

  for ( i=0; i<TeamDomCount;i++ )
  {
    DomainTeamInfo[i].dom_nr = InterestDomain[relevante_domaenen[i]].dom_nr;
    DomainTeamInfo[i].match_nr = InterestDomain[relevante_domaenen[i]].match_nr;

    DomainGlobalInfo[DomainTeamInfo[i].dom_nr].
	      DomMatches[DomainTeamInfo[i].match_nr].relevant = true;

    /****************************/
    /* Einsatz wird eingetragen */
    /****************************/
    DomainGlobalInfo[DomainTeamInfo[i].dom_nr].DomMatches[DomainTeamInfo[i].match_nr].einsaetze
	    [DomainGlobalInfo[DomainTeamInfo[i].dom_nr].DomMatches[DomainTeamInfo[i].match_nr].anz_einsaetze++] = CycleCount;

  } /* Ende von for */
} /* Ende von SelectDomains */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DefineWeights                                            */
/*                                                                           */
/*  Parameter    :  kein                                                     */
/*                                                                           */
/*  Returnwert   :  kein                                                     */
/*                                                                           */
/*  Beschreibung :  In Abhaengigkeit, ob mndestens eine Domaene im naechsten */
/*                  Zyklus bekannt ist, werden in dieser Prozedur die Fakto- */
/*                  ren zur Bewertung der Experten festgelegt. Dies geschieht*/
/*                  zur Zeit mit Hilfe von Konstanten, kann spaeter aber mit */
/*                  Hilfe von Funktionen erfolgen.                           */
/*                  Insbesondere kann der Domaenenfaktor differenzierter be- */
/*                  trachtet werden.                                         */
/*                                                                           */
/*  Globale Var. :  ExpDomFaktor                                             */
/*                  ExpRobFaktor                                             */
/*                  ExpBewFaktor                                             */
/*                  ExpKZGFaktor                                             */
/*                  d_expdomfaktor  d_exprobfaktor                           */
/*                  d_expbewfaktor  d_expkzgfaktor                           */
/*                  nod_expdomfaktor   nod_exprobfaktor                      */
/*                  nod_expbewfaktor   nod_expkzgfaktor                      */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                  plan_doc                                                 */
/*                  PlanDocum                                                */
/*                                                                           */
/*****************************************************************************/

static void DefineWeights ( void )
{
  if ( TeamDomCount )  /* Also sind Domaenen bekannt */
  {
    ExpDomFaktor = d_expdomfaktor;
    ExpRobFaktor = d_exprobfaktor;
    ExpBewFaktor = d_expbewfaktor;
    ExpKZGFaktor = d_expkzgfaktor * CycleCount;
  }
  else
  {
    ExpDomFaktor = nod_expdomfaktor; 
    ExpRobFaktor = nod_exprobfaktor;
    ExpBewFaktor = nod_expbewfaktor;
    ExpKZGFaktor = nod_expkzgfaktor * CycleCount;
  }

  if ( PlanDocum )
  {
    fprintf( plan_doc, "\n\nBerechnete Faktoren :\n");
    fprintf( plan_doc, "       Faktor fuer das Domaenenwissen : %2f\n",ExpDomFaktor );
    fprintf( plan_doc, "       Faktor fuer die Robustheit     : %2f\n",ExpRobFaktor );
    fprintf( plan_doc, "       Faktor fuer die Beweisphase    : %2f\n",ExpBewFaktor );
    fprintf( plan_doc, "       Faktor fuer das KZG            : %2f\n",ExpKZGFaktor );
    fprintf( plan_doc, "\n" );
  }
} /* Ende von DefineWeights */
    

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ResetDomainVariables                                     */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur setzt alle globalen Variablen, die fuer   */
/*                  die Domaenenbehandlung relevant sind wieder auf ihren    */
/*                  Initialwert.                                             */
/*                                                                           */
/*  Globale Var. :  FoundDomainCount                                         */
/*                                                                           */
/*  Externe Var. :  NotIntCount                                              */
/*                  IntCount                                                 */
/*                                                                           */
/*****************************************************************************/

void ResetDomainVariables ( void )
{
  FoundDomainCount = 0;
  NotIntCount = 0;
  IntCount = 0;
} /* Ende von ResetDomainVariables */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DomainManagement                                         */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur ist die zentrale Verwaltungseinheit fuer  */
/*                  die Arbeit mit Domaenen waehrend der Planungsphase des   */
/*                  Leiters.                                                 */
/*                                                                           */
/*  Globale Var. :  DomainCount                                              */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

void DomainManagement ( void )
{
  
  if( CycleCount && exp_im_alten_team( DATABASE ) )
  {
    AnalyseFoundDomains();

    DefineKnownDomains();
  }

  if ( DomainCount )
  {
    JudgeDomains();

    SelectDomains();
  }

  DefineWeights();
  
  ResetDomainVariables();

} /* Ende von DomainManagement */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  dom_global_debug                                         */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Dieses Modul gibt die Belegungen aller globalen Variablen*/
/*                  des Moduls domain.c aus. Fuer Testzwecke.                */
/*                                                                           */
/*                                                                           */
/*  Globale Var. :  ExpDomFaktor  ExpBewFaktor                               */
/*                  ExpRobFaktor  ExpKZGFaktor                               */
/*                  InterestDomain                                           */
/*                  TeamDomCount                                             */
/*                  DomainTeamInfo                                           */
/*                  DomainCount                                              */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void dom_global_debug ( void )
{
  short i;

  printf("Anzahl der relevanten Domaenen des naechsten Zyklus : %d \n",TeamDomCount);
  for(i=0;i<TeamDomCount;i++)
  {
    printf("%d-te relevante Domaene : %d\n",i,DomainTeamInfo[i].dom_nr);
  }
  
  printf("\nAnzahl der interessanten Domaenen : %d\n", DomainCount );
  for(i=0;i<DomainCount;i++)
  {
    printf("%d-te interessante Domaene : %d\n",i,InterestDomain[i].dom_nr);
  }
  
  printf("\nBewertungsfaktor fuer Domaenenteil : %2f\n",ExpDomFaktor);
  printf("\nBewertungsfaktor fuer Beweisanteil : %2f\n",ExpBewFaktor);
  printf("\nBewertungsfaktor fuer Robustheit   : %2f\n",ExpRobFaktor);
  printf("\nBewertungsfaktor fuer KZG-Anteil   : %2f\n",ExpKZGFaktor);

} /* Ende von dom_global_debug */

