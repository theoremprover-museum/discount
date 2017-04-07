/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          scanner
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul erlaubt ein einfaches Einlesen von
//                      Dateien
//-----------------------------------------------------------------------------
//      $Log: scanner.c,v $
//      Revision 1.0   1993/07/26  15:23:58  kronburg
//      Einlesen mehrerer Dateien gleichzeitig ermoeglicht
//
//      Revision 0.13  1992/06/25  10:10:12  pitz
//      FEELGOOD->FEELWELL
//      (Maxfactor korrigiert)
//
//      Revision 0.12  1992/03/26  12:32:21  pitz
//      GOALSIM eingefuehrt.
//
//      Revision 0.11  1992/02/19  11:49:08  pitz
//      FEEL_GOOD -> FEELGOOD
//
//      Revision 0.10  1991/10/30  09:23:16  pitz
//      WKBO eingefuehrt.
//
//      Revision 0.9  1991/09/25  14:15:23  pitz
//      Neue Schluesselwoerter: ADD_RWEIGHT, MAX_RWEIGHT, GT_RWEIGHT
//                              ADD_EWEIGHT, MAX_EWEIGHT, GT_EWEIGHT
//
//      Revision 0.8  1991/09/20  10:07:42  pitz
//      Schluesselwort DATABASE.
//
//      Revision 0.7  1991/09/19  12:02:41  pitz
//      Symbol EXTENDED eingefuehrt.
//
//      Revision 0.6  1991/09/11  08:34:29  pitz
//      Neues SetScanMode und Symbol-Tabelle SymbolTabD.
//
//      Revision 0.5  1991/09/05  12:57:39  pitz
//      Neue Schluesselwoerter: SPEC_ADD, SPEC_MAX und SPEC_GT.
//
//      Revision 0.4  1991/08/26  15:26:32  pitz
//      SPECIAL als Schluesselwort erklaert.
//
//      Revision 0.3  1991/08/26  13:15:08  pitz
//      REDUCE_CPP als Scannersymbol eingefuehrt.
//
//      Revision 0.2  1991/08/23  09:55:16  pitz
//      Fehler behoben: Zeilenzaehler wird bei neuer Eingabedatei zurueckgesetzt.
//
//      Revision 0.1  1991/08/19  09:50:21  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <ctype.h>
#include    <stdio.h>
#include    <string.h>


#include    "scanner.h"


/*****************************************************************************/
/*                                                                           */
/*                          Modulinterne Konstanten                          */
/*                                                                           */
/*****************************************************************************/

#define FILEINITIAL { NULL, true, false, false, ' ', "", 0, 0, 0, NULL }
#define MAXFILES  10


/*****************************************************************************/
/*                                                                           */
/*                     Modulinterne Typedefinitionen                         */
/*                                                                           */
/*****************************************************************************/

typedef struct { char   symbol[IDENTLENGTH];
                 Symbol code;       } SymbolEntry;

/* Aufnahme aller Informationen ueber eine geoeffnete Datei */
typedef struct { FILE              *input;
                 bool               eoi;
                 bool               inputopen;
                 bool               uppercase;
                 char               ScanChar;
                 char               scantext[LENGTH];
                 short              scanline;
                 short              scanpos;
                 short              scanlength;
                 SymbolEntry       *SymbolTab;
               } file_des;



/*
//-----------------------------------------------------------------------------
//      Schluesselworte
//-----------------------------------------------------------------------------
*/

/* Symboltabelle fuer die Problembeschreibung */
static  SymbolEntry  SymbolTabX[] =   { { "MODE"        , SMODE         },
                                        { "COMPLETION"  , SCOMPLETION   },
                                        { "PROOF"       , SPROOF        },
                                        { "PARAMOD"     , SPARAMOD      },
                                        { "NAME"        , SNAME         },
                                        { "ORDERING"    , SORDERING     },
                                        { "KBO"         , SKBO          },
                                        { "RPO"         , SRPO          },
                                        { "LPO"         , SLPO          },
                                        { "XKBO"        , SXKBO         },
                                        { "WKBO"        , SWKBO         },
                                        { "EQUATIONS"   , SEQUATIONS    },
                                        { "SPECIAL"     , SSPECIAL      },
                                        { "CANCELLATION", SCANCELLATION },
                                        { "CONCLUSION"  , SCONCLUSION   },
                                        { ""            , 0             } };



/* Symboltabelle fuer Konfigurationsdatei */
static  SymbolEntry  SymbolTabC[] =
{ 
   { "HOSTS"           , SHOSTS           },
   { "DATA_DIR"        , SDATA_DIR        },
   { "EXPERT_DIR"      , SEXPERT_DIR      },
   { "REFEREE_DIR"     , SREFEREE_DIR     },
   { "PROTOCOL_DIR"    , SPROTOCOL_DIR    },
   { "DOMAIN_DIR"      , SDOMAIN_DIR      },
   { "KNOWLEDGE_DIR"   , SKNOWLEDGE_DIR   },
   { "STATIC"          , SSTATIC          },
   { "INITIAL"         , SINITIAL         },
   { "SELECT"          , SSELECT          },
   { "DOMAIN_SELECT"   , SDOMAIN_SELECT   },
   { "TIMEBASE"        , STIMEBASE        },
   { "CONSTANT"        , SCONSTANT        },
   { "LINEAR"          , SLINEAR          },
   { "EXPONENTIAL"     , SEXPONENTIAL     },
   { "SUPERVISOR_FILE" , SSUPERVISOR_FILE },
   { ""                , 0                }
};

