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

#ifndef __DOMAIN
#define __DOMAIN

#include "scanner.h"
#include "termpair.h"


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* Rueckgabewert von get_dom_nr, falls der      */
	       /* uebergebene String kein bekannter            */
               /* Doamenenname ist.                            */
               /************************************************/
#define NODOMAIN        -1

               /************************************************/
               /* Maximale Anzahl von relevanten Domaenen fuer */
	       /* einen Zyklus                                 */
               /************************************************/
#define MAXRELDOMAIN     1

               /************************************************/
               /* maximale Zahl von Parametern in einer Para-  */
	       /* meterliste                                   */
               /************************************************/
#define MAXPARAM        20 

               /************************************************/
		/* maximale Zahl von Domaenendefinitionen      */
               /************************************************/
#define MAX_DEF         10

               /************************************************/
	       /* max. Zahl von Startzyklen bei einer Domaene  */
               /************************************************/
#define MAX_ZYKLEN_ANZ  10

               /************************************************/
               /* maximale Zahl von Zyklen, ueber die Infor-   */
	       /* mationen gespeichert werden                  */ 
               /************************************************/
#define MAXCYCLE       500   

               /************************************************/
               /* Eine Domaene kann ja eventuell in einer Pro- */
	       /* blemspezifikation auf mehrere Funktions-     */
	       /* symbolgruppen zutreffen ( z.B. in einem Ring */
	       /* bilden beide Verknuepfungen eine Gruppe). Die*/
	       /* folgende Konstante beschraenkt die Anzahl,   */
               /* wievielen verschiedenen Gruppen von Funk-    */
	       /* tionssymbolen dieselbe Domaene zugeordnet    */
	       /* werden kann.                                 */
               /************************************************/
#define MAXFUNCMATCH     3

               /************************************************/
               /* In berechne_akt_dom_bewertung, wenn kein     */
	       /* Experte fuer eine Domaene im Team            */
               /************************************************/
#define NODOMEXP         0

               /************************************************/
               /* Die folgenden Konstanten belegen die 3 glo-  */
	       /* balen Variablen in Abhaengigkeit davon, ob   */
	       /* eine Domaene bekannt ist oder nicht :        */
               /************************************************/
               /* Domaene bekannt : */
	       /*********************/
#define  D_EXPDOMFAKTOR  4
#define  D_EXPROBFAKTOR  1
#define  D_EXPBEWFAKTOR  2
#define  D_EXPKZGFAKTOR  2

	       /***************************/
               /* keine Domaene bekannt : */
	       /***************************/
#define  NOD_EXPDOMFAKTOR  0
#define  NOD_EXPROBFAKTOR  3
#define  NOD_EXPBEWFAKTOR  3
#define  NOD_EXPKZGFAKTOR  2

               /************************************************/
               /* Wert, der der ersten Komponete der Liste der */
	       /* angegebenen Zyklendauern zugewiesen wird,    */
	       /* wenn in der Domaenenspezifikation keine Zy-  */
	       /* klusdauer angegewurde.                       */
               /************************************************/
#define  NO_CYCLE_TIME     -11


/*****************************************************************************/
/*                                                                           */
/*                            Typdefinitionen                                */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* Statustyp der Domaene anhand der Konfigura-  */
	       /* tionsdatei                                   */
               /************************************************/
typedef enum { SUPPOSED, KNOWN } DomStatusType;


               /************************************************/
               /* Datenstruktur zur Aufnahme der Information   */
	       /* ueber ein Funktionssymbol in einer Domaenen- */
	       /* spezifikation; gespeichert wird zunaechst :  */
               /*          - der Identifier                    */
               /*          - die Stelligkeit                   */
               /* Wenn die Domaene vom Database-Experten be-   */
	       /* trachtet wird, wird auch noch eine eventuell */
	       /* gefundene Substitution fuer dieses Funktions-*/
	       /* symbol in fmatch abgelegt, sowie noch zwei   */
	       /* Feldindizes, die auf die Variable SubstMatrix*/
	       /* bezogen sind und zum Finden aller moeglichen */
	       /* Substitutionen benoetigt werden.             */
               /* Dies ist einmal der Feldindex von fmatch in  */
	       /* SubstMatrix : akt_match_index und zum anderen*/
	       /* der Feldindex in SubstMatrix, der als        */
	       /* naechster betrachtet wird : next_match_index.*/
               /************************************************/
typedef struct { char       ident[IDENTLENGTH];
		 short      arity;
		 function   fmatch;
                 short      akt_match_index;
		 short      next_match_index;
	       } F_Info;


               /************************************************/
               /* Datenstruktur fuer ein Element in einer      */
	       /* Parameterliste                               */
               /************************************************/
