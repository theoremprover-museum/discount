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
//      $Log: cpweight.c,v $
//      Revision 0.2  1992/02/14  07:49:48  pitz
//      GOALTEST-Test
//
//      Revision 0.1  1991/08/15  13:46:13  pitz
//      diff_weight implementiert.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    "cpweight.h"


/*
//=============================================================================
//      Tabelle der Experten
//=============================================================================
*/

XInfo       XpertTab[]      = 
{ 
   { "max",            max_tweight },
   { "fifo",           fifo_weight },
   { "team_fifo",      team_fifo_weight },
   { "c_1tet_learn",   C1TETLearnWeight },
   { "c_2tet_learn",   C2TETLearnWeight },
   { "c_3tet_learn",   C3TETLearnWeight },
   { "c_global_learn", CGlobalLearnWeight },
   { "l_global_learn", LGlobalLearnWeight },
   { "c_goal_bound",   CGoalBoundLearnWeight },
   { "l_goal_bound",   LGoalBoundLearnWeight },
   { "c_spec_bound",   CSpecBoundLearnWeight },
   { "cgflearn",       CGFloatLearnWeight },
   { "tsmstandard",    StandardTSMWeight },
   { "otsmstandard",   StandardOTSMWeight },
   { "add",            sum_tweight },
   { "gt",             gt_tweight  },
   { "occnest",        occnest     },
   { "max_kbo",        max_kbo     },
   { "add_kbo",        sum_kbo     },
   { "gt_kbo",         gt_kbo      },
   { "gt_fcount",      gt_fcount   },
   { "sum_fcount",     sum_fcount  },
   { "diff",           diff_weight },
   { "goalmatch",      goalmatch   },
   { "goalsim",        goalsim     },
   { "goaltest",       goaltest    },
   { "unif_goal",      unif_goal   },
   { "diff_goal",      diff_goal   },
   { "-",              NULL        } 
};

/*
//=============================================================================
//      Pointer auf aktuelle Bewertungsfunktion fuer kritische Paare
//=============================================================================
*/

double        (*CPWeight)( termpair *tp ) = sum_tweight;

/*
//=============================================================================
//      Pointer auf aktuelle Bewertungsfunktion fuer kritische Ziele
//=============================================================================
*/

double        (*CGWeight)( termpair *tp ) = unif_goal;


short       special_factor = 0;

/* short       team_cycle = 0;  */

long        DiffFactor          = 10000;
long        MaxFactor           =    -1;

bool        SkolemGoal          = false;

long        DoubleMatchWeight   =   1;
long        SingleMatchWeight   =   5;
long        DoubleUnifyWeight   =   5;
long        SingleUnifyWeight   =   9;

bool        GoalMatch           = false;

long        GDoubleMatchWeight  =   5;
long        GSingleMatchWeight  =   9;
long        GDoubleUnifyWeight  =  15;
long        GSingleUnifyWeight  =  23;

long        NoMatchWeight       =  51;
long        NumberOfFunctions   =  2;
long        NumberOfVariables   =  3;
long        LowerBound          =  7;

bool        considerAll         = true;

/* Faktor beim kritischen Ziel-Experten diff_goal */
long        NoUnifyFactor       =  2;
/* Mindetsgewicht der Teilterme in unif_goal */
long        MinWeight           =  4;

/*****************************************************************************/
/*                                                                           */
/*                            lokale Variablen                               */
/*                                                                           */
/*****************************************************************************/

static long fifo_count = 0;
static long fweight = 2;
static long vweight = 1;

static bool     f_flag[MAXFUNCTION];

/* Gewichte fuer Funktionssymbole und Variablen bei kritischen Zielen */
static long cg_fweight = 2;
static long cg_vweight = 1;

static int goal_fn_nr;
static struct {function func; int m0,m1;} goal_fn_m[MAXFUNCTION];


/*
//-----------------------------------------------------------------------------
//      Lokale Vereinbarungen
//-----------------------------------------------------------------------------
*/

static long     tweight ( term *t );
static long     kbo     ( term *t );
static void     fcounth ( term *t );
static long     fcount  ( term *t );

static long     tfweight  ( term *t );
static long     pweight   ( term *t );

static long     testgmatch    ( long max, term *t1, term *t2 );
static long     testgunify    ( long max, term *t1, term *t2 );
static void     testgoalmatch ( termpair *goal );

/* Funktionen fuer das Bearbeiten kritischer Ziele */
static long     cg_tweight ( term *t );

static short    measure0 (function op, term *t);
static short    measure1 (function op, term *t, short currentMax, short *totalMax);
static void     add_fn (term *t);



/*
//=============================================================================
//      Auswahl des Experten
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       SetCGFunc
//
//  Parameter:      cgweight    Neue Bewertungsfunktion fuer kritische Ziele
//
//  Beschreibung:   Auswahl einer Bewertungsfunktion fuer kritische Ziele
//-----------------------------------------------------------------------------
*/