/* Symboltabelle fuer die Spezifikationsdatei, die vom Database-Experten */
/* gelesen wird.                                                         */
static  SymbolEntry  SymbolTabD[] =   { { "FIND_ALL"    , SFIND_ALL     },
                                        { "FIND_FIRST"  , SFIND_FIRST   },
                                        { ""            , 0             } };

/* Die folgende Tabelle enthaelt alle Schluesselwoerter fuer die Definition */
/* eines Experten.                                                          */
static  SymbolEntry  SymbolTabExp_spec[] =  
{ 
   { "EXPERTENNAME"  , SEXPERTENNAME   },
   { "EXP_OR_SPEC"   , SEXP_OR_SPEC    },
   { "CP_FUNKTION"   , SCP_FUNKTION    },
   { "CG_FUNKTION"   , SCG_FUNKTION    },
   { "FAIRNESS"      , SFAIRNESS       },
   { "GUTE_GUTACHTER", SGUTE_GUTACHTER },
   { "GUTE_DOMAENEN" , SGUTE_DOMAENEN  },
   { "ROBUSTHEIT"    , SROBUSTHEIT     },
   { "WISSENSANTEIL" , SWISSENSANTEIL  },
   { "BEWEISPHASE"   , SBEWEISPHASE    },
   { "ZYKLUSDAUER"   , SZYKLUSDAUER    },
   { "ANFANG"        , SANFANG         },
   { "MITTE"         , SMITTE          },
   { "ENDE"          , SENDE           },
   { "AEHNLICHE_EXP" , SAEHNLICHE_EXP  },
   { "NACHFOLGER"    , SNACHFOLGER     },
   { "UNVERTRAEGL"   , SUNVERTRAEGL    },
   { "MOEGL_TEAMEXP" , SMOEGL_TEAMEXP  },
   { "PROZEDUR"      , SPROZEDUR       },
   { "BESCHREIBUNG"  , SBESCHREIBUNG   },
   /* Moegliche CP und CG Funktionen */
   { "ADD_WEIGHT"      , SX_ADD          },
   { "FIFO"            , SX_FIFO         },
   { "TEAM_FIFO"       , SX_TEAM_FIFO    },
   { "GLOBAL_LEARN"    , SX_GLOBAL_LEARN },
   { "GOAL_BOUND_LEARN", SX_GOAL_BOUND_LEARN},
   { "SPEC_BOUND_LEARN", SX_SPEC_BOUND_LEARN},
   { "1TET_LEARN"      , SX_1TET_LEARN,  },
   { "2TET_LEARN"      , SX_2TET_LEARN,  },
   { "MAX_WEIGHT"      , SX_MAX          },
   { "GT_WEIGHT"       , SX_GT           },
   { "DIFF_WEIGHT"     , SX_DIFF         },
   { "SPEC_ADD"        , SX_SPEC_ADD     },
   { "SPEC_MAX"        , SX_SPEC_MAX     },
   { "SPEC_GT"         , SX_SPEC_GT      },
   { "ADD_KBO"         , SX_KBO_ADD      },
   { "MAX_KBO"         , SX_KBO_MAX      },
   { "GT_KBO"          , SX_KBO_GT       },
   { "DATABASE"        , SX_DATABASE     },
   { "REDUCE_CP"       , SX_REDUCE_CP    },
   { "REDUCE_CPP"      , SX_REDUCE_CPP   },
   { "REDUCE_CG"       , SX_REDUCE_CG    },
   { "REDUCE_CGP"      , SX_REDUCE_CGP   },
   { "NONE"            , SX_NONE         },
   { "ADD_FWEIGHT"     , SX_ADD_FWEIGHT  },
   { "MAX_FWEIGHT"     , SX_MAX_FWEIGHT  },
   { "GT_FWEIGHT"      , SX_GT_FWEIGHT   },
   { "ADD_RWEIGHT"     , SX_ADD_RWEIGHT  },
   { "MAX_RWEIGHT"     , SX_MAX_RWEIGHT  },
   { "GT_RWEIGHT"      , SX_GT_RWEIGHT   },
   { "ADD_EWEIGHT"     , SX_ADD_EWEIGHT  },
   { "MAX_EWEIGHT"     , SX_MAX_EWEIGHT  },
   { "GT_EWEIGHT"      , SX_GT_EWEIGHT   },
   { "ADD_POLYNOM"     , SX_POLY_ADD     },
   { "MAX_POLYNOM"     , SX_POLY_MAX     },
   { "GT_POLYNOM"      , SX_POLY_GT      },
   { "GOALMATCH"       , SX_GOALMATCH    },
   { "GOALSIM"         , SX_GOALSIM      },
   { "GOALTEST"        , SX_GOALTEST     },
   { "UNIF_GOAL"       , SX_UNIF_GOAL    },
   { "DIFF_GOAL"       , SX_DIFF_GOAL    },
   { "OCCNEST"         , SX_OCCNEST      },
   { ""              , 0               }
};


