/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          tc
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Hauptprogramm
//                      Vervollstaendigung ohne Abbruch
//-----------------------------------------------------------------------------
//      $Log: tc.c,v $
//
//      Neu: Maximale Beispielanzahl und maximales Delta
//
//      Neue Optionen: Zeitbeschraenkung, Basis-Gewicht fuer
//      unbekannte CPs beim lernen.
//      
//      Wichtige Aenderung: Der Makro "REPRO" exitiert nicht mehr. Er
//      ist durch die globale Varibale ReproMode in complet.[ch]
//      ersetzt worden. 
//
//      Revision 1.0  1993/03/22  21:10:12  lind
//      Broadcast-Port kommt jetzt als Commandline-Parameter, ebenso der
//      Accept-Port des initialen Teamleiters
//
//      Revision 0.9  1993/03/12  15:12:15  lind
//      Aufruf von StartXpert( ... ) angepasst (zweiter Parameter
//      entfaellt
//
//      Revision 0.8  1992/03/25  13:21:36  pitz
//      weight_flag-Abfrage korrigiert.
//
//      Revision 0.7  1992/03/25  13:12:26  pitz
//      -w ist jetzt negierte Option !
//
//      Revision 0.6  1992/02/28  16:54:43  pitz
//      Ausgabedatei im Silentmode nun mit freopen statt mit fopen und bcopy.
//
//      Revision 0.5  1992/02/24  12:16:37  pitz
//      Option -C -> Konfigurationsdatei
//
//      Revision 0.4  1991/09/06  09:07:30  pitz
//      Fehlermeldung erweitert. (Angabe der Optionen !)
//
//      Revision 0.3  1991/09/06  08:52:54  pitz
//      Option -S %d: SPECIAL-FACTOR implementiert
//
//      Revision 0.2  1991/08/19  09:50:29  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.1  1991/08/14  13:19:27  pitz
//      -P Teamprotokoll eingefuehrt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include <sys/time.h>
#include <sys/resource.h>

#include    "vartree.h"
#include    "order.h"
#include    "cpweight.h"
#include    "reduce.h"
#include    "buildcp.h"
#include    "complet.h"
#include    "scanner.h"
#include    "parser.h"
#include    "subst.h"
#include    "pcl.h"
#include    "domain.h"
#include    "database.h"
#include    "expert.h"
#include    "parseexp.h"
#include    "team.h"
#include    "br_types.h"
#include    "comphase.h"


/*
//-----------------------------------------------------------------------------
//      Lokale Prototypen
//-----------------------------------------------------------------------------
*/

static void CommandLine   ( int argc, char *argv[] );
static void CommandError  ( void );

#ifdef PCL
/* Diese Funktion dient zur Interpretation der Commandline-Optionen StS */
ExtractType FindExtOpt(char* value);
#endif


/*
//-----------------------------------------------------------------------------
//      Globale Daten
//-----------------------------------------------------------------------------
*/

extern tReceiver br_Receivers[];
extern int       br_BroadcastPort;


/*
//-----------------------------------------------------------------------------
//      Lokale Daten
//-----------------------------------------------------------------------------
*/


static char     *filename   = NULL;
       char     *Outfile    = NULL;
static char     *cfg        = NULL;
static bool     pretty      = false;
static bool     checkonly   = false;
static bool     teamflag    = false;

typedef enum { TEAM_OPT, 
               CHECK_OPT,
               FUNC_OPT,
               VAR_OPT,
               MEM_OPT,
               FILE_OPT,
               SILENT_OPT,
               NULL_OPT,
               EXPERT_OPT,
               PRINT_OPT,
               SPECIAL_OPT,
               WEIGHT_OPT,
               CFG_OPT,
               DEMO_OPT,
               LOG_OPT,
#ifdef PCL
               EXT_OPT,
               FEXT_OPT,
	       ASYNC_OPT,
#endif
	       KNOW_OPT,
	       GD_OPT,
	       TOTW_OPT,
               AVEW_OPT,
               PRFW_OPT,
               GDW_OPT,
	       TOTL_OPT,
	       PRFL_OPT,
	       GDL_OPT,
	       CP_COST_OPT,
	       NEG_L_OPT,
	       P_L_N_OPT,
	       TSM_ABS_OPT,
	       UNK_OPT,
               UNKB_OPT,
	       LEARN_OPT,
	       E_METHOD_OPT,
               PRK_OPT,
               KAPUR_OPT,

               PLAN_OPT,      /* Dokumentation der Planung des Leiters */

               CG_EXP_OPT,    /* CG-Experte wenn sequentiell */          
               CG_FUNC_OPT,   /* Funktionsgewicht f"ur CG-Experte */
               CG_VAR_OPT,    /* Variablengewicht f"ur CG-Experte */
               CPUTIME_OPT,   /* Limit CPU-Time (only for sequential */
			      /* system) */
	       CPS_LIMIT_OPT,

               UNKNOWN_OPT,

               MAXEXAS_OPT,   /* Fuer die Beispielauswahl */
               MAXDELTA_OPT,
               NOEXASEL_OPT,
               WNA_OPT,
               WAD_OPT,
               WDD_OPT,
               WGD_OPT,
               WAF_OPT,
               WTSM_OPT } Option;

