/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         termpair
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung von Termpaaren
//
//      Datentypen:     termpair    Termpaar
//                      pairset     Menge von Termpaaren
//
//      Funktionen:
//
//          termpair  *newpair          Neues Termpaar anlegen
//          void      deletepair        Termpaar loeschen
//
//          void      printtpair        Termpaar ausgeben
//          void      fprinttpair       Termpaar auf Stream ausgeben
//
//          void      SwapSides         Seiten eines Paares vertauschen
//          bool      tpequal           Gleichheit von Termpaaren
//          bool      tpsubsum          Test auf Subsumtion
//
//          void      tpnewvars         Variablenumbenennung
//          void      tpcopynew         Kopie eines Termpaares anlegen,
//                                      mit gleichzeitiger Variablenumbenennung
//
//          void      Add               Allgemeine Mengenoperationen
//          void      Insert
//          void      Delete
//
//          void      AddRule           Neue Regel hinzufuegen
//          void      AddEqu            Neue Gleichung hinzufuegen
//          void      AddCP             Neues kritisches Paar hinzufuegen
//          void      AddGoal           Neues Ziel hinzufuegen
//          void      AddCritGoal       Neues kritisches Ziel hinzufuegen
//
//          void      InsertRule        Neue Regel einfuegen
//          void      InsertEqu         Neue Gleichung einfuegen
//          void      InsertCP          Neues kritisches Paar einfuegen
//          void      InsertGoal        Neues Ziel einfuegen
//          void      InsertCritGoal    Neues kritisches Ziel einfuegen
//
//          void      ReInsertRule      Regel erneut einfuegen
//          void      ReInsertCP        Kritisches Paar erneut einfuegen
//
//          void      NInsertRule       Neue Regel einfuegen
//          void      NInsertEqu        Neue Gleichung einfuegen
//          void      NInsertCP         Neues kritisches Paar einfuegen
//          void      NInsertGoal       Neues Ziel einfuegen
//          void      NInsertCritGoal   Neues kritisches Ziel einfuegen
//
//          void      DeleteRule        Regel loeschen
//          void      DeleteEqu         Gleichunk loeschen
//          void      DeleteCP          kritisches Paar loeschen
//          void      DeleteGoal        Ziel loeschen
//          void      DeleteCritGoal    Kritisches Ziel loeschen
//
//          termpair  *RuleByNumber     Regel anhand der Nummer finden
//          termpair  *EquByNumber      Gleichung anhand der Nummer finden
//          termpair  *CPByNumber       CP anhand der Nummer finden
//
//          bool      Empty             Ist eine Menge leer ?
//          termpair  *DelFirst         Erstes Element einer geordneten Menge
//                                      loeschen
//          termpair  *DelLast          Ditto fuer letztes
//          short     FindPair          Eine Termpaar in einer Menge suchen
//          termpair  *FindSubsum       Eine Subsumption finden
//          void      InititalSet       Markieren einer Menge als initial
//          void      ClearSet          Menge loeschen und Speicher freigeben
//          void      TPClearData       Alle Mengen loeschen
//
//          void      ForAllRulesDo     Operation mit Regeln durchfuehren
//          void      ForAllFRulesDo    Operation mit Regeln einer Funktion
//                                      durchfuehren
//          void      ForAllEquDo       Operation mit Gleichungen durchfuehren
//          void      ForAllCPsDo       Operation mit kritischen Paaren durchfuehren
//          void      ForAllGoalsDo     Operation mit Zielen durchfuehren
//          void      ForAllCritGoalsDo Operation mit kritischen Zielen durchfuehren
//
//          void      PrintSet          Menge ausgeben
//          void      PrintRules        Alle Regeln ausgeben
//          void      PrintEquations    Alle Gleichungen ausgeben
//
//          void      MoveSet           Menge verschieben
//          void      MergeCPs          Mengen Vereinigen
//
//          bool      Cancellation      Streichen von Topsymbolen falls
//                                      dies moeglich und erlaubt.
//
//          long      SumWeightCP       Bestimmen des Gewichts der 
//                                      ersten n kritschen Paare
//-----------------------------------------------------------------------------
//      $Log: termpair.h,v $
//      Revision 0.1  1991/08/26  13:50:27  pitz
//      special_flag eingefuehrt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __TERMPAIR
#define __TERMPAIR

#include "systime.h"
#include "newmem.h"
#include "term.h"
#include "values.h"