/* Die folgende Tabelle enthaelt alle moglichen CP- und CG-Funktionen    */
static  SymbolEntry  SymbolTabExp[] = 
{ 
   { "ADD_WEIGHT"      , SX_ADD         },
   { "FIFO"            , SX_FIFO        },
   { "TEAM_FIFO"       , SX_TEAM_FIFO   },
   { "MAX_WEIGHT"      , SX_MAX         },
   { "GLOBAL_LEARN"    , SX_GLOBAL_LEARN},
   { "GOAL_BOUND_LEARN", SX_GOAL_BOUND_LEARN},
   { "SPEC_BOUND_LEARN", SX_SPEC_BOUND_LEARN},
   { "1TET_LEARN"      , SX_1TET_LEARN, },
   { "2TET_LEARN"      , SX_2TET_LEARN, },
   { "GT_WEIGHT"       , SX_GT          },
   { "OCCNEST"         , SX_OCCNEST     },
   { "DIFF_WEIGHT"     , SX_DIFF        },
   { "SPEC_ADD"        , SX_SPEC_ADD    },
   { "SPEC_MAX"        , SX_SPEC_MAX    },
   { "SPEC_GT"         , SX_SPEC_GT     },
   { "ADD_KBO"         , SX_KBO_ADD     },
   { "MAX_KBO"         , SX_KBO_MAX     },
   { "GT_KBO"          , SX_KBO_GT      },
   { "DATABASE"        , SX_DATABASE    },
   { "REDUCE_CP"       , SX_REDUCE_CP   },
   { "REDUCE_CPP"      , SX_REDUCE_CPP  },
   { "REDUCE_CG"       , SX_REDUCE_CG   },
   { "REDUCE_CGP"      , SX_REDUCE_CGP  },
   { "NONE"            , SX_NONE        },
   { "ADD_FWEIGHT"     , SX_ADD_FWEIGHT },
   { "MAX_FWEIGHT"     , SX_MAX_FWEIGHT },
   { "GT_FWEIGHT"      , SX_GT_FWEIGHT  },
   { "ADD_RWEIGHT"     , SX_ADD_RWEIGHT },
   { "MAX_RWEIGHT"     , SX_MAX_RWEIGHT },
   { "GT_RWEIGHT"      , SX_GT_RWEIGHT  },
   { "ADD_EWEIGHT"     , SX_ADD_EWEIGHT },
   { "MAX_EWEIGHT"     , SX_MAX_EWEIGHT },
   { "GT_EWEIGHT"      , SX_GT_EWEIGHT  },
   { "ADD_POLYNOM"     , SX_POLY_ADD    },
   { "MAX_POLYNOM"     , SX_POLY_MAX    },
   { "GT_POLYNOM"      , SX_POLY_GT     },
   { "GOALMATCH"       , SX_GOALMATCH   },
   { "GOALSIM"         , SX_GOALSIM     },
   { "GOALTEST"        , SX_GOALTEST    },
   { "DIVERGENCE"      , SX_DIVERGENCE  },
   { "UNIF_GOAL"       , SX_UNIF_GOAL   },
   { "DIFF_GOAL"       , SX_DIFF_GOAL   },
   { ""                , 0              }
};

/* Die folgende Tabelle enthaelt alle moglichen Gutachter             */
static  SymbolEntry  SymbolTabGa[] = {  { "STATISTIC"   , SR_STATISTIC   },
                                        { "EXTENDED"    , SR_EXTENDED    },
                                        { "FEELWELL"    , SR_FEELGOOD    },
                                        { "DIVERG_REF"  , SR_DIVERGENCE  },
                                        { "LAST"        , SS_LAST        },
                                        { "NONE"        , SR_NONE        },
                                        { ""            , 0              } };
				  
/* Die folgende Tabelle enthaelt alle moglichen Spezialisten */
static  SymbolEntry  SymbolTabSp[] = {  { "DATABASE"     , SX_DATABASE    },
                                        { "REDUCE_CP"    , SX_REDUCE_CP   },
                                        { "REDUCE_CPP"   , SX_REDUCE_CPP  },
                                        { "REDUCE_CG"    , SX_REDUCE_CG   },
                                        { "REDUCE_CGP"   , SX_REDUCE_CGP  },
                                        { ""             , 0              } };



static  SymbolEntry  SymbolTabDom_spec[] = 
				  { { "DOMAENENNAME"   , SDOMAENENNAME   },
				    { "SIGNATUR"       , SSIGNATUR       },
				    { "DEF_REGELN"     , SDEF_REGELN     },
				    { "DEF_GLEICH"     , SDEF_GLEICH     },
				    { "ALT_DEF"        , SALT_DEF        },
				    { "STARTTEAM"      , SSTARTTEAM      },
				    { "ZYKLENZAHL"     , SZYKLENZAHL     },
				    { "ZYKLENDAUER"    , SZYKLENDAUER    },
				    { "GUTE_EXP"       , SGUTE_EXP       },
				    { "UNGEEIGN_EXP"   , SUNGEEIGN_EXP   },
				    { "GUTE_GA"        , SGUTE_GA        },
				    { "UEBER_DOM"      , SUEBER_DOM      },
				    { "SPEZIAL_DOM"    , SSPEZIAL_DOM    },
				    { "VERWANDT_DOM"   , SVERWANDT_DOM   },
				    { "BEKANNTE_RES"   , SBEKANNTE_RES   },
				    { "SCHLECHTE_RES"  , SSCHLECHTE_RES  },
				    { "BESCHREIBUNG"   , SBESCHREIBUNG   },
				    { "LITERATUR"      , SLITERATUR      },
				    { "BEISPIELE"      , SBEISPIELE      },
				    { ""               , 0               } };