typedef struct { char   shortcut;
                 char   option[20];
                 Option code;        } CommandOption;

CommandOption   OptionList[] = { 
   { 'c',  "check",            CHECK_OPT   },
   { 'f',  "function-weight",  FUNC_OPT    },
   { 'm',  "memory",           MEM_OPT     },
   { 'p',  "print",            PRINT_OPT   },
   { 's',  "silent",           SILENT_OPT  },
   { 'f',  "file",             FILE_OPT    },
   { 'n',  "null",             NULL_OPT    },
   { 't',  "team",             TEAM_OPT    },
   { 'v',  "variable-weight",  VAR_OPT     },
   { 'w',  "weight",           WEIGHT_OPT  },
   { 'x',  "expert",           EXPERT_OPT  },
   { 'S',  "special",          SPECIAL_OPT },
   { 'C',  "config",           CFG_OPT     },
   { 'd',  "demo",             DEMO_OPT    },
   { 'l',  "no_log",           LOG_OPT     },
#ifdef PCL
   { 'X',  "extract",          EXT_OPT     }, 
   { 'F',  "fextract",         FEXT_OPT    }, 
   { 'a',  "async",            ASYNC_OPT   },
#endif
   { 'K',  "knowledge_base",   KNOW_OPT    },
   { 'D',  "goal_dist_strat",  GD_OPT      },
   { 'T',  "total_weight",     TOTW_OPT    },
   { 'A',  "average_weight",   AVEW_OPT    },
   { 'R',  "proofs_weight",    PRFW_OPT    },
   { 'G',  "goal_dist_weight", GDW_OPT     },
   { '\n', "total_limit"     , TOTL_OPT    },
   { '\n', "proofs_limit"    , PRFL_OPT    },
   { '\n', "gd_limit" ,        GDL_OPT     },
   { '\n', "cp_cost_limit",    CP_COST_OPT },
   { '\n', "neg_limit",        NEG_L_OPT   },
   { '\n', "pos_limits_neg",   P_L_N_OPT   },
   { '\n', "tsm_absolutes",    TSM_ABS_OPT },
   { 'U',  "unknown_weight",   UNK_OPT     },
   { 'B',  "unknown_base",     UNKB_OPT    },
   { 'I',  "learn_influence",  LEARN_OPT   },
   { 'M',  "eval_found_method",E_METHOD_OPT},
   { 'L',  "log_file",         PRK_OPT     },
   { 'k',  "kapur",            KAPUR_OPT   },
   { 'P',  "plan",             PLAN_OPT    },
   { 'g',  "g-expert",         CG_EXP_OPT  },
   { 'E',  "CG-func-weight",   CG_FUNC_OPT },
   { 'V',  "CG-var-weight",    CG_VAR_OPT  },
   { '\n', "cputime-limit",    CPUTIME_OPT },
   { '\n', "cps_limit",        CPS_LIMIT_OPT},
   { '\n', "max_examples",     MAXEXAS_OPT },
   { '\n', "max_delta",        MAXDELTA_OPT},
   { 'N',  "no_selection",     NOEXASEL_OPT},
   { '\n', "w_NA",             WNA_OPT     },
   { '\n', "w_AD",             WAD_OPT     },
   { '\n', "w_DD",             WDD_OPT     },
   { '\n', "w_GD",             WGD_OPT     },
   { '\n', "w_AF",             WAF_OPT     },
   { '\n', "w_TSM",            WTSM_OPT    },
   { '\0', "",                 UNKNOWN_OPT }
};


/*
//-----------------------------------------------------------------------------
//  Funktion:       CommandLine ( int argc, char **argv, char *filename )
//
//  Parameter:      argc        Anzahl der Argumente
//                  argv        Argumente
//
//
//  Beschreibung:   Die Argumentliste wird bearbeitet.
//-----------------------------------------------------------------------------
*/