/*
//-----------------------------------------------------------------------------
//      Typendefinition
//-----------------------------------------------------------------------------
*/


#ifdef PCL /* Aenderung von StS   */

typedef struct pclidcell
{
   int cycle;
   int host;
   int count;
}PclIdCell,*PclId_p;

#endif


typedef struct tpcell { 
                        term              *left;
                        term              *right;
                        short             type;
                        long              count;
                        long              number;
                        double            weight;
                        double            quality;
#ifdef PCL  /* Aenderung von StS   */
                        PclIdCell         pclid; 
#endif
/*			long              knowledge; */
			/* Evaluation functions can use this to */
			/* transmit additional information...*/

                        function          setcode;

                        bool              special_flag;

                        int               ref_red_count;        /* Referee */
                        int               ref_red_right;
                        int               ref_red_left;
                        int               ref_red_equ;
                        int               ref_red_goal;
                        int               ref_subsum;
                        int               ref_cp_count;
                        long              ref_weight;

                        short             state;
                        struct tpcell     *parent1;
                        struct tpcell     *parent2;
                        long              coparent1;
                        long              coparent2;
                        struct tpcell     *next;
                        struct tpcell     *prev;

                        #ifdef MEMDEBUG                         /* Debugging */
                          short           debug;
                        #endif
                                                        } termpair;

/* In der folgenden Struktur gibt setcount stets die aktuelle Maechtigkeit   */
/* der Menge an, und count gibt eine aufsteigende Nummerierung aller jemals  */
/* in diese Menge aufgenommenen Termpaare.                                   */
typedef struct tpset  { termpair          *first;
                        termpair          *last;
                        long              count;
                        long              setcount;     } pairset;


/*
//-----------------------------------------------------------------------------
//      sichtbare Variablen
//-----------------------------------------------------------------------------
*/

extern  pairset     SetOfRules [MAXFUNCTION];
extern  pairset     SetOfEquations;    /* [MAXFUNCTION]; */
extern  pairset     SetOfCriticalPairs;
extern  pairset     SetOfGoals;
extern  pairset     SetOfCriticalGoals;
extern  long        CPsLimit;

extern  pairset     SetOfAxioms;

/*
//-----------------------------------------------------------------------------
//      Makrodefinitionen fuer Referee
//-----------------------------------------------------------------------------
*/

#define ClrReferee(tp)                                                  \
{                                                                       \
    tp->ref_red_count   = 0;                                            \
    tp->ref_red_right   = 0;                                            \
    tp->ref_red_left    = 0;                                            \
    tp->ref_red_equ     = 0;                                            \
    tp->ref_red_goal    = 0;                                            \
    tp->ref_subsum      = 0;                                            \
    tp->ref_cp_count    = 0;                                            \
}

#define Referee_Red_Count(tp)       tp->ref_red_count++
#define Referee_Red_Right(tp)       tp->ref_red_right++
#define Referee_Red_Left(tp)        tp->ref_red_left++
#define Referee_Red_Equation(tp)    tp->ref_red_equ++
#define Referee_Red_Goal(tp)        tp->ref_red_goal++
#define Referee_Subsum(tp)          tp->ref_subsum++
#define Referee_CountCP(tp)         tp->ref_cp_count++


/*
//-----------------------------------------------------------------------------
//      sonstige Makrodefinitionen
//-----------------------------------------------------------------------------
*/

#define         INITIAL       1         /* Flag fuer Initiales Paar     */
#define         NEW           2         /* Flag fuer neues Paar         */
#define         MODIFIED      4         /* Flag fuer Veraendertes Paar  */
#define         CLEARED       8         /* Flag fuer geloeschtes Paar   */
#define         MARKED       16         /* Flag fuer markiertes Paar    */


#define         State(ptr)   (ptr->state)
#define         New(ptr)     (ptr->state = NEW)
#define         Initial(ptr) (ptr->state = INITIAL)
#define         Modify(ptr)  (ptr->state |= MODIFIED)
#define         Clear(ptr)   (ptr->state = CLEARED)
#define         Mark(ptr)    (ptr->state = MARKED)

#define         EmptySet     { NULL, NULL, 0, 0 }

#define         RuleCounter  (SetOfRules[0].count)
#define         EquCounter   (SetOfEquations.count)
#define         CPCounter    (SetOfCriticalPairs.count)
#define         GoalCounter  (SetOfGoals.count)
#define         CGCounter    (SetOfCriticalPairs.count)


