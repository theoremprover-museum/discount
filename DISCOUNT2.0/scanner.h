/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         scanner
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul erlaubt ein einfaches Einlesen von
//                      Dateien
//
//      Funktionen:     OpenInput       Eingabedatei oeffnen
//                      CloseInput      Eingabedatei schliessen
//
//                      SetUppercase    Unterscheidung Gross- und
//                                      Kleinschrifft
//
//                      GetSymbol       ein Symbol einlesen
//                      GetFilename     ein Dateiname wird eingelesen
//                      PrintScanText   aktuellen Scantext ausgeben (Fehler)
//                      PrintSymbol     ein Symbol ausgeben (Test)
//
//                      SetScanMode     Auswahl der Symboltabelle
//-----------------------------------------------------------------------------
//      $Log: scanner.h,v $
//      Revision 0.9  1992/03/26  12:32:21  pitz
//      GOALSIM eingefuehrt.
//
//      Revision 0.8  1991/10/30  09:23:16  pitz
//      WKBO eingefuehrt.
//
//      Revision 0.7  1991/09/25  14:15:23  pitz
//      Neue Schluesselwoerter: ADD_RWEIGHT, MAX_RWEIGHT, GT_RWEIGHT
//                              ADD_EWEIGHT, MAX_EWEIGHT, GT_EWEIGHT
//
//      Revision 0.6  1991/09/20  10:07:42  pitz
//      Schluesselwort DATABASE.
//
//      Revision 0.5  1991/09/19  12:02:41  pitz
//      Symbol EXTENDED eingefuehrt.
//
//      Revision 0.4  1991/09/11  08:34:29  pitz
//      Neues SetScanMode und Symbol-Tabelle SymbolTabD.
//
//      Revision 0.3  1991/09/05  12:57:39  pitz
//      Neue Schluesselwoerter: SPEC_ADD, SPEC_MAX und SPEC_GT.
//
//      Revision 0.2  1991/08/26  15:26:32  pitz
//      SPECIAL als Schluesselwort erklaert.
//
//      Revision 0.1  1991/08/26  13:15:08  pitz
//      REDUCE_CPP als Scannersymbol eingefuehrt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __SCANNER
#define __SCANNER

#include "error.h"

/*****************************************************************************/
/*                                                                           */
/*                             Konstanten                                    */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*                         Typendefinitionen                                 */
/*                                                                           */
/*****************************************************************************/
/*
//-----------------------------------------------------------------------------
//      Symbolvereinbarungen
//-----------------------------------------------------------------------------
*/