static void CommandLine ( int argc, char *argv[] )
{
    int     i = 1;
    int     j = 0;
    char    path[MAXPATHLEN];
    Option  opt;
    bool    fflag = false;
    long    fweight = 2;
    bool    vflag = false;
    long    vweight = 1;
    bool    xflag = false;
    bool    sflag = false;
    bool    cflag = false;

    /* Parameter fuer die Lernenden Experten */
    DistanceStrategy gd_strat = aggressive;
    long    tot_weight = 1,
            ave_weight = 0,
            proofs_weight = 0,
            goal_dist_weight = 0,
            unknown_weight = 3, 
            learn_infl = 3,
            unk_b = UNK_BASE_DEFAULT,
            eval_method = DEFAULT_EVAL_FCT;

    /* Zus"atzliche Parameter f"ur das Lernen mit Flieskomma-Gewichten */
      
    double  proof_l   = 0.2,
            total_l   = 0.2, 
            goal_l    = 0.2,
            cp_cost_l = 0.2,
            neg_l     = 0.2;

    bool    pos_l_neg = false,
            tsm_abs = false;

    /* flags f"ur die CG-Funktion */
    bool    cg_xflag = false;
    bool    cg_fflag = false;
    long    cg_fweight = 2;
    bool    cg_vflag = false;
    long    cg_vweight = 1;

    /* Default-Parameter f"ur die Beispielauswahl */

    no_exasel   = FALSE;
    maxexamples = 10;
    maxdelta    = 0.5;
    weightNA    = 0;
    weightAD    = 0;
    weightDD    = 0;
    weightGD    = 0;
    weightAF    = 0;
    weightTSM   = 1;

    getcwd( path, MAXPATHLEN );
    /* Name der auszuf"uhrenden Dateien = Name dieses Programmes */
    strncpy(Execfile,argv[0],MAXPATHLEN);

    while (i < argc)
    {
        if ((strlen (argv[i]) > 1) && (argv[i][0] == '-'))
        {
            opt = UNKNOWN_OPT;
            if (strlen (argv[i]) == 2)
            {
                for (j = 0; OptionList[j].shortcut; j++)
                    if (argv[i][1] == OptionList[j].shortcut)
                        opt = OptionList[j].code;
            }
            else
            {
                for (j = 0; OptionList[j].shortcut; j++)
                    if (!strcmp (&(argv[i][1]), OptionList[j].option))
                        opt = OptionList[j].code;
            }
            i++;

            switch (opt)
            {
            case TEAM_OPT:   if (teamflag)
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Team-Option mehrfach gesetzt." );
                             teamflag = true;
                             break;

            case CHECK_OPT:  if (checkonly)
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Check-Option mehrfach gesetzt." );
                             checkonly = true;
                             break;

            case MEM_OPT:    if (MemStatistic)
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Speicher-Option mehrfach gesetzt." );
                             MemStatistic = true;
                             break;

            case FUNC_OPT:   if (fflag)
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Funktionsgewicht mehrfach gesetzt." );
                             if (!IsNumber (argv[i]))
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Zahl nach Option -f erwartet." );
                             fweight = (long)(atoi (argv[i++]));
                             break;

            case VAR_OPT:    if (vflag)
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Variablengewicht mehrfach gesetzt." );
                             if (!IsNumber (argv[i]))
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Zahl nach Option -v erwartet." );
                             vweight = (long)(atoi (argv[i++]));
                             break;


            case CG_EXP_OPT: if (cg_xflag)
			     {
			       Error ( __FILE__ ": "  "CommandLine",
				       "Mehrere krit. Ziel Experten definiert." );
			     }
                             cg_xflag = true;

                             j = 0;
                             while ((XpertTab[j].xpert) && 
				    (strcmp ( argv[i], XpertTab[j].cmd)))
			     {
                               j++;
			     }

                             if (!(XpertTab[j].xpert))
                             {
                                 CommandError ();
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "Unbekannter Experte." );
                             }

                             SetCGFunc ( XpertTab[j].xpert );

                             i++;
                             break;

            case CG_FUNC_OPT: if (cg_fflag)
			      {
                                Error ( __FILE__ ": "  "CommandLine",
                                        "Funktionsgewicht fuer CG-Funktion mehrfach gesetzt." );
			      }
			      cg_fflag = true;

			      if (!IsNumber (argv[i]))
			      {
			        Error ( __FILE__ ": "  "CommandLine",
				        "Zahl nach Option -E erwartet." );
			      }
		              cg_fweight = (long)(atoi (argv[i++]));

			      break;

            case CG_VAR_OPT: if (cg_vflag)
			     {
			       Error ( __FILE__ ": "  "CommandLine",
				       "Variablengewicht fuer CG-Funktion mehrfach gesetzt." );
			     }
                             cg_vflag = true;

                             if (!IsNumber (argv[i]))
			     {
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "Zahl nach Option -V erwartet." );
			     }
                             cg_vweight = (long)(atoi (argv[i++]));

                             break;
	    case CPUTIME_OPT:if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-cputime-limit erwartet Argument");
	                     {
                                struct rlimit rlp;

                                getrlimit(RLIMIT_CPU,&rlp);
				
				rlp.rlim_cur = atoi(argv[i++]);
				if(!setrlimit(RLIMIT_CPU,&rlp))
				{		   
				   printf("cputime limited to %ld\n",rlp.rlim_cur);
				}
				else
				{
				   perror(NULL);
				}
				getrlimit(RLIMIT_CPU,&rlp);
				printf("cputime limit returned as %ld\n",rlp.rlim_cur);
			     }
	                     break;
	    case CPS_LIMIT_OPT:
	                     if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-cps-limit erwartet Argument");
			     CPsLimit = atol(argv[i++]);
			     break;
            case SPECIAL_OPT:if (sflag)
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "SPECIAL_FACTOR mehrfach gesetzt." );
                             if (!IsNumber (argv[i]))
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Zahl nach Option -S erwartet." );
                             special_factor = (long)(atoi (argv[i++]));
                             break;

            case WEIGHT_OPT: if (!weight_flag)
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "-weight mehrfach gesetzt." );
                             weight_flag = false;
                             break;

            case SILENT_OPT: if (SilentMode)
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "-silent mehrfach gesetzt." );
                             SilentMode = true;
                             break;

            case FILE_OPT:   if (Outfile)
                                  Error ( __FILE__ ": "  "CommandLine",
                                          "-file mehrfach gesetzt." );
                             j = 0;
                             Outfile = argv[i++];
                             break;

            case NULL_OPT:   if (NullMode)
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "-null mehrfach gesetzt." );
                             NullMode = true;
                             break;

            case EXPERT_OPT: if (xflag)
                                  Error ( __FILE__ ": "  "CommandLine", 
                                          "Mehrere Experten definiert." );
                             xflag = true;
                             j = 0;
                             while ((XpertTab[j].xpert) &&
                                    (strcmp ( argv[i], XpertTab[j].cmd)))
                                 j++;
                             if (!(XpertTab[j].xpert))
                             {
                                 CommandError ();
                                 Error ( __FILE__ ": "  "CommandLine", 
                                         "Unbekannter Experte." );
                             }
                             i++;
                             SetCPWeight ( XpertTab[j].xpert );
                             if (XpertTab[j].xpert==occnest &&
                                 i < argc &&
                                 (!strcmp(argv[i],"0") || !strcmp(argv[i],"1"))
                                )
                             {
                               considerAll = (strcmp(argv[i],"0")) ?
                                                true : false;
                               i++;
                             }
                             break;

            case CFG_OPT:    if (cflag)
                                  Error ( __FILE__ ": "  "CommandLine", 
                                          "Mehrere Konfigurationen definiert." );
                             cflag = true;
                             j = 0;
                             cfg = argv[i++];
                             break;

            case DEMO_OPT:   if (DemoMode)
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "-demo mehrfach gesetzt." );
                             DemoMode = true;
                             break;

            case LOG_OPT:    if (!ProtocolMode)
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "-no_log mehrfach gesetzt."
                                        );
                             ProtocolMode = false;
                             break;
