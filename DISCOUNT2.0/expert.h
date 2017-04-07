/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : expert                                                    */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle fuer die Planungsphase der    */
/*                 Experten notwendigen globalen Variablen zur Verfuegung    */
/*                 gestellt, alle elementaren Hilfsfunktion hierfuer sowie   */
/*                 die globale Steuerungsfunktion SelectNextTeamExperts.     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#ifndef  __EXPERT
#define  __EXPERT

#include "domain.h"

/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/

	       /************************************************/
	       /* Endemarkierung von PraeOrderList und         */
	       /* OrderedExpList                               */
	       /************************************************/
#define LISTENENDE     -1

	       /************************************************/
	       /* Rueckgabewert von get_exp_nr, falls der      */
	       /* uebergebene Name kein Expertenname ist.      */
	       /************************************************/
#define NOEXPERT       -1

	       /************************************************/
               /* Jeder Spezialist erhaelt eine eigene         */
	       /* Systemeindeutige Nummer, die stets groesser  */
	       /* ist als MAXEXPERT ( groesste Nummer eines    */
	       /* Experten in ExpertGlobalInfo ). Daran kann   */
	       /* leicht ein Spezialist erkannt werden.        */
	       /************************************************/
#define DATABASE          MAXEXPERT + 1
#define REDUCE_1          MAXEXPERT + 2
#define REDUCE_2          MAXEXPERT + 3
#define REDUCE_3          MAXEXPERT + 4

/*****************************************************************************/
/*                                                                           */
/*                            Typdefinitionen                                */
/*                                                                           */
/*****************************************************************************/

	       /************************************************/
               /* Datenstruktur fuer die Auswahl der Experten; */
	       /* mit ihrer Hilfe wird eine geordnete Liste    */
	       /* aufgebaut, die Verkettung erfolgt durch die  */
	       /* zweite Komponente. Man waehlt eine statische */
	       /* Liste, weil die maximale Laenge bekannt und  */
	       /* nicht allzu gross ist. Man spart also die    */
	       /* gesamte Arbeit, die bei der Allokation       */
	       /* waehrend des Programmlaufes anfallen wuerde. */
	       /************************************************/
typedef struct { short   exp_nr;
		 short   ptr;
	       } exp_listen_elt;	  

	       /************************************************/
               /* Datenstruktur, in der der einzelne Experte   */
	       /* seine Konfiguration fuer den naechsten Zyklus*/
	       /* ablegt.                                      */
	       /************************************************/
typedef struct { short       exp_nr; /* Nummer des Experten                    */
		 long        cp_parameter[MAXPARAXPERT];
		 long        cg_parameter[MAXPARAXPERT];
               } ExpertConfig;



	       /************************************************/
               /* Datenstruktur fuer das Abspeichern aller     */
	       /* Daten eines Experten;                        */
               /* Der erste Teil dient dazu, die Daten aus der */
	       /* Datei zur Expertenspezifikation abzulegen,   */
	       /* der zweite Teil nimmt die Daten des KZG auf. */
	       /************************************************/