#define         UNKNOWN       0         /* Unbekanntes Termpaar         */
#define         RULE          1         /* Kennung fuer Regel           */
#define         EQUATION      2         /* Kennung fuer Gleichung       */
#define         CRITPAIR      3         /* Kennung fuer kritsches Paar  */
#define         GOAL          4         /* Kennung fuer Ziel            */
#define         CRITGOAL      5         /* Kennung fuer kritsches Ziel  */


#define SizeOf(set)         (set.setcount)

#define IsAct(tp)  (    ((!tp->parent1) || (tp->parent1->count == tp->coparent1))   \
                     && ((!tp->parent2) || (tp->parent2->count == tp->coparent2)) )


/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/



void      inittermpair ( void );

termpair  *newpair ( term     *left,     term     *right,
		    termpair *parent1,  termpair *parent2 );

void      deletepair ( termpair *pair );

void      printtpair  ( termpair *pair );
void      fprinttpair ( FILE *stream, termpair *pair, char *string );
char      *sprinttpair ( termpair *pair );

void      SwapSides  ( termpair *pair );

bool      tpequal    ( termpair *pair1, termpair *pair2 );
bool      tpsubsum   ( termpair *pair1, termpair *pair2 );

void      tpnewvars  ( term *left, term *right );
void      tpcopynew  ( termpair *pair, term **left, term **right );


void      Add             ( pairset *set, termpair *pair );
void      Insert          ( pairset *set, termpair *pair );
void      Delete          ( pairset *set, termpair *pair );

void      AddRule         ( termpair *rule  );
void      AddEqu          ( termpair *equ   );
void      AddCP           ( termpair *cp    );
void      AddGoal         ( termpair *goal  );
void      AddCritGoal     ( termpair *cgoal );

void      InsertRule      ( termpair *rule  );
void      InsertEqu       ( termpair *equ   );
void      InsertCP        ( termpair *cp    );
void      InsertGoal      ( termpair *goal  );
void      InsertCritGoal  ( termpair *cgoal );

void      ReInsertRule    ( termpair *rule  );
void      ReInsertCP      ( termpair *cp    );

void      NInsertRule     ( termpair *rule  );
void      NInsertEqu      ( termpair *equ   );
void      NInsertCP       ( termpair *cp    );
void      NInsertGoal     ( termpair *goal  );
void      NInsertCritGoal ( termpair *cgoal );

void      DeleteRule      ( termpair *rule  );
void      DeleteEqu       ( termpair *equ   );
void      DeleteCP        ( termpair *cp    );
void      DeleteGoal      ( termpair *goal  );
void      DeleteCritGoal  ( termpair *cgoal );

termpair  *RuleByNumber   ( long number );
termpair  *EquByNumber    ( long number );
termpair  *CPByNumber     ( long number );
termpair  *GoalByNumber   ( long number );
termpair  *CGByNumber     ( long number );

bool      Empty      ( int type );
termpair  *DelFirst  ( pairset *set );
termpair  *DelLast   ( pairset *set );

short     FindPair   ( pairset *set, termpair *equ );
termpair  *FindSubsum( pairset *set, termpair *pair );

void      InititalSet      ( pairset *set );
void      ClearSet         ( pairset *set );
void      TPClearData      ( void );
void      TPClearCPCache   ( void );
void      TPClearCPCachePtr( termpair *ptr );
void      TPClearCGCache   ( void );
void      TPClearCGCachePtr( termpair *ptr );
void      ReorgTermpair    ( bool silence );

void      ForAllRulesDo     ( void (*proc)(termpair *ptr) );
void      ForAllFRulesDo    ( function fcode,
			     void (*proc)(termpair *ptr) );
void      ForAllEquDo       ( void (*proc)(termpair *ptr) );
void      ForAllCPsDo       ( void (*proc)(termpair *ptr) );
void      ForAllGoalsDo     ( void (*proc)(termpair *ptr) );
void      ForAllCritGoalsDo ( void (*proc)(termpair *ptr) );

void      PrintSet   ( pairset *set );
void      PrintRules ( void );
void      PrintEquations ( void );

void      MoveSet  ( pairset *dest, pairset *source );
void      MergeCPs ( pairset *source );
void      MergeCGs ( pairset *source );

bool      Cancellation ( termpair *pair );

long      SumWeightCP ( int n );

long      akt_termpaare_anz_best ( void );


#endif