#ifdef PCL
            case EXT_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-extract erwartet Argument");
                             extract = FindExtOpt(argv[i++]);
                             break;
            case FEXT_OPT:   if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-fextract erwartet Argument");
                             fextract = FindExtOpt(argv[i++]);
                             break;
            case ASYNC_OPT:  if(f_async)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-async mehrfach gesetzt.");
                             f_async = true;
                             break;
#endif
	    case KNOW_OPT:   if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-knowledge_base erwartet Argument");
	                     if (KnowledgeBase[0])
                                 Error ( __FILE__ ": "  "CommandLine",
					"-knowledge_base mehrfach gesetzt." );
	                     strcpy(KnowledgeBase,argv[i++]);
	                     break;
	    case GD_OPT:     if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-goal_dist_strat erwartet Argument");
	                     gd_strat = atoi (argv[i++]);
	                     break;
	    case TOTW_OPT:   if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-total_weight erwartet Argument");
	                     tot_weight = atoi (argv[i++]);
	                     break;
	    case AVEW_OPT:   if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-average_weight erwartet Argument");
	                     ave_weight = atoi (argv[i++]);
	                     break;
	    case PRFW_OPT:   if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-proofs_weight erwartet Argument");
	                     proofs_weight = atoi (argv[i++]);
	                     break;
	    case GDW_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-goal_dist_weight erwartet Argument");
	                     goal_dist_weight = atoi (argv[i++]);
	                     break;
	    case TOTL_OPT:   if(i>=argc)
               	                Error ( __FILE__ ": "  "CommandLine",
                                       "-total_limit erwartet Argument");
	                     total_l = atof (argv[i++]);
	                     break;
	    case PRFL_OPT:   if(i>=argc)
               	                Error ( __FILE__ ": "  "CommandLine",
                                       "-proofs_limit erwartet Argument");
	                     proof_l = atof (argv[i++]);
	                     break;
	    case GDL_OPT:    if(i>=argc)
               	                Error ( __FILE__ ": "  "CommandLine",
                                       "-gd_limit erwartet Argument");
	                     goal_l = atof (argv[i++]);
	                     break;
	    case CP_COST_OPT:if(i>=argc)
	                        Error ( __FILE__ ": "  "CommandLine",
					"-cp_cost_limit erwartet Argument");
	                     cp_cost_l = atof (argv[i++]);
			     break;
	    case NEG_L_OPT:  if(i>=argc)
               	                Error ( __FILE__ ": "  "CommandLine",
                                       "-neg_limit erwartet Argument");
	                     neg_l = atof (argv[i++]);
	                     break;
	    case P_L_N_OPT:  pos_l_neg = true;
		             break;
	    case TSM_ABS_OPT:tsm_abs = true;
	                     break;
	    case UNK_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-unknown_weight erwartet Argument");
	                     unknown_weight = atoi (argv[i++]);
	                     break;
	    case LEARN_OPT:  if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-learn_influence erwartet Argument");
	                     learn_infl = atoi (argv[i++]);
	                     break;
	    case UNKB_OPT:   if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-unknown_base erwartet Argument");
	                     unk_b = atoi (argv[i++]);
	                     break;
	    case E_METHOD_OPT:
		             if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-eval_found_method erwartet Argument");
	                     eval_method = atoi (argv[i++]);
	                     break;
            case WTSM_OPT:   if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-w_TSM erwartet Argument");
                             weightTSM = atof (argv[i++]);
                             break;
            case WNA_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-w_NA erwartet Argument");
                             weightNA = atof (argv[i++]);
                             break;
            case WAD_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-w_AD erwartet Argument");
                             weightAD = atof (argv[i++]);
                             break;
            case WDD_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-w_DD erwartet Argument");
                             weightDD = atof (argv[i++]);
                             break;
            case WGD_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-w_GD erwartet Argument");
                             weightGD = atof (argv[i++]);
                             break;
            case WAF_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-w_AF erwartet Argument");
                             weightAF = atof (argv[i++]);
                             break;
            case MAXEXAS_OPT:if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-max_examples erwartet Argument");
                             maxexamples = atoi (argv[i++]);
                             break;
            case MAXDELTA_OPT:
                             if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-max_delta erwartet Argument");
                             maxdelta = atof (argv[i++]);
                             break;
            case NOEXASEL_OPT:  if(no_exasel)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-no_selection mehrfach gesetzt.");
                             no_exasel = true;
                             break;
            case PRK_OPT:    if(i>=argc)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "-log_file erwartet Argument");
                             if (log_base)
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "-log_file mehrfach gesetzt." );
                             if(!ProtocolMode)
                                Error ( __FILE__ ": "  "CommandLine",
                                       "Protokollmodus nicht aktiv bei -log_file");
                             log_base = argv[i++];
                             break;

            case KAPUR_OPT:  if (KapurCriteria)
                                 Error ( __FILE__ ": "  "CommandLine",
                                         "-kapur mehrfach gesetzt." );
                             KapurCriteria = true;
                             break;

            case PRINT_OPT:  pretty = true;
                             break;
	    case PLAN_OPT :  if ( PlanDocum )
                                  Error ( __FILE__ ": "  "CommandLine",
                                          "-plan mehrfach gesetzt." );
                             PlanDocum = true;
                             break;

            default:         Error ( __FILE__ ": "  "CommandLine", 
                                     "Unbekannte Option" );
                             break;
            }

        }
        else
        {
            if (filename)
                Error ( __FILE__ ": "  "CommandLine", "Nur eine Datei kann bearbeitet werden." );
            filename = argv[i];
            i++;
        }
    }

    SetWeights ( fweight, vweight );
    SetLearnEvalParams (gd_strat, tot_weight, ave_weight, proofs_weight,
			goal_dist_weight, true, true, unknown_weight,
			learn_infl, unk_b, eval_method);
    SetFLearnParams(gd_strat, proof_l, total_l, goal_l, cp_cost_l,
		    neg_l,pos_l_neg, tsm_abs);
    if (!filename)
    {
        CommandError ();
        Error ( __FILE__ ": "  "CommandLine", "Dateiname fehlt." );
    }
    if(!teamflag)
       ProtocolMode = false; 

    if(ReproMode) /* Don't write .prk of Repro, of course   StS */
    {
       ProtocolMode = false; 
    }