typedef struct {  char                exp_name[IDENTLENGTH];
		  /****************************************/
		  /* SSELECT, SINITIAL oder SSTATIC       */
		  /****************************************/
		  Symbol              typ;
		  /******************************************/
		  /* beim Database-Spezialisten true, wenn  */
		  /* er angeben wurde; sonst true, wenn ein */
		  /* "S" bei EXP_OR_SPEC angegeben wurde.   */
		  /******************************************/
                  bool                spezialist;
                  Symbol              cpfkt_nr;
                  ParameterElement    cpparamliste[MAXPARAM];
                  short               cpparam_anz;
                  Symbol              cgfkt_nr;
                  ParameterElement    cgparamliste[MAXPARAM];
                  short               cgparam_anz;
                  bool                fair;
                  ObjectInFrameInfo   geeignete_ga[MAXGA_ANZ];
                  short               ga_anz;
                  ObjectInFrameInfo   domaenen[MAXDOM_ANZ];
                  short               dom_anz;
                  double              robust;
                  double              wissen;
                  double              phase_anfang;
                  double              phase_mitte;
                  double              phase_ende;
                  short               zeit_anf_min;
                  short               zeit_anf_max;
                  short               zeit_mit_min;
                  short               zeit_mit_max;
                  short               zeit_end_min;
                  short               zeit_end_max;
                  ObjectInFrameInfo   aehnl_exp[MAXEXP_ANZ];
                  short               aehnl_exp_anz;
                  ObjectInFrameInfo   nachf_exp[MAXEXP_ANZ];
                  short               nachf_exp_anz;
                  ObjectInFrameInfo   unvertraegl_exp[MAXEXP_ANZ];
                  short               unvertraegl_exp_anz;
                  ObjectInFrameInfo   team_exp[MAXEXP_ANZ];
                  short               team_exp_anz;
                  char                func_name[IDENTLENGTH];
                  char                *Beschreibung;
		  /******************************************/
                  /* Jetzt kommen die Daten des KZG ueber   */
		  /* einen Experten                         */
		  /******************************************/
                  int                 anz_einsaetze;
		  /******************************************/
	          /* In der folgenden Variablen werden die  */
		  /* Zyklusnummern eingetragen, in denen der*/
		  /* Experte im Team war.                   */
		  /******************************************/
                  int                 einsaetze[MAXCYCLE];
		  /******************************************/
	          /* In die folgenden Komponenten werden    */
		  /* seine Bewertungen fuer seine Einsaetze */
		  /* eingetragen.                           */
		  /******************************************/
                  long                abs_bewertung[MAXCYCLE];
		  double              norm_abs_bew[MAXCYCLE];
                  double              rel_bewertung[MAXCYCLE];
		  /******************************************/
		  /* Korrekturfaktor zur Relativierung der  */
		  /* absoluten Beurteilungen.               */
		  /******************************************/
		  double              korrektur_fac[MAXCYCLE];
		  /******************************************/
	          /* Die folgenden Komponenten werden beim  */
		  /* Festlegen des naechsten zu             */
		  /* beurteilenden Experten benoetigt :     */
		  /******************************************/
		  int                 wartekonto;
		  double              eignung;
		  bool                beurteilt;
		  bool                in_siegerliste;
		  /******************************************/
	          /* Die folgende Komponente ist true, wenn */
		  /* der Experte als Experte einer Domaene  */
		  /* waehrend deren Startphase ins Team kam.*/
		  /******************************************/
		  bool                dom_start_exp;
		  /******************************************/
	          /* In der folgenden Komponente wird die   */
		  /* durch den Leiter eventuell bestimmte   */
		  /* aktuelle Bewertung abgelegt.           */
		  /******************************************/
		  double              akt_bewertung;
		  /******************************************/
	          /* In den folgenden Komponenten werden die*/
		  /* durch den Leiter eventuell bestimmten  */
		  /* Grenzen fuer das Erfuellen der Sieger- */
		  /* bzw Verliererkriterien abgelegt.       */
		  /******************************************/
		  long                siegergrenze;
		  long                verlierergrenze;
		  /******************************************/
	          /* Die folgenden Komponenten werden in der*/
		  /* Konigurationsdatei versorgt und nach   */
		  /* deren Lesen allen Rechnern bekannt     */
		  /* gemacht. Dies ist die Ordnung, die der */
		  /* Experte verwenden soll. Dies wird in   */
	          /* der Konfigurationsdatei spezifiziert.  */
		  /* Die Nummerierung erfolgt gemaess der   */
		  /* Reihenfolge, in der die Ordnungen in   */
		  /* der Konfigurationsdatei aufgefuehrt    */
		  /* sind.                                  */
		  /******************************************/
		  short               order;
               }  ExpertFrame;

	       /************************************************/
               /* Typ des Reduktionsspezialisten :             */
               /*   CP -> Nur Funktion f"ur kritische Paare    */
               /*   CG -> Nur Funktion f"ur kritische Ziele    */
               /*   UNDEFINED -> Noch nicht festgesetzt        */
	       /************************************************/