void    SetCGFunc ( double (*cgweight)(termpair *tp) )
{
    CGWeight = cgweight;
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       SetCPWeight
//
//  Parameter:      cpweight    Neue Bewertungsfunktion fuer kritische Paare
//
//  Beschreibung:   Auswahl einer Bewertungsfunktion fuer kritische Paare
//-----------------------------------------------------------------------------
*/

void    SetCPWeight ( double (*cpweight)(termpair *tp) )
{
    CPWeight = cpweight;
}


/*
//=============================================================================
//      Statistische Experten mit fixen Gewichten fuer 
//      Funktionen und Variablen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       tweight  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  globale Werte:  fweight = Gewicht von Funktionssymbolen
//                  vweight = Gewicht von Variablen
//
//  Beschreibung:   Ermittelt ein einfaches Gewicht eines Terms
//                  Funktionssymbole haben ein Gewicht von fweight
//                  Variablen ein Gewicht von vweight
//
//                  fweight und vweight werden mittels SetWeights
//                  festgelegt.
//-----------------------------------------------------------------------------
*/

static long tweight ( term *t )
{
    short   i;
    long    sum = 0;

    if (varp(t))
        return vweight;

    for ( i = 0; i < t->arity; sum += tweight (t->argument[i++]) );
    return sum+fweight;
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       cg_tweight  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  globale Werte:  cg_fweight = Gewicht von Funktionssymbolen bei kritischen Zielen
//                  cg_vweight = Gewicht von Variablen bei kritischen Zielen
//
//  Beschreibung:   Ermittelt ein einfaches Gewicht eines Terms
//                  Funktionssymbole haben ein Gewicht von cg_fweight
//                  Variablen ein Gewicht von cg_vweight
//
//                  cg_fweight und cg_vweight werden mittels SetCGWeights
//                  festgelegt.
//-----------------------------------------------------------------------------
*/

static long cg_tweight ( term *t )
{
    short   i;
    long    sum = 0;

    if (varp(t))
        return cg_vweight;

    for ( i = 0; i < t->arity; sum += cg_tweight (t->argument[i++]) );
    return sum+cg_fweight;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetWeights
//
//  Parameter:      Fweigth     Gewicht von Funktionssymbolen in einem Term
//                  Vweight     Gewicht von Variablensymbolen in eimen Term
//                              jeweils bezogen auf kritische Paare
//
//  Beschreibung:   Gewichte zur Termbewertung werden gesetzt
//-----------------------------------------------------------------------------
*/

void    SetWeights  ( long Fweight, long Vweight )
{
    fweight = NumberOfFunctions = Fweight;
    vweight = NumberOfVariables = Vweight;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetCGWeights
//
//  Parameter:      CGFweigth     Gewicht von Funktionssymbolen in einem Term
//                  CGVweight     Gewicht von Variablensymbolen in eimen Term
//                                jeweils bezogen auf kritische Ziele
//
//  Beschreibung:   Gewichte zur Termbewertung werden gesetzt
//-----------------------------------------------------------------------------
*/

void    SetCGWeights  ( long CGFweight, long CGVweight )
{
    cg_fweight = CGFweight;
    cg_vweight = CGVweight;
}


/*
//=============================================================================
*/




/*
//-----------------------------------------------------------------------------
//  Funktion:       fifo_weight ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Mit der Zeit groesser werdende Zahl.
//
//  Anmerkung:      Vermutlich nicht team-fair, nur zu
//                  Vergleichszwecken. StS, 18.11.1994
//-----------------------------------------------------------------------------
*/

double    fifo_weight ( termpair *tp )
{
   return fifo_count++;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       team_fifo_weight ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Mit der Zeit groesser werdende Zahl. Dabei wird
//                  versucht, die Zyklen einzurechnen, so dass der
//                  Experte teamfair wird. Funktioniert nur bis zu 5
//                  Millionen CPs und fuer etwa 400 Zyklen...
//
//  Anmerkung:      Nur zu Vergleichszwecken. StS, 18.11.1994
//-----------------------------------------------------------------------------
*/

double    team_fifo_weight ( termpair *tp )
{
   return CycleCount*5000000+fifo_count++;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sum_tweight ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Summe der Einzelgewichte fuer linke und rechte Seite
//
//  Anmerkung:      Mit fweight =2 und vweight=1 einer der 
//                  besten Einzelexperten. 
//-----------------------------------------------------------------------------
*/

double    sum_tweight ( termpair *tp )
{
    return  (tweight (tp->left) + tweight (tp->right));
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       max_tweight ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Maximum der Einzelgewichte fuer linke und rechte Seite
//-----------------------------------------------------------------------------
*/

double    max_tweight ( termpair *tp )
{
    long    lweight = tweight (tp->left);
    long    rweight = tweight (tp->right);

    return  ((lweight < rweight) ? rweight : lweight);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       gt_tweight
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Das Einzelgewicht des bezueglich der aktuellen Ordnung
//                  groesseren Terms.
//                  Bei unvergleichbaren Termen wird das arithmetische Mittel
//                  bestimmt.
//-----------------------------------------------------------------------------
*/

double    gt_tweight  ( termpair *tp )
{
    switch (Compare (tp->left, tp->right))
    {
    case TLESS:     return tweight ( tp->right );
    case TGREATER:  return tweight ( tp->left);
    case TEQUAL:    return (tweight (tp->left)+tweight (tp->right)) / 2;
    }
    return 0;
}



/*
//=============================================================================
//      Statistische Experten mit Gewichtsermittlung nach KBO
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       kbo  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Ermittelt das KBO-Gewicht eines Terms
//-----------------------------------------------------------------------------
*/

static long kbo ( term *t )
{
    short   i;
    long    sum;

    if (varp(t))
        return KBO_ALPHA;

    sum = Weight ( t->fcode );
    for ( i = 0; i < t->arity; sum += kbo (t->argument[i++]) );

    return sum;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       max_kbo ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Maximum der Einzelgewichte fuer linke und rechte Seite
//-----------------------------------------------------------------------------
*/

double    max_kbo ( termpair *tp )
{
    long    lweight = kbo (tp->left);
    long    rweight = kbo (tp->right);

    return  ((lweight < rweight) ? rweight : lweight);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sum_kbo ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Summe der Einzelgewichte fuer linke und rechte Seite
//-----------------------------------------------------------------------------
*/

double    sum_kbo ( termpair *tp )
{
    return  (kbo (tp->left) + kbo (tp->right));
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       gt_kbo ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   KBO-Gewicht des bezueglich der aktuellen Ordnung
//                  groesseren Terms. Scheint eigentlich nur fuer
//                  XKBO interessant.
//-----------------------------------------------------------------------------
*/

double    gt_kbo  ( termpair *tp )
{
    switch (Compare (tp->left, tp->right))
    {
    case TLESS:     return kbo ( tp->right );
    case TGREATER:  return kbo ( tp->left);
    case TEQUAL:    return (kbo (tp->left)+kbo (tp->right)) / 2;
    }
    return 0;
}



/*
//=============================================================================
//      Statistische Experte mit Anzahl der Funktionen
//=============================================================================
*/


static void     fcounth ( term *t )
{
    int     i;

    if (varp(t))
        return;

    f_flag[t->fcode] = true;
    for ( i = 0; i < t->arity; fcounth (t->argument[i++]) );
}


static long     fcount  ( term *t )
{
    long    res = 0;
    int     i;

    for (i = 1; i <= FuncCount; f_flag[i++] = false);

    fcounth ( t );

    for (i = 1; i <= FuncCount; i++)
       if (f_flag[i])
           res++;

    return res;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       gt_fcount ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Bewertet analog zu gt_tweight, allerdings
//                  wird mit der Zahl der vorkommenden Funktionssymbole mul

//-----------------------------------------------------------------------------
*/

double    gt_fcount ( termpair *tp )
{
    switch (Compare (tp->left, tp->right))
    {
    case TLESS:     return fcount ( tp->right ) * tweight ( tp->right );
    case TGREATER:  return fcount ( tp->left )  * tweight ( tp->left );
    case TEQUAL:    return (fcount(tp->left)*tweight (tp->left)+
			    fcount(tp->right)*tweight (tp->right)) /
			       2; 
    }
    return 0;
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       sum_fcount ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Bewertet analog zu sum_tweight, allerdings
//                  wird mit der Zahl der vorkommenden Funktionssymbole mul

//-----------------------------------------------------------------------------
*/

double    sum_fcount ( termpair *tp )
{
/*    switch (Compare (tp->left, tp->right))
    {
    case TLESS:     return fcount ( tp->right ) * (tweight (tp->left)
                           + tweight (tp->right)); 
    case TGREATER:  return fcount ( tp->left )  * (tweight (tp->left)
                           + tweight (tp->right)); 
    case TEQUAL:    return (fcount (tp->left)+fcount (tp->right)) * 
                           (tweight (tp->left) + tweight (tp->right));
    }
    return 0;
*/
    long    res = 0;
    int     i;

    for (i = 1; i <= FuncCount; f_flag[i++] = false);
    fcounth ( tp->left );
    fcounth ( tp->right );

    for (i = 1; i <= FuncCount; i++)
       if (f_flag[i])
           res++;

    return res * (tweight (tp->left) + tweight (tp->right));
}


/*
//=============================================================================
//      Statistische Experten mit variablen Gewichten fuer Funktionen.
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       tfweight  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  globale Werte:  fweight = Gewicht von Funktionssymbolen
//                  vweight = Gewicht von Variablen
//
//  Beschreibung:   Ermittelt des Gewicht eines Terms
//                  anhand von cpweight der Funktionssymbole
//                  Die einzelnen Funktionsgewichte sind in der Tabelle
//                  der Funktionen eingetragen.
//-----------------------------------------------------------------------------
*/

static long tfweight ( term *t )
{
    short   i;
    long    sum = 0;

    if (varp(t))
        return Function[0].cpweight;

    for ( i = 0; i < t->arity; sum += tfweight (t->argument[i++]) );
    return sum + Function[t->fcode].cpweight;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       max_tfweight ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Maximum der Einzelgewichte fuer linke und rechte Seite
//-----------------------------------------------------------------------------
*/

double    max_tfweight ( termpair *tp )
{
    long    lweight = tfweight (tp->left);
    long    rweight = tfweight (tp->right);

    return  ((lweight < rweight) ? rweight : lweight);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sum_tfweight ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Summe der Einzelgewichte fuer linke und rechte Seite
//-----------------------------------------------------------------------------
*/

double    sum_tfweight ( termpair *tp )
{
    return  (tfweight (tp->left) +tfweight (tp->right));
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       gt_tfweight ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Das Einzelgewicht des bezueglich der aktuellen Ordnung
//                  groesseren Terms.
//                  Bei unvergleichbaren Termen wird das arithmetische Mittel
//                  bestimmt.
//-----------------------------------------------------------------------------
*/

double    gt_tfweight  ( termpair *tp )
{
    switch (Compare (tp->left, tp->right))
    {
    case TLESS:     return tfweight ( tp->right );
    case TGREATER:  return tfweight ( tp->left);
    case TEQUAL:    return (tfweight (tp->left)+tfweight (tp->right)) / 2;
    }
    return 0;
}



/*
//=============================================================================
//      Polynombewertung
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       pweight  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Ermittelt ein Gewicht aus einem Term anhand der an
//                  die Funktionen gebundenen Polynome.
//-----------------------------------------------------------------------------
*/

static long    pweight  ( term *t )
{
    short   i;
    long    var[100];

    if (varp(t))
        return Function[0].cpweight;

    for ( i = 0; i < t->arity; i++ )
        var[i] = pweight (t->argument[i]);

    return polyval (Function[t->fcode].poly, var);
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       sum_poly ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Summe der Polynomgewichte fuer linke und rechte Seite
//
//  Anmerkung:      Mit fweight =2 und vweight=1 einer der 
//                  besten Einzelexperten. 
//-----------------------------------------------------------------------------
*/

double    sum_poly ( termpair *tp )
{
    return  (pweight (tp->left) + pweight (tp->right));
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       max_poly ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Maximum der Polynomgewichte fuer linke und rechte Seite
//-----------------------------------------------------------------------------
*/

double    max_poly ( termpair *tp )
{
    long    lweight = pweight (tp->left);
    long    rweight = pweight (tp->right);

    return  ((lweight < rweight) ? rweight : lweight);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       gt_poly
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Das Polynomgewichte des bezueglich der aktuellen Ordnung
//                  groesseren Terms.
//                  Bei unvergleichbaren Termen wird das arithmetische Mittel
//                  bestimmt.
//-----------------------------------------------------------------------------
*/

double    gt_poly  ( termpair *tp )
{
    switch (Compare (tp->left, tp->right))
    {
    case TLESS:     return pweight ( tp->right );
    case TGREATER:  return pweight ( tp->left);
    case TEQUAL:    return (pweight (tp->left)+pweight (tp->right)) / 2;
    }
    return 0;
}



/*
//=============================================================================
//      Differenzenexperte
//=============================================================================
*/

double    diff_weight  ( termpair *tp )
{
    long    lweight = tweight (tp->left);
    long    rweight = tweight (tp->right);
    long    diff;

    diff = (lweight > rweight) ? (lweight - rweight)
                               : (rweight - lweight);

    return   (diff * DiffFactor * MaxFactor) +(lweight + rweight);
}


/*
//=============================================================================
//      Divergenz
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       divergence
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Divergenzexperte
//-----------------------------------------------------------------------------
*/


double    divergence   ( termpair *tp )
{
    Error ( __FILE__ ": "  "divergence", "Divergenzexperte nicht implementiert!." );
    return 0;
/*
    switch (Compare (tp->left, tp->right))
    {
    case TLESS:     return tfweight ( tp->right );
    case TGREATER:  return tfweight ( tp->left );
    case TEQUAL:    return (tfweight (tp->left)+tfweight (tp->right)) / 2;
    }
    return 0;
*/
}



/*
//=============================================================================
//      GOALMATCH
//=============================================================================
*/

static term     *Left, *Right;
static long     result;


/*
//-----------------------------------------------------------------------------
//  Funktion:       testgmatch  ( long max, term *t1, term *t2 )
//
//  Parameter:      max     bisheriges Maximum
//                  t1      matchender Term
//                  t2      auf diesen Term soll 'gematcht' werden
//
//  Rueckgabe:      Das maximale Gewicht des Terms in t2, auf den t1 
//                  matched; d.h. das Gewicht des groessten Teilterms.
//
//  Beschreibung:   Hilfsfunktion fuer testgoalmatch
//                  Wird auf von Auswahl-Referee GOALMATCH verwendet.
//-----------------------------------------------------------------------------
*/

static long     testgmatch  ( long max, term *t1, term *t2 )
{
    short  i;
    long   result;
    subst  *sigma = NULL;

    if (varp (t2) || (max > t2->weight))
        return max;

    if (match (t1,t2, &sigma))
    {
        deletematch ( sigma );
        sigma = NULL;
        return t2->weight;
    }

    for (i = 0; i < t2->arity; i++)
        if (max < (result = testgmatch (max, t1, t2->argument[i])))
           max = result;

    return max;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       testgunify  ( long max, term *t1, term *t2 )
//
//  Parameter:      max     bisheriges Maximum
//                  t1      unifizierender Term
//                  t2      mit diesen Term soll unifiziert werden
//
//  Rueckgabe:      Das maximale Gewicht des Terms in t2, der sich mit t1
//                  unifizieren laesst; d.h. das Gewicht des groessten Teilterms.
//
//  Beschreibung:   Analog zu testgmatch wird t2 rekursiv durchlaufen.
//-----------------------------------------------------------------------------
*/

static long     testgunify  ( long max, term *t1, term *t2 )
{
    short  i;
    long   result;
    subst  *sigma = NULL;

    if (varp (t2) || (max > t2->weight))
        return max;

    if (unify (t1,t2, &sigma))
    {
        deletematch ( sigma );
        sigma = NULL;
        return t2->weight;
    }

    for (i = 0; i < t2->arity; i++)
        if (max < (result = testgunify (max, t1, t2->argument[i])))
           max = result;

    return max;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       testgoalmatch ( termpair *goal )
//
//  Parameter:      goal    Eine Ziel-Gleichung
//
//  globale Werte:  Left    Linke Seite des zu bewertenden kritischen Paares
//                  Right   Rechte Seite des zu bewertenden kritischen Paares
//                  result  bisheriges Minimum
//
//  Rueckgabe:      Aehnlichkeitsmass
//
//  Beschreibung:   Bestimmt ein Aehnlichkeitsmass zwischen <Left,Right> und
//                  goal aufgrund von Matchs des kritischen Paares auf ein Ziel
//                  oder (falls GoalMatch = true) des Ziels auf 
//                  das kritische Paar
//-----------------------------------------------------------------------------
*/

static void     testgoalmatch ( termpair *goal )
{
    long  match_1 = testgmatch ( 0, Left,  goal->left  );
    long  match_2 = testgmatch ( 0, Right, goal->right );
    long  match_3 = testgmatch ( 0, Left,  goal->right );
    long  match_4 = testgmatch ( 0, Right, goal->left  );

    long  test;
    long  mx;

    if (match_1 && match_2)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-match_1 + goal->right->weight-match_2) 
                 * DoubleMatchWeight;
        result = min(result, test);
    }
    else
    if (match_3 && match_4)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-match_4 + goal->right->weight-match_3) 
                 * DoubleMatchWeight;
        result = min(result, test);
    }

    if (match_1 || match_4)
    {
        mx     = max (match_1,match_4);
        test   = (Left->weight + Right->weight) +
                 (goal->right->weight + goal->left->weight - mx) * SingleMatchWeight;
        result = min(result, test);
    }
 
    if (match_2 || match_3)
    {
        mx     = max (match_2,match_3);
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight + goal->right->weight - mx) * SingleMatchWeight;
        result = min(result, test);
    }

/*
    if (!GoalMatch)
        return;

    match_1 = testgmatch ( 0, goal->left , Left  );
    match_2 = testgmatch ( 0, goal->right, Right );
    match_3 = testgmatch ( 0, goal->right, Left  );
    match_4 = testgmatch ( 0, goal->left , Right );

    if (match_1 && match_2)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-match_1 + goal->right->weight-match_2)
                 * GDoubleMatchWeight;
        result = min(result, test);
    }
    else
    if (match_3 && match_4)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-match_4 + goal->right->weight-match_3)
                 * GDoubleMatchWeight;
        result = min(result, test);
    }

    if (match_1 || match_4)
    {
        mx     = max (match_1,match_4);
        test   = (Left->weight + Right->weight) +
                 (goal->right->weight + goal->left->weight - mx) * GSingleMatchWeight;
        result = min(result, test);
    }

    if (match_2 || match_3)
    {
        mx     = max (match_2,match_3);
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight + goal->right->weight - mx) * GSingleMatchWeight;
        result = min(result, test);
    }
*/
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       goalmatch
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Aehnlichkeit mit Ziel(en) bewerten.
//-----------------------------------------------------------------------------
*/

double    goalmatch ( termpair *tp )
{
    result = NoMatchWeight * ((tp->left)->weight + (tp->right)->weight);

    Left  = tp->left;
    Right = tp->right;
    ForAllGoalsDo ( testgoalmatch );

    return result;
}



/*
//=============================================================================
//      GOALTEST
//=============================================================================
*/


/*
//-----------------------------------------------------------------------------
//  Funktion:       testmatch  ( long max, term *t1, term *t2 )
//
//  Parameter:      max     bisheriges Maximum
//                  t1      matchender Term
//                  t2      auf diesen Term soll 'gematcht' werden
//
//  Rueckgabe:      Das maximale Gewicht des Terms in t2, auf den t1 
//                  matched; d.h. das Gewicht des groessten Teilterms.
//
//  Beschreibung:   Hilfsfunktion fuer testgoal
//                  Wird auf von Auswahl-Referee GOALTEST verwendet.
//-----------------------------------------------------------------------------
*/

long     testmatch ( long max, term *t1, term *t2 )
{
    short  i;
    long   result;
    subst  *sigma = NULL;

    if (varp (t2) || (max > t2->weight))
        return max;

    if (match (t1,t2, &sigma))
    {
        deletematch ( sigma );
        sigma = NULL;
        return t2->weight;
    }

    for (i = 0; i < t2->arity; i++)
        if (max < (result = testmatch (max, t1, t2->argument[i])))
           max = result;

    return max;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       testunify  ( long max, term *t1, term *t2 )
//
//  Parameter:      max     bisheriges Maximum
//                  t1      matchender Term
//                  t2      auf diesen Term soll 'gematcht' werden
//
//  Rueckgabe:      Das maximale Gewicht des Terms in t2, mit dem t1
//                  unifizierbar ist; d.h. das Gewicht des groessten Teilterms.
//
//  Beschreibung:   Hilfsfunktion fuer testgoal
//                  Wird auf von Auswahl-Referee GOALTEST verwendet.
//-----------------------------------------------------------------------------
*/

long     testunify  ( long max, term *t1, term *t2 )
{
    short  i;
    long   result;
    subst  *sigma = NULL;

    if (varp (t2) || (max > t2->weight))
        return max;

    if (unify (t1,t2, &sigma))
    {
        deletematch ( sigma );
        sigma = NULL;
        return t2->weight;
    }

    for (i = 0; i < t2->arity; i++)
        if (max < (result = testunify (max, t1, t2->argument[i])))
           max = result;

    return max;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       testgoal ( termpair *goal )
//
//  Parameter:      goal    Eine Ziel-Gleichung
//
//  globale Werte:  Left    Linke Seite des zu bewertenden kritischen Paares
//                  Right   Rechte Seite des zu bewertenden kritischen Paares
//                  result  bisheriges Minimum
//
//  Rueckgabe:      Aehnlichkeitsmass
//
//  Beschreibung:   Bestimmt ein Aehnlichkeitsmass zwischen <Left,Right> und
//                  goal.
//-----------------------------------------------------------------------------
*/

void     testgoal ( termpair *goal )
{
    subst  *sigma       = NULL;
    subst  *tau         = NULL;
    long   match_1, match_2, match_3, match_4;
    long   unify_1, unify_2, unify_3, unify_4;
    long   test, mx;


/* ----- Tests der Gruppe 1 ----------------------------------------------- */

    if   (match (Left,  goal->left,  &sigma) && match (Right, goal->right, &sigma))
    {
        deletematch ( sigma );
        result = 0;                         /* Erfolgreiches DOUBLEMATCH    */
/*        DEBUG ("DM\n");       */
        return;
    }
    if   (match (Right, goal->left,  &sigma) && match (Left,  goal->right, &sigma))
    {
        deletematch ( sigma );
        result = 0;                         /* Erfolgreiches DOUBLEMATCH    */
/*        DEBUG ("DM\n");       */
        return;
    }


    if   (match (Left,  goal->left, &sigma) || match (Right, goal->right, &tau))
    {
        deletematch ( sigma );
        deletematch ( tau );
        result = 0;                         /* Erfolgreiches SINGLEMATCH    */
        result =   Left->weight + Right->weight 
                 - goal->left->weight - goal->right->weight;
/*      DEBUG ("SM\n");         */
        return;
    }
    if   (match (Right,  goal->left, &sigma) || match (Left, goal->right, &tau))
    {
        deletematch ( sigma );
        deletematch ( tau );
        result = 0;                         /* Erfolgreiches SINGLEMATCH    */
        result =   Left->weight + Right->weight 
                 - goal->left->weight - goal->right->weight;
/*      DEBUG ("SM\n");         */
        return;
    }


     DEBUG ("No Match, No Unify\n");
    if (!SkolemGoal)
    {
        if   (unify (Left,  goal->left, &sigma) && unify (Right, goal->right, &sigma))
        {
            deletematch ( sigma );
            result = 0;                     /* Erfolgreiches DOUBLEUNIFY    */
            DEBUG ("DU\n");
            return;
        }
        if   (unify (Right,  goal->left, &sigma) && unify (Left, goal->right, &sigma))
        {
            deletematch ( sigma );
            result = 0;                     /* Erfolgreiches DOUBLEUNIFY    */
            DEBUG ("DU\n");
            return;
        }


        if   (unify (Left,  goal->left, &sigma) || unify (Right, goal->right, &tau))
        {
            deletematch ( sigma );
            deletematch ( tau );
            result = 0;                     /* Erfolgreiches SINGLEUNIFY    */
            DEBUG ("SU\n");
            return;
        }
        if   (unify (Right,  goal->left, &sigma) || unify (Left, goal->right, &tau))
        {
            deletematch ( sigma );
            deletematch ( tau );
            result = 0;                     /* Erfolgreiches SINGLEUNIFY    */
            DEBUG ("SU\n");
            return;
        }
    }


/* ----- Tests der Gruppe 2 ----------------------------------------------- */

    if (SkolemGoal)
    {
        if (   (member (goal->left,  Left ) && (member (goal->right, Right)))
            || (member (goal->left,  Right) && (member (goal->right, Left ))))
        {
            result =   Left->weight  - goal->left->weight
                     + Right->weight - goal->right->weight;
                                            /* Doppelte Teiltermeigenschaft */
            DEBUG ("DTT\n");
            return;
        }

        if (member (goal->left, Left))
        {
            result =   Left->weight + Right->weight 
                     - goal->left->weight;
            DEBUG ("TT\n");
            return;
        }
        if (member (goal->left, Right))
        {
            result =   Left->weight + Right->weight 
                     - goal->left->weight;
            DEBUG ("TT\n");
            return;
        }

        if (member (goal->right, Left))
        {
            result =   Left->weight + Right->weight 
                     - goal->right->weight;
            DEBUG ("TT\n");
            return;
        }
        if (member (goal->right, Right))
        {
            result =   Left->weight + Right->weight 
                     - goal->right->weight;
            DEBUG ("TT\n");
            return;
        }
    }
    else
    {
        Error ( __FILE__ ": "  "testgoal", 
                "Test der Gruppe 2 fuer Ziele mit Variablen noch nicht implementiert." );
    }


/* ----- Tests der Gruppe 3: Match krit. Paar auf Ziel -------------------- */

    match_1 = testgmatch ( 0, Left,  goal->left  );
    match_2 = testgmatch ( 0, Right, goal->right );
    match_3 = testgmatch ( 0, Left,  goal->right );
    match_4 = testgmatch ( 0, Right, goal->left  );

    if (match_1 && match_2)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-match_1 + goal->right->weight-match_2) 
                 * DoubleMatchWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Match 2\n");
    }
    else
    if (match_3 && match_4)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-match_4 + goal->right->weight-match_3) 
                 * DoubleMatchWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Match 2\n");
    }

    if (match_1 || match_4)
    {
        mx     = max (match_1,match_4);
        test   = (Left->weight + Right->weight) +
                 (goal->right->weight + goal->left->weight - mx) * SingleMatchWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Match\n");
    }
 
    if (match_2 || match_3)
    {
        mx     = max (match_2,match_3);
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight + goal->right->weight - mx) * SingleMatchWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Match\n");
    }

/* ----- Tests der Gruppe 3: Unify krit. Paar auf Ziel -------------------- */

    if (SkolemGoal)
        return;

    unify_1 = testgunify ( 0, Left,  goal->left  );
    unify_2 = testgunify ( 0, Right, goal->right );
    unify_3 = testgunify ( 0, Left,  goal->right );
    unify_4 = testgunify ( 0, Right, goal->left  );

    if (unify_1 && unify_2)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-unify_1 + goal->right->weight-unify_2) 
                 * DoubleUnifyWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Unify 2\n");
    }
    else
    if (unify_3 && unify_4)
    {
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight-unify_4 + goal->right->weight-unify_3) 
                 * DoubleUnifyWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Unify 2\n");
    }

    if (unify_1 || unify_4)
    {
        mx     = max (unify_1,unify_4);
        test   = (Left->weight + Right->weight) +
                 (goal->right->weight + goal->left->weight - mx) * SingleUnifyWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Unify\n");
    }
 
    if (unify_2 || unify_3)
    {
        mx     = max (unify_2,unify_3);
        test   = (Left->weight + Right->weight) +
                 (goal->left->weight + goal->right->weight - mx) * SingleUnifyWeight;
        result = min(result, test);
        DEBUG ("Gruppe 3: Unify\n");
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       goaltest
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Aehnlichkeit mit Ziel(en) bewerten.
//-----------------------------------------------------------------------------
*/

double    goaltest ( termpair *tp )
{
    result = NoMatchWeight * ((tp->left)->weight + (tp->right) ->weight);

    Left  = tp->left;
    Right = tp->right;
    ForAllGoalsDo ( testgoal );

    return result;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SpecialWeight ( termpair *tp )
//
//  Parameter:      tp      ein Termpaar (kritisches Paar)
//
//  Beschreibung:   Die Bewertung eines kritischen Paares wird um
//                  special_factor verbessert.
//                  Dies wird zur bevorzugten Behandlung kritischer Paare
//                  die von Regeln aus dem Set Of Support stammen
//                  genutzt.
//-----------------------------------------------------------------------------
*/

void    SpecialWeight ( termpair *tp )
{
    if (tp->weight > 0)
        tp->weight -= (special_factor * tp->weight) / 100;
    else
        tp->weight += (special_factor * tp->weight) / 100;
}


/*
//=============================================================================
//      GOALSIM
//=============================================================================
*/


static long    part_match ( term *t, term *goal, long max )
{
    subst    *tau = NULL;
    short    i;
    long     result;


    if (varp (t) || (t->weight < LowerBound) || (max > t->weight))
        return max;

    if ((t->weight <= goal->weight) && (match (t, goal, &tau)))
    {
        deletematch ( tau );
        tau = NULL;
        return ((NumberOfFunctions <= count_func (t)) && 
                (NumberOfVariables <= count_var (t)))    ? t->weight
                                                         : max;
    }

    for (i = 0; i < t->arity; i++)
        if (max < (result = part_match (t->argument[i], goal, max)))
            max = result;

    return max;
}


void    simgoal ( termpair *goal )
{
    long    pmatch_ll = part_match ( Left,  goal->left,  0 );
    long    pmatch_lr = part_match ( Left,  goal->right, 0 );
    long    pmatch_rl = part_match ( Right, goal->left,  0 );
    long    pmatch_rr = part_match ( Right, goal->right, 0 );
    long    test;
    bool    flag    = true;


    if (pmatch_ll && pmatch_rr)
    {
        test   = ((Left->weight + Right->weight) - (pmatch_ll + pmatch_rr))
                 * DoubleMatchWeight;
        result = min (result, test);
        flag = false;
    }

    if (pmatch_rl && pmatch_lr)
    {
        test   = ((Left->weight + Right->weight) - (pmatch_rl + pmatch_lr))
                 * DoubleMatchWeight;
        result = min (result, test);
        flag = false;
    }

    if (flag && (pmatch_ll || pmatch_lr || pmatch_rl || pmatch_rr))
    {
        test   =  ((Left->weight + Right->weight) 
                 - (pmatch_ll + pmatch_lr + pmatch_rl + pmatch_rr))
                 * SingleMatchWeight;
        result = min (result, test);
        flag = false;
    }
/*
    if (flag)
        putchar ('.');
*/
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       goalsim ( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Aehnlichkeit mit Ziel(en) bewerten.
//-----------------------------------------------------------------------------
*/

double    goalsim ( termpair *tp )
{
    LowerBound = 2*NumberOfFunctions + NumberOfVariables;
    result = NoMatchWeight * ((tp->left)->weight + (tp->right)->weight);

    Left  = tp->left;
    Right = tp->right;
    ForAllGoalsDo ( simgoal );

    return result;
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       occnest( termpair *tp )
//
//  Parameter:      left    linke Seite des kritischen Paares
//                  right   rechte Seite des kritischen Paares
//
//  Rueckgabe:      Bewertung eines kritischen Paares
//
//  Beschreibung:   Ueberschreitung von 'measures' wird nachteilig bewertet.
//-----------------------------------------------------------------------------
*/

#define PSI(x) ((x<1) ? 1 : x+1)

extern bool DemoMode;

double occnest (termpair *tp)
{
   register int i,h1,h2,nf,mf;
   short maxi;
   long d;
   
   for (i=0,d=1;i<=goal_fn_nr;i++)
   {
      h1=measure0(goal_fn_m[i].func,tp->left);
      h2=measure0(goal_fn_m[i].func,tp->right);
      nf=max(h1,h2)-goal_fn_m[i].m0;
      if (Function[goal_fn_m[i].func].arity)
      {
	 maxi=0;
	 measure1(goal_fn_m[i].func,tp->right,0,&maxi);
	 measure1(goal_fn_m[i].func,tp->left,0,&maxi);
	 mf=maxi-goal_fn_m[i].m1;
      }
      else
      {
	 mf = 0;
      }
      d=d*PSI(nf)*PSI(mf);
   }
   return sum_tweight(tp)*d;
}

static short measure0 (function op, term *t)
{
   if (!t || varp(t))
   {
      return 0;
   }
   else
   {
      short i, result=(t->fcode==op) ? 1 : 0;
      for (i=0;i<t->arity;i++)
      {
	 result+=measure0(op,t->argument[i]);
      }
      return result;
   }
}

static short measure1 (function op, term *t, short currentMax, short *totalMax)
{
  register int i;

  if(!t || t->fcode < 0)
  {
    *totalMax=max(*totalMax,currentMax);
    return *totalMax;
  }
  if (t->fcode==op)
    currentMax++;
  else
  {
    *totalMax=max(*totalMax,currentMax);
    currentMax=0;
  }
  for (i=0;i<t->arity;i++)
    measure1(op,t->argument[i],currentMax,totalMax);
  return *totalMax;
}

void init_goal_fn_m (void)
{
  termpair *h;
  function i;
  short inter1,inter2;

  if (considerAll)
  {
    goal_fn_nr=FuncCount-1;
    for (i=0;i<=goal_fn_nr;i++)
    {
      goal_fn_m[i].func=i+1;
      goal_fn_m[i].m0=goal_fn_m[i].m1=0;
      for (h=SetOfGoals.first;h;h=h->next)
      {
        inter1=measure0(goal_fn_m[i].func,h->left);
        inter2=measure0(goal_fn_m[i].func,h->right);
        inter1=max(inter1,inter2);
        goal_fn_m[i].m0=max(goal_fn_m[i].m0,inter1);
        if (Function[goal_fn_m[i].func].arity)
        {
          inter2=0;
          measure1(goal_fn_m[i].func,h->left,0,&inter2);
          measure1(goal_fn_m[i].func,h->right,0,&inter2);
          goal_fn_m[i].m1=max(goal_fn_m[i].m1,inter2);
        }
      }
    }
  }
  else
  {
    goal_fn_nr=-1;
    for (h=SetOfGoals.first;h;h=h->next)
    {
      add_fn(h->left);
      add_fn(h->right);
    }
    for (i=0;i<=goal_fn_nr;i++)
    {
      goal_fn_m[i].m0=goal_fn_m[i].m1=0;
      for (h=SetOfGoals.first;h;h=h->next)
      {
        inter1=measure0(goal_fn_m[i].func,h->left);
        inter2=measure0(goal_fn_m[i].func,h->right);
        inter1=max(inter1,inter2);
        goal_fn_m[i].m0=max(goal_fn_m[i].m0,inter1);
        if (Function[goal_fn_m[i].func].arity)
       {
          inter2=0;
          measure1(goal_fn_m[i].func,h->left,0,&inter2);
          measure1(goal_fn_m[i].func,h->right,0,&inter2);
          goal_fn_m[i].m1=max(goal_fn_m[i].m1,inter2);
        }
      }
    }
  }
  if (!DemoMode)
  {
    printf("\nMeasures der Funktionssymbole im Ziel:\n");
    for (i=0;i<=goal_fn_nr;i++)
      printf("Funktionssymbol: %s; m0 = %i, m1 = %i\n",
             Function[goal_fn_m[i].func].ident,
             goal_fn_m[i].m0,goal_fn_m[i].m1);
    printf("\n");
  }
}

static void add_fn (term *t)
{
  int i=0;
  bool available=false;

  if (!t || varp(t))
    return;
  while (i<=goal_fn_nr && !available)
  {
    if (goal_fn_m[i].func==t->fcode)
      available=true;
    else
      i++;
  }
  if (!available)
    goal_fn_m[++goal_fn_nr].func=t->fcode;
  for (i=0;i<t->arity;i++)
    add_fn(t->argument[i]);
}

/*****************************************************************************/
/*                                                                           */
/*                 Spezielle Experten fuer kritische Ziele                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  diff_goal_weight(_terms)                                 */
/*                                                                           */
/*  Parameter    :  left    linke Seite eines Teilterms eines kr. Zieles     */
/*                  right   rechte Seite eines Teilterms eines kr. Zieles    */
/*                                                                           */
/*  Returnwert   :  Bewertung der Teilterme.                                 */
/*                                                                           */
/*  Beschreibung :  Diese Funktion bestimmt das Gewicht eines kritischen     */
/*                  Zieles.                                                  */
/*                  Sie stimmt mit diff_goal ueberein, ausser dass, wenn     */
/*                  die beiden Terme nicht unifizierbar sind, nicht mit dem  */
/*                  Faktor NoUnifyFactor zusaetzlich multipliziert wird.     */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/


long diff_goal_weight_terms(term* left, term* right)
{
   int i;
   long gewicht = 0;
   
   if ( varp(left) || varp(right) )
   {
      return 0;
   }
   
   if ( left->fcode != right->fcode )
   {
      return ( cg_tweight( left ) + cg_tweight( right ) );
   }
   
   /* Top-Funktionssymbole sind gleich -> selbe Stelligkeit */
   for ( i=0; i< left->arity; i++ )
   {
      gewicht += diff_goal_weight_terms( left->argument[i], right->argument[i] );
   }
   
   return gewicht;
} /* Ende von diff_goal_weight_terms */


long diff_goal_weight ( termpair *tp )
{
   return diff_goal_weight_terms(tp->left, tp->right);
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  diff_goal(_terms)                                        */
/*                                                                           */
/*  Parameter    :  left    linke Seite des kritischen Zieles                */
/*                  right   rechte Seite des kritischen Zieles               */
/*                                                                           */
/*  Returnwert   :  Bewertung eines kritischen Zieles                        */
/*                                                                           */
/*  Beschreibung :  Diese Funktion bestimmt das Gewicht eines kritischen     */
/*                  Zieles. Dabei werden die beiden Terme des kritischen     */
/*                  Zieles von oben nach unten an Stellen verglichen.        */
/*                  Treffen an einer Stelle 2 verschiedene Funktionssymbole  */
/*                  aufeinander, so wird das Termgewicht der beiden Teil-    */
/*                  terme zu dem Gewicht des kritischen Ziels hinzuaddiert.  */
/*                                                                           */
/*  Globale Var. :  NoUnifyFactor                                            */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

long diff_goal_terms ( term *left, term *right )
{
  int i;
  long gewicht = 0;

  if ( varp(left) || varp(right) )
  {
    return 0;
  }
  
  if ( left->fcode != right->fcode )
  {
    return ( ( cg_tweight( left ) + cg_tweight( right ) ) * NoUnifyFactor );
  }
  
  /* Top-Funktionssymbole sind gleich -> selbe Stelligkeit */
  for ( i=0; i<left->arity; i++ )
  {
    gewicht += diff_goal_terms( left->argument[i], right->argument[i] );
  }
  
  return gewicht;
} /* Ende von diff_goal */


double diff_goal ( termpair *tp )
{
   return diff_goal_terms(tp->left,tp->right);
}



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  max_right_unif                                           */
/*                                                                           */
/*  Parameter    :  left    linke Seite des kritischen Zieles                */
/*                  right   rechte Seite des kritischen Zieles               */
/*                                                                           */
/*  Returnwert   :  Bewertung eines kritischen Zieles                        */
/*                                                                           */
/*  Beschreibung :  Hilfsfunktion fuer unif_goal.                            */
/*                  Diese Funktion durchlaeuft alle Teilterme der rechten    */
/*                  Seite und sucht jeweils nach Unifikatoren mit der gesam- */
/*                  ten linken Seite.                                        */
/*                  Die linke Seite bleibt dabei stets fest.                 */
/*                                                                           */
/*  Globale Var. :  MinWeight                                                */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

long max_right_unif ( term *left, term *right )
{
  int i;
  long max_gew = 0;
  long zw_max;
  subst *tau = NULL;

  /* Die linke Seite kann keine Variable sein */
  if ( varp(right) )
  {
    return 0;
  }
  
  if ( unify( left, right, &tau ) )
  {
    return ( cg_tweight( left ) + cg_tweight( right ) );
  }
  
  for ( i=0; i<right->arity; i++ )
  {
    if ( (zw_max = max_right_unif( left, right->argument[i] ) ) > max_gew )
    {
      max_gew = zw_max;
    }
  }
  
  return max_gew;
} /* Ende von max_right_unif */




/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  max_unif                                                 */
/*                                                                           */
/*  Parameter    :  left    linke Seite des kritischen Zieles                */
/*                  right   rechte Seite des kritischen Zieles               */
/*                                                                           */
/*  Returnwert   :  Bewertung eines kritischen Zieles                        */
/*                                                                           */
/*  Beschreibung :  Hilfsfunktion fuer unif_goal.                            */
/*                  Diese Funktion versucht zuerts auf Top-level zu unifi-   */
/*                  zieren. Falls nicht dort unifizierbar, werden fuer den   */
/*                  linken Term alle Teilterme des rechten betrachtet.       */
/*                  Dann erfolgt dasselbe rekursiv fuer alle Teilterme der   */
/*                  linken Seite.                                            */
/*                                                                           */
/*  Globale Var. :  MinWeight                                                */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

long max_unif ( term *left, term *right )
{
  long  max_gew = 0;
  long  zw_max;
  subst       *tau = NULL;
  int   i;

  if ( varp(left) || varp(right) )
  {
    return 0;
  }
  
  /* Unifizieren auf Top-level testen */
  if ( unify( left, right, &tau ) )
  {
    return ( cg_tweight( left ) + cg_tweight( right ) );
  }
  
  /* Es werden nun die Teilterme der rechten Seite betrachtet */
  for ( i=0; i<right->arity; i++ )
  {
    if ( (zw_max = max_right_unif( left, right->argument[i] ) ) > max_gew )
    {
      max_gew = zw_max;
    }
  }
  
  /* Es werden nun die Teilterme der linken Seite durchlaufen */
  for ( i=0; i<left->arity; i++ )
  {
    if ( (zw_max = max_unif( left->argument[i], right ) ) > max_gew )
    {
      max_gew = zw_max;
    }
  }
  
  return max_gew;
} /* Ende von max_unif */


  


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  unif_goal                                                */
/*                                                                           */
/*  Parameter    :  left    linke Seite des kritischen Zieles                */
/*                  right   rechte Seite des kritischen Zieles               */
/*                                                                           */
/*  Returnwert   :  Bewertung eines kritischen Zieles                        */
/*                                                                           */
/*  Beschreibung :  Diese Funktion bestimmt das Gewicht eines kritischen     */
/*                  Zieles. Dabei wird versucht in den beiden Seiten des     */
/*                  kritischen Ziels moeglichst grosse Teilterme zu finden,  */
/*                  die sich unifizieren lassen. Das Gewicht der groessten   */
/*                  unifizierbaren Teilterme wird vom Gesamtgewicht der      */
/*                  beiden Terme des kritischen Zieles subtrahiert.          */
/*                  Dabei muessen die unifizierbaren Teilterme jedoch eine   */
/*                  Mindestgewicht von MinWeight aufweisen, um z.B. trivi-   */
/*                  ale oder "uniteressante" Unifikatoren zu umgehen.        */
/*                                                                           */
/*  Globale Var. :  MinWeight                                                */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

double unif_goal ( termpair *tp )
{
  long   max;

  max = max_unif( tp->left, tp->right );

  if ( max > MinWeight )
  {
    return ( cg_tweight( tp->left ) + cg_tweight( tp->right ) - max );
  }
  
  return ( cg_tweight( tp->left ) + cg_tweight( tp->right ) );
} /* Ende von unif_goal */