#ifdef PCL /* Initialize extract and fextract... StS */
    if((extract==NOT_SELECTED) && (fextract == NOT_SELECTED))
    {
       extract = fextract = MEXTRACT;
    }
    else if(extract==NOT_SELECTED)
    {
       extract = fextract;
    }
    else if(fextract==NOT_SELECTED)
    {
       fextract = extract;
    }
#endif

/*
    if (!Outfile)
        Outfile = filename;
*/
}


#ifdef PCL

typedef struct
{ 
   char*       option;
   ExtractType code;        
}ExtOption;

ExtOption ExtArray[] =
{
   {"none"   ,  NO_EXTRACT },
   {"mextract", MEXTRACT },
   {"revert",   REV_REXTRACT },
   {"tac",      TAC_REXTRACT },
   {NULL,       NOT_SELECTED}
};

/*-----------------------------------------------------------------------------
                                                                           
Funktion        : ExtractType FindExtOpt(char* value)

Autor           : StS

Beschreibung    : Die Funktion nimmt einen String und testet, ob es
sich um eine legale Beschreibung einer Extractionsmethode, d.h. einen
String aus der Liste none, mextract, revert, tac handelt. Ist das der
Fall, so wird der zugeordnete Wert zur"uckgegeben, sonst eine
Fehlermeldung erzeugt.

Globale Variable: ExtArray

Seiteneffekte   : -

-----------------------------------------------------------------------------*/

