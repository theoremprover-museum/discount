/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_form_next_t                                           */
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

#ifndef  __EXP_FORM_NEXT_T
#define  __EXP_FORM_NEXT_T


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/

/* Die folgenden Konstanten werden benoetigt, um festzustellen, ob der Da-   */
/* tabase-Experte eingesetzt werden soll. -> Suffix _DB                      */
/* Beschreibung siehe bei den Funktionen !                                   */
/* verwendet in database_bed_erfuellt */
#define HOST_LIMIT_DB    2
#define CYCLE_DIFF_DB    3
#define DOM_LIMIT_DB     3

/* Die folgenden Konstanten werden benoetigt, um festzustellen, ob ein Spe-  */
/* zialist fuer kritische Parre eingesetzt werden soll.  -> Suffix _CP       */
/* Beschreibung der Konstanten siehe bei den Funktionen                      */
/* verwendet in krit_paar_spez_bed_erfuellt */
#define HOST_LIMIT_CP            2
#define CYCLE_DIFF_CP            3
#define CRIT_P_LIMIT_CP        750

/* Die folgenden Konstanten werden benoetigt, um festzustellen, ob ein Spe-  */
/* zialist fuer kritische Ziele eingesetzt werden soll.  -> Suffix _CG       */
/* Beschreibung der Konstanten siehe bei den Funktionen                      */
/* verwendet in krit_ziel_spez_bed_erfuellt */
#define HOST_LIMIT_CG            2
#define CYCLE_DIFF_CG            3
#define CRIT_G_LIMIT_CG        750

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Variablen                              */
/*                                                                           */
/*****************************************************************************/
extern  int           host_limit_db;
extern  int           cycle_diff_db;
extern  int           dom_limit_db;

extern  int           host_limit_cp;
extern  int           cycle_diff_cp;
extern  int           crit_p_limit_cp;

extern  int           host_limit_cg;
extern  int           cycle_diff_cg;
extern  int           crit_g_limit_cg;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

bool leiter_faehiger_exp_in_neuem_team ( void );

void spezialisten_einbauen ( void );

void FormNextTeam ( void );

#endif