static SymbolEntry  SymbolTabPlan[] =
				 { { "NODOMEXP"         , SNODOMEXP         },
				   { "MAXRELDOMAIN"     , SMAXRELDOMAIN     },
				   { "D_EXPDOMFAKTOR"   , SD_EXPDOMFAKTOR   },
				   { "D_EXPROBFAKTOR"   , SD_EXPROBFAKTOR   },
				   { "D_EXPBEWFAKTOR"   , SD_EXPBEWFAKTOR   },
				   { "D_EXPKZGFAKTOR"   , SD_EXPKZGFAKTOR   },
				   { "NOD_EXPDOMFAKTOR" , SNOD_EXPDOMFAKTOR },
				   { "NOD_EXPROBFAKTOR" , SNOD_EXPROBFAKTOR },
				   { "NOD_EXPBEWFAKTOR" , SNOD_EXPBEWFAKTOR },
				   { "NOD_EXPKZGFAKTOR" , SNOD_EXPKZGFAKTOR },
				   { "W_SIZE_R"         , SW_SIZE_R         },
				   { "W_SIZE_E"         , SW_SIZE_E         },
				   { "W_SIZE_G"         , SW_SIZE_G         },
				   { "W_SIZE_CP"        , SW_SIZE_CP        },
				   { "W_SIZE_CG"        , SW_SIZE_CG        },
				   { "W_NEW_R"          , SW_NEW_R          },
				   { "W_RED_R"          , SW_RED_R          },
				   { "W_DEL_R"          , SW_DEL_R          },
				   { "W_NEW_E"          , SW_NEW_E          },
				   { "W_RED_E"          , SW_RED_E          },
				   { "W_DEL_E"          , SW_DEL_E          },
				   { "W_NEW_G"          , SW_NEW_G          },
				   { "W_RED_G"          , SW_RED_G          },
				   { "W_NEW_CP"         , SW_NEW_CP         },
				   { "W_DEL_CP"         , SW_DEL_CP         },
				   { "W_NEW_CG"         , SW_NEW_CG         },
				   { "W_REDCOUNT"       , SW_REDCOUNT       },
				   { "INCREASE_PERCENT" , SINCREASE_PERCENT },
				   { "L_SIZE_R"         , SL_SIZE_R         },
				   { "L_SIZE_E"         , SL_SIZE_E         },
				   { "L_SIZE_G"         , SL_SIZE_G         },
				   { "L_SIZE_CP"        , SL_SIZE_CP        },
				   { "L_SIZE_CG"        , SL_SIZE_CG        },
				   { "L_NEW_R"          , SL_NEW_R          },
				   { "L_RED_R"          , SL_RED_R          },
				   { "L_DEL_R"          , SL_DEL_R          },
				   { "L_NEW_E"          , SL_NEW_E          },
				   { "L_RED_E"          , SL_RED_E          },
				   { "L_DEL_E"          , SL_DEL_E          },
				   { "L_NEW_G"          , SL_NEW_G          },
				   { "L_RED_G"          , SL_RED_G          },
				   { "L_NEW_CP"         , SL_NEW_CP         },
				   { "L_DEL_CP"         , SL_DEL_CP         },
				   { "L_NEW_CG"         , SL_NEW_CG         },
				   { "L_REDCOUNT"       , SL_REDCOUNT       },
				   { "DECREASE_PERCENT" , SDECREASE_PERCENT },
				   { "VERLIERERMAXCYCL" , SVERLIERERMAXCYCL },
				   { "VERLIERERGRENZE3" , SVERLIERERGRENZE3 },
				   { "TEAMNOTCHANGED"   , STEAMNOTCHANGED   },
				   { "HOST_LIMIT_DB"    , SHOST_LIMIT_DB    },
				   { "CYCLE_DIFF_DB"    , SCYCLE_DIFF_DB    },
				   { "DOM_LIMIT_DB"     , SDOM_LIMIT_DB     },
				   { "HOST_LIMIT_CP"    , SHOST_LIMIT_CP    },
				   { "CYCLE_DIFF_CP"    , SCYCLE_DIFF_CP    },
				   { "CRIT_P_LIMIT_CP"  , SCRIT_P_LIMIT_CP  },
				   { "HOST_LIMIT_CG"    , SHOST_LIMIT_CG    },
				   { "CYCLE_DIFF_CG"    , SCYCLE_DIFF_CG    },
				   { "CRIT_G_LIMIT_CG"  , SCRIT_G_LIMIT_CG  },
				   { "DOMAIN_SUITED"    , SDOMAIN_SUITED    },
				   { "WINNER_SUITED"    , SWINNER_SUITED    },
				   { "J_NOT_IN_TEAM"    , SJ_NOT_IN_TEAM    },
				   { "NOT_JUDGED"       , SNOT_JUDGED       },
				   { "MIN_INSPECTED_X"  , SMIN_INSPECTED_X  },
				   { "EXP_PART"         , SEXP_PART         },
				   { "EXTRA_EXP1"       , SEXTRA_EXP1       },
				   { "LOSER_PART"       , SLOSER_PART       },
				   { "EXTRA_EXP2"       , SEXTRA_EXP2       },
				   { "LOSER_PERCENT"    , SLOSER_PERCENT    },
				   { "EXTRA_EXP3"       , SEXTRA_EXP3       },
				   { "MIN_BETTER_EXP"   , SMIN_BETTER_EXP   },
				   { "GOOD_DOM_EXP_KNO" , SGOOD_DOM_EXP_KNO },
				   { "BAD_DOM_EXP_KNO"  , SBAD_DOM_EXP_KNO  },
				   { "NO_DOM_EXP_KNO"   , SNO_DOM_EXP_KNO   },
				   { "RELEVANTLASTCYCL" , SRELEVANTLASTCYCL },
				   { "MINIMUMOFACTIONS" , SMINIMUMOFACTIONS },
				   { "INSPECTEDACTIONS" , SINSPECTEDACTIONS },
				   { "MAXDIFFERENCE"    , SMAXDIFFERENCE    },
				   { "CONSTANTFACTOR"   , SCONSTANTFACTOR   },
				   { "TRENDEXPFACTOR"   , STRENDEXPFACTOR   },
				   { "RELKZGFACTOR"     , SRELKZGFACTOR     },
				   { "PROOF_MID"        , SPROOF_MID        },
				   { "TIMEBASE"         , STIMEBASE         },
				   { "TP_PART"          , STP_PART          },
				   { "TIME_EXTRA"       , STIME_EXTRA       },
				   { "GOOD_TEAM_T_FAC"  , SGOOD_TEAM_T_FAC  },
				   { "BAD_TEAM_T_FAC"   , SBAD_TEAM_T_FAC   },
				   { "UPPER_LIMIT1"     , SUPPER_LIMIT1     },
				   { "DOWN_LIMIT1"      , SDOWN_LIMIT1      },
				   { "UPPER_LIMIT2"     , SUPPER_LIMIT2     },
				   { "DOWN_LIMIT2"      , SDOWN_LIMIT2      },
				   { "GOALFAKTOR"       , SGOALFAKTOR       },
				   { ""                 , 0                 } };