typedef enum 
{
   /* allgemeine Symbole f"ur den Parser */
   SUNKNOWN, SIDENT,
   SEQUAL, SGREATER, SCRSYM, SCOLLON, SSEMICOLLON, SCOMMA, SPUNKT,
   SBRACKET_L, SBRACKET_R,

   /* Symbole f"ur die Datei mit der Problemstellung */
   SMODE, SCOMPLETION, SPROOF, SPARAMOD,
   SNAME,
   SORDERING, SKBO, SRPO, SLPO, SXKBO, SWKBO,
   SEQUATIONS, SSPECIAL, SCANCELLATION, SCONCLUSION,
   
   /* Symbole f"ur den Konfigurationsfile */
   SHOSTS,
   SDATA_DIR, SEXPERT_DIR, SREFEREE_DIR, SDOMAIN_DIR, SKNOWLEDGE_DIR,
   SPROTOCOL_DIR, 
   SSTATIC, SINITIAL, SSELECT,
   SDOMAIN_SELECT,
   STIMEBASE, SCONSTANT, SLINEAR, SEXPONENTIAL,
   SSUPERVISOR_FILE,

   /* Symbole aller CP- und CG-Funktionen */
   SX_ADD, SX_FIFO, SX_TEAM_FIFO, SX_GLOBAL_LEARN,
   SX_GOAL_BOUND_LEARN,
   SX_SPEC_BOUND_LEARN,SX_1TET_LEARN, SX_2TET_LEARN,
   SX_MAX, SX_GT, SX_DIFF, 
   SX_OCCNEST,
   SX_SPEC_ADD, SX_SPEC_MAX, SX_SPEC_GT,
   SX_KBO_ADD, SX_KBO_MAX, SX_KBO_GT,
   SX_ADD_FWEIGHT, SX_MAX_FWEIGHT, SX_GT_FWEIGHT,
   SX_ADD_RWEIGHT, SX_MAX_RWEIGHT, SX_GT_RWEIGHT,
   SX_ADD_EWEIGHT, SX_MAX_EWEIGHT, SX_GT_EWEIGHT,
   SX_POLY_ADD, SX_POLY_MAX, SX_POLY_GT,
   SX_GOALMATCH, SX_GOALSIM, SX_GOALTEST,
   SX_DATABASE,
   SX_UNIF_GOAL, SX_DIFF_GOAL,
   SX_REDUCE_CP, SX_REDUCE_CPP, SX_REDUCE_CG, SX_REDUCE_CGP, SX_NONE,
   SX_DIVERGENCE, 

   /* Symbole f"ur Beurteile- und Auswahlfunktionen der Gutachter */
   SR_STATISTIC, SR_EXTENDED, SR_DIVERGENCE, SR_FEELGOOD, SR_NONE,
   SS_LAST,
   
   SFUNCTIONS, SRULES, SRESULTS, 
   
   /* Symbole f"ur den Rahmen der Experten */
   SEXPERTENNAME, SEXP_OR_SPEC, SCP_FUNKTION, SCG_FUNKTION, SFAIRNESS,
   SGUTE_GUTACHTER, SGUTE_DOMAENEN, SROBUSTHEIT,
   SWISSENSANTEIL, SBEWEISPHASE, SZYKLUSDAUER,
   SANFANG, SMITTE, SENDE,

   SAEHNLICHE_EXP, SNACHFOLGER,
   SUNVERTRAEGL, SMOEGL_TEAMEXP,
   SPROZEDUR, SBESCHREIBUNG,

   SDOMAENENNAME, SSIGNATUR, SDEF_REGELN, SDEF_GLEICH,
   SALT_DEF, SSTARTTEAM, SZYKLENZAHL, SZYKLENDAUER,
   SGUTE_EXP, SUNGEEIGN_EXP, SGUTE_GA, SUEBER_DOM,
   SSPEZIAL_DOM, SVERWANDT_DOM, SBEKANNTE_RES,
   SSCHLECHTE_RES, SLITERATUR, SBEISPIELE,

   /* Symbole f"ur den Rahmen des Database-Experten */
   SFIND_ALL, SFIND_FIRST,

   /* Fuer den Gutachter Parser */
   SGUTACHTERNAME, SBEURTEILE_FKT, SRESULTATE_FKT,
   SGROESSE_REGELN, SGROESSE_GLEICH, SGROESSE_CP,
   SGROESSE_ZIELE, SGROESSE_CG, SNEUE_REGELN, 
   SRED_REGELN, SLOESCH_REGELN, SNEUE_GLEICH,
   SRED_GLEICH, SLOESCH_GLEICH, SNEUE_CP,SLOESCH_CP, SNEUE_ZIELE,
   SRED_ZIELE, SNEUE_CG, SGESAMT_RED, SGEEIGNETE_EXP,
   SGEEIGNETE_DOM, SAEHNLICHE_GA, SLEITER_MOEGLICH,
   SLETZTE_CP, SLETZTE_CG, SVERGANGEN_FAKTOR,
   SNAECHSTE_CP, SNAECHSTE_CG, SZUKUNFTS_FAKTOR,
   SKORREKTUR_FAKTOR, SMAX_REGELN, SMAX_GLEICH,
   SMAX_ZIELE, SSCHWELLE_REGELN, SSCHWELLE_GLEICH,
   SSCHWELLE_ZIELE, SANZAHL_RED, SANZAHL_RED_R,
   SANZAHL_RED_L, SANZAHL_RED_GLEICH, SANZAHL_RED_ZIELE,
   SANZAHL_SUBSUM, SANZAHL_CP, SANZAHL_CG,

   /* Fuer den Parser der Planungsdatei */
   SNODOMEXP, SMAXRELDOMAIN, 
   SD_EXPDOMFAKTOR, SD_EXPROBFAKTOR, SD_EXPBEWFAKTOR, SD_EXPKZGFAKTOR, 
   SNOD_EXPDOMFAKTOR, SNOD_EXPROBFAKTOR, SNOD_EXPBEWFAKTOR, SNOD_EXPKZGFAKTOR, 
   SW_SIZE_R, SW_SIZE_E, SW_SIZE_G, SW_SIZE_CP, SW_SIZE_CG,
   SW_NEW_R, SW_RED_R, SW_DEL_R, SW_NEW_E, SW_RED_E, SW_DEL_E, SW_NEW_G, SW_RED_G,
   SW_NEW_CP, SW_DEL_CP, SW_NEW_CG, SW_REDCOUNT,
   SINCREASE_PERCENT,
   SL_SIZE_R, SL_SIZE_E, SL_SIZE_G, SL_SIZE_CP, SL_SIZE_CG,
   SL_NEW_R, SL_RED_R, SL_DEL_R, SL_NEW_E, SL_RED_E, SL_DEL_E, SL_NEW_G, SL_RED_G,
   SL_NEW_CP, SL_DEL_CP, SL_NEW_CG, SL_REDCOUNT,
   SDECREASE_PERCENT, SVERLIERERMAXCYCL, SVERLIERERGRENZE3, STEAMNOTCHANGED, 
   SHOST_LIMIT_DB, SCYCLE_DIFF_DB, SDOM_LIMIT_DB,
   SHOST_LIMIT_CP, SCYCLE_DIFF_CP, SCRIT_P_LIMIT_CP, 
   SHOST_LIMIT_CG, SCYCLE_DIFF_CG, SCRIT_G_LIMIT_CG, 
   SDOMAIN_SUITED, SWINNER_SUITED, SJ_NOT_IN_TEAM, SNOT_JUDGED,
   SMIN_INSPECTED_X, SEXP_PART, SEXTRA_EXP1, SLOSER_PART, SEXTRA_EXP2,
   SLOSER_PERCENT, SEXTRA_EXP3, SMIN_BETTER_EXP,
   SGOOD_DOM_EXP_KNO, SBAD_DOM_EXP_KNO, SNO_DOM_EXP_KNO, 
   SRELEVANTLASTCYCL, SMINIMUMOFACTIONS, SINSPECTEDACTIONS,
   SMAXDIFFERENCE,
   SCONSTANTFACTOR, STRENDEXPFACTOR, SRELKZGFACTOR, SPROOF_MID,
   STP_PART, STIME_EXTRA, SGOOD_TEAM_T_FAC,
   SBAD_TEAM_T_FAC, SUPPER_LIMIT1, 
   SDOWN_LIMIT1, SUPPER_LIMIT2, SDOWN_LIMIT2, SGOALFAKTOR, 

   /* Paramter der Reduktionsspezialisten */
   SSTART, SREDUCE, SSUBSUM, SDOUBLE  
} Symbol;


