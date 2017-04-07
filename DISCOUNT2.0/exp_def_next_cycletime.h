/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_def_next_cycletime                                    */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die bei der Bestimmung der naechsten Zyklusdauer  */
/*                 benoetigt werden.                                         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#ifndef  __EXP_DEF_NEXT_CYCLETIME
#define  __EXP_DEF_NEXT_CYCLETIME


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/
/* Die folgenden Konstanten werden bei der Festlegung der naechsten Zyklus-  */
/* dauer benoetigt. Beschreibung siehe bei den Funktionen !                  */
/* verwendet in DefineNextCycleTime   */
#define TIMEBASE                20
#define TP_PART                200
#define TIME_EXTRA               1
#define GOOD_TEAM_T_FAC          1
#define BAD_TEAM_T_FAC           1

/* verwendet in gut_beurteiltes_team  */
#define UPPER_LIMIT1          4000
#define UPPER_LIMIT2           200

/* verwendet in schlecht_beurteiltes_team */
#define DOWN_LIMIT1           2000
#define DOWN_LIMIT2            100

#define GOALFAKTOR               1.8

/*****************************************************************************/
/*                                                                           */
/*                            Typdefinitionenen                              */
/*                                                                           */
/*****************************************************************************/
typedef enum { SECONDS, MILLISECONDS } timeunit_type;

/*****************************************************************************/
/*                                                                           */
/*                           Globale Variablen                               */
/*                                                                           */
/*****************************************************************************/
extern  int             timebase;
extern  int             tp_part;
extern  double          time_extra;
extern  double          good_team_t_fac;
extern  double          bad_team_t_fac;

extern  int             upper_limit1;
extern  int             down_limit1;

extern  int             upper_limit2;
extern  int             down_limit2;

extern  double          goalfaktor;

extern  bool            timebase_angegeben;
extern  short           timemode;
extern  timeunit_type   timeunit;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void ClassifyTeamExperts ( void );

long DefineNextCycleTime ( void );

#endif