typedef enum { CP, CG, UNDEFINED } RedTyp;

	       /************************************************/
               /* Datenstruktur fuer die Reduktionsspezialisten*/
	       /************************************************/
typedef struct { /******************************************/
	         /* Daten des Langzeitged"achtnis          */
		 /******************************************/
		 char       name[IDENTLENGTH];
		   /****************************************/
		   /* SSELECT, SINITIAL oder SSTATIC       */
		   /****************************************/
		 Symbol     typ;
		   /****************************************/
		   /* CP oder CG , in Abh. der             */
		   /* angegebenen CP und CG-Funktione      */
		   /****************************************/
		 RedTyp     red_typ;
		 Symbol     reduce_cp_fkt;
		 int        cp_start;
		 bool       reducecp;
		 bool       subsumcp;
		 bool       doublecp;
		 Symbol     reduce_cg_fkt;
		 int        cg_start;
		 bool       reducecg;
		 bool       subsumcg;
		 bool       doublecg;
		 /******************************************/
	         /* Daten des Kurzzeitged"achtnis          */
		 /******************************************/
		 int        anz_einsaetze;
		 int        einsaetze[MAXCYCLE];
	       } ReduceFrame;




/*****************************************************************************/
/*                                                                           */
/*                            externe Variablen                              */
/*                                                                           */
/*****************************************************************************/

	       /************************************************/
               /* Infos ueber alle Experten                    */
	       /************************************************/
extern ExpertFrame      ExpertGlobalInfo[MAXEXPERT];

	       /************************************************/
               /* Variablen fuer die Spezialisten              */
	       /************************************************/
extern ExpertFrame      Database;
extern ReduceFrame      RedSpecInfo[MAXREDSPEC];

	       /************************************************/
               /* Indizes der Experten, die im naechsten       */
	       /* Zyklus ins Team kommen                       */
	       /************************************************/
extern short            NewTeam[MAXEXPERT];

	       /************************************************/
               /* Anzahl der Plaetze, die in NewTeam bereits   */
	       /* belegt sind.                                 */
	       /************************************************/
extern short            NewTeamLength;

	       /************************************************/
               /* Konfigurationen der Experten, die im         */
	       /* aktuellen Zyklus eingesetzt sind             */
	       /************************************************/
extern ExpertConfig     ExpertTeamInfo[MAXEXPERT];

	       /************************************************/
               /* Anzahl der Experten, die im laufenden Zyklus */
	       /* eingesetzt sind/waren                        */
	       /************************************************/
extern short            TeamExpCount;

	       /************************************************/
               /* Liste der Experten, die vom Leiter als Sieger*/
	       /* des letzten Zyklus erkannt wurden.           */
               /* Es werden nur die Indizes der Experten       */
	       /* gemaess der globalen Variablen               */
	       /* ExpertGlobalInfo abgelegt, also die Nummer   */
	       /* des Experten.                                */
	       /************************************************/
extern short            ExpWinnerList[MAXEXPERT];
extern short            ExpWinnerListLength;

	       /************************************************/
               /* Anzahl der Zyklen, in denen das Team von der */
	       /* Zusammensetzung der Experten her unveraendert*/
	       /* ist.                                         */
	       /************************************************/
extern short            TeamNotChanged;

	       /************************************************/
               /* Die folgende Variable gibt an, ob der Sieger */
	       /* des letzten Zyklus auch im naechsten Zyklus  */
	       /* vertreten ist. Wenn ja, so arbeitet er auf   */
	       /* demselben Rechner weiter und die krit. Paare */
	       /* und krit. Ziele muessen nicht neu bewertet   */
	       /* werden. Ist der Sieger auch im neuen Team, so*/
	       /* hat die folgende Variable den Wert false,    */
	       /* sonst true.                                  */
	       /************************************************/
extern  bool          neuer_leiter_exp;

	       /************************************************/
               /* Liste der Experten, die vom Leiter als       */
	       /* Verlierer des letzten Zyklus erkannt wurden. */
               /* Es werden nur die Indizes der Experten       */
	       /* gemaess der globalen Variablen               */ 
	       /* ExpertGlobalInfo abgelegt, also die Nummer   */
	       /* des Experten.                                */
	       /************************************************/