static  SymbolEntry  SymbolTabSpec[] = { { "START"        , SSTART        },
					 { "REDUCE"       , SREDUCE       },
					 { "SUBSUM"       , SSUBSUM       },
					 { "DOUBLE"       , SDOUBLE       },
				         { ""             , 0             } };

static  SymbolEntry SymbolTabRef[] = { { "GUTACHTERNAME"   , SGUTACHTERNAME   },
				       { "BEURTEILE_FKT"   , SBEURTEILE_FKT   },
				       { "RESULTATE_FKT"   , SRESULTATE_FKT   },
	     /* Fuer die Beurteile Gutachter */
             /* Fuer STATISTIC und EXTENDED */
				       { "STATISTIC"       , SR_STATISTIC     },
				       { "EXTENDED"        , SR_EXTENDED      },
				       { "GROESSE_REGELN"  , SGROESSE_REGELN  },
				       { "GROESSE_GLEICH"  , SGROESSE_GLEICH  },
				       { "GROESSE_CP"      , SGROESSE_CP      },
				       { "GROESSE_ZIELE"   , SGROESSE_ZIELE   },
				       { "GROESSE_CG"      , SGROESSE_CG      },
				       { "NEUE_REGELN"     , SNEUE_REGELN     },
				       { "RED_REGELN"      , SRED_REGELN      },
				       { "LOESCH_REGELN"   , SLOESCH_REGELN   },
				       { "NEUE_GLEICH"     , SNEUE_GLEICH     },
				       { "RED_GLEICH"      , SRED_GLEICH      },
				       { "LOESCH_GLEICH"   , SLOESCH_GLEICH   },
				       { "NEUE_CP"         , SNEUE_CP         },
				       { "LOESCH_CP"       , SLOESCH_CP       },
				       { "NEUE_ZIELE"      , SNEUE_ZIELE      },
				       { "RED_ZIELE"       , SRED_ZIELE       },
				       { "NEUE_CG"         , SNEUE_CG         },
				       { "GESAMT_RED"      , SGESAMT_RED      },
				       { "GEEIGNETE_EXP"   , SGEEIGNETE_EXP   },
				       { "GEEIGNETE_DOM"   , SGEEIGNETE_DOM   },
				       { "ROBUSTHEIT"      , SROBUSTHEIT      },
				       { "WISSENSANTEIL"   , SWISSENSANTEIL   },
				       { "AEHNLICHE_GA"    , SAEHNLICHE_GA    },
				       { "LEITER_MOEGLICH" , SLEITER_MOEGLICH },
	      /* Fuer FEELGOOD  */
				       { "FEELGOOD"        , SR_FEELGOOD      },
				       { "LETZTE_CP"       , SLETZTE_CP       },
				       { "LETZTE_CG"       , SLETZTE_CG       },
				       { "VERGANGEN_FAKTOR", SVERGANGEN_FAKTOR},
				       { "NAECHSTE_CP"     , SNAECHSTE_CP     },
				       { "NAECHSTE_CG"     , SNAECHSTE_CG     },
				       { "ZUKUNFTS_FAKTOR" , SZUKUNFTS_FAKTOR },
				       { "KORREKTUR_FAKTOR", SKORREKTUR_FAKTOR},
	      /* Fuer die Finde Resultate Gutachter */
				       { "LAST"            , SS_LAST          },
				       { "MAX_REGELN"      , SMAX_REGELN      },
				       { "MAX_GLEICH"      , SMAX_GLEICH      },
				       { "MAX_ZIELE"       , SMAX_ZIELE       },
				       { "SCHWELLE_REGELN" , SSCHWELLE_REGELN },
				       { "SCHWELLE_GLEICH" , SSCHWELLE_GLEICH },
				       { "SCHWELLE_ZIELE"  , SSCHWELLE_ZIELE  },
				       { "ANZAHL_RED"      , SANZAHL_RED      },
				       { "ANZAHL_RED_R"    , SANZAHL_RED_R    },
				       { "ANZAHL_RED_L"    , SANZAHL_RED_L    },
				       { "ANZAHL_RED_GLEICH", SANZAHL_RED_GLEICH},
				       { "ANZAHL_RED_ZIELE", SANZAHL_RED_ZIELE},
				       { "ANZAHL_SUBSUM"   , SANZAHL_SUBSUM   },
				       { "ANZAHL_CP"       , SANZAHL_CP       },
				       { "ANZAHL_CG"       , SANZAHL_CG       },
				       { ""                , 0                } };

