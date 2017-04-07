/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_value_exp                                             */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die fuer die Bewertung eines Experten benoetigt   */
/*                 werden.                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#ifndef  __EXP_VALUE_EXP   
#define  __EXP_VALUE_EXP


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/
/* verwendet in exp_dom_bez_best                                             */
#define GOOD_DOM_EXP_KNO 3.0
#define BAD_DOM_EXP_KNO  0.0
#define NO_DOM_EXP_KNO   1.0

/* verwendet in kzg_anteil_best */
#define RELEVANTLASTCYCL       5
#define MINIMUMOFACTIONS       3
#define INSPECTEDACTIONS       3

/* fuer die obigen 3 Konstanten muss folgende Ungleichungskette gelten :     */
/*    2 <= INSPECTEDACTIONS <= MINIMUMOFACTIONS <= RELEVANTLASTCYCLES        */
/* Dies wird vom Compiler im folgenden getestet                              */
#define TESTKZGWERTE ( 2 > INSPECTEDACTIONS || \
                       INSPECTEDACTIONS > MINIMUMOFACTIONS || \
                       MINIMUMOFACTIONS > RELEVANTLASTCYCL )
#if ( 2 > INSPECTEDACTIONS || INSPECTEDACTIONS > MINIMUMOFACTIONS || \
      MINIMUMOFACTIONS > RELEVANTLASTCYCL )
#error
#endif

/* verwendet in exp_konstant_test */
#define MAXDIFFERENCE          500

#define CONSTANTFACTOR    2
#define TRENDEXPFACTOR    4

#define RELKZGFACTOR      2

#define PROOF_MID         4

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Variablen                              */
/*                                                                           */
/*****************************************************************************/
extern  double        good_dom_exp_kno;
extern  double        bad_dom_exp_kno;
extern  double        no_dom_exp_kno;

extern  int           relevantlastcycl;
extern  int           minimumofactions;
extern  int           inspectedactions;

extern  double        maxdifference;

extern  int           constantfactor;
extern  int           trendexpfactor;

extern  double        relkzgfactor;

extern  int           proof_mid;

/* aktuelle Beweisphaseneinstufung durch den Leiter : */
extern double     ProofPhase;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void    DefineProofPhase ( void );
double  ValueExperts     ( short exp_nr );

#endif