typedef enum { FILE_MODE, CONFIG_MODE, DATA_MODE, EXP_MODE,
	       DOM_MODE, PLAN_MODE, SPEC_MODE, REF_MODE, NULL_MODE } ScanMode;

/* Menge von Symboltabellen, nach denen Symbole auf Korrektheit getestet */
/* werden koennen.                                                       */
typedef enum { CHECK_EXP, CHECK_GA, CHECK_SPEC } ChekMode;



/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/


short   OpenInput    ( char *filename );
void    CloseInput   ( short index );

void    SetUppercase ( short index, bool flag );

bool    IsNumber     ( char *ident );
Symbol  GetSymbol    ( short index, char *ident );
bool    GetFilename  ( short index, char *file );
void    PrintScanText( short index );
void    PrintSymbol  ( short index, short sym, char *ident );

void    SetScanMode  ( short index, ScanMode mode );
bool    Check_Symbol ( char *ident, ChekMode mode);
bool    EOI          ( short index );


/********************** Makrovereinbarungen **************************/

/* Den Parametern wird jeweils ein "m_" vorgestellt */

/*****************************************************************************/
/*                                                                           */
/*  Makro        :  SkipSpace                                                */
/*                                                                           */
/*  Parameter    :  - interner Dscriptor auf die zu lesende Datei            */
/*                  - einen String, der einen Identifier aufnehmen kann      */
/*                  - eine Variable vom Typ Symbol                           */
/*                                                                           */
/*  Beschreibung :  Dieses Makro ueberspringt Leerraum, d.h. Leerzeichenr,   */
/*                  Leerzeilen, Tabulatoren, Kommentarzeilen.                */
/*                  Kommt sie dabei an das Dateiende, so erfolgt eine Feh-   */
/*                  lermeldung.                                              */
/*                  Das erste Symbol, das niht vom Typ SCRSYM ist, wird in   */
/*                  den Parametern als Identifier und interpretiertes Symbol */
/*                  zurueckgegeben.                                          */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

#define SkipSpace(m_fd,m_ident,m_sym) \
   while ((m_sym = GetSymbol ((m_fd), m_ident)) == SCRSYM)\
     CheckError ( EOI( (m_fd) ), "Unerwartetes Dateiende." );


/*****************************************************************************/
/*                                                                           */
/*  Makro        :  SkipSymbol                                               */
/*                                                                           */
/*  Parameter    :  - interner Dscriptor auf die zu lesende Datei            */
/*                  - einen String, der einen Identifier aufnehmen kann      */
/*                  - eine Variable vom Typ Symbol                           */
/*                  - eine Konstante vom Typ Symbol, die angibt, welches Sym-*/
/*                    bol uebersprungen werden soll.                         */
/*                  - ein String, der im Fehlerfalle angibt, was an dieser   */
/*                    Stelle erwartet wurde.                                 */
/*                                                                           */
/*  Beschreibung :  Dieses Makro ueberspringt zunaechst Leerraum.            */
/*                  Das erste Symbol ungleich SCRSYM wird dann daraufhin     */
/*                  untersucht, ob es mit dem als Konstante uebergebenen     */
/*                  Symbol uebereinstimmt; wenn nicht erfolgt eine Fehler-   */
/*                  meldung.                                                 */
/*                                                                           */
/*****************************************************************************/

#define SkipSymbol(m_fd,m_ident,m_varsym,m_constsym,m_message) \
   while ((m_varsym = GetSymbol ((m_fd), m_ident)) == SCRSYM)\
     CheckError ( EOI( (m_fd) ), "Unerwartetes Dateiende." );\
   CheckError ( ( (m_varsym) != (m_constsym) ), #m_message " erwartet.");



#endif











