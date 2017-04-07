/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         cpweight
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   In diesem Modul werden die einzelnen
//                      Bewerter fuer kritische Paare Implementiert
//-----------------------------------------------------------------------------
//      $Log: cpweight.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//      Wichtige Aenderung: Die CP-Weight-FUnktionen umgestellt von
//      *left,*right auf ein einzelnes Termpaar (tp). Damit kann man
//      auch auch auf im Termpaar gespeicherte Informationen
//      zugreifen. 
//=============================================================================
*/

#ifndef __CPWEIGHT

#define __CPWEIGHT

#include "order.h"
#include "subst.h"
#include "learn_tsmexperts.h"
#include "team.h"

/*****************************************************************************/
/*                                                                           */
/*                            Typdefinitionen                                */
/*                                                                           */
/*****************************************************************************/

typedef struct 
{ 
   char       cmd[16];
   double     (*xpert)( termpair *tp ); 
} XInfo;


/*****************************************************************************/
/*                                                                           */
/*                            externe Variablen                              */
/*                                                                           */
/*****************************************************************************/

extern      XInfo       XpertTab[];

extern      double      (*CPWeight)( termpair *tp );
extern      double      (*CGWeight)( termpair *tp );

extern      short       special_factor;

/* extern      short       team_cycle; */

extern      long        DiffFactor;
extern      long        MaxFactor;

extern      bool        SkolemGoal;

extern      long        DoubleMatchWeight;
extern      long        SingleMatchWeight;
extern      long        DoubleUnifyWeight;
extern      long        SingleUnifyWeight;

extern      bool        GoalMatch;
extern      long        GDoubleMatchWeight;
extern      long        GSingleMatchWeight;
extern      long        GDoubleUnifyWeight;
extern      long        GSingleUnifyWeight;

extern      long        NoMatchWeight;
extern      long        NumberOfFunctions;
extern      long        NumberOfVariables;
extern      long        LowerBound;

extern      bool        considerAll;

extern      long        NoUnifyFactor;
extern      long        MinWeight;



void    SetWeights  ( long Fweight, long Vweight );
void    SetCGWeights  ( long CGFweight, long CGVweight );

void    SetCPWeight ( double (*cpweight)( termpair *tp) );
void    SetCGFunc   ( double (*cgweight)( termpair *tp ) );

double    fifo_weight ( termpair *tp );
double    team_fifo_weight ( termpair *tp );

double    max_tweight ( termpair *tp );
double    sum_tweight ( termpair *tp );
double    gt_tweight  ( termpair *tp );

double    occnest      (termpair *tp);
void    init_goal_fn_m (void);

double    max_kbo     ( termpair *tp );
double    sum_kbo     ( termpair *tp );
double    gt_kbo      ( termpair *tp );

double    sum_fcount  ( termpair *tp );
double    gt_fcount   ( termpair *tp );

double    sum_tfweight ( termpair *tp );
double    max_tfweight ( termpair *tp );
double    gt_tfweight  ( termpair *tp );

double    max_poly     ( termpair *tp );
double    sum_poly     ( termpair *tp );
double    gt_poly      ( termpair *tp );

double    diff_weight  ( termpair *tp );

double    divergence   ( termpair *tp );



double    goaltest     ( termpair *tp );

long      testmatch    ( long max, term *t1, term *t2 );
long      testunify    ( long max, term *t1, term *t2 );
void      testgoal     ( termpair *goal );
double    goalmatch    ( termpair *tp );


double    goalsim      ( termpair *tp );

double    unif_goal    ( termpair *tp );
double    diff_goal    ( termpair *tp );
long       max_unif     ( term *left, term *right );

void    SpecialWeight ( termpair *tp );

#endif