extern short      ExpLoserList[MAXEXPERT];
extern short      ExpLoserListLength;

	       /************************************************/
               /* Liste der Experten, die vom Leiter weder als */
	       /* Sieger noch als Verlierer desletzten Zyklus  */
	       /* klassifiziert wurden.                        */
               /* Es werden nur die Indizes der Experten       */
	       /* gemaess der globalen Variablen               */
               /* ExpertGlobalInfo abgelegt, also die Nummer   */
	       /* des Experten.                                */
	       /************************************************/
extern short      ExpAverageList[MAXEXPERT];
extern short      ExpAverageListLength;

	       /************************************************/
               /* true, falls alle globalen Felder             */
	       /* initialisiert wurden.                        */
	       /************************************************/
extern bool       ExperteInitialized;

	       /************************************************/
               /* Die folgenden Variablen dienen zur Verwaltung*/
	       /* der Liste von Experten, in die die Experten, */
	       /* die nicht in der Siegerliste sind, gemaess   */
	       /* ihrer Eignung sortiert eingetragen werden.   */
	       /************************************************/
extern exp_listen_elt  PraeOrderList[MAXEXPERT];
extern short           PraeOrderLength;
extern short           PraeOrderStart;

	       /************************************************/
               /* Die folgenden Variablen dienen zur Verwaltung*/
	       /* der Experten, die aus der PraeOrderListe     */
	       /* ausgewaehlt und beurteilt wurden. Aus ihnen  */
	       /* waehlt der Leiter letztendlich die Experten  */
	       /* fuer sein Team aus.                          */
	       /************************************************/
extern exp_listen_elt  OrderedExpList[MAXEXPERT];
extern short           OrderedExpStart;
extern short           OrderedExpLength;

	       /************************************************/
               /* Die folgenden Variablen werden bei der       */
	       /* Bestimmung der naechsten Zyklusdauer         */
	       /* benoetigt. Summe der absoluten Beurteilungen */
	       /* aller Experten, die im letzten Team waren    */
	       /* und auch wieder im neuen Team sind.          */
	       /************************************************/
extern long            SumOldExp;
	       /************************************************/
               /* Anzahl der Experten, die im letzten Team     */
	       /* waren und auch im neuen Team sind            */
	       /************************************************/
extern short           CountOldExp;

	       /************************************************/
               /* Summe der Bewertungen durch den Leiter aller */
	       /* Experten, die neu ins Team kommen, d.h. im   */
	       /* Zyklus davor nicht im Team waren.            */
	       /************************************************/
extern double          SumNewExp;
	       /************************************************/
               /* Anzahl der Experten, die neu ins Team kommen.*/
	       /************************************************/
extern short           CountNewExp;

/*****************************************************************************/
/*                                                                           */
/*                 Makros in der Rolle von Funktionsaufrufen                 */
/*            ( nur solche, die nach aussen sichtbar sein sollen )           */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_ist_spezialist                                       */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                                                                           */
/*  Returnwert   :  true, falls der durch die Nummer angegebene Experte ein  */
/*                        Spezialist ist                                     */
/*                  false, sonst                                             */
/*                                                                           */
/*  Beschreibung :  Ein Spezialist hat eine Nummer, die groesser als         */
/*                  MAXEXPERT ist; dies wird ueberprueft.                    */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

	       /************************************************/
               /* Index des Reduktionsspezialisten in der      */
	       /* globalen Variablen RedSpecInfo.              */
	       /* Argument ist die Nummer des Spezialisten,d.h.*/
	       /* REDUCE_1, REDUCE_2 oder REDUCE_3             */
	       /************************************************/
#define exp_ist_spezialist(exp_nr)    ( (exp_nr) > MAXEXPERT )

	       /************************************************/
               /* Index des Reduktionsspezialisten in der      */
	       /* globalen Variablen RedSpecInfo.              */
	       /* Argument ist die Nummer des Spezialisten,d.h.*/
	       /* REDUCE_1, REDUCE_2 oder REDUCE_3             */
	       /************************************************/