typedef struct {  char     name[IDENTLENGTH];
                  long     bereich_anfang;
                  long     bereich_ende;
                  long     normal;
               } ParameterElement;


               /************************************************/
               /* Datentyp zur Aufnahme der Information ueber  */
	       /* einen Experten oder Gutachter (eines Objekts)*/
	       /* im Rahmen einer anderen Spezifikation (Do-   */
	       /* maene oder Experte). Es wird hier der Name   */
	       /* abgelegt wie in der entsprechenden Spefika-  */
	       /* tionsdatei angegeben wurde. Zusaetzlich wird */
	       /* beim Einlesen die Nummer innerhalb des glo-  */
	       /* balen Feldes bestimmt.                       */
               /************************************************/
typedef struct { char              name[IDENTLENGTH];
		 short             nr;
		 ParameterElement  paramliste[MAXPARAM];
		 short             param_anz;
		 double            guete;
	       } ObjectInFrameInfo;

               /************************************************/
               /* Datenstruktur fuer die Informationen ueber   */
	       /* einen Startzyklus                            */
               /************************************************/
typedef struct { short   bereich_anfang;
                 short   bereich_ende;
                 long     normal;
               } Zyklen_Info;

               /************************************************/
               /* Datenstruktur fuer die eindeutige Charak-    */
	       /* terisierung eines Funktionssymbolmatches     */
	       /* anhand der Domaene und seiner Nummer inner-  */
	       /* halb dieser Domaene.                         */
               /************************************************/
typedef struct { short    dom_nr;
		 short    match_nr;
	       } DomMatchSpec;

               /************************************************/
               /* Datenstruktur fuer die Aufnahme der Infor-   */
	       /* mation des KZG ueber eine Domaene.           */
               /************************************************/
typedef struct { bool                erkannt;
		 bool                interessant;
		 bool                relevant;
		 int                 einsaetze[MAXCYCLE];
		 int		     anz_einsaetze;
		 long	             bewertung;
		 short               fkt_symb_zuord[MAXFUNCTION];
               } KZGDomainFrame;


               /************************************************/
               /* Datenstruktur fuer eine Domaenenspezifikation*/
               /************************************************/
typedef struct { char                dom_name[IDENTLENGTH];
		 bool                supposed;
                 F_Info              dom_funktion[MAXFUNCTION];
                 short               dom_fkt_anz;
                 pairset             def_gleichungen[MAX_DEF];
		 short               def_anz;
                 ObjectInFrameInfo   start_team_exp[MAXEXP_ANZ];
                 ObjectInFrameInfo   start_team_ga[MAXGA_ANZ];
                 short               start_team_groesse;
                 short               startzyklen_anz;
                 Zyklen_Info         zyklen_info[MAX_ZYKLEN_ANZ];
                 ObjectInFrameInfo   geeignete_exp[MAXEXP_ANZ];
                 short               geeignete_exp_anz;
                 ObjectInFrameInfo   ungeeignete_exp[MAXEXP_ANZ];
                 short               ungeeignete_exp_anz;
                 ObjectInFrameInfo   geeignete_ga[MAXGA_ANZ];
                 short               geeignete_ga_anz;
                 char                ueber_dom[IDENTLENGTH];
                 char                spezielle_dom[MAXDOM_ANZ][IDENTLENGTH];
                 short               spezielle_dom_anz;
                 char                verwandte_dom[MAXDOM_ANZ][IDENTLENGTH];
                 short               verwandte_dom_anz;
                 pairset             bekannte_res;
                 pairset             schlechte_res;
       /* Die folgenden Komponenten sind fuer das KZG des Leiters */
		 bool                startphase;
		 short               anz_gef_matches;
                 KZGDomainFrame      DomMatches[MAXFUNCMATCH];
               } DomainFrame;




/*****************************************************************************/
/*                                                                           */
/*                            externe Variablen                              */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* Infos ueber alle Domaenen                    */
               /************************************************/
extern DomainFrame        DomainGlobalInfo[MAXDOM_ANZ];

               /************************************************/
               /* Anzahl der verschiedenen Domaenen, die in    */
               /* DomainGlobalInfo abgelegt wurden; d.h. Anzahl*/
               /* der Plaetze, die in DomainGlobalInfo belegt  */
               /* sind                                         */
               /************************************************/
extern short              DGICount;

               /************************************************/
               /* Anzahl der relevanten Domaenen fuer den      */
               /* naechsten Zyklus                             */
               /************************************************/
extern short              TeamDomCount;

               /************************************************/
               /* relevanten Domaenen des naechsten Zyklus     */
               /************************************************/
