/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_class_t_exp                                           */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die fuer die Klassifizierung der Experten, die im */
/*                 letzten Zyklus eingesetz waren, benoetigt werden.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#ifndef  __EXP_CLASS_T_EXP
#define  __EXP_CLASS_T_EXP


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/
/* verwendet in siegerkriterien_erfuellt :                                   */
#define INCREASE_PERCENT    0.5

/* verwendet in verlierekriterien_erfuellt                                   */
#define DECREASE_PERCENT    0.5
#define VERLIERERMAXCYCL      3
#define VERLIERERGRENZE3    0.4

/* verwendet in ClassifyTeamExperts                                          */
#define TEAMNOTCHANGED     3

/* verwendet in siegergrenze_best */
/* Praefix "W_" steht fuer Winner */
/* Ansonsten, wie in referee.h bezeichnet */
#define     W_SIZE_R           100
#define     W_SIZE_E           50
#define     W_SIZE_G           20
#define     W_SIZE_CP          500
#define     W_SIZE_CG          300
#define     W_NEW_R            30
#define     W_RED_R            10
#define     W_DEL_R            5
#define     W_NEW_E            3
#define     W_RED_E            10
#define     W_DEL_E            5
#define     W_NEW_G            4
#define     W_RED_G            3
#define     W_NEW_CP           150
#define     W_DEL_CP           50
#define     W_NEW_CG           80
#define     W_REDCOUNT         500

/* verwendet in verlierergrenze_best */
/* Praefix "L_" steht fuer Loser */
/* Ansonsten, wie in referee.h bezeichnet */
#define     L_SIZE_R           80
#define     L_SIZE_E           60
#define     L_SIZE_G           10
#define     L_SIZE_CP          500
#define     L_SIZE_CG          300
#define     L_NEW_R            20
#define     L_RED_R            5
#define     L_DEL_R            2
#define     L_NEW_E            5
#define     L_RED_E            8
#define     L_DEL_E            3
#define     L_NEW_G            2
#define     L_RED_G            1
#define     L_NEW_CP           150
#define     L_DEL_CP           30
#define     L_NEW_CG           80
#define     L_REDCOUNT         250

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Variablen                              */
/*                                                                           */
/*****************************************************************************/
extern  double        increase_percent;

extern  double        decrease_percent;

extern  int           verlierermaxcycl;
extern  double        verlierergrenze3;

extern  int           teamnotchanged;

extern  int           w_size_r;
extern  int           w_size_e;
extern  int           w_size_g;
extern  int           w_size_cp;
extern  int           w_size_cg;
extern  int           w_new_r;
extern  int           w_red_r;
extern  int           w_del_r;
extern  int           w_new_e;
extern  int           w_red_e;
extern  int           w_del_e;
extern  int           w_new_g;
extern  int           w_red_g;
extern  int           w_new_cp;
extern  int           w_del_cp;
extern  int           w_new_cg;
extern  int           w_redcount;

extern  int           l_size_r;
extern  int           l_size_e;
extern  int           l_size_g;
extern  int           l_size_cp;
extern  int           l_size_cg;
extern  int           l_new_r;
extern  int           l_red_r;
extern  int           l_del_r;
extern  int           l_new_e;
extern  int           l_red_e;
extern  int           l_del_e;
extern  int           l_new_g;
extern  int           l_red_g;
extern  int           l_new_cp;
extern  int           l_del_cp;
extern  int           l_new_cg;
extern  int           l_redcount;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void ClassifyTeamExperts ( void );

#endif