#define GetRedSpecIdx(red_spec_nr) ( red_spec_nr - MAXEXPERT - 2 )

	       /************************************************/
               /* Nummer des Reduktionsspezialisten im System  */
	       /* Argument ist der Index in der globalen Va-   */
	       /* riablen RedSpecInfo                          */
	       /************************************************/
#define RedSpecNo(red_spec_idx) (MAXEXPERT + red_spec_idx + 2)

	       /************************************************/
               /* true, falls der durch die Nummer angegebene  */
	       /* Experte als STATIC deklariert wurde          */
	       /************************************************/
#define StaticExp(xpert_nr) \
    ( ExpertGlobalInfo[xpert_nr].typ == SSTATIC )

	       /************************************************/
               /* true, falls der durch die Nummer angegebene  */
	       /* Experte als INITIAL deklariert wurde         */
	       /************************************************/
#define InitialExp(xpert_nr) \
    ( ExpertGlobalInfo[xpert_nr].typ == SINITIAL )

	       /************************************************/
               /* true, falls der durch die Nummer angegebene  */
	       /* Experte als STATIC deklariert wurde          */
	       /************************************************/
#define StaticRedSpec(redspec_nr) \
    ( RedSpecInfo[redspec_nr].typ == SSTATIC )

	       /************************************************/
               /* true, falls der durch die Nummer angegebene  */
	       /* Experte als INITIAL deklariert wurde         */
	       /************************************************/
#define InitialRedSpec(redspec_nr) \
    ( RedSpecInfo[redspec_nr].typ == SINITIAL )

	       /************************************************/
               /* Bestimmt wird der Robustheitsgrad des        */
	       /* angegebenen Experten. Der Parameter muss also*/
	       /* die Nummer eines Experten sein.              */
	       /************************************************/
#define robustheit_best(exp_nr) ExpertGlobalInfo[(exp_nr)].robust

	       /************************************************/
               /* Gibt an wieviele frei Plaetze momentan im    */
	       /* neuen Team noch zu vergeben sind             */
	       /************************************************/
#define freie_plaetze() ( HostCount - NewTeamLength )

	       /************************************************/
               /* true, wenn alle Plaetze ( das ist die Anzahl */
	       /* der zur Verfuegung stehenden Rechner ) durch */
	       /* Experten besetzt sind; false sonst.          */
               /* Sinnvoller Einsatz nur im Modul FormNextTeam */
	       /************************************************/
#define team_besetzt() ( NewTeamLength  == HostCount )

	       /************************************************/
               /* true, wenn der angegebene Experte Leiter     */
	       /* werden darf                                  */
	       /************************************************/
#define leiterfaehig( exp_nr )                                              \
  ( exp_nr < MAXEXPERT ? RefGlobalInfo[(exp_nr)].beurt_paramliste[REF_NOMASTER].normal : false )

	       /************************************************/
               /* Der angegebene Experte wird als leiterfaehig */
	       /* deklariert.                                  */
	       /************************************************/
#define set_exp_leiterfaehig( exp_nr )                                       \
  RefGlobalInfo[(exp_nr)].beurt_paramliste[REF_NOMASTER].normal = true;

	       /************************************************/
               /* Das folgende Makro ist ein Dummy fuer eine   */
	       /* noch zu implementierende Funktion gleichen   */
	       /* Namens.                                      */
	       /************************************************/
#define speziellere_domaenen_bekannt() false

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void initialize_exp ( void );

bool ziel_experte ( short exp_nr );

bool exp_im_alten_team ( short exp_nr );

bool exp_im_neuen_team ( short exp_nr );

char *get_exp_name( short exp_nr );

bool DomStartExp( short exp_nr );

bool exp_bei_exp_geeignet ( short exp_nr1, short exp_nr2, double *guete );

short get_exp_nr ( char *exp_name );

void SelectNextTeamExperts ( void );

void prae_orderedliste_aus( void );

void orderedliste_aus( void );

void exp_global_debug( void );

#endif