extern DomMatchSpec       DomainTeamInfo[MAXRELDOMAIN];

               /************************************************/
               /* Anzahl der zur Zeit interessanten Domaenen   */
               /************************************************/
extern short              DomainCount;

               /************************************************/
               /* Domaenennummern der interessanten Domaenen   */
               /************************************************/
extern DomMatchSpec       InterestDomain[MAXDOM_ANZ];

               /************************************************/
               /* Liste der vom Databaseexperten erkannten Do- */
               /* maenen. Dabei kann eine Domaene auch oefters */
               /* vorkommen.                                   */
               /************************************************/
extern short              FoundDomain[MAXDOM_ANZ];

               /************************************************/
               /* Anzahl der vom Databaseexperten erkannten    */
               /* Domaenen.                                    */
               /************************************************/
extern short              FoundDomainCount;

               /************************************************/
               /* Zuordnung der Funktionssymbole zwischen Pro- */
               /* blemspezifikation und der vom Database-      */
               /* experten erkannten Domaenen                  */
               /************************************************/
extern short              FoundDomainMatchFunc[MAXDOM_ANZ][MAXFUNCTION];

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
extern short              MatchIdx[MAXDOM_ANZ];

               /************************************************/
               /* Liste der in der Konfigurationsdatei angege- */
               /* benen Domaenen, nach denen der Database-Ex-  */
               /* perte suchen soll. Es werden die Indizes     */
               /* gemaess DomainGlobalinfo abgelegt.           */
               /************************************************/
extern short              SupposedDomains[MAXDOM_ANZ];

               /************************************************/
               /* Anzahl der Domaenen, die in der Konfigura-   */
               /* tionsdatei angegeben wurden, nach denen ge-  */
               /* sucht werden soll.                           */
               /************************************************/
extern short              SupposedDomainsCount;


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
extern double             ExpDomFaktor;

               /************************************************/
               /* fuer den Anteil der Robustheit               */
               /************************************************/
extern double             ExpRobFaktor;

               /************************************************/
               /* fuer den beweisphasenspezifischen Anteil     */
               /************************************************/
extern double             ExpBewFaktor;

               /************************************************/
               /* fuer den Anteil des KZG                      */
               /************************************************/
extern double             ExpKZGFaktor;

               /************************************************/
               /* Die folgenden Variablen dienen zur Aufnahme  */
               /* bestimmter Werte, die bei der Planungsphase  */
               /* fuer Domaene benoetigt werden. Diese koennen */
               /* in der Planungskonfigurationsdatei           */
               /* angegeben werden. Siehe die Beschreibung auch*/
               /* dort! In der Bezeichnung stimmen sie mit den */
               /* Konstanten in domain.h ueberein.             */
               /************************************************/
extern int                maxreldomain;
extern int                nodomexp;
extern double             d_expdomfaktor;
extern double             d_exprobfaktor;
extern double             d_expbewfaktor;
extern double             d_expkzgfaktor;
extern double             nod_expdomfaktor;
extern double             nod_exprobfaktor;
extern double             nod_expbewfaktor;
extern double             nod_expkzgfaktor;


/*****************************************************************************/
/*                                                                           */
/*                Makros in der Rolle von Funktionsaufrufen                  */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* true, wenn die Domaene im letzten Zyklus     */
               /* relevant war.                                */
	       /* Uebergeben werden muss ein Pointer auf       */
	       /* DomMatchSpec                                 */
               /************************************************/
#define domaene_relevant( domspec ) \
	(DomainGlobalInfo[(domspec)->dom_nr].\
	 DomMatches[(domspec)->match_nr].relevant)

               /************************************************/
               /* true, wenn die Domaene noch nicht eingesetzt */
	       /* wurde.                                       */
               /* Uebergeben werden muss ein Pointer auf       */
	       /* DomMatchSpec                                 */
               /************************************************/
#define domaene_neu( domspec ) \
	(!DomainGlobalInfo[(domspec)->dom_nr].\
	 DomMatches[(domspec)->match_nr].anz_einsaetze )


/*****************************************************************************/
/*                                                                           */
/*                          exportierte Funktionen                           */
/*                                                                           */
/*****************************************************************************/

void initialize_dom ( void );

short dom_symbolumwandlung ( short sym );

bool exp_bei_dom_geeignet( short exp_nr, short dom_nr, double *guete );

bool exp_bei_dom_ungeeignet( short exp_nr, short dom_nr, double *guete );

function FindFuncInDom ( char *ident, short dom_nr );

bool new_rel_dom ( DomMatchSpec *domspec );

bool domaene_in_startphase ( DomMatchSpec *domspec );

void ResetDomainVariables ( void );

void DomainManagement ( void );

void dom_global_debug ( void );

#endif