ExtractType FindExtOpt(char* value)
{
   long        j;
   ExtractType ret=NOT_SELECTED;

   for (j = 0; ExtArray[j].code != NOT_SELECTED; j++)
   {
      if (!strcmp (value, ExtArray[j].option))
      {
         ret = ExtArray[j].code;      
      }
   }
   if(ret == NOT_SELECTED)
   {
      Error ( __FILE__ ": "  "FindExtOpt",
             "Falsches Argument zu -extract oder -fextract" );  
   }
   return ret;
}
   
#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       CommandError
//
//  Parameter:      -keine-
//
//  Beschreibung:   Kurzbeschreibung der Kommandozeilenoptionen
//-----------------------------------------------------------------------------
*/

static void CommandError ( void )
{
    printf ( "TEAM-COMPLETION\n" 
             "===============\n\n" );
    printf ( "Ein Programm von Werner Pitz\n" 
             "Einzelversion: " __DATE__ "mit Broadcast und Protokoll\n\n"
             "Aufruf: discount [Optionen] <Dateiname>\n\n");
    printf ( "Diese Programm verhaelt sich je nach Aufruf in der\n"
             "Komandozeile verschieden. Beginnt der Name mit 'r' oder\n"
             "'R', so geht das System von einem Reproduktionslauf aus!\n\n");

    printf ( "Optionen:\n" 
             "---------\n\n" 

             "-c -check                    Syntaxpruefung der Aufgabenstellung.\n" 
             "-f -function-weight <Zahl>   Gewicht von Funktionen bei CP-Funktion.\n" 
             "-v -variable-weight <Zahl>   Gewicht von Variablen bei CP-Funktion.\n" 
             "-k -kapur                    Verwende Kapur-Kriterium\n" 
             "                             (nur Sequentiell)\n" 
             "-S -special <Zahl>           Unterstuetzung des Set Of Supports in %%.\n" 
             "-w -weight                   initiale Gleichungen werden nicht bewertet.\n" 
             "-d -demo                     Demonstrationsmodus.\n" 
             "-m -memory                   Speicherstatistik ausgeben.\n" 
             "-s -silent                   Ausgabe in Datei.\n" 
             "                             (mehrere Dateien bei Teamwork)\n" 
             "-f -file <name>              Name der Ausgabedatei festlegen.\n"
             "                             (nur in Kombination mit -silent)\n"
             "-n -null                     keine Ausgabe.\n" 
             "                             (genau Ausgabe nach /dev/null)\n" 
             "-p -print                    kompaktes Format ohne Kommentare erzeugen.\n"
             "-t -team                     Teamwork.\n" 
             "-C -config                   Konfigurationsdatei.\n" 
	     "-P -plan                     Planung dokumentieren \n"
             "-l -no_log                   Kein Protokoll anlegen.\n"
	     "-L -log_file <name>          Angabe eines alternativen Namens fuer das\n"
             "                             Kurzprotokoll. Entfaellt diese Option, so\n"
             "                             wird der Name der Problemdatei mit dem\n"
             "                             Suffix .prk als Name der Protokolldatei\n"
             "                             gewaehlt.\n"
	     "   -cputime-limit            Setzt maximale CPU-Zeit (in Sekunden) bis zum\n"
             "                             Abbruch des Prozesses (unter Solaris 2.X nur\n"
             "                             bei gutem Wetter einsetzbar...)\n"
#ifdef PCL
	     "-a -async                    Ist diese Option gesetzt, so wird die\n"
             "                             Extraktion am Ende des Beweislaufes\n"
             "                             asynchron gestartet, das Beweissystem kann\n"
             "                             noch waehrend der Extraktion terminieren.\n"
             "                             Sinnvoll fuer grosse Beispiele.\n"
             "-X -extract <arg>            Extraktionsmethode fuer Extraktion waehrend\n"
             "                             des Laufes festlegen.\n"
             "-F -fextract <arg>           Extraktionsmethode fuer Extraktion nach\n"
             "                             dem Laufes festlegen.\n"
             "                             <arg> kann folgende Werte annehmen:\n"
             "                                none     - Keine Extraktion\n"
             "                                mextract - Extraktion mit mextract\n"
             "                                revert   - Extraktion mit revert und\n"
             "                                           rextract\n"
             "                                tac      - Extraktion mit tac und\n"
             "                                           rextract\n"
             "                             Default fuer die letzten beiden Optionen\n"
             "                             ist mextract, wird nur eine von beiden\n"
             "                             verwendet, setzt sie beide Methoden auf\n"
             "                             den angegebenen Wert.\n"
#endif
	     "\n       ============ Lernende Experten ==============\n\n"
	     "-K -knowledge_base <arg>     Wissensbasis angeben\n"
             "-D -goal_dist_strat <0..3>   Strategie fuer die Bewertung der\n"
             "                             Entfernung zum Ziel.\n"
             "-T -total_weight <arg>       Gewichtung der absoluten Anzahl aller\n"
             "                             Referenzen in allen gelernten Beweisen.\n" 
             "-A -average_weight <arg>     Gewichtung der durchschnittlichen\n"
             "                             Referenzzahl.\n"
             "-R -proofs_weight <arg>      Gewichtung der Anzahl der Beweise.\n"
             "-G -goal_dist_weight         Gewichtung des Zielabstandes.\n"
             "-U -unknown_weight <arg>     Straffaktor fuer vom TET nicht erfasste\n"
             "                             Teilterme.\n"
	     "-B -unknown_base <arg>       Strafsummand fuer unbekannte\n"
	     "                             Gleichungen.\n"
	     "-M -eval_found_method <arg>  Auswahl der Bewertungsfunktion fuer\n"
	     "                             Pattern-basierte Verfahren mit\n"
	     "   	                   Linearkombinationsbewertung (0/1).\n"
	     "   -total_limit <arg>        Gewichtskorrektur fuer Referenzanzahl ist\n"
             "                             maximal <arg>.\n"
	     "   -proofs_limit <arg>       Ditto fuer Anzahl der Beweise.\n"
	     "   -gd_limit <arg>           Ditto fuer Zielabstand.\n"
	     "   -cp_cost_limit            Halbe Intervallgrenze des\n"
             "                             Multiplikators, der aus den CP-Kosten\n"
	     "                             berechnet wird.\n"
             "   -neg_limit                Ditto fuer Negative Beispiele\n"
	     "   -pos_limits_neg           Falls gesetzt, kann der Faktor fuer\n"
	     "                             auch positive TSM-Zellen hoechstens\n"
	     "                             1 werden.\n"
	     "   -tsm_absolutes            Use absolute maximums to compute the\n"
             "                             node evaluations for TSM nodes.\n"
             "-I learn_influence <arg>     Gewichtung des gelernten Wissens\n"
	     "                             gegenueber der Standard-Strategie.\n"
             "\n");
    printf ( "-x -expert <CP-Funktion>     Auswahl einer CP-Funktion.\n\n" 
	     "-g -g-expert <CG-Funktion>   Auswahl einer CG-Funktion.\n\n"
             "   -max_examples <arg>       Maximal Anzahl der Beispiele, Default: 10\n"
             "   -max_delta <arg>          Schwellwert fuer die Abstandsfunktion, Default: 0.5\n"
             "   -feature                  DeltaFTR anstatt DeltaTSM, Default aus\n"
             "   -w_NA <arg>, -w_AD <arg>, Gewichte fuer DeltaFeature, Default 1\n"
             "   -w_DD <arg>, -w_GD <arg>\n"
             "   -w_AF <arg>\n\n"          
             "erlaubte CP- und CG-Funktionen\n" 
             "------------------------------\n\n" 
             "add       -> ADD_WEIGHT (Default-Wert fuer CP-Funktion)\n" 
	     "fifo      -> First-In, First-Out\n"
             "max       -> MAX_WEIGHT\n" 
             "gt        -> GT_WEIGHT\n" 
	     "occnest   -> OCCNEST\n"
	     "max_kbo   -> MAX_KBO\n"
	     "add_kbo   -> ADD_KBO\n"
	     "gt_kbo    -> GT_KBO\n"
	     "gt_fcount -> GT_FCOUNT\n"
	     "sum_fcount-> SUM_FCOUNT\n"
             "diff      -> DIFF_WEIGHT\n" 
             "goalmatch -> GOALMATCH\n" 
	     "goalsim   -> goalsim\n"
	     "goaltest  -> goaltest\n"
	     "unif_goal -> unif_goal (Default-Wert fuer CG-Funktion)\n"
	     "diff_goal -> diff_goal\n\n"
     "Lernende Funktionen (nur als CP-Funktionen sinnvoll)\n"
	     "----------------------------------------------------\n"
	     "c_global_learn, p_global_learn\n"
	     "  Verwende eine Linearkombination verschiedener Werte in\n"
             "  der ungefilterten Wissensbasis.\n"
	     "c_goal_bound, p_goal_bound\n"
	     "  Ditto, aber Filterung der Wissensbasis nach Zielen.\n"
	     "c_spec_bound, p_spec_bound\n"
	     "  Ditto, Filterung nach Spezifikation.\n"
	     "cgflearn\n"
	     "  Wie c_global_learn, aber relative Gewichtung der\n"
	     "  Features der Wissensbasis.\n"
             "c_1tet_learn, c_2tet_learn\n"
	     "  TETs und TATs mit Funktionen auf den absoluten\n"
	     "  Features der Termzellen.\n"
	     "c_3tet_learn\n"
	     "  Ditto, aber wieder mit relativer Gewichtung\n"
	     "tsmstandard\n"
	     "  Verwende Term Space Maps\n\n");
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       main ( int argc, char **argv )
//
//  Parameter:      argc    Anzahl der Argumente
//                  argv    Argumente
//
//  Beschreibung:   Hauptprogramm
//-----------------------------------------------------------------------------
*/

short   main ( int argc, char *argv[] )
{
    char        example[200];
    char        outfile[200];
    short       i,j;

#ifdef BERLIN   /* Fuer Speichern des Hostnamens */
    char        *thishostname;
#endif

    InitInterruptHandler();
    InitKillHandler ();
    AllocReserve();

/*
getchar();
*/



#ifdef PCL   /* tmpdir is defined in pcl.c and used to set the */
	     /* environment for tac   - StS                    */
    tmpdir = (char*)getenv("TMPDIR"); /* Der cast sollte EIGENTLICH */
				      /* nicht notwendig */
				      /* sein...scheinbar ist aber */
				      /* <stdlib.h> nicht ganz */
				      /* sauber... */
#endif

    /* Check wether this is to be a reproduction run...in this case */
    /* the name should start with a "r" or "R"  */
    if((*argv[0] == 'r')||(*argv[0] == 'R'))
    {
       ReproMode = true;
    }
    
if ((argc == 6) && (!strcmp (argv[1], "-XPERT")))
    {
        MODUS = EXPERT;
        ParallelMode = true;
	br_Receivers[0].Port = atoi( argv[3] );
	br_BroadcastPort     = atoi( argv[4] );
        StartXpert ( argv[2], argv[5] );
        StartCompletion ();
        exit ( 0 );
    }

    CommandLine ( argc, argv );

    if (NullMode)
    {
        freopen ( "/dev/null", "w", stdout );
    }
    else if (SilentMode)
    {
        if (Outfile)
        {
            freopen ( Outfile, "w", stdout );
        }
        else
        {
            sprintf ( outfile, "%s.0", filename );
            freopen ( outfile, "w", stdout );
        }
    }

    printf ( "Version vom: " __DATE__ " mit Broadcast und Protoll.\n\n\n" );

    #ifdef MEMDEBUG
        printf ( "***********************************************\n" );
        printf ( "****  Fehlersuche bei Speicherverwaltung  **** \n" );
        printf ( "***********************************************\n\n\n" );
    #endif


    if (teamflag)
    {
        MODUS = MASTER;
        ParallelMode = true;
        StartTeam ( cfg, filename );
        StartCompletion ();
        exit (0);
    }

#ifdef BERLIN /* Hostnamen und Pid ausgeben - fuer AG Dahn in Berlin */
    printf("\n----------------------------------------------------------------\n");
    printf("Sequential configuration is up and running.\n\n");
    thishostname = (char*)SecureMalloc(200);
    gethostname(thishostname,200);
    printf("This process runs on host %s and has pid %ld.\n",
	    thishostname, getpid());
    free(thishostname);
    printf("----------------------------------------------------------------\n");
#endif

    Parse ( filename, example );

#ifdef PCL
    if(Paramodulation)
    {
       Error ( __FILE__ ": "  "main", 
	      "PCL fuer Paramod noch nicht unterstuetzt." );
    }
#endif
    if (pretty)
    {
        PrintFile ( example );
        return 0;
    }

    PCL_OPEN (filename,"w",0,0);
    PCL_INIT ( );

    if (checkonly)
    {
        printf ( "Datei konnte fehlerfrei gelesen werden.\n" );
        return 0;
    }

    /*************************************************************************/
    /* Es werden nun noch zwei Tabellen angelegt, die der Database-Experte   */
    /* benoetigt, wenn er die Substitutionen zweiter Ordnung bestimmt.       */
    /*************************************************************************/

    /* Fuer die Verteilte version steht dieser Code am Ende von */
    /* StartTeam() in team.c ! */

    for ( j=0; j<MAXARITY; j++ )
    {
       ArityLength[j] = 0;
    }
    
    for ( j=1; j<=FuncCount; j++ )
    {
       SubstMatrix[Function[j].arity][ArityLength[Function[j].arity]].fktnr = j;
       SubstMatrix[Function[j].arity][ArityLength[Function[j].arity]++].belegt = false;
    }
    
    Initialize ( true );

    Completion ();

    Completed  ( false );

    return 0;
}