static  SymbolEntry EmptyTab[] = { { ""   , 0 } };


/*
//-----------------------------------------------------------------------------
//      Modulinterne Variablen
//-----------------------------------------------------------------------------
*/

file_des     FileArray[MAXFILES] = { FILEINITIAL, FILEINITIAL, FILEINITIAL,
				     FILEINITIAL, FILEINITIAL, FILEINITIAL,
				     FILEINITIAL, FILEINITIAL, FILEINITIAL,
				     FILEINITIAL };

/*
//-----------------------------------------------------------------------------
//      Lokale Funktionsvereinbarung
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    static void    NextChar ( short index );
    static void    Skip     ( short index );
    static void    SkipLine ( short index );

    static bool    ScanIdent  ( short index, char *ident );
    static Symbol  ScanSymbol ( short index, char *ident );

#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       OpenInput ( char *filename )
//
//  Parameter:      Dateiname der Spezifikation
//
//  Returnwert:     Index fuer die Adressierung des interen Filedescriptors
//
//  Beschreibung:   Oeffnet eine Spezifikation zum Lesen.
//-----------------------------------------------------------------------------
*/

short    OpenInput ( char *filename )
{
    int i;
    char   ErrorMsg[100];

    for ( i=0; i<MAXFILES; i++)
      if ( !FileArray[i].inputopen )
	break;
    if ( i == MAXFILES )
      Error( __FILE__ ": " "OpenInput", "Zu viele Dateien geoeffnet." );

    FileArray[i].input = fopen ( filename, "r" );
    if (!FileArray[i].input)
    {
      sprintf(ErrorMsg, "Eingabedatei %s kann nicht geoeffnet werden.", filename );
      Error ( __FILE__ ": "  "OpenInput", ErrorMsg );
    }

    FileArray[i].inputopen = true;
    FileArray[i].scanline = 0;

    FileArray[i].eoi = feof ( FileArray[i].input );
    if (!FileArray[i].eoi)
    {
        fgets ( FileArray[i].scantext, (int)(sizeof FileArray[i].scantext), FileArray[i].input );
        FileArray[i].scanline++;
        FileArray[i].scanpos    = 0;
        FileArray[i].scanlength = strlen ( FileArray[i].scantext );
        FileArray[i].ScanChar   = (FileArray[i].scanlength) ? ' ' : CR;
    }

    #ifdef PROTOCOL
        printf ("----------------------------------------------------------");
        printf ("\n    Eingabedatei: %s\n", filename );
        printf ("----------------------------------------------------------");
        printf ("\n");
    #endif

    return i;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CloseInput ( short index )
//
//  Parameter:      Index des internen Filedescriptors, der geschlossen wird
//
//  Beschreibung:   Schliesst die Eingabedatei.
//-----------------------------------------------------------------------------
*/

void    CloseInput ( short index )
{
    if (FileArray[index].inputopen)
        fclose ( FileArray[index].input );

    FileArray[index].input = NULL;
    FileArray[index].inputopen = false;
    FileArray[index].eoi = true;

    #ifdef PROTOCOL
        printf ("----------------------------------------------------------");
        printf ("\n\n");
    #endif
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetUppercase ( short index, bool flag )
//
//  Parameter:      Index des internen Filedescriptors
//                  flag - siehe Beschreibung
//
//  Beschreibung:   flag = false   Scanner unterscheidet zwischen
//                                 Gross- und Kleinbuchstaben.
//                  flag = true    Scanner unterscheidet nicht zwischen
//                                 Gross- und Kleinbuchstaben.
//-----------------------------------------------------------------------------
*/

void    SetUppercase ( short index, bool flag )
{
    FileArray[index].uppercase = flag;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       NextChar ( short index )
//
//  Parameter:      Index des internen Filedescriptors
//
//  Beschreibung:   Holt wenn moeglich das naechste Zeichen der Eingabe
//                  CR = Zeilenende
//-----------------------------------------------------------------------------
*/

static void    NextChar ( short index )
{
    register int    i = 0;

    FileArray[index].eoi = FileArray[index].eoi || 
		           (feof (FileArray[index].input) && (FileArray[index].ScanChar == CR));

    if (!FileArray[index].eoi)
    {
        if (FileArray[index].ScanChar == CR)
        {
            while ( FileArray[index].scantext[i] )
                FileArray[index].scantext[i++] = 0;
            fgets ( FileArray[index].scantext, 
		    (int)(sizeof FileArray[index].scantext), FileArray[index].input );
            FileArray[index].scanline++;
            FileArray[index].scanlength = strlen ( FileArray[index].scantext );
            FileArray[index].scanpos    = 0;
        }

        FileArray[index].ScanChar   = (FileArray[index].scanlength > FileArray[index].scanpos) ? 
				      FileArray[index].scantext[FileArray[index].scanpos++] : 
			              CR;

        FileArray[index].eoi = (feof (FileArray[index].input) && (FileArray[index].ScanChar == CR));
        if (FileArray[index].eoi)
            FileArray[index].ScanChar = CR;

        #ifdef PROTOCOL
            putchar ( FileArray[index].ScanChar );
        #endif

        FileArray[index].ScanChar = (FileArray[index].uppercase) ? 
					 toupper (FileArray[index].ScanChar) : 
					 FileArray[index].ScanChar;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Skip ( short index )
//
//  Parameter:      Index des internen Filedescriptors
//
//  Beschreibung:   Ueberspringt Leerzeichen
//-----------------------------------------------------------------------------
*/

static void Skip ( short index )
{
    while (!EOI( index ) && ((FileArray[index].ScanChar == ' ') || (FileArray[index].ScanChar == TAB)))
        NextChar( index );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SkipLine ( short index )
//
//  Parameter:      Index des internen Filedescriptors
//
//  Beschreibung:   Ueberspringt den Rest der Eingabezeile
//-----------------------------------------------------------------------------
*/

static void SkipLine ( short index )
{
    while (FileArray[index].ScanChar != CR)
        NextChar( index );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ScanIdent ( short index, char *ident )
//
//  Parameter:      Index des internen Filedescriptors
//                  ident    - Zeiger auf Speicher fuer Ident
//
//  Rueckgabewert:  true     korrekter Identifier gelesen
//                  false    kein korrekter Identifierer
//
//  Beschreibung:   Einlesen eines Identifiers
//-----------------------------------------------------------------------------
*/

static bool    ScanIdent ( short index, char *ident )
{
    Skip ( index );

    if (FileArray[index].ScanChar == '\"')
    {
        NextChar( index );
        while (FileArray[index].ScanChar != '\"')
        {
           if (FileArray[index].ScanChar == CR)
               Error ( __FILE__ ": "  "ScanIdent", "Fehler beim Einlesen eines Strings." );
           *ident++ = FileArray[index].ScanChar;
           NextChar( index );
        }
        NextChar( index );
        *ident = 0;
        return true;
    }

    if (!isalnum (FileArray[index].ScanChar) && (FileArray[index].ScanChar != '_')
                            && (FileArray[index].ScanChar != '+')
                            && (FileArray[index].ScanChar != '-'))
        return false;

    while (isalnum (FileArray[index].ScanChar) || (FileArray[index].ScanChar == '_')
                              || (FileArray[index].ScanChar == '+')
                              || (FileArray[index].ScanChar == '-'))
    {
        *ident++ = FileArray[index].ScanChar;
        NextChar( index );
    }

    *ident = 0;
    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ScanSymbol ( short index, char *ident )
//
//  Parameter:      Index des internen Filedescriptors
//                  Zeiger auf Speicher fuer Identifier
//
//  Rueckgabewert:  Klassifizierung der gelesenen Zeichen
//
//  Beschreibung:   Einlesen und Klassifizieren eines Symbols
//-----------------------------------------------------------------------------
*/

static Symbol   ScanSymbol ( short index, char *ident )
{
    if (ScanIdent ( index, ident ))
        return SIDENT;

    switch (FileArray[index].ScanChar)
    {
    case '#' :  SkipLine ( index );
                return SCRSYM;
    case '=' :  NextChar( index );
                return SEQUAL;
    case '>' :  NextChar( index );
                return SGREATER;
    case CR  :  NextChar( index );
                return SCRSYM;
    case '.' :  NextChar( index );
		return SPUNKT;
    case ':' :  NextChar( index );
                return SCOLLON;
    case ',' :  NextChar( index );
                return SCOMMA;
    case ';' :  NextChar( index );
		return SSEMICOLLON;
    case '(' :  NextChar( index );
                return SBRACKET_L;
    case ')' :  NextChar( index );
                return SBRACKET_R;
    default  :  printf ( "***** %c %xd\n", FileArray[index].ScanChar, FileArray[index].ScanChar );
                Error ( __FILE__ ": "  "ScanSymbol", "Unbekanntes Zeichen eingelesen." );
                return SUNKNOWN;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       IsNumber ( char *ident )
//
//  Parameter:      ident    - Zeiger auf Speicher fuer Ident
//
//  Rueckgabewert:  true     falls ident eine Ziffernfolge darstellt
//                  false    sonst
//
//  Beschreibung:   Prueft, ob ident eine Ziffernfolge ist.
//-----------------------------------------------------------------------------
*/

bool    IsNumber ( char *ident )
{
    register short    i = 0;

    if ((ident[0] == '+') || (ident[0] == '-'))
        i++;

    while (ident[i] && isdigit (ident[i]))
        i++;

    return ident[i] ? false : true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       GetSymbol ( short index, char *ident )
//
//  Parameter:      Index des internen Filedescriptors
//                  Zeiger auf Speicher fuer Identifier
//
//  Rueckgabewert:  Klassifizierung der gelesenen Zeichen
//
//  Beschreibung:   Einlesen und Klassifizieren eines Symbols.
//                  Schluesselworte werden erkannt und als Symbole
//                  zurueckgegeben.
//-----------------------------------------------------------------------------
*/

Symbol  GetSymbol ( short index, char *ident )
{
    register short  i;
    register Symbol sym;
             char   bident[IDENTLENGTH];

    sym = ScanSymbol ( index, ident );

    if (sym != SIDENT)
        return sym;

    i = 0;
    while (ident[i])
    {
        if (islower (bident[i] = ident[i]))
            bident[i] = toupper (bident[i]);
        i++;
    }
    bident[i] = 0;

    i = 0;
    while (FileArray[index].SymbolTab[i].code)
    {
        if (! strcmp(bident, FileArray[index].SymbolTab[i].symbol ))
            return FileArray[index].SymbolTab[i].code;
        i++;
    }

    return SIDENT;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       GetFilename  ( short index, char *file )
//
//  Parameter:      Index des internen Filedescriptors
//                  file    Dateiname
//
//  Rueckgabewert:  true    korrekter Dateiname eingelesen
//                  false   sonst
//
//  Beschreibung:   Einlesen eines Dateinamens
//-----------------------------------------------------------------------------
*/

bool    GetFilename ( short index, char *file )
{
    Skip ( index );

    if (!isgraph (FileArray[index].ScanChar))
        return false;

    while (isgraph (FileArray[index].ScanChar))
    {
        *file++ = FileArray[index].ScanChar;
        NextChar( index );
    }

    *file = 0;
    return true;
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintScanText ( short index )
//
//  Parameter:      Index des internen Filedescriptors
//
//  Beschreibung:   Ausgabe des Scantextes
//                  (Dient der Lokalisierung von Fehlern bei Syntaxfehlern)
//-----------------------------------------------------------------------------
*/

void    PrintScanText ( short index )
{
    printf ( "****  (Zeile %ld) %s\n", (long)FileArray[index].scanline, FileArray[index].scantext );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintSymbol ( short index, short sym, char *ident )
//
//  Parameter:      Index des internen Filedescriptors
//                  sym     Kennung des Symbols
//                  ident   Identifier
/
//  Beschreibung:   Kontrollausgabe eines Symbols
//-----------------------------------------------------------------------------
*/

void    PrintSymbol ( short index, short sym, char *ident )
{
    short     i     = 0;

    if (sym == SIDENT)
    {
        printf ( "#%s#", ident );
        return;
    }

    while ((FileArray[index].SymbolTab[i].code) && (FileArray[index].SymbolTab[i].code != sym))
        i++;

    if (i)
        printf ( "<<%s>>", FileArray[index].SymbolTab[i].symbol );
    else
        Error ( __FILE__ ": "  "PrintSymbol", "Unbekanntes Symbol." );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetScanMode ( short index, ScanMode mode )
//
//  Parameter:      Index des internen Filedescriptors
//                  mode    Flag fuer gewaehlten Modus
//
//  Beschreibung:   Auswahl der Symboltabelle
//                  FILE_MODE   Aufgabenstellung
//                  CONFIG_MODE Konfigurationsdatei
//                  DATA_MODE   Datenbasis
//                  EXP_MODE    Expertenbeschreibung
//                  DOM_MODE    Domaenenbeschreibung
//                  PLAN_MODE   Konfigurationsdatei fuer die Planungsphase
//                  SPEC_MODE   Spezialistenbeschreibung
//                  REF_MODE    Gutachterbeschreibung
//                  NULL_MODE   Keine Symboltabelle
//-----------------------------------------------------------------------------
*/

void    SetScanMode ( short index, ScanMode mode )
{
    switch (mode)
    {
      case FILE_MODE  : FileArray[index].SymbolTab= SymbolTabX;
                        break;
      case CONFIG_MODE: FileArray[index].SymbolTab= SymbolTabC;
                        break;
      case DATA_MODE  : FileArray[index].SymbolTab= SymbolTabD;
                        break;
      case EXP_MODE   : FileArray[index].SymbolTab= SymbolTabExp_spec;
			break;
      case DOM_MODE   : FileArray[index].SymbolTab= SymbolTabDom_spec;
			break;
      case PLAN_MODE  : FileArray[index].SymbolTab= SymbolTabPlan;
			break;
      case SPEC_MODE  : FileArray[index].SymbolTab= SymbolTabSpec;
			break;
      case REF_MODE   : FileArray[index].SymbolTab = SymbolTabRef;
			break;
      case NULL_MODE  : FileArray[index].SymbolTab = EmptyTab;
			break;
    }
}

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  Check_Symbol                                             */
/*                                                                           */
/*  Parameter    :  - Identifier, von dem getestet werden soll, ob er ein er-*/
/*                    laubtes Symbol an dieser Stelle ist.                   */
/*                  - Modus, der angibt, in welcher Tabelle die erlaubten    */
/*                    Symbole stehen.                                        */
/*                                                                           */
/*  Returnwert   :  true, wenn das Symbol ein Rerlaubtes Symbol ist          */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Die angegebene Symboltabelle wird durchlaufen und dem    */
/*                  Vergleichsergebnis entsprechend gehandelt.               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
bool   Check_Symbol ( char *ident, ChekMode mode)
{
  register int i = 0;
  SymbolEntry *tab = NULL;

  switch ( mode )
  {
    case CHECK_EXP   : tab = SymbolTabExp;
		       break;
    case CHECK_GA    : tab = SymbolTabGa;
		       break;
    case CHECK_SPEC  : tab = SymbolTabSp;
		       break;
    default        :
      Error ( __FILE__ ": "  "Check_Symbol", "Unbekannter Check Modus." );
  }

  while (tab[i].code)
  {
      if ( !strcmp(tab[i].symbol, ident) )
      {
	return true;
      }
      i++;
  }

  return false;
} /* Ende von Check_Symbol */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  EOI                                                      */
/*                                                                           */
/*  Parameter    :  Index des internen Filedescriptors                       */
/*                                                                           */
/*  Returnwert   :  true, wenn die angegebene Datei ganz gelesen wurde       */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  klar                                                     */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool EOI ( short index )
{
  return FileArray[index].eoi;
}
