/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          team
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung eines Teams
//-----------------------------------------------------------------------------
//      $Log: team.c,v $
//
//      Revision 0.16  1993/03/12  14:02:12  lind
//      gesammte alte Kommunikationsstruktur beseitigt, ab sofort
//      werden nur noch die von br_OpenSendBroadcast( ... ) bzw.
//      br_OpenRecvBroadcast( ... ) aufgebauten Verbindungen genutzt.
//      Das Feld Output[i] entfaellt voellig
//      Aenderungen in: -StartTeam
//                      -StartCompletion
//                      -Master
//                      -Slave
//                      -NewMaster
//                      -TerminateTeam
//                      -InitConnection
//                      -SendResults
//                      -ReceiveResults
//                      -globale Variablen
//                !!!   -Execute             !!!
//                !!!   -StartXpert          !!!
//      Zweiter Parameter in StartXpert( ... ) entfernt
//
//      Revision 0.15  1993/03/05  10:14:02  lind
//      auf Broadcasting umgestellt
//      Aenderungen in: -StartTeam
//                      -StartCompletion
//                      -Master
//                      -Slave
//                      -NewMaster
//                      -TerminateTeam
//                      -globale Variablen
//
//      Revision 0.14  1992/05/27  09:47:32  pitz
//      Pfadkorrektur fuer Auto-Mounter.
//
//      Revision 0.13  1992/02/24  12:16:37  pitz
//      Option -C -> Konfigurationsdatei
//
//      Revision 0.12  1991/09/27  08:14:25  pitz
//      ..._RWEIGHT und ..._EWEIGHT implementiert.
//
//      Revision 0.11  1991/09/25  14:16:44  pitz
//      Fehler bei Uebertragung des Ergebnisses des Datenbank-Expertens
//      behoben. (Absolut ueberfluessiges getchar() ).
//
//      Revision 0.10  1991/09/25  12:04:22  pitz
//      Experte DATABASE eingebunden.
//
//      Revision 0.9  1991/09/19  12:35:58  pitz
//      Parameterzahl von EXTENDED auf 13 erhoeht.
//
//      Revision 0.8  1991/09/19  12:10:47  pitz
//      Einlesen von EXTENDED-Referee.
//
//      Revision 0.7  1991/09/11  08:28:51  pitz
//      An neues SetScanMode angepasst.
//
//      Revision 0.6  1991/09/05  14:42:35  pitz
//      SPEC_ADD, SPEC_MAX und SPEC_GT eingefuehrt.
//
//      Revision 0.5  1991/08/26  15:26:10  pitz
//      REDUCE_CPP eingefuehrt.
//
//      Revision 0.4  1991/08/22  11:19:34  pitz
//      Fehler korigiert: goalmatch wird im Team richtig gesetzt !
//
//      Revision 0.3  1991/08/19  09:50:31  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.2  1991/08/14  13:19:27  pitz
//      Teamprotokoll vor Uebertragung von Regeln, Gleichungen,
//      kritischen Paaren und Zielen.
//
//      Revision 0.1  1991/08/14  11:56:31  pitz
//      Direkter Wechsel zwischen Spezialisten ermoeglicht.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>
#include    <string.h>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <netdb.h>

#include    "defines.h"
#include    "error.h"
#include    "polynom.h"
#include    "vartree.h"
#include    "term.h"
#include    "termpair.h"
#include    "subst.h"
#include    "order.h"
#include    "cpweight.h"
#include    "reduce.h"
#include    "buildcp.h"
#include    "complet.h"
#include    "scanner.h"
#include    "parser.h"
#include    "socket.h"
#include    "referee.h"
#include    "transfer.h"
#include    "database.h"
#include    "exp_def_next_cycletime.h"
#include    "exp_value_exp.h"
#include    "expert.h"
#include    "exp_set_next_config.h"
#include    "parseexp.h"
#include    "parseprk.h"
#include    "pcl.h"
#include    "team.h"
#include    "parseplan.h"
#include    "parseref.h"
#include    "parsedom.h"
#include    "comphase.h"
#include    "br_types.h"

#ifdef STATISTICS

#include          "br_stat.h"
extern tStatistic br_BroadcastStatistic;
extern tString    br_Hostname;

#endif

/*****************************************************************************/
/*                                                                           */
/*                       Modulinterne Konstanten                             */
/*                                                                           */
/*****************************************************************************/

/* Name der Datei, in der die Ergebnisse der Planung abgelegt werden         */
#define         PLAN_LOGFILE      "plan.doc"

/* Default-Directories f"ur Experten, Gutachter, Dom"anen und Protokoll      */
/* Die Pfade gehen dabei von dem durch DATA_DIR angegebenen Directory aus.   */
#define         DEF_EXPERT_DIR    "expert"
#define         DEF_REFEREE_DIR   "referee"
#define         DEF_DOMAIN_DIR    "domain"
#define         DEF_PROTOCOL_DIR  "protocol"
#define         DEF_KNOWLEDGE_DIR "KNOWLEDGE"

/*
//-----------------------------------------------------------------------------
//      aktuelle Einstellung
//-----------------------------------------------------------------------------
*/

short           MODUS       = SINGLE;
char            *log_base = 0;
char            logfile[MAXPATHLEN] = "";
FILE            *log_f        = NULL;


               /************************************************/
               /* true, falls Dokumentation der Planungsphase  */
               /* erwuenscht                                   */
               /************************************************/
bool               PlanDocum = false;

/* File, in dem die Planungsergebnisse abgelegt werden */
FILE            *plan_doc   = NULL;

/*****************************************************************************/
/*                                                                           */
/*                            externe Variablen                              */
/*                                                                           */
/*****************************************************************************/
/***************************/
/* Broadcast-Informationen */
/***************************/
extern tReceiver br_Receivers[];
extern tbool     br_FirstTime;
extern tString   br_Master;
extern int       br_BroadcastPort;
extern int       br_AcceptPort;

/*
//-----------------------------------------------------------------------------
//      Globale Daten
//-----------------------------------------------------------------------------
*/

/***************************************************/
/* Variablen f"ur den globalen Zustand des Systems */
/***************************************************/
int       Input;                 /* Socket, an dem Infos vom Leiter empfangen werden */

int       Hostpid  [MAXEXPERT];  /* Prozess-Ids aller Prozesse */

short     CycleCount  = 0;       /* aktuelle Zyklusnummer */

char      Execfile [MAXPATHLEN]; /* Name des ausf"uhrbaren Files */

short     ThisHost    = 0;       /* Nummer dieses Rechners im Gesamtsystem */
short     MasterHost  = 0;       /* Nummer des aktuellen Leiters */
short     WinnerHost  = 0;       /* Nummer des Siegers des letzten Zyklus */

                  /*****************************************/
                  /* Gesamtgr"o"se des Systems vor einem   */
                  /* Zyklus                                */
                  /*****************************************/
long      GesamtGroesse = 0;

/*********************************************************************/
/* Variablen, in denen der Experte seine aktuelle Einstellung ablegt */
/*********************************************************************/
/* Konfiguration des Experten f"ur den n"achsten Zyklus */
ExpertConfig    OwnConfig;

/* Parameterwerte f"ur die dem Experten zugeordnete Beurteilungs- und Auswahl- */
/* Funktion.                                                                   */
short           beurteile_ga_param[MAXPARAM];
short           resultate_ga_param[MAXPARAM];

/*******************************************************************************/
/* Variablen, die mit Informationen aus dem Konfigurationsfile versorgt werden */
/*******************************************************************************/
short     HostCount   = 1;           /* Anzahl der Rechner */

char      DataDir[MAXPATHLEN];       /* Pfad fuer alle folgenden Verzeichnisse */
char      ExpertDir[MAXPATHLEN];     /* Verzeichnis, in dem die Experten abgelegt sind */
char      RefereeDir[MAXPATHLEN];    /* Verzeichnis, in dem die Gutachter abgelegt sind */
char      DomainDir[MAXPATHLEN];     /* Verzeichnis, in dem die Dom"anen abgelegt sind */ 
char      ProtocolDir[MAXPATHLEN];   /* Verzeichnis, in dem alle Protokolle abgelegt werden */
extern 
char      KnowledgeBase[MAXPATHLEN]; /* Name des Verzeichnisses, das */
				     /* die Wissensbasis darstellt. */
				     /* Aus learn_files.c - steht */
				     /* hier nur, um den Zusammenhang */
				     /* deutlich zu machen. */


short     ExpertCount = 0;           /* Anzahl der in der Konfigurationsdatei angegebenen Experten */
short     RedSpecCount = 0 ;  /* Anzahl der in der Konfigurationsdatei angegebenen Reduktionsspezialisten */

long          CycleTime[MAXCYCLE]; /* Zykluszeiten aller Durchlaeufe    */

pairset       ReceivedEq = EmptySet; /* empfangene Gleichungen der Ga */

pairset       ReceivedGoals = EmptySet; /* empfangene Ziele der Ga */

/*
//-----------------------------------------------------------------------------
//      Lokale Daten
//-----------------------------------------------------------------------------
*/

static char      SupervisorFile[MAXPATHLEN];/* File, in dem die Planungsdaten stehen */

static char          problem_path[MAXPATHLEN] = "";  /* absolute path name  */

static char          *xtermopt = "+ls -sb -sl 1024 -s -fn 6x13 -j -si";

static char          Hostname [MAXEXPERT][MAXHOSTNAMELEN];

static char          hostname [MAXHOSTNAMELEN]; 

extern char          *Outfile;

static char          Filename[MAXPATHLEN]; /* Dateiname der Konfigurationsdatei */

static short         cfg_descr; /* Descriptor, der vom Scanner vergeben wird */

               /************************************************/
               /* Anzahl der als STATIC oder INITIAL dekla-    */
	       /* rierten Spezialisten                         */
               /************************************************/
static short         static_initial_spec_count = 0;

               /************************************************/
               /* true, wenn ein Reduktionsspezialist bereits  */
	       /* als STATIC oder INITIAL deklariert ist       */
               /************************************************/
static short         static_initial_ref = false;

/*
//-----------------------------------------------------------------------------
//      Lokale Funktionsdefinitionen
//-----------------------------------------------------------------------------
*/

static void  SyntaxError ( char *error );
static void  CheckError ( bool cond, char *msg );

static short ReadExperts ( Symbol sym );
static void  ParseConfig ( char *filename ); 

static void  Execute ( short number, char *host, char *file, char *cfgfile );

static void  Slave    ( void );

static void  Master   ( void );



/*
//-----------------------------------------------------------------------------
//  Funktion:       SyntaxError
//
//  Parameter:      error       Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung
//-----------------------------------------------------------------------------
*/

static void    SyntaxError ( char *error )
{
    PrintScanText ( cfg_descr );
    printf ( "****  Fehler in der eingelesenen Datei : %s.\n", Filename );
    printf ( "****  %s\n", error );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CheckError
//
//  Parameter:      cond    Bedingung fuer Fehlerfall
//                  msg     Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung, falls cond erfuellt.
//-----------------------------------------------------------------------------
*/

static void CheckError ( bool cond, char *msg )
{
    if (cond)
    {
        SyntaxError ( msg );
        CloseInput ( cfg_descr );
        exit ( 1 );
    }
}



/*
//=============================================================================
//      Einlesen der Konfigurationsdatei
//=============================================================================
*/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadExperts                                              */
/*                                                                           */
/*  Parameter    :  Typ der eingelesenen Experten (STATIC,INITIAL,SELECT)    */
/*                                                                           */
/*  Returnwert   :  letztes gelesenens Symbol                                */
/*                                                                           */
/*  Beschreibung :  Es werden alle Experten, die unter dem angegebenen Typ   */
/*                  in der Konfigurationsdatei stehen eingelesen. Dabei      */
/*                  werden sofort auch die Spezifikationen der Experten      */
/*                  und deren Gutachter eingelesen.                          */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static short ReadExperts ( Symbol typ )
{
    char          ident[IDENTLENGTH];
    short         sym;
    ExpertType    exp_type;
    ExpertFrame  *exp;

    SkipSpace( cfg_descr, ident, sym );

    do
    {
      CheckError( sym != SBRACKET_L, "( erwartet." );
      /**********************************/
      /* Experten/Spezialisten einlesen */
      /**********************************/
      CheckError( ((ExpertCount+1) >= MAXEXPERT), "Zuviele Experten." );

      SkipSpace( cfg_descr, ident, sym );
      CheckError( sym != SIDENT, "Identifyer (Expertendatei) erwartet.");

      exp_type = ParseExpert( ident, &(ExpertGlobalInfo[ExpertCount]) );

      /*****************************************/
      /* Reduktionsspezialist wurde eingelesen */
      /*****************************************/
      if( exp_type == RED )
      {
	/************************************************/
	/* Warnung, wenn initialer Reduktionsspezialist */
	/************************************************/
	if( typ == SINITIAL )
	{
	  printf("\n***\n");
	  printf("*** WARNUNG : Reduktionsspezialist als Initial gekennzeichnet!\n");
	  printf("***\n");
	}

	/**********************************************************/
	/* Nur ein statischer oder initialer Reduktionsspezialist */
	/**********************************************************/
	if( ( (typ == SINITIAL) || (typ == SSTATIC) ) )
	{
	  static_initial_spec_count++;

	  if( static_initial_ref )
	  {
	    CheckError( true, "Maximal ein Reduktionsspezialist als STATIC/INITIAL.");
	  }
	  else
	  {
	    static_initial_ref = true;
	  }
	}

	RedSpecInfo[RedSpecCount].typ = typ;

	RedSpecCount++;

	SkipSymbol( cfg_descr, ident, sym, SBRACKET_R, ")" );

	while ( ((sym = GetSymbol( cfg_descr, ident )) == SCRSYM) && !EOI( cfg_descr ) );
	/* Leerer while-Schleifenkoerper */

	continue;
      } /* Ende von if( exp_type == RED ) */

      /*******************************************************************/
      /* Unterscheidung zwischen Database-Experten und normalem Experten */
      /*******************************************************************/
      if( exp_type == DAT )
      {
	if( ( (typ == SINITIAL) || (typ == SSTATIC) ) )
	{
	  static_initial_spec_count++;
	}

	exp = &Database;
      }
      else
      {
	exp = &(ExpertGlobalInfo[ExpertCount]);
      }

      exp->typ = typ;

      /********************************************************/
      /* Einlesen der Ordnung ( d.h. welche der angegebenen ) */
      /********************************************************/
      CheckError ( (GetSymbol ( cfg_descr, ident) != SBRACKET_L), "( erwartet." );
      SkipSpace( cfg_descr, ident, sym );

      CheckError ( (sym != SIDENT) || !IsNumber (ident), "Ganzzahl erwartet." );
      exp->order = atoi (ident);

      SkipSymbol( cfg_descr, ident, sym, SBRACKET_R, ")" );
      SkipSpace( cfg_descr, ident, sym );

      /**********************/
      /* Gutachter einlesen */
      /**********************/
      CheckError( (sym != SIDENT) && (sym != SR_NONE),
		  "Identifyer (Gutachter-Datei) erwartet.");

      if( exp_type == DAT )
      {
	ParseRef( ident, &DatabaseRef );
      }
      else
      {
	ParseRef( ident, &(RefGlobalInfo[ExpertCount]) );
      }

      SkipSymbol( cfg_descr, ident, sym, SBRACKET_R, ")" );

      while ( ((sym = GetSymbol( cfg_descr, ident )) == SCRSYM) && !EOI( cfg_descr ) );
      /* Leerer while-Schleifenkoerper */

      if( exp_type == EXP )
      {
	ExpertCount++; 
      }
  } while (sym == SBRACKET_L);

  return sym;
} /* Ende von ReadExperts */





/*
//-----------------------------------------------------------------------------
//  Funktion:       ParseConfig ( char *filename )
//
//  Parameter:      filename    Dateiname der Konfiguartions-Datei
//
//  Beschreibung:   Einlesen der Konfiguration
//-----------------------------------------------------------------------------
*/

static void    ParseConfig ( char *filename )
{
    char        ident[IDENTLENGTH];
    Symbol      sym;

    strcpy( Filename, filename );

    cfg_descr = OpenInput ( filename );
    SetScanMode ( cfg_descr, CONFIG_MODE );

    /**********************/
    /* Einlesen der Hosts */
    /**********************/
    SkipSymbol( cfg_descr,ident, sym, SHOSTS, HOSTS );

    SkipSpace( cfg_descr, ident, sym);

    while ( sym == SIDENT )
    {

      /* Die Hostnamen und Hostcount werden nur vom Urprozess verarbeitet */
      if ( MODUS == MASTER )
      {
	/* Ueberpruefen, ob der eigene Rechner auch als Host angegeben wurde */
	CheckError( !strcmp(ident, hostname), "Eigenen Rechner als Host angegeben.");

        if (!strncpy (Hostname[HostCount++], ident, MAXHOSTNAMELEN))
        {
          Error ( __FILE__ ": "  "ParseConfig", "Hostname zu lang." );
        }
      } /* Ende von if ( MODUS == MASTER ) */

      SkipSpace( cfg_descr, ident, sym);

      /* Wenn ein Komma angegeben wurde, mu"s noch ein weiterer Experte angegeben werden */
      /* Ansonsten ist die Liste der Rechner zu Ende */
      if( sym == SCOMMA )
      {
	SkipSpace( cfg_descr, ident, sym);
      }
    } /* Ende von while ( sym == SIDENT ) */

    /**********************/
    /* Einlesen der Pfade */
    /**********************/
    CheckError( sym != SDATA_DIR, "DATA_DIR erwartet.");
    GetFilename( cfg_descr,  DataDir );

    SkipSpace( cfg_descr, ident, sym );
    if( sym == SEXPERT_DIR ) /* Pfade werden explizit angegeben */
    {
      GetFilename( cfg_descr,  ExpertDir );

      SkipSymbol( cfg_descr,ident, sym, SREFEREE_DIR, REFEREE_DIR);
      GetFilename( cfg_descr,  RefereeDir );

      SkipSymbol( cfg_descr,ident, sym, SDOMAIN_DIR, DOMAIN_DIR);
      GetFilename( cfg_descr,  DomainDir );

      SkipSymbol( cfg_descr,ident, sym, SKNOWLEDGE_DIR, KNOWLEDGE_DIR);
      GetFilename( cfg_descr,  KnowledgeBase );

      SkipSpace( cfg_descr, ident, sym );
      if( sym == SPROTOCOL_DIR )
      {
	SkipSymbol( cfg_descr,ident, sym, SPROTOCOL_DIR, PROTOCOL_DIR);
	GetFilename( cfg_descr,  ProtocolDir );

	SkipSpace( cfg_descr, ident, sym );
      } /* Ende von if( sym == SPROTOCOL_DIR ) */
      else
      {
	strcpy( ProtocolDir, DataDir );
	strcat( ProtocolDir, DEF_PROTOCOL_DIR );
      } /* Ende von else */
    } /* Ende von if( sym == SEXPERT_DIR ) */
    else /* Default-Directories werden genommen */
    {
      strcpy( ExpertDir, DataDir );
      strcat( ExpertDir, DEF_EXPERT_DIR );

      strcpy( RefereeDir, DataDir );
      strcat( RefereeDir, DEF_REFEREE_DIR );

      strcpy( DomainDir, DataDir );
      strcat( DomainDir, DEF_DOMAIN_DIR );

      strcpy( KnowledgeBase, DataDir );
      strcat( KnowledgeBase, DEF_KNOWLEDGE_DIR );


      strcpy( ProtocolDir, DataDir );
      strcat( ProtocolDir, DEF_PROTOCOL_DIR );
    } /* Ende von else - Default-Directories nehmen */

 /* Beachte: Hier ist das n"achste Symbol (STATIC, INITIAL oder SELECT) bereits gelesen */
    /*************************/
    /* Einlesen der Experten */
    /*************************/
    CheckError ( (sym != SSTATIC) && (sym != SINITIAL) && (sym != SSELECT),
                 "STATIC, INITIAL oder SELECT erwartet." );

    if (sym == SSTATIC)
    {
        sym = ReadExperts ( sym );
    }
    CheckError ((ExpertCount + static_initial_spec_count > HostCount), 
		"Mehr STATIC-Experten/Spezialisten als Hosts." );

    if (sym == SINITIAL)
    {
        sym = ReadExperts ( sym );
    }
    CheckError ((ExpertCount + static_initial_spec_count > HostCount), 
                "Mehr STATIC- und INITIAL-Experten/Spezialisten als Rechner." );

    CheckError( ((ExpertCount + static_initial_spec_count == HostCount) && !LeiterVorhanden ),
		 "Rechner durch STATIC/INITIAL-Experten/Spezialisten belegt, aber keiner darf Leiter werden.");

    CheckError( (static_initial_spec_count == HostCount),
		"Genausoviele Spezialisten wie Rechner als STATIC oder INITIAL.");

    if (sym == SSELECT)
    {
        sym = ReadExperts ( sym );
    }

    CheckError ((ExpertCount + (int)Database.spezialist + RedSpecCount < HostCount), 
                "Weniger Experten/Spezialisten als Rechner." );

    CheckError( !LeiterVorhanden, 
		"Kein Experte darf Leiter werden ( Gutachter-Spezifikation aendern ).");

    /* Beachte ReadExperts liest ein Zeichen weiter */
    /*********************************************************/
    /* Einlesen der Domaenen, nach denen gesucht werden soll */
    /*********************************************************/
    if( sym == SDOMAIN_SELECT )
    {
      while ( ((sym = GetSymbol( cfg_descr, ident )) == SCRSYM) && !EOI( cfg_descr ) );
      /* Leerer while-Schleifenkoerper */

      CheckError( !EOI( cfg_descr ) && ( sym != SIDENT ), "Domaenenname erwartet." );

      while ( !EOI( cfg_descr ) && (sym == SIDENT) )
      {
	SupposedDomains[SupposedDomainsCount++] = ParseDom(ident, SUPPOSED );

	do
	{
	  sym = GetSymbol( cfg_descr, ident );
	} while (!EOI( cfg_descr ) && ( sym == SCRSYM ) );
      } /* Ende von while ( !EOI( cfg_descr ) && (sym == SIDENT) ) */
    } /* Ende von if( sym == SDOMAIN_SELECT ) */

    if( !SupposedDomainsCount && Database.spezialist )
    {
      printf("\n***\n");
      printf("*** WARNUNG : Database-Experte angegeben, aber keine Domaenen, nach denen!\n");
      printf("              gesucht werden soll -> Database-Experte wird nicht eingesetzt!\n");
      printf("***\n");
    }


    /******************************************/
    /* Einlesen von TIMEBASE, falls angegeben */
    /******************************************/
    if( sym == STIMEBASE )
    {
      timebase_angegeben = true;

      CheckError ( ((sym = GetSymbol (cfg_descr, ident)) != SIDENT) && !IsNumber (ident),
		   "Ganzahl erwartet." );
      timebase = atoi (ident);
      CheckError ( (timebase <= 0), "Timebase muss groesser als Null sein !" );

      /* "Uberpr"ufen, ob Millisekunden angegeben sind und welcher Modus vorliegt */
      if( (sym = GetSymbol( cfg_descr, ident )) == SIDENT )
      {
	CheckError( strcmp( ident, "ms" ), "'ms' erwartet.");
	timeunit = MILLISECONDS;

        timemode = GetSymbol (cfg_descr, ident);
      } /* Ende von if( (timeunit ... */
      else
      {
	timemode = sym;
      }

      CheckError ( (timemode != SCONSTANT) && 
		   (timemode != SLINEAR) &&
		   (timemode != SEXPONENTIAL),
		    "CONSTANT, LINEAR oder EXPONENTIAL erwartet." ); 
      
      while ( ((sym = GetSymbol( cfg_descr, ident )) == SCRSYM) && !EOI( cfg_descr ) );
      /* Leerer while-Schleifenkoerper */
    } /* Ende von if( sym == STIMEBASE ) */

    /**************************************************************************/
    /* Einlesen des Supervisor-Files mit allen Planungsdaten, falls angegeben */
    /**************************************************************************/
    if( sym == SSUPERVISOR_FILE )
    {
      GetFilename( cfg_descr, SupervisorFile );

      /******************************/
      /* Einlesen der Planungsdatei */
      /******************************/
      ParsePlan( SupervisorFile );
    } 
    else
    {
      SetPlanParams();
    }

    while (!EOI( cfg_descr ))
    {
      CheckError ( (GetSymbol (cfg_descr, ident) != SCRSYM), "Dateiende erwartet." );
    }

    CloseInput ( cfg_descr );
} /* Ende von ParseConfig */


/*
//=============================================================================
//      Konfiguration einstellen.
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       SetConfiguration ( bool silence )
//
//  Parameter:      silence     true falls keine Ausgabe erwuenscht
//
//  Beschreibung:   Aktivieren einer Konfiguration, d.h. es wird festgelegt,
//                  welche CP-, welche CG-Funktion, welche Ordnung und 
//                  welche Parameter verwendet werden
//-----------------------------------------------------------------------------
*/

void  SetConfiguration ( bool silence )
{
   short   i;
   short   red_spec_idx;  /* Index eines Reduktionspezialisten in RedSpecInfo */
   
   special_factor = 0;         /* Sicher ist sicher !! */
   RulesOnly      = false;
   EquationsOnly  = false;
   
   if (!silence)
   {
      if ( !exp_ist_spezialist( OwnConfig.exp_nr ) )
      {
	 printf ( "    Konfiguration (Nummer: %d)   Experte: %s\n\n", OwnConfig.exp_nr, ExpertGlobalInfo[OwnConfig.exp_nr].exp_name );
      }
   }
   
   if(ReproMode)
   {
      if(OwnConfig.exp_nr == NO_CONFIG)
      {
	 if(!silence)
	 {
	    printf ( "    Experte:                 NO_CONFIG\n\n");
	    printf ( "    Process will sleep soon...\n\n");
	 }
	 return;
      }  
   }
   
   /* Es wird zunaechst ueberprueft, ob ein Spezialist vorliegt, weil diese */
   /* nicht in ExpertGlobalInfo enthalten sind.                             */
   /*******************************/
   /* Einstellen der Spezialisten */
   /*******************************/
   if ( exp_ist_spezialist( OwnConfig.exp_nr ) )
   {
      switch ( OwnConfig.exp_nr )
      {
      case DATABASE  : 
	 SetWeights ( 2, 1 );
	 RedInst = 1;
	 SetOrdering ( 0 );
	 SetCPWeight ( sum_tweight );
	 
	 if (silence)
	 {
	    return;
	 }
	 
	 printf ( "    Experte:                       DATABASE\n\n");
	 printf ( "    Es wird nach %d Domaenen gesucht :\n",
		 OwnConfig.cp_parameter[0] );
	 for ( i = 1; i<=OwnConfig.cp_parameter[0]; i++ )
	 {
	    printf( "        %s\n", DomainGlobalInfo[OwnConfig.cp_parameter[i]].dom_name );
	 }
	 printf("      Suchmodus ist : %s\n",
		OwnConfig.cg_parameter[0] == SFIND_ALL ?
		"FIND_ALL" : "FIND_FIRST" );
	 break;
	 
	case REDUCE_1  :
	case REDUCE_2  :
	case REDUCE_3  : 
	   red_spec_idx = GetRedSpecIdx( OwnConfig.exp_nr );
	 
	 if( RedSpecInfo[red_spec_idx].red_typ == CP )
	 {
	    StartCP   = OwnConfig.cp_parameter[0];
	    ReduceCPs = OwnConfig.cp_parameter[1];
	    SubsumCPs = OwnConfig.cp_parameter[2];
	    DoubleCPs = OwnConfig.cp_parameter[3];
	    
	    if( !silence )
	    {
	       printf ( "    Reduktionsspezialist: %s\n\n", RedSpecInfo[red_spec_idx].name);
	       if( RedSpecInfo[red_spec_idx].reduce_cp_fkt == SX_REDUCE_CP )
	       {
		  printf ( "    Starte mit krit. Paar No.:     %d\n", StartCP );
	       }
	       else
	       {
		  printf ( "    Starte bei krit. Paaren mit prozent. Anteil : %d\n", StartCP );
	       }
	       printf ( "    Reduktion kritischer Paare:    %s\n",
		       (ReduceCPs) ? "Ja" : "Nein" );
	       printf ( "    Ueberdeckung:                  %s\n",
		       (SubsumCPs) ? "Ja" : "Nein" );
	       printf ( "    Doppelte kritische Paare:      %s\n",
		       (DoubleCPs) ? "Ja" : "Nein" );
	    }
	 }
	 
	 if( RedSpecInfo[red_spec_idx].red_typ == CG )
	 {
	    StartCG   = OwnConfig.cp_parameter[0];
	    ReduceCGs = OwnConfig.cp_parameter[1];
	    SubsumCGs = OwnConfig.cp_parameter[2];
	    DoubleCGs = OwnConfig.cp_parameter[3];
	    
	    if( !silence )
	    {
	       printf ( "    Reduktionsspezialist: %s\n\n", RedSpecInfo[red_spec_idx].name);
	       if( RedSpecInfo[red_spec_idx].reduce_cp_fkt == SX_REDUCE_CG )
	       {
		  printf ( "    Starte mit krit. Ziel No.:     %d\n", StartCG );
	       }
	       else
	       {
		  printf ( "    Starte bei krit. Zielen mit prozent. Anteil : %d\n", StartCG );
	       }
	       printf ( "    Reduktion kritischer Ziele:    %s\n",
		       (ReduceCGs) ? "Ja" : "Nein" );
	       printf ( "    Ueberdeckung:                  %s\n",
		       (SubsumCGs) ? "Ja" : "Nein" );
	       printf ( "    Doppelte kritische Ziele:      %s\n",
		       (DoubleCGs) ? "Ja" : "Nein" );
	    }
	 }
	 
	 break;
	 
      default        : 
	 Error ( __FILE__ ": "  "SetConfiguration", "Unbekannter Spezialist." );
      } /* Ende von switch */
      
      return;
   } /* Ende von if ( exp_ist_spezialist( OwnConfig.exp_nr ) ) */
   
   /***************************/
   /* Einstellen der Experten */
   /***************************/
   /********************************/
   /* Einstellen der CP-Funktionen */
   /********************************/
   
   switch ( ExpertGlobalInfo[OwnConfig.exp_nr].cpfkt_nr )
   {
   case SX_ADD         : 
      SetWeights ( OwnConfig.cp_parameter[0], OwnConfig.cp_parameter[1] );
      RedInst = OwnConfig.cp_parameter[2];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( sum_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   ADD_WEIGHT\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja"
	      : "Nein" );
      break;

   case SX_FIFO:      
      SetCPWeight(fifo_weight);
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      if(silence)
      {
	 return;
      }
      printf ( "    CP-Funktion:                   FIFO\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      break;
   case SX_TEAM_FIFO:      
      SetCPWeight(team_fifo_weight);
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      if(silence)
      {
	 return;
      }
      printf ( "    CP-Funktion:                   TEAM_FIFO\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      break;
   case SX_GLOBAL_LEARN:
      if(OwnConfig.cp_parameter[0])
      {
	 SetCPWeight(CGlobalLearnWeight);
      }
      else
      {
	 SetCPWeight(LGlobalLearnWeight);
      }
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      RedInst = OwnConfig.cp_parameter[1];
      SetLearnEvalParams
	 (
	  OwnConfig.cp_parameter[2],
	  OwnConfig.cp_parameter[3],
	  OwnConfig.cp_parameter[4],
	  OwnConfig.cp_parameter[5],
	  OwnConfig.cp_parameter[6],
	  false,
	  false,
          3,
          2,
          UNK_BASE_DEFAULT,
	  DEFAULT_EVAL_FCT
	  );

      if(silence)
      {
	 return;
      }
      printf ( "    CP-Funktion:                   GLOBAL_LEARN \n");
      printf ( "                                   %s\n\n",
	      OwnConfig.cp_parameter[0] ? "Complete Data" : "Lemmas Only");
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      break;
   case SX_GOAL_BOUND_LEARN:
      if(OwnConfig.cp_parameter[0])
      {
	 SetCPWeight(CGoalBoundLearnWeight);
      }
      else
      {
	 SetCPWeight(LGoalBoundLearnWeight);
      }
      
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      RedInst = OwnConfig.cp_parameter[1];
      SetLearnEvalParams
	 (
	  OwnConfig.cp_parameter[2],
	  OwnConfig.cp_parameter[3],
	  OwnConfig.cp_parameter[4],
	  OwnConfig.cp_parameter[5],
	  OwnConfig.cp_parameter[6],
	  OwnConfig.cp_parameter[7],
	  false,
	  3,
          2,
          UNK_BASE_DEFAULT,
	  DEFAULT_EVAL_FCT
	  );

      if(silence)
      {
	 return;
      }
      printf ( "    CP-Funktion:                   GOAL_BOUND_LEARN \n");
      printf ( "                                   %s\n\n",
	      OwnConfig.cp_parameter[0] ? "Complete Data" : "Lemmas Only");
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      break;
   case SX_SPEC_BOUND_LEARN:
      if(OwnConfig.cp_parameter[0])
      {
	 SetCPWeight(CSpecBoundLearnWeight);
      }
      else
      {
	 SetCPWeight(LSpecBoundLearnWeight); /* Aendern wenns laeuft! */
      }
      
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      RedInst = OwnConfig.cp_parameter[1];
      SetLearnEvalParams
	 (
	  OwnConfig.cp_parameter[2],
	  OwnConfig.cp_parameter[3],
	  OwnConfig.cp_parameter[4],
	  OwnConfig.cp_parameter[5],
	  OwnConfig.cp_parameter[6],
	  OwnConfig.cp_parameter[7],
	  OwnConfig.cp_parameter[8],
          3,
          2,
          UNK_BASE_DEFAULT,
	  DEFAULT_EVAL_FCT
	  );

      if(silence)
      {
	 return;
      }
      printf ( "    CP-Funktion:                   SPEC_BOUND_LEARN \n");
      printf ( "                                   %s\n\n",
	      OwnConfig.cp_parameter[0] ? "Complete Data" : "Lemmas Only");
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      break;
    case SX_2TET_LEARN: 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( C2TETLearnWeight );
      SetLearnEvalParams
	 (
	  aggressive,
	  1,
	  0,
	  0,
          1,
          false,
          false,
          OwnConfig.cp_parameter[1],
          OwnConfig.cp_parameter[2],
          UNK_BASE_DEFAULT,
	  DEFAULT_EVAL_FCT
	  );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   C2TETLearnWeight\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja"
	      : "Nein" );
      break;
    case SX_1TET_LEARN: 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( C1TETLearnWeight );
      SetLearnEvalParams
	 (
	  aggressive,
	  1,
	  0,
	  OwnConfig.cp_parameter[1],
          1,
          false,
          false,
          OwnConfig.cp_parameter[2],
          OwnConfig.cp_parameter[3],
          UNK_BASE_DEFAULT,
	  DEFAULT_EVAL_FCT
	  );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   C1TETLearnWeight\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja"
	      : "Nein" );
      break;
   case SX_MAX         : 
      SetWeights ( OwnConfig.cp_parameter[0], OwnConfig.cp_parameter[1] );
      RedInst = OwnConfig.cp_parameter[2];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( max_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   MAX_WEIGHT\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja"
	      : "Nein" );
      break;
      
   case SX_OCCNEST     : 
      SetWeights ( OwnConfig.cp_parameter[0], OwnConfig.cp_parameter[1] );
      RedInst = OwnConfig.cp_parameter[2];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( occnest );
      considerAll = (OwnConfig.cp_parameter[3]) ? true : false;

      if (silence)
	 return;
      
      printf ( "    CP-Funktion:                   OCCNEST\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja"
	      : "Nein" );
      if (considerAll)
	 printf ( "    Alle Funktionssymbole werden beruecksichtigt.\n" );
      else
	 printf ( "    Nur im Goal vorkommende Funktionssymbole werden beruecksichtigt.\n" );
      break;
      
   case SX_GT          : 
      SetWeights ( OwnConfig.cp_parameter[0], OwnConfig.cp_parameter[1] );
      RedInst = OwnConfig.cp_parameter[2];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( gt_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   GT_WEIGHT\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja" 
	      : "Nein" );
      break;
      
   case SX_DIFF        : 
      SetWeights ( OwnConfig.cp_parameter[0], OwnConfig.cp_parameter[1] );
      RedInst = OwnConfig.cp_parameter[2];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      DiffFactor = OwnConfig.cp_parameter[4];
      MaxFactor  = (OwnConfig.cp_parameter[5]) ?  1 : -1;
      SetCPWeight ( diff_weight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   DIFF_WEIGHT\n\n");
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja"
	      : "Nein" );
      printf ( "    Factor fuer Differenz:         %4d\n",
	      OwnConfig.cp_parameter[4] );
      printf ( "    Suche nach:                    %s\n",
	      (MaxFactor == -1) ? "Maximale Differenz"
	      : "Minimale Differenz" );
      break;
      
      
   case SX_SPEC_ADD    : 
      special_factor = OwnConfig.cp_parameter[0];
      SetWeights ( OwnConfig.cp_parameter[1],
		  OwnConfig.cp_parameter[2] );
      RedInst = OwnConfig.cp_parameter[3];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( sum_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   SPEC_ADD\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Bewertung von speziellen Gl.:  %4d %%\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[2] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[3]) ? "Ja" 
	      : "Nein" );
      break;
      
   case SX_SPEC_MAX    : 
      special_factor = OwnConfig.cp_parameter[0];
      SetWeights ( OwnConfig.cp_parameter[1],
		  OwnConfig.cp_parameter[2] );
      RedInst = OwnConfig.cp_parameter[3];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( max_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   SPEC_MAX\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Bewertung von speziellen Gl.:  %4d %%\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[2] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[3]) ? "Ja" 
	      : "Nein" );
      break;
      
   case SX_SPEC_GT     : 
      special_factor = OwnConfig.cp_parameter[0];
      SetWeights ( OwnConfig.cp_parameter[1],
		  OwnConfig.cp_parameter[2] );
      RedInst = OwnConfig.cp_parameter[3];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( gt_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   SPEC_GT\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    Bewertung von speziellen Gl.:  %4d %%\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[2] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[3]) ? "Ja" 
	      : "Nein" );
      break;
      
      
   case SX_KBO_ADD     : 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( sum_kbo );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   ADD_KBO\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja" 
	      : "Nein" );
      
      break;
      
   case SX_KBO_MAX     : 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( max_kbo );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   MAX_KBO\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja" 
	      : "Nein" );
      
      break;
      
   case SX_KBO_GT      : 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( gt_kbo );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   GT_KBO\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja" 
	      : "Nein" );
      
      break;
      
   case SX_ADD_RWEIGHT :  
   case SX_ADD_EWEIGHT :  
   case SX_ADD_FWEIGHT : 
      RedInst = OwnConfig.cp_parameter[2];
      RulesOnly     = ( ExpertGlobalInfo[OwnConfig.exp_nr].cpfkt_nr == SX_ADD_RWEIGHT );
      EquationsOnly = ( ExpertGlobalInfo[OwnConfig.exp_nr].cpfkt_nr == SX_ADD_EWEIGHT );
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( sum_tfweight );
      /* Festlegen des Gewichts fuer Variablen */
      Function[0].cpweight = OwnConfig.cp_parameter[1];
      /* Gewicht fuer Funktionssymbole */
      for (i = 1; i <= FuncCount; 
	   Function[i++].cpweight = OwnConfig.cp_parameter[0] );
      /* Leerer for-schleifenkoerper */
      
      /* Festlegen der speziellen Gewichte fuer Funktionssymbole*/
      i = 3;
      while ( i<ExpertGlobalInfo[OwnConfig.exp_nr].cpparam_anz )
      {
	 Function[OwnConfig.cp_parameter[i]].cpweight
	    = OwnConfig.cp_parameter[i+1];
	 i += 2;
      }
      
      if (silence)
      {
	 return;
      }
      
      if (RulesOnly)
      {
	 printf ( "    CP-Funktion:                   ADD_RWEIGHT\n\n" );
      }
      else if (EquationsOnly)
      {
	 printf ( "    CP-Funktion:                   ADD_EWEIGHT\n\n" );
      }
      else
      {
	 printf ( "    CP-Funktion:                   ADD_FWEIGHT\n\n" );
      }
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja" 
	      : "Nein" );
      
      printf ( "    Gewichte:   Funktionen:\n" );
      for (i = 1; i <= FuncCount; i++)
      {
	 printf ( "                %-10s:         %4d\n",
		 Function[i].ident, Function[i].cpweight );
      }
      
      break;
      
   case SX_MAX_RWEIGHT:  
   case SX_MAX_EWEIGHT:  
   case SX_MAX_FWEIGHT: 
      RedInst = OwnConfig.cp_parameter[2];
      RulesOnly     = ( ExpertGlobalInfo[OwnConfig.exp_nr].cpfkt_nr == SX_MAX_RWEIGHT );
      EquationsOnly = ( ExpertGlobalInfo[OwnConfig.exp_nr].cpfkt_nr == SX_MAX_EWEIGHT );
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( max_tfweight );
      /* Festlegen des Gewichts fuer Variablen */
      Function[0].cpweight = OwnConfig.cp_parameter[1];
      /* Gewicht fuer Funktionssymbole */
      for (i = 1; i <= FuncCount; 
	   Function[i++].cpweight = OwnConfig.cp_parameter[0] );
      /* Leerer for-schleifenkoerper */
      
      /* Festlegen der speziellen Gewichte fuer Funktionssymbole*/
      i = 3;
      while ( i<ExpertGlobalInfo[OwnConfig.exp_nr].cpparam_anz )
      {
	 Function[OwnConfig.cp_parameter[i]].cpweight
	    = OwnConfig.cp_parameter[i+1];
	 i += 2;
      }
      
      if (silence)
      {
	 return;
      }
      
      if (RulesOnly)
      {
	 printf ( "    CP-Funktion:                   MAX_RWEIGHT\n\n" );
      }
      else if (EquationsOnly)
      {
	 printf ( "    CP-Funktion:                   MAX_EWEIGHT\n\n" );
      }
      else
      {
	 printf ( "    CP-Funktion:                   MAX_FWEIGHT\n\n" );
      }
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja" 
	      : "Nein" );
      
      printf ( "    Gewichte:   Funktionen:\n" );
      for (i = 1; i <= FuncCount; i++)
      {
	 printf ( "                %-10s:         %4d\n",
		 Function[i].ident, Function[i].cpweight );
      }
      
      break;
      
   case SX_GT_RWEIGHT:  
   case SX_GT_EWEIGHT:  
   case SX_GT_FWEIGHT:  
      RedInst = OwnConfig.cp_parameter[2];
      RulesOnly     = ( ExpertGlobalInfo[OwnConfig.exp_nr].cpfkt_nr == SX_GT_RWEIGHT );
      EquationsOnly = ( ExpertGlobalInfo[OwnConfig.exp_nr].cpfkt_nr == SX_GT_EWEIGHT );
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( gt_tfweight );
      /* Festlegen des Gewichts fuer Variablen */
      Function[0].cpweight = OwnConfig.cp_parameter[1];
      /* Gewicht fuer Funktionssymbole */
      for (i = 1; i <= FuncCount; 
	   Function[i++].cpweight = OwnConfig.cp_parameter[0] );
      /* Leerer for-Schleifenkoerper */
      
      /* Festlegen der speziellen Gewichte fuer Funktionssymbole*/
      i = 3;
      while ( i<ExpertGlobalInfo[OwnConfig.exp_nr].cpparam_anz )
      {
	 Function[OwnConfig.cp_parameter[i]].cpweight
	    = OwnConfig.cp_parameter[i+1];
	 i += 2;
      }
      
      if (silence)
      {
	 return;
      }
      
      if (RulesOnly)
      {
	 printf ( "    CP-Funktion:                   GT_RWEIGHT\n\n" );
      }
      else if (EquationsOnly)
      {
	 printf ( "    CP-Funktion:                   GT_EWEIGHT\n\n" );
      }
      else
      {
	 printf ( "    CP-Funktion:                   GT_FWEIGHT\n\n" );
      }
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[2]) ? "Ja" : "Nein" );
      
      printf ( "    Gewichte:   Funktionen:\n" );
      for (i = 1; i <= FuncCount; i++)
      {
	 printf ( "                %-10s:         %4d\n",
		 Function[i].ident, Function[i].cpweight );
      }
      
      break;
      
   case SX_POLY_ADD    :
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      Function[0].cpweight = OwnConfig.cp_parameter[2];
      
      SetCPWeight ( sum_poly );
      
      for (i = 1; i <= FuncCount; i++)
      {
	 Function[OwnConfig.cp_parameter[2*i+1]].poly
	    = (polynom *)OwnConfig.cp_parameter[2*i+2];
      }
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   ADD_POLYNOM\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja"
	      : "Nein" );
      printf ( "    Polynome:\n" );
      
      for (i = 1; i <= FuncCount; i++)
      {
	 printf ( "      %-10s: ",
		 Function[i].ident );
	 printpoly ( Function[i].poly );
	 printf ( "\n" );
      }
      break;
      
   case SX_POLY_MAX    : 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      Function[0].cpweight = OwnConfig.cp_parameter[2];
      
      SetCPWeight ( max_poly );
      
      for (i = 1; i <= FuncCount; i++)
      {
	 Function[OwnConfig.cp_parameter[2*i+1]].poly
	    = (polynom *)OwnConfig.cp_parameter[2*i+2];
      }
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   MAX_POLYNOM\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja"
	      : "Nein" );
      printf ( "    Polynome:\n" );
      
      for (i = 1; i <= FuncCount; i++)
      {
	 printf ( "      %-10s: ",
		 Function[i].ident );
	 printpoly ( Function[i].poly );
	 printf ( "\n" );
      }
      break;
      
   case SX_POLY_GT     : 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      Function[0].cpweight = OwnConfig.cp_parameter[2];
      
      SetCPWeight ( gt_poly );
      
      for (i = 1; i <= FuncCount; i++)
      {
	 Function[OwnConfig.cp_parameter[2*i+1]].poly
	    = (polynom *)OwnConfig.cp_parameter[2*i+2];
      }
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   GT_POLYNOM\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja"
	      : "Nein" );
      printf ( "    Polynome:\n" );
      
      for (i = 1; i <= FuncCount; i++)
      {
	 printf ( "      %-10s: ",
		 Function[i].ident );
	 printpoly ( Function[i].poly );
	 printf ( "\n" );
      }
      break;
      
   case SX_GOALMATCH   :
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      
      SetCPWeight ( goalmatch );
      NoMatchWeight      = OwnConfig.cp_parameter[1];
      DoubleMatchWeight  = OwnConfig.cp_parameter[2];
      SingleMatchWeight  = OwnConfig.cp_parameter[3];
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   GOALMATCH\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja"
	      : "Nein" );
      printf ( "\n" );
      printf ( "    Kein Match     :               %4ld\n",
	      NoMatchWeight );
      printf ( "    Einfacher Match:               %4ld\n",
	      SingleMatchWeight );
      printf ( "    Doppelter Match:               %4ld\n",
	      DoubleMatchWeight );
      printf ( "\n" );
      break;
      
   case SX_GOALSIM     : 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      
      SetCPWeight ( goalsim );
      NumberOfFunctions  = OwnConfig.cp_parameter[ 1];
      NumberOfVariables  = OwnConfig.cp_parameter[ 2];
      NoMatchWeight      = OwnConfig.cp_parameter[ 3];
      DoubleMatchWeight  = OwnConfig.cp_parameter[ 4];
      SingleMatchWeight  = OwnConfig.cp_parameter[ 5];
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   GOALSIM\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja"
	      : "Nein" );
      printf ( "\n" );
      printf ( "    Mindestanzahl Funktionen:      %4ld\n",
	      NumberOfFunctions );
      printf ( "                  Variablen:       %4ld\n",
	      NumberOfVariables );
      printf ( "    Kein Match     :               %4ld\n",
	      NoMatchWeight );
      printf ( "    Einfacher Match:               %4ld\n",
	      SingleMatchWeight );
      printf ( "    Doppelter Match:               %4ld\n",
	      DoubleMatchWeight );
      printf ( "\n" );
      break;
      
   case SX_GOALTEST    : 
      RedInst = OwnConfig.cp_parameter[0];
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      
      SetCPWeight ( goaltest );
      NoMatchWeight      = OwnConfig.cp_parameter[ 2];
      DoubleMatchWeight  = OwnConfig.cp_parameter[ 3];
      SingleMatchWeight  = OwnConfig.cp_parameter[ 4];
      DoubleUnifyWeight  = OwnConfig.cp_parameter[ 5];
      SingleUnifyWeight  = OwnConfig.cp_parameter[ 6];
      GoalMatch          = OwnConfig.cp_parameter[ 7];
      GDoubleMatchWeight = OwnConfig.cp_parameter[ 8];
      GSingleMatchWeight = OwnConfig.cp_parameter[ 9];
      GDoubleUnifyWeight = OwnConfig.cp_parameter[10];
      GSingleUnifyWeight = OwnConfig.cp_parameter[11];
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   GOALTEST\n\n" );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      printf ( "    RedInst:                       %s\n",
	      (OwnConfig.cp_parameter[0]) ? "Ja"
	      : "Nein" );
      printf ( "\n" );
      printf ( "    Doppelter Match:               %4ld\n",
	      DoubleMatchWeight );
      printf ( "    Einfacher Match:               %4ld\n",
	      SingleMatchWeight );
      printf ( "    Doppelte Unifikation:          %4ld\n",
	      DoubleUnifyWeight );
      printf ( "    Einfache Unifikation:          %4ld\n",
	      SingleUnifyWeight );
      printf ( "\n" );
      printf ( "    Ziel-Match:                    %s\n",
	      (GoalMatch) ? "Ja" : "Nein" );
      printf ( "    Doppelter Match:               %4ld\n",
	      GDoubleMatchWeight );
      printf ( "    Einfacher Match:               %4ld\n",
	      GSingleMatchWeight );
      printf ( "    Doppelte Unifikation:          %4ld\n",
	      GDoubleUnifyWeight );
      printf ( "    Einfache Unifikation:          %4ld\n",
	      GSingleUnifyWeight );
      printf ( "\n" );
      break;
      
   case SX_DIVERGENCE:   
      SetOrdering ( ExpertGlobalInfo[OwnConfig.exp_nr].order );
      SetCPWeight ( divergence );
      for (i = 1; i < MAXFUNCTION; 
	   Function[i++].cpweight = OwnConfig.cp_parameter[1]);
      /* Leerer for-Schleifenkoerper */
      
      i = 1;
      while (OwnConfig.cp_parameter[i])
      {
	 Function[OwnConfig.cp_parameter[i]].cpweight = -1;
	 i++;
      }
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CP-Funktion:                   DIVERGENCE\n\n " );
      printf ( "    Ordnung:                       %4d\n",
	      ExpertGlobalInfo[OwnConfig.exp_nr].order );
      
      printf ( "    Gewichte:   Funktionen:\n" );
      for (i = 1; i <= FuncCount; i++)
      {
	 printf ( "                %-10s:         %4d\n",
		 Function[i].ident, Function[i].cpweight );
      }
      
      break;
      
   default:             
      Error ( __FILE__ ": "  "SetConfiguration", "Unbekannte Funktion fuer kritische Paare." );
      break;
   } /* Ende von switch, Einstellen der CP-Funktion */
   printf ( "\n" );
   flush ();
   LastCPCount = (RefGlobalInfo[OwnConfig.exp_nr].beurteile_ga == SR_FEELGOOD) ? 
      RefGlobalInfo[OwnConfig.exp_nr].beurt_paramliste[REF_LAST_CP].normal : 0;
   
   /***********************************************************************/
   /* Einstellen der CG-Funktionen, falls Paramodulation angegeben wurde. */
   /***********************************************************************/
   if ( !Paramodulation )
   {
      return;
   }
   
   switch ( ExpertGlobalInfo[OwnConfig.exp_nr].cgfkt_nr )
   {
   case SX_UNIF_GOAL   : 
      SetCGWeights ( OwnConfig.cg_parameter[0], OwnConfig.cg_parameter[1] );
      MinWeight = OwnConfig.cg_parameter[2];
      SetCGFunc ( unif_goal );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CG-Funktion :                  UNIF_GOAL\n\n" );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cg_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cg_parameter[1] );
      printf ( "    Mindeststruktur :              %4d\n",
	      OwnConfig.cg_parameter[2] );
      break;
   case SX_DIFF_GOAL   :
      SetCGWeights ( OwnConfig.cg_parameter[0], OwnConfig.cg_parameter[1] );
      NoUnifyFactor = OwnConfig.cg_parameter[2];
      SetCGFunc ( diff_goal );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CG-Funktion :                  DIFF_GOAL\n\n" );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cg_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cg_parameter[1] );
      printf ( "    NoUnifyFactor :                %4d\n",
	      OwnConfig.cg_parameter[2] );
      break;
      
   case SX_ADD         : 
      SetCGWeights ( OwnConfig.cg_parameter[0], OwnConfig.cg_parameter[1] );
      SetCGFunc ( sum_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CG-Funktion :                  ADD_WEIGHT\n\n" )
	 ;
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cg_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cg_parameter[1] );
      break;
      
   case SX_MAX         : 
      SetCGWeights ( OwnConfig.cg_parameter[0], OwnConfig.cg_parameter[1] );
      SetCGFunc ( max_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CG-Funktion :                  MAX_WEIGHT\n\n" )
	 ;
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      break;
      
   case SX_GT          : 
      SetCGWeights ( OwnConfig.cg_parameter[0], OwnConfig.cg_parameter[1] );
      SetCGFunc ( gt_tweight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CG-Funktion :                  GT_WEIGHT\n\n" );
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      break;
      
      
      
   case SX_DIFF        : 
      SetCGWeights ( OwnConfig.cg_parameter[0], OwnConfig.cg_parameter[1] );
      DiffFactor = OwnConfig.cg_parameter[4];
      MaxFactor  = (OwnConfig.cg_parameter[5]) ?  1 : -1;
      SetCGFunc ( diff_weight );
      
      if (silence)
      {
	 return;
      }
      
      printf ( "    CG-Funktion :                  DIFF_WEIGHT\n\n")
	 ;
      printf ( "    Gewichte:   Funktionen:        %4d\n",
	      OwnConfig.cp_parameter[0] );
      printf ( "                Variablen:         %4d\n",
	      OwnConfig.cp_parameter[1] );
      printf ( "    Factor fuer Differenz:         %4d\n",
	      OwnConfig.cp_parameter[4] );
      printf ( "    Suche nach:                    %s\n",
	      (MaxFactor == -1) ? "Maximale Differenz"
	      : "Minimale Differenz" );
      break;
      
      
   default             : 
      Error ( __FILE__ ": "  "SetConfiguration", "Unbekannte Funktion fuer kritische Ziele." );
      break;
   } /* Ende von switch, Einstellen der CG-Funktion */
} /* Ende von SetConfiguration */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SetRefParams                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion belegt die Parameterwerte fuer die Gut-   */
/*                  achter. Bis jetzt werden nur die angegebenen Default-    */
/*                  Werte uebergeben.                                        */
/*                  Beachte : Spezialisten bekommen keinen Gutachter.        */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

void SetRefParams ( void )
{
   RefFrame *ref;
   
   /*************************************/
   /* Sonderbehandlung bei Spezialisten */
   /*************************************/
   if ( exp_ist_spezialist( OwnConfig.exp_nr ) )
   {
      return;
   }
   
   ref = &(RefGlobalInfo[OwnConfig.exp_nr]);
   /*********************************************************/
   /* Versorgen der Parameter der BeurteileExpertenFunktion */
   /*********************************************************/
   switch ( ref->beurteile_ga )
   {
   case SR_STATISTIC   :
      beurteile_ga_param[REF_SIZE_R] = ref->beurt_paramliste[REF_SIZE_R].normal;
      beurteile_ga_param[REF_SIZE_E] =ref->beurt_paramliste[REF_SIZE_E].normal;
      beurteile_ga_param[REF_SIZE_CP] =ref->beurt_paramliste[REF_SIZE_CP].normal;
      beurteile_ga_param[REF_SIZE_G] =ref->beurt_paramliste[REF_SIZE_G].normal;
      beurteile_ga_param[REF_SIZE_CG] =ref->beurt_paramliste[REF_SIZE_CG].normal;
      beurteile_ga_param[REF_REDCOUNT] =ref->beurt_paramliste[REF_REDCOUNT].normal;
      beurteile_ga_param[REF_NOMASTER] =ref->beurt_paramliste[REF_NOMASTER].normal;
      break;
      
   case SR_EXTENDED    : 
      beurteile_ga_param[REF_SIZE_R] = ref->beurt_paramliste[REF_SIZE_R].normal;
      beurteile_ga_param[REF_SIZE_E] = ref->beurt_paramliste[REF_SIZE_E].normal;
      beurteile_ga_param[REF_SIZE_CP] = ref->beurt_paramliste[REF_SIZE_CP].normal;
      beurteile_ga_param[REF_SIZE_G] = ref->beurt_paramliste[REF_SIZE_G].normal;
      beurteile_ga_param[REF_SIZE_CG] = ref->beurt_paramliste[REF_SIZE_CG].normal;
      beurteile_ga_param[REF_NEW_R] = ref->beurt_paramliste[REF_NEW_R].normal;
      beurteile_ga_param[REF_RED_R] = ref->beurt_paramliste[REF_RED_R].normal;
      beurteile_ga_param[REF_DEL_R] = ref->beurt_paramliste[REF_DEL_R].normal;
      beurteile_ga_param[REF_NEW_E] = ref->beurt_paramliste[REF_NEW_E].normal;
      beurteile_ga_param[REF_RED_E] = ref->beurt_paramliste[REF_RED_E].normal;
      beurteile_ga_param[REF_DEL_E] = ref->beurt_paramliste[REF_DEL_E].normal;
      beurteile_ga_param[REF_NEW_CP] = ref->beurt_paramliste[REF_NEW_CP].normal;
      beurteile_ga_param[REF_DEL_CP] = ref->beurt_paramliste[REF_DEL_CP].normal;
      beurteile_ga_param[REF_NEW_G] = ref->beurt_paramliste[REF_NEW_G].normal;
      beurteile_ga_param[REF_RED_G] = ref->beurt_paramliste[REF_RED_G].normal;
      beurteile_ga_param[REF_NEW_CG] = ref->beurt_paramliste[REF_NEW_CG].normal;
      beurteile_ga_param[REF_REDCOUNT] = ref->beurt_paramliste[REF_REDCOUNT].normal;
      beurteile_ga_param[REF_NOMASTER] =ref->beurt_paramliste[REF_NOMASTER].normal;
      break;
      
   case SR_FEELGOOD    : 
      beurteile_ga_param[REF_LAST_CP] = ref->beurt_paramliste[REF_LAST_CP].normal;
      beurteile_ga_param[REF_LAST_CG] = ref->beurt_paramliste[REF_LAST_CG].normal;
      beurteile_ga_param[REF_PAST_FAC] = ref->beurt_paramliste[REF_PAST_FAC].normal;
      beurteile_ga_param[REF_NEXT_CP] = ref->beurt_paramliste[REF_NEXT_CP].normal;
      beurteile_ga_param[REF_NEXT_CG] = ref->beurt_paramliste[REF_NEXT_CG].normal;
      beurteile_ga_param[REF_FUTURE_FAC] = ref->beurt_paramliste[REF_FUTURE_FAC].normal;
      beurteile_ga_param[REF_CORRECCT_FAC] = ref->beurt_paramliste[REF_CORRECCT_FAC].normal;
      beurteile_ga_param[REF_NOMASTER] =ref->beurt_paramliste[REF_NOMASTER].normal;
      break;
   default             : 
      Error ( __FILE__ ": " "SetRefParams", "Unbekannte BeurteileExpertenFunktion." );
      break;
   }
   
   /**********************************************************/
   /* Versorgen der Parameter der GuteResultateFindeFunktion */
   /**********************************************************/
   switch( RefGlobalInfo[OwnConfig.exp_nr].resultate_ga )
   {
   case SS_LAST        : 
      resultate_ga_param[REF_MAX_RULE] = ref->res_paramliste[REF_MAX_RULE].normal;
      resultate_ga_param[REF_MAX_EQU] = ref->res_paramliste[REF_MAX_EQU].normal;
      resultate_ga_param[REF_MAX_GOAL] = ref->res_paramliste[REF_MAX_GOAL].normal;
      resultate_ga_param[REF_RULE_LOW] = ref->res_paramliste[REF_RULE_LOW].normal;
      resultate_ga_param[REF_EQU_LOW] = ref->res_paramliste[REF_EQU_LOW].normal;
      resultate_ga_param[REF_GOAL_LOW] = ref->res_paramliste[REF_GOAL_LOW].normal;
      break;
      
   case SR_STATISTIC   : 
      resultate_ga_param[REF_MAX_RULE] = ref->res_paramliste[REF_MAX_RULE].normal;
      resultate_ga_param[REF_MAX_EQU] = ref->res_paramliste[REF_MAX_EQU].normal;
      resultate_ga_param[REF_MAX_GOAL] = ref->res_paramliste[REF_MAX_GOAL].normal;
      resultate_ga_param[REF_RULE_LOW] = ref->res_paramliste[REF_RULE_LOW].normal;
      resultate_ga_param[REF_EQU_LOW] = ref->res_paramliste[REF_EQU_LOW].normal;
      resultate_ga_param[REF_GOAL_LOW] = ref->res_paramliste[REF_GOAL_LOW].normal;
      resultate_ga_param[REF_RED_COUNT] = ref->res_paramliste[REF_RED_COUNT].normal;
      resultate_ga_param[REF_RED_RIGHT] = ref->res_paramliste[REF_RED_RIGHT].normal;
      resultate_ga_param[REF_RED_LEFT] = ref->res_paramliste[REF_RED_LEFT].normal;
      resultate_ga_param[REF_RED_EQU] = ref->res_paramliste[REF_RED_EQU].normal;
      resultate_ga_param[REF_RED_GOAL] = ref->res_paramliste[REF_RED_GOAL].normal;
      resultate_ga_param[REF_SUBSUM] = ref->res_paramliste[REF_SUBSUM].normal;
      resultate_ga_param[REF_CP_COUNT] = ref->res_paramliste[REF_CP_COUNT].normal;
      resultate_ga_param[REF_CG_COUNT] = ref->res_paramliste[REF_CG_COUNT].normal;
      break;
      
   default             : 
      Error ( __FILE__ ": " "SetRefParams", "Unbekannte GuteResultateFindeFunktion." );
      break;
   }
}




/*
//=====================================================================================
//      Funktionen zur Verwaltung des Teams
//=====================================================================================
*/

/*
//-------------------------------------------------------------------------------------
//  Funktion:       Execute ( short number, char *host, char *file, char *cfgfile )
//
//  Parameter:      number      Kennnummer des Prozesses
//                  host        Name des Rechners
//                  file        Dateiname der Problemdatei
//                  cfgfile     Dateiname der Konfigurationsdatei
//
//  Beschreibung:   Starten eines Experten auf dem Rechner host
//-------------------------------------------------------------------------------------
*/

static void  Execute ( short number, char *host, char *file, char *cfgfile )
{
    char        syscommand [MAXPATHLEN];
    char        path [MAXPATHLEN];
    char        *display;
    char        cfg[MAXPATHLEN];

    getcwd ( path, MAXPATHLEN );
    if (!strncmp (path, "/tmp_mnt", 8))                 /* Automounter ! */
        strcpy ( path, (char*)((long)((&path))+8) );

    /*******************************************************/
    /* kompletten Pfad fuers Konfigurationsdatei bestimmen */
    /*******************************************************/
    sprintf(cfg, "%s/%s", path, cfgfile );

    if (NullMode)
        sprintf ( syscommand, "%s \"%s -XPERT %s %d %d %s> /dev/null \" &", 
                               host, Execfile,
                               hostname, br_AcceptPort, br_BroadcastPort, cfg );
    else if (SilentMode)
        sprintf ( syscommand, "%s \"%s -XPERT %s %d %d %s> %s/%s.%d.%s \" &", 
                               host, Execfile,
                               hostname, br_AcceptPort, br_BroadcastPort, cfg,
                               path, (Outfile) ? Outfile : file, number, host);
    else
    {
        display = (char *)getenv ( "DISPLAY" );
        if (!display)
            Error ( __FILE__ ": "  "Execute", "Environmentvariable DISPLAY nicht gesetzt." );
        sprintf ( syscommand, "xterm %s -d %s -T %s -g 80x25+4+%d -e %s %s -XPERT %s %d %d %s &", 
                               xtermopt, display,
                               host, number*340-250,
                               host, Execfile,
                               hostname, br_AcceptPort, br_BroadcastPort, cfg );
    }
    system ( syscommand );

}



/*
//=====================================================================================
//      Starten des Teams
//=====================================================================================
*/

/*
//-------------------------------------------------------------------------------------
//  Funktion:       StartXpert ( char *chost, char *cfgfile )
//
//  Parameter:      chost       Name des Hosts auf dem der Starter laeuft
//                  cfgfile     Name der Konfigurationsdatei
//
//  Beschreibung:   Hochfahren eines Experten.
//-------------------------------------------------------------------------------------
*/

void    StartXpert ( char *chost, char *cfgfile )
{
    char        cwd [MAXPATHLEN];
    short       j;


    /************************************************************/
    /* Initialisierung der globalen Experten- und Domaenendaten */
    /************************************************************/
    initialize_exp();
    initialize_dom();

    strcpy( br_Master, chost );

    gethostname ( hostname, MAXHOSTNAMELEN );
    if( br_OpenAcceptSock() == -1 )
	br_PrintError();

    printf ("    /--------------------------------------------------------------\\ \n");
    printf ("    |   TEAM-COMPLETION                Experte/Spezialist          |  \n");
    printf ("    |   -------------------------------------------------          |  \n");
    printf ("    |                                                              |  \n");
    printf ("    |   Ein Programm von Werner Pitz                               |  \n");
    printf ("    |   Neue Version mit folgenden Erweiterungen:                  |  \n");
    printf ("    |     - Broadcast von Juergen Lind                             |  \n");
    printf ("    |     - Moeglichkeiten zur Beweisanalyse von Stephan Schulz    |  \n");
    printf ("    |     - Einsatzplanung durch den Leiter von Martin Kronenburg  |  \n");
    printf ("    |   Universitaet Kaiserslautern                                |  \n");
    printf ("    |   Fachbereich Informatik                                     |  \n");
    printf ("    \\--------------------------------------------------------------/ \n");
    printf ("\n\n");
    flush ();

    if( br_OpenRecvBroadcast() == -1 )
	br_PrintError();

    Input = br_Receivers[0].Sock;

    /***************************/
    /* Einstellungen empfangen */
    /***************************/
    ThisHost  = ReadInt ( Input );             /* Eigene Rechner-Nummer im System */
    HostCount = ReadInt ( Input );             /* Anzahl der Rechner              */
    ReadString ( Input, cwd, MAXPATHLEN );     /* Pfadname des akt. Verzeichnisses*/
    ReadString ( Input, logfile, MAXPATHLEN ); /* Name des Logfiles               */

#ifdef PCL
    if((tmpdir = (char*) ReadInt(Input)))
    {
       tmpdir = (char*)SecureMalloc(MAXPATHLEN);
       ReadString ( Input, tmpdir, MAXPATHLEN );
    }
    ReadString ( Input, problem_path, MAXPATHLEN);
#endif

    if (chdir ( cwd ))
    {
        printf ( "Was ist denn das ?\n\n" );
        printf ( "Directory: [%s]\n", cwd );
        getcwd ( cwd, MAXPATHLEN );
        printf ( "Directory: [%s]\n", cwd );
        exit ( 1 );
    }

    /*****************************************************/
    /* Informationen ueber die anderen Rechner empfangen */
    /*****************************************************/
    for (j = 0; j < HostCount; j++ )
    {
        ReadString ( Input, Hostname[j], MAXHOSTNAMELEN );
        Hostpid [j] = ReadInt ( Input );
    }

    /***********************************************/
    /* Daten aus der Spezifikationsdatei empfangen */
    /***********************************************/
    /******************/
    /* Modi empfangen */
    /******************/
    Proofmode      = ReadInt ( Input );
    DemoMode       = ReadInt ( Input );
    ProtocolMode   = ReadInt ( Input );
    CancelActive   = ReadInt ( Input );
    Paramodulation = ReadInt ( Input );
    ParaCount      = ReadInt ( Input );
    PlanDocum      = ReadInt ( Input );

    /************************************/
    /* Informationen ueber die Signatur */
    /************************************/
    FuncCount = ReadInt( Input);
    for (j = 1; j <= FuncCount; j++ )
    {
        ReadStream ( Input, (char *)&(Function[j]), sizeof (FunctionInfo) );
        Function[j].freelist = NULL;
    }

    /***********************************/
    /* Informationen ueber die Ordnung */
    /***********************************/
    OrderCount = ReadInt ( Input );
    for (j = 0; j < OrderCount; j++ )
        ReadStream ( Input, (char *)&(Order[j]), sizeof (OrderInfo) );

#ifdef PCL
    extract        = ReadInt ( Input );
    fextract       = ReadInt ( Input ); 
    f_async        = ReadInt ( Input );
#endif
    if(ReproMode)
    {
       Cycles         = ReadInt ( Input );
       printf("Expecting data for cycles 0..%d\n",Cycles);
       
       FirstStepsPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       SecondStepsPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       ConfigPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       MasterPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       
       for(j=0;j<=Cycles;j++)
       {
	  FirstStepsPerCycle[j] = ReadInt ( Input );
	  SecondStepsPerCycle[j] = ReadInt ( Input );
	  ConfigPerCycle[j] = ReadInt ( Input );
	  MasterPerCycle[j] = ReadInt ( Input );
	  printf("Master: %2d   FirstSteps: %6d  SecondSteps: %6d   Config: %2d\n",
		 MasterPerCycle[j],FirstStepsPerCycle[j],SecondStepsPerCycle[j],ConfigPerCycle[j]);
       }
       printf("...accepted data.\n");
    }

    Terminate = TerminateTeam;

    /******************************************************************/
    /* Anlegen zweier Tabelle fuer den Database-Experten. Werden fuer */
    /* das Bestimmen der Substitutionen zweiter Ordnung gebraucht.    */
    /******************************************************************/
    for ( j=0; j<MAXARITY; j++ )
    {
      ArityLength[j] = 0;
    }

    for ( j=1; j<=FuncCount; j++ )
    {
      SubstMatrix[Function[j].arity][ArityLength[Function[j].arity]].fktnr = j;
      SubstMatrix[Function[j].arity][ArityLength[Function[j].arity]++].belegt = false;
    }

    /*****************************/
    /* Konfigurationsdatei lesen */
    /*****************************/
    ParseConfig( cfgfile );
} /* Ende von StartXpert */


/*
//-------------------------------------------------------------------------------------
//  Funktion:       StartTeam ( char *cfgfile, char *problemfile )
//
//  Parameter:      cfgfile     Konfigurationsfile                        
//                  problemfile Problemfile
//
//  Beschreibung:   Einlesen der Konfiguiration und der Problemdatei
//                  Starten der Konfiguration
//-------------------------------------------------------------------------------------
*/

void    StartTeam ( char *cfgfile, char *problemfile )
{
    char                example [200];
    char                cwd [MAXPATHLEN];
    char                cfg [MAXPATHLEN];
    short               i, j;
    int                 out;
    FILE                *testfile;

    CycleList_p         ReproData, handle;
    
    gethostname ( hostname, MAXHOSTNAMELEN );
    if( br_OpenAcceptSock() == -1 )
	br_PrintError();

    printf ("    /--------------------------------------------------------------\\ \n");
    printf ("    |   TEAM-COMPLETION                Experte/Spezialist          |  \n");
    printf ("    |   -------------------------------------------------          |  \n");
    printf ("    |                                                              |  \n");
    printf ("    |   Ein Programm von Werner Pitz                               |  \n");
    printf ("    |   Neue Version mit folgenden Erweiterungen:                  |  \n");
    printf ("    |     - Broadcast von Juergen Lind                             |  \n");
    printf ("    |     - Moeglichkeiten zur Beweisanalyse von Stephan Schulz    |  \n");
    printf ("    |     - Einsatzplanung durch den Leiter von Martin Kronenburg  |  \n");
    printf ("    |   Universitaet Kaiserslautern                                |  \n");
    printf ("    |   Fachbereich Informatik                                     |  \n");
    printf ("    \\--------------------------------------------------------------/ \n");
    printf ("\n\n");
    flush ();

    /************************************************************/
    /* Initialisierung der globalen Experten- und Domaenendaten */
    /************************************************************/
    initialize_exp();
    initialize_dom();

    /*******************************/
    /* Pfadnamen evtl. korrigieren */
    /*******************************/

    getcwd ( cwd, MAXPATHLEN );
    if (!strncmp (cwd, "/tmp_mnt", 8))
    {
        if (!DemoMode)
            printf ( "Pfadkorrektur an Auto-Mounter.\n\n"
                     "urspruenglicher Pfad: %s\n", cwd );
        strcpy ( cwd, (char*)((long)((&cwd))+8) );
        if (!DemoMode)
            printf ( "neuer Pfad:           %s\n\n", cwd );
    }

    /*********************/
    /* Problemfile lesen */
    /*********************/
    if (!DemoMode)
    {
        printf ( "Aufgabenstellung lesen.\n" );
        flush ();
    }
    Parse ( problemfile, example );
#ifdef PCL
    if(Paramodulation)
    {
       Error ( __FILE__ ": "  "main", 
	      "PCL fuer Paramod noch nicht unterstuetzt." );
    }
#endif

    /*****************************/
    /* Konfigurationsdatei lesen */
    /*****************************/
    /* In  cfg steht nachher der verwendete Name der Konfigurationsdatei, */
    /* der auch an die anderen Rechner geschickt wird.                    */
    if (!DemoMode)
    {
        printf ( "Konfigurationsdatei lesen.\n" );
        flush ();
    }

    if (cfgfile)
    {
        strcpy( cfg, cfgfile );
    }
    else
    {
        strcpy ( cfg, problemfile );
        strcat ( cfg, ".cfg" );

        if ((testfile = fopen(cfg, "r")))
        {
            fclose ( testfile );
        }
        else
        {
	    strcpy( cfg, "team.cfg" );
        }
    }
    if (!DemoMode)
    {
	printf ( "Konfigurationsdatei: %s.\n", cfg );
    }
    ParseConfig ( cfg );


    if (!DemoMode)
    {
        printf ( "Konfiguration starten.\n" );
        printf ( "Executable is <%s>.\n",Execfile);
        flush ();
    }

    strncpy(problem_path,cwd,MAXPATHLEN);
    strncat(problem_path,"/",MAXPATHLEN-strlen(problem_path));
    strncat(problem_path,problemfile,MAXPATHLEN-strlen(problem_path));

    PCL_OPEN(problem_path,"w",0,0);
    PCL_COMMENT("Initialisierung: Axiome und Hypothesen",true);
    PCL_INIT();
    PCL_COMMENT("Initialisierung Ende",true);
    PCL_CLOSE();

    if(log_base)
    {
       strncpy(logfile,cwd,MAXPATHLEN);
       strncat(logfile,"/",MAXPATHLEN-strlen(problem_path));
       strncat(logfile,log_base,MAXPATHLEN-strlen(problem_path));
    }
    else
    {
       strcpy(logfile,problem_path);
       strncat(logfile,".prk",MAXPATHLEN-strlen(logfile));
    }
    if (ProtocolMode)
    {
       printf("Logfile: %s\n\n",logfile);
       
       if (!(log_f = fopen (logfile, "w")))
          Error ( __FILE__ ": "  "StartTeam", "LOGFILE-Fehler.");
       
       fprintf ( log_f, "#############################################################\n" 
                      "##      DISCOUNT\n");
       fprintf ( log_f, "##\n"
                      "##      Aufgabenstellung:       %s\n"
                      "##      Konfigurationsdatei:    %s\n",
                      problemfile, cfg );
       fclose ( log_f );
    }

    if(ReproMode)
    {
       printf("Logfile for reproduction: %s\n\n",logfile);
       Cycles = ReadPrk(logfile,&ReproData); 
       
       FirstStepsPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       SecondStepsPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       ConfigPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       MasterPerCycle = SecureMalloc(sizeof(int)*(Cycles+1));
       
       j=0;
       for(handle = ReproData;handle;handle = handle->next)  
       {  /* Hier kriegt der ROOT-Prozess seine Daten - weiter unten */
	  /* werden sie an die anderen Prozesse uebertragen ! */
	  FirstStepsPerCycle[j]  = handle->first_steps[0];
	  SecondStepsPerCycle[j]  = handle->second_steps[0];
	  ConfigPerCycle[j] = handle->confi[0];
	  MasterPerCycle[j] = handle->master;
	  j++;
       }
    }

    /**********************/
    /* Eigene Daten holen */
    /**********************/
    strcpy ( Hostname[0], hostname );
    Hostpid[0]  = getpid ();

    if( br_ConfigureBroadcast ( Hostname, HostCount ) == -1 )
        br_PrintError ();

    /**********************************/
    /* Hochfahren der anderen Rechner */
    /**********************************/
    for (i = 1; i < HostCount; i++ )
        Execute ( i, Hostname[i], problemfile, cfg );

    if( br_OpenSendBroadcast() == -1 )
	br_PrintError();

/* debug 

   getchar();

debug */

#ifdef BERLIN /* Hostnamen und Pids ausgeben - fuer AG Dahn in Berlin */
    
    printf("\n----------------------------------------------------------------\n");
    printf("Initial connection established:\n\n");
    for (i = 0; i < HostCount; i++ )
    {
       printf("Process %ld running on host %s has pid %ld.\n",
	      i, Hostname[i], Hostpid[i]);
    }
    printf("----------------------------------------------------------------\n");
#endif

    /***********************************************************************/
    /* Versenden der Systeminformationen und der Daten aus dem Problemfile */
    /* an die anderen Rechner.                                             */
    /***********************************************************************/
    for (i = 1; i < HostCount; i++ )
    {
        out = br_Receivers[i].Sock;

	/*****************************/
	/* Versenden der Einstellung */
	/*****************************/
        WriteInt ( out, i );
        WriteInt ( out, HostCount );
        WriteString ( out, cwd );
        WriteString ( out, logfile );
#ifdef PCL
	WriteInt ( out , (int)tmpdir);
	if(tmpdir)
	{
	   WriteString ( out, tmpdir );
	}
        WriteString ( out, problem_path );
#endif
        /*******************************/
	/* Versenden der Rechner-Daten */
	/*******************************/
        for (j = 0; j < HostCount; j++ )
        {
            WriteString ( out, Hostname[j] );
            WriteInt    ( out, Hostpid [j] );
        }

        /**********************/
	/* Versenden der Modi */
	/**********************/
	WriteInt( out, Proofmode );
	WriteInt( out, DemoMode );
	WriteInt( out, ProtocolMode );
	WriteInt( out, CancelActive );
	WriteInt( out, Paramodulation );
	WriteInt( out, ParaCount );
	WriteInt( out, PlanDocum );

	/****************************************/
	/* Versenden der Signatur-Informationen */
	/****************************************/
        WriteInt( out, FuncCount   );
	for (j = 1; j <= FuncCount; j++ )
	{
	    WriteStream ( out, (char *)&(Function[j]), sizeof (FunctionInfo) );
	}

	/****************************************/
	/* Versenden der Ordnungs-Informationen */
	/****************************************/
        WriteInt( out, OrderCount );
        for (j = 0; j < OrderCount; j++ )
            WriteStream ( out, (char *)&(Order[j]), sizeof (OrderInfo) );

#ifdef PCL
        WriteInt( out, extract );
        WriteInt( out, fextract );     
        WriteInt( out, f_async );
#endif

	if(ReproMode) /* Master muss seine Daten woanders */
		      /* aufsammeln...hier kriegen nur die Experten */
		      /* ihre Daten */
	{
	   WriteInt( out, Cycles );
	   handle = ReproData;
	   for(handle = ReproData;handle;handle = handle->next)
	   {
	      WriteInt(out, handle->first_steps[i]);
	      WriteInt(out, handle->second_steps[i]);
	      WriteInt(out, handle->confi[i]);
	      WriteInt(out, handle->master);
	   }
	}
     }

    if (!DemoMode)
    {
        printf ( "Konfiguration gestartet.\n" );
        flush ();
    }
    Terminate = TerminateTeam;

    /*************************************************************************/
    /* Es werden nun noch zwei Tabellen angelegt, die der Database-Experte   */
    /* benoetigt, wenn er die Substitutionen zweiter Ordnung bestimmt.       */
    /*************************************************************************/
    for ( j=0; j<MAXARITY; j++ )
    {
      ArityLength[j] = 0;
    }

    for ( j=1; j<=FuncCount; j++ )
    {
      SubstMatrix[Function[j].arity][ArityLength[Function[j].arity]].fktnr = j;
      SubstMatrix[Function[j].arity][ArityLength[Function[j].arity]++].belegt = false;
    }
} /* Ende von StartTeam */


/*
//=====================================================================================
//      Terminieren des Teams
//=====================================================================================
*/

void    TerminateTeam ( void )
{
    int     i;
    char    command[200];

#ifdef STATISTICS

    tString filename;

#endif


    for (i = 0; i < HostCount; i++)
    if (i != ThisHost)
    {
        sprintf ( command, "%s kill -QUIT %d &", Hostname[i], Hostpid[i] );
        system ( command );
    }

    br_CloseSendBroadcast();

#ifdef STATISTICS

    strcpy( filename, br_Hostname );
    strcat( filename, ".stat" );
    stat_ShowStatistic( &br_BroadcastStatistic, filename );

#endif

}

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  StartNextCycle                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur startet in Abhaengigkeit davon, welcher   */
/*                  Experte/Spezialist auf dem Rechner arbeitet die Funk-    */
/*                  tion fuer den naechsten Zyklus.                          */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void StartNextCycle ( void )
{
  /***************************************************************/
  /* Starten des Experten, Sonderbehandlung bei den Spezialisten */
  /***************************************************************/
  if ( exp_ist_spezialist( OwnConfig.exp_nr ) )
  {
    switch ( OwnConfig.exp_nr )
    {
      case DATABASE  : if( ThisHost == MasterHost )
                       {
                         Error ( __FILE__ ": "  "StartNextCycle",
                                 "Ein Spezialist kann nie Master werden." );
                       }
                       else
                       {
                         DatabaseExpert ( CycleTime[CycleCount-1],
                                          OwnConfig.cp_parameter[0],
                                          &(OwnConfig.cp_parameter[1]),
					  OwnConfig.cg_parameter[0] );
                       }
                       break;

      case REDUCE_1   :
      case REDUCE_2   :
      case REDUCE_3   : if( ThisHost == MasterHost )
                        {
                          Error ( __FILE__ ": "  "StartNextCycle",
                                  "Ein Spezialist kann nie Master werden." );
                        }
                        else
                        {
			  switch( RedSpecInfo[GetRedSpecIdx(OwnConfig.exp_nr)].red_typ )
			  {
			    case CP:
			      switch( RedSpecInfo[GetRedSpecIdx(OwnConfig.exp_nr)].reduce_cp_fkt )
			      {
				case SX_REDUCE_CP  : ParallelCheckCPs ( CycleTime[CycleCount-1] );
						     break;
				case SX_REDUCE_CPP : ParallelCheckCPPs ( CycleTime[CycleCount-1] );
						     break;
				default            : Error ( __FILE__ ": "  "StartNextCycle",
							"Unbekannte Reduktionsfunktion fuer krit. Paare.");
						     break;
			      } /* Ende des inneren switch */
			      break;

			    case CG:
			      switch( RedSpecInfo[GetRedSpecIdx(OwnConfig.exp_nr)].reduce_cg_fkt )
			      {
				case SX_REDUCE_CG  : ParallelCheckCGs ( CycleTime[CycleCount-1] );
						     break;
				case SX_REDUCE_CGP : ParallelCheckCGPs ( CycleTime[CycleCount-1] );
						     break;
				default            : Error ( __FILE__ ": "  "StartNextCycle",
							"Unbekannte Reduktionsfunktion fuer krit. Ziele.");
						     break;
			      } /* Ende des inneren switch */
			      break;

			    default       : Error ( __FILE__ ": "  "StartNextCycle",
						    "Unbekannter Reduktionstyp.");
			  } /* Ende des "au"seren switch */
                        } /* Ende von else */

                        break;

      default         : Error ( __FILE__ ": "  "StartNextCycle", "Unbekannter Spezialist." );
                        break;
    } /* Ende von switch ( OwnConfig.exp_nr ) */
  } /* Ende von if ( exp_ist_spezialist... */
  else  /* Experte */
  {
     switch(OwnConfig.exp_nr)
     {
     default:	   
	ParallelCompletion ( CycleTime[CycleCount-1] );
	break;
     }
  }
} /* Ende von StartNextCycle */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  BuildAndSendNextStartsystem                              */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur wird nur vom Leiter aufgerufen.           */
/*                  Der Leiter versendet zunaechst seine kritischen Paare    */
/*                  und Ziele. Interreduziert dann die empfangenen Glei-     */
/*                  chungen der anderen Experten und versendet dann diese    */
/*                  neuen kritischen Paare und die restlichen Termpaarmengen.*/
/*                                                                           */
/*  Globale Var. :  HostCount                                                */
/*                  CycleCount                                               */
/*                  MasterHost                                               */
/*                  ReceivedEq                                               */
/*                                                                           */
/*  Externe Var. :  SetOfCriticalPairs                                       */
/*                  weight_flag                                              */
/*                  BytesTransfered                                          */
/*                  DemoMode                                                 */
/*                  ExpertGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

static void BuildAndSendNextStartsystem( void )
{
  pairset     copycps = EmptySet;
  pairset     neue_cps  = EmptySet;

  pairset     copygoals = EmptySet;
  pairset     neue_goals = EmptySet;

  termpair   *ptr;

  TPClearCPCache();
  TPClearCGCache();
  Initialize( false );

  PCL_OPEN(problem_path,!CycleCount? "a":"w",CycleCount*2,ThisHost);
  PCL_COMMENT("MASTER: Verwaltungskram, Interreduktion",true);

  if (CPWeight==occnest)
      init_goal_fn_m();

  /***********************************************************************/
  /* Spezielle Behandlung beim ersten Zyklus : Die in SetOfCriticalPairs */
  /* stehenden in der Spezifikation angegebenen Gleichungen werden ge-   */
  /* wichtet, falls weight_flag gestzt.                                  */
  /***********************************************************************/
  if ( (!CycleCount) && weight_flag )
  {
    MoveSet( &copycps, &SetOfCriticalPairs);
    TPClearCPCache();
    ptr = DelFirst ( &copycps );
    while ( ptr )
    {
      ptr->weight = CPWeight ( ptr );
      InsertCP ( ptr );
      ptr = DelFirst ( &copycps );
    }
  } /* if */
  /* Beachte die Gleichungen stehen danach wieder in SetOfCriticalPairs  */

  /* Wenn nur ein Experte sequentiell in der parallelen Version l"auft,  */
  /* so mu"s hier nichts weiter gemacht werden.                          */
  if( HostCount == 1 )
  {
    return;
  }

  /**************************************************************************/
  /* Versenden der Ziele, kritischen Ziele und kritischen Paare des Siegers */
  /**************************************************************************/
  /* Es muessen erst die Ziele versendet werden, weil sie von den    */
  /* Zielaehnlichkeitsexperten bei der Bewertung der kritischen Paare*/
  /* benoetigt werden. Deshalb werden sie in SendAllPairs nicht mehr */
  /* mitgesendet!!!! ( MK, den 11.11.1993, Gruss an Sanne!!          */

  SendGoals( BROADCAST );
  if( Paramodulation )
  {
    SendCriticalGoals ( BROADCAST );
  }
  SendCriticalPairs ( BROADCAST );

  if( br_FlushBuffer() == -1 )
      br_PrintError();

  /***********************************************************************/
  /* Wenn auf dem Rechner des Siegers nicht der alte Sieger laeuft,      */
  /* sondern ein neuer Experte, muessen die kritischen Paare und Ziele   */
  /* neu gewichtet werden!                                               */
  /***********************************************************************/
  if( neuer_leiter_exp )
  {
    /* zuerst die kritischen Paare */

    MoveSet( &copycps, &SetOfCriticalPairs);
    TPClearCPCache();
    ptr = DelFirst ( &copycps );
    while ( ptr )
    {
      ptr->weight = CPWeight ( ptr );
      InsertCP ( ptr );
      ptr = DelFirst ( &copycps );
    }

    /* jetzt die kritischen Ziele, falls vorhanden */
    if( Paramodulation )
    {
      MoveSet( &copygoals, &SetOfCriticalGoals);
      TPClearCGCache();
      ptr = DelFirst ( &copygoals );
      while ( ptr )
      {
        ptr->weight = CGWeight ( ptr );
        InsertCritGoal ( ptr );
        ptr = DelFirst ( &copygoals );
      } /* Ende von while */
    } /* Ende von Paramodulation */
  } /* Ende von if neuer_leiter_exp */

  /***********************************************************************/
  /* Es werden nun die empfangenen Gleichungen der anderen Experten      */
  /* in das System des Siegers integriert.                               */
  /***********************************************************************/
  /* Beachte : Um spaeter, zwischen den urspruenglichen kritischen Paaren*/
  /*           des Siegers und denen, die durch die Daten der anderen    */
  /*           Experten hinzukamen werden die ersteren in copycps ge-    */
  /*           sichert. Am Ende dieser Funktion werden beide Mengen      */
  /*           wieder zusammengefuegt.                                   */
  /***********************************************************************/

  MoveSet( &copycps, &SetOfCriticalPairs );
  TPClearCPCache();
  Interreduce ( &ReceivedEq );
  Initialize ( false );
   flush();

  /***********************************************************************/
  /* Es werden nun die empfangenen Ziele der anderen Experten in das     */
  /* System des Siegers integriert.                                      */
  /***********************************************************************/
  /* Beachte : Um spaeter, zwischen den urspruenglichen kritischen Zielen*/
  /*           des Siegers und denen, die durch die Daten der anderen    */
  /*           Experten hinzukamen, unterscheiden zu koennen,  werden die*/
  /*           ersteren in copygoals gesichert. Am Ende dieser Funktion  */
  /*           werden beide Mengen wieder zusammengefuegt.               */
  /***********************************************************************/

  if ( Paramodulation )
  {
    MoveSet( &copygoals, &SetOfCriticalGoals );
    TPClearCGCache();

    InterreduceGoals ( &ReceivedGoals );
    flush();
  }

  /********************************/
  /* Versenden der Leiter-Ordnung */
  /********************************/
  WriteInt( BROADCAST, ExpertGlobalInfo[OwnConfig.exp_nr].order );

  /***********************************************************************/
  /* Es werden nun die restlichen Termpaar-Mengen des Siegers und die    */
  /* intereduzierten Termpaare der anderen Experten versendet.           */
  /* Beachte SendAllPairs verschickt zur Zeit nicht die Ziele!!!!!!!     */
  /***********************************************************************/
  SendAllPairs( BROADCAST );
  SendGoals( BROADCAST );

  if( br_FlushBuffer() == -1 )
      br_PrintError();

  /*************************************************************************/
  /* Ausgabe ueber alte ( d.h. allein im System des Siegers befindlichen ) */
  /* kritische Paare und kritische Ziele sowie ueber die durch die anderen */
  /* Experten hinzugekommenen krtischen Paare und Ziele.                   */
  /* Die beiden Mengen werden dann vereinigt.                              */
  /*************************************************************************/
  /* Zunaechst die kritischen Paare */
  /**********************************/
  if (!DemoMode)
  {
    printf ( "%ld neue kritische Paare.\n", SizeOf (SetOfCriticalPairs) );
  }

  MoveSet ( &neue_cps, &SetOfCriticalPairs );
  TPClearCPCache();

  MoveSet ( &SetOfCriticalPairs, &copycps );
  TPClearCPCache();

  if (!DemoMode)
  {
    printf ( "%ld alte kritische Paare.\n", SizeOf (SetOfCriticalPairs) );
  }

  MergeCPs ( &neue_cps );

  printf ( "%ld kritische Paare versendet.\n", SizeOf (SetOfCriticalPairs) );
  printf ( "\n" );
  
  /******************************/
  /* Jetzt die kritischen Ziele */
  /******************************/
  if ( Paramodulation )
  {
    if (!DemoMode)
    {
      printf ( "%ld neue kritische Ziele.\n", SizeOf (SetOfCriticalGoals) );
    }

    MoveSet ( &neue_goals, &SetOfCriticalGoals );
    TPClearCGCache();

    MoveSet ( &SetOfCriticalGoals, &copygoals );
    TPClearCGCache();

    if (!DemoMode)
    {
      printf ( "%ld alte kritische Ziele.\n", SizeOf (SetOfCriticalGoals) );
    }

    MergeCGs ( &neue_goals );

    printf ( "%ld kritische Ziele versendet.\n", SizeOf (SetOfCriticalGoals) );
    printf ( "\n" );
  }

  flush ();

  /*******************************************/
  /* Bestimmen der Gr"o"se des Gesamtsystems */
  /*******************************************/
  GesamtGroesse = akt_termpaare_anz_best();

  PCL_COMMENT("MASTER: Interreduktion ende",true);
  PCL_CLOSE();
  PCL_EXTRACT(true);
  PCL_OPEN(problem_path,"w",CycleCount*2+1,ThisHost);
  PCL_COMMENT("MASTER: Vervollstaendigung",true);

  if(ReproMode)
  {
     if(OwnConfig.exp_nr == -1)
     {
	printf("----------------------------------------------\n\n");
	printf("      Waiting for termination...              \n\n");
	printf("----------------------------------------------\n\n");
	flush();
	PCL_COMMENT("Master waiting for termination...",true);
	pause();
     }
  }
} /* Ende von BuildAndSendNextStartsystem */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  Master                                                   */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur startet alle Aktionen, die der Leiter     */
/*                  waehrend eines Teammeatings durchfuehren muss.           */
/*                                                                           */
/*  Globale Var. :  CycleTime                                                */
/*                  CycleCount                                               */
/*                  OwnConfig                                                */
/*                  RefGlobalInfo                                            */
/*                                                                           */
/*  Externe Var. :  referee_report                                           */
/*                                                                           */
/*****************************************************************************/

static void     Master ( )
{

    long startplanungszeit;
    long endeplanungszeit;

    /*pairset     copycps = EmptySet;
    pairset     newcps  = EmptySet;
    termpair    *ptr;
    long        runtime;
    long        time;
    int         i;*/

    printf ( "\n\n" );
    
    if(!ReproMode)
    {
       /* Ausgabe nur, wenn wirklich verteilt wird                                  */
       /* also nicht, wenn ein Experte sequentiell in der Parallelen Version l"auft */
       if( HostCount != 1 )
       {
	  printf ( "-------------------------------------------------------------------\n" );
	  printf ( "    TEAM - MASTER\n" );
	  printf ( "\n" );
	  printf ( "-------------------------------------------------------------------\n" );
       } /* Ende von if */
    }
    else
    {
       printf ( "-------------------------------------------------------------------\n" );
       printf ( "    Reproduction: TEAM - MASTER\n" );
       printf ( "\n" );
       printf ( "    Zyklus:            %3d\n", CycleCount+1);
       printf ( "    Configuration:     %3d\n", ConfigPerCycle[CycleCount] );
       printf ( "    FirstSteps:        %3d\n", FirstStepsPerCycle[CycleCount] );
       printf ( "    SecondSteps:       %3d\n", SecondStepsPerCycle[CycleCount]);
       printf ( "-------------------------------------------------------------------\n" );
    }
    printf ( "\n\n" );
    flush ();

/* Bei Reproduktionsl"aufen ist die Planung ausgeschaltet */
    if(!ReproMode)
    {
       startplanungszeit = systime();
       if ( PlanDocum )
       {
	  plan_doc = fopen( PLAN_LOGFILE, "a" );
       }
       
       if ( PlanDocum )
       {
	  fprintf( plan_doc, "*****************************************************************\n");
	  fprintf( plan_doc, "*****************************************************************\n");
	  fprintf( plan_doc, "**********        PLANUNG FUER DEN %4d. ZYKLUS        **********\n", CycleCount+1);
	  fprintf( plan_doc, "*****************************************************************\n");
	  fprintf( plan_doc, "*****************************************************************\n\n");
	  
	  fprintf( plan_doc, "*****************************************************************\n");
	  fprintf( plan_doc, "**********         PLANUNGSPHASE FUER DOMAENEN         **********\n");
	  fprintf( plan_doc, "*****************************************************************\n\n");
       }
       DomainManagement();
       
       if ( PlanDocum )
       {
	  
	  fprintf( plan_doc, "*****************************************************************\n");
	  fprintf( plan_doc, "**********         PLANUNGSPHASE FUER EXPERTEN         **********\n");
	  fprintf( plan_doc, "*****************************************************************\n\n");
       }
       
       DefineProofPhase();
       
       SelectNextTeamExperts();
       
       /* SelectNextTeamReferees(); * fuer spaeter!!!!! */
       
       referee_report.runtime = CycleTime[CycleCount] = DefineNextCycleTime();
       
       endeplanungszeit = systime();
       if ( PlanDocum )
       {
	  fprintf( plan_doc, "\n\n*****************************************************************\n");
	  fprintf( plan_doc, "*****************************************************************\n");
	  fprintf( plan_doc, "**********            ENDE DER PLANUNGSPHASE           **********\n");
	  fprintf( plan_doc, "**********           Dauer : %3ld.%03ld Sekunden          **********\n",
		  (endeplanungszeit - startplanungszeit)/1000,
		  (endeplanungszeit - startplanungszeit)%1000);
	  fprintf( plan_doc, "*****************************************************************\n");
	  fprintf( plan_doc, "*****************************************************************\n\n\n\n\n");
	  
	  fclose( plan_doc );
       }
       GesamtPlanungszeit += endeplanungszeit - startplanungszeit;
       
       DistributeTeam();
    }
    else
       /* Bei einem Reproduktionslauf wird die Konfiguration aus ConfigPerCycle gesetzt */
       /* Falls sich die Parameter der CP- und CG-Funktionen oder die Gutachter "andern */
       /* k"onnen, m"ussen diese ebenfalls hier gesetzt werden.                         */
       /* Es wird auch von der zur Zeit g"ultigen Voraussetzung ausgegangen, da"s stets */
       /* alle Rechner belegt sind und damit die Anzahl der eingesetzten Experten/Spe-  */
       /* zialisten TeamExpCount mit der Anzahl der zur Verf"ugung stehenden Rechner    */
       /* "ubereinstimmt. Ist dies mal nicht mehr gew"ahrleistet, so mu"s diese Angabe  */
       /* zus"atzlich in den Protokollfile geschrieben werden.                          */
       /* Es mu"s die Variable neuer_leiter_exp versorgt werden, damit bei einem Leiter-*/
       /* wechsel die krit. Termpaare neu gewichtet werden.                             */
    {
       TeamExpCount = HostCount;
       OwnConfig.exp_nr = ConfigPerCycle[CycleCount];
       if( OwnConfig.exp_nr != NO_CONFIG )
       {
	  set_parameter( &OwnConfig );
       }
       
       if( CycleCount )
       {
	  if( OwnConfig.exp_nr != ConfigPerCycle[CycleCount-1] )
	  {
	     neuer_leiter_exp = true;
	  }
	  else
	  {
	     neuer_leiter_exp = false;
	  }
       } /* Ende von if CycleCount */
    }/* Ende von #if(!ReproMode) -> keine Planung bei Reproduktionsl"aufen */
    
    SetConfiguration ( DemoMode );

    if(ReproMode)
    {
       if(OwnConfig.exp_nr != NO_CONFIG)
       {
	  SetRefParams();
       }
    }
    else
    {
       SetRefParams();
       SendCycleTime();
    }

    BuildAndSendNextStartsystem();

    CycleCount++;

    if(!ReproMode)
    {
       
       /* Ausgabe nur, wenn wirklich verteilt wird                                  */
       /* also nicht, wenn ein Experte sequentiell in der Parallelen Version l"auft */
       if( HostCount != 1 )
       {
	  printf ( "-------------------------------------------------------------------\n" );
	  printf ( "    Zyklus: %4d       Laufzeit: ", CycleCount );
	  if( timeunit == SECONDS )
	  {
	     printf ( "%4ld s\n", CycleTime[CycleCount-1] );
	  }
	  else
	  {
	     printf ( "%4ld.%03ld s\n", CycleTime[CycleCount-1]/1000, CycleTime[CycleCount-1]%1000 );
	  }
	  printf ( "-------------------------------------------------------------------\n" );
	  printf ( "\n\n" );
	  flush ();
       } /* Ende von if ( HostCount != 1 ) */
    }

    StartNextCycle();

    PCL_COMMENT("Vervollstaendigung Ende",true);

    referee_report.master = true;

    /* Einem Leiter muss stets ein Gutachter zugeordnet werden, insbesondere */
    /* darf er kein Spezialist sein.                                         */
    Referee( RefGlobalInfo[OwnConfig.exp_nr].beurteile_ga, beurteile_ga_param,
	     RefGlobalInfo[OwnConfig.exp_nr].resultate_ga, resultate_ga_param );

    referee_report.masterflag = true;

    if ( PlanDocum )
      plan_doc = fopen( PLAN_LOGFILE, "a" );

    ReceiveJudges();

    if ( PlanDocum )
      fclose( plan_doc );

    SendWinnerNo();

} /* Ende von Master */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReceiveNextStartsystem                                   */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur wird von allen Prozessen aufgerufen, die  */
/*                  nicht Sieger des letzten Zyklus waren. Diese empfangen   */
/*                  hier das Startsystem fuer den nexten Zyklus.             */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void ReceiveNextStartsystem ( void )
{
  int   leiter_ordnung;

  /* Es muessen erst die Ziele empfangen werden, weil sie von den       */
  /* Zielaehnlichkeitsexperten bei der Bewertung der kritischen Paare   */
  /* benoetigt werden. Deshalb werden sie in ReceiveAllPairs nicht mehr */
  /* empfangen !!!! ( MK, den 11.11.1993, Gruss an Sanne!!              */
  ReceiveGoals( BROADCAST );

  if (CPWeight==occnest)
      init_goal_fn_m();

  if( Paramodulation )
  {
    ReceiveCriticalGoals( BROADCAST );
  }
  ReceiveCriticalPairs( BROADCAST );


  /*************************************/
  /* Empfangen der Ordnung des Leiters */
  /*************************************/
  leiter_ordnung = ReadInt( BROADCAST );

  /* Falls der Experte auf diesem Rechner ein Spezialist ist, erhaelt er     */
  /* die Ordnung des Leiters                                                 */
  if ( exp_ist_spezialist( OwnConfig.exp_nr ) )
  {
    SetOrdering( leiter_ordnung );

    ReceiveAllPairs( BROADCAST, false );
    ReceiveGoals( BROADCAST );
  }
  else
  {
    /* Falls der Leiter eine andere Ordnung hat, muss neu sortiert werden.   */
    ReceiveAllPairs( BROADCAST,
                     leiter_ordnung != ExpertGlobalInfo[OwnConfig.exp_nr].order );
    ReceiveGoals( BROADCAST );
  }

  ForAllGoalsDo( (void *) reduce_pair );
  GetCpParents();
  GetCgParents();
  Initialize( false );
  flush();

  /*******************************************/
  /* Bestimmen der Gr"o"se des Gesamtsystems */
  /*******************************************/
  GesamtGroesse = akt_termpaare_anz_best();

  PCL_COMMENT("SLAVE: Vervollstaendigung",true);

  if(ReproMode)
  {
     if(OwnConfig.exp_nr == -1)
     {
	printf("----------------------------------------------\n\n");
	printf("      Waiting for termination...              \n\n");
	printf("----------------------------------------------\n\n");
	flush();
	PCL_COMMENT("Slave waiting for termination...",true);
	pause();
     }
  }

} /* Ende von ReceiveNextStartsystem */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  Slave                                                    */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur startet alle Aktionen, die ein "normales" */
/*                  Teammitglied, d.h. das nicht Leiter ist, waehrend eines  */
/*                  Teammeatings durchfuehren muss.                          */
/*                                                                           */
/*  Globale Var. :  RefGlobalInfo                                            */
/*                  OwnConfig                                                */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/


static void     Slave ( void )
{
    printf ( "\n\n" );
    if(!ReproMode)
    {
       printf ( "-------------------------------------------------------------------\n" );
       printf ( "    Zyklus %d:  TEAM - EXPERT / SPECIALIST\n",CycleCount+1);
       printf ( "-------------------------------------------------------------------\n" );
    }
    else
    {
       printf ( "-------------------------------------------------------------------\n" );
       printf ( "   Reproducing:  TEAM - EXPERT / SPECIALIST\n");
       printf ( "   Zyklus:            %3d\n", CycleCount+1);
       printf ( "   Configuration:     %3d\n", ConfigPerCycle[CycleCount] );
       printf ( "   FirstSteps:        %3d\n", FirstStepsPerCycle[CycleCount] );
       printf ( "   SecondSteps:       %3d\n", SecondStepsPerCycle[CycleCount]);
       printf ( "-------------------------------------------------------------------\n" );
    }
    printf ( "\n\n" );
    flush ();

    PCL_OPEN(problem_path,"w",CycleCount*2+1,ThisHost);
    PCL_COMMENT("SLAVE: Uebertragung",true);

    /*****************************************************************************/
    /* Loeschen aller bisherigen Termpaare und der Caches fuer kritische Paare   */
    /* und kritische Ziele.                                                      */
    /*****************************************************************************/
    ClearData ();
    TPClearCPCache();
    TPClearCGCache();

    #ifndef NOPAGEOPT 
      Free ();
      initsubst ();
      initterm ();
      inittermpair ();
      VTinit ();
    #endif 

    if(!ReproMode)
    {
       ReceiveNextConfiguration();
    }
    else
       /* Bei einem Reproduktionslauf wird die Konfiguration aus ConfigPerCycle gesetzt */
       /* Falls sich die Parameter der CP- und CG-Funktionen oder die Gutachter "andern */
       /* k"onnen, m"ussen diese ebenfalls hier gesetzt werden.                         */
       /* Dann gen"ugt der Aufruf von set_parameter wohl nicht mehr, stattdessen m"ussen*/
       /* die Parameter ebenfalls in der Datei, in der auch die Konfigurationsnummern   */
       /* der Experten stehen, abgespeichert werden.                                    */
       /* Es wird auch von der zur Zeit g"ultigen Voraussetzung ausgegangen, da"s stets */
       /* alle Rechner belegt sind und damit die Anzahl der eingesetzten Experten/Spe-  */
       /* zialisten TeamExpCount mit der Anzahl der zur Verf"ugung stehenden Rechner    */
       /* "ubereinstimmt. Ist dies mal nicht mehr gew"ahrleistet, so mu"s diese Angabe  */
       /* zus"atzlich in den Protokollfile geschrieben werden. */
    {
       TeamExpCount = HostCount;
       OwnConfig.exp_nr = ConfigPerCycle[CycleCount];
       if( OwnConfig.exp_nr != NO_CONFIG )
       {
	  set_parameter( &OwnConfig );
       }
    }
    SetConfiguration ( DemoMode );
    
    if(ReproMode)
    {
       if(OwnConfig.exp_nr != NO_CONFIG)
       {
	  SetRefParams();
       }
    }
    else
    {
       SetRefParams();
       ReceiveCycleTime();
    }

    ReceiveNextStartsystem();

    CycleCount++;

    StartNextCycle();

    PCL_COMMENT("Vervollstaendigung Ende",true);

   
    /* Sonderbehandlung beim Database-Experten, er ruft Referee selbst auf */
    if ( OwnConfig.exp_nr != DATABASE )
    {
      /* Sonderbehandlung bei den Reduktionsspezialisten */
      if ( exp_ist_spezialist( OwnConfig.exp_nr ) )
      {
	Referee( SR_NONE, NULL, SR_NONE, NULL );
      }
      else
      {
	Referee( RefGlobalInfo[OwnConfig.exp_nr].beurteile_ga, beurteile_ga_param,
		 RefGlobalInfo[OwnConfig.exp_nr].resultate_ga, resultate_ga_param );
      }
    }

    referee_report.masterflag = false;

    SendJudges();

    ReceiveWinnerNo();
} /* Ende von Slave */

/*
//-------------------------------------------------------------------------------------
//  Funktion:       StartCompletion
//
//  Parameter:      -keine-
//
//  Beschreibung:   Die Verbindungen eines Programms aufbauen
//-------------------------------------------------------------------------------------
*/

void    StartCompletion ( void )
{
#ifdef PCL
    int     i;
#endif

    while (true)
    {
        if (MasterHost == ThisHost)
        {

            Master ();/* Master does PCL_OPEN ! */


	    if( MasterHost != WinnerHost)
	    {
	      printf ( "\n\n" );
	      printf ( "-------------------------------------------------------------------\n" );
	      printf ( "    Senden der Leiter-Informationen von Rechner %d an Rechner %d\n", MasterHost, WinnerHost );
	      printf ( "-------------------------------------------------------------------\n" );
	      printf ( "\n\n" );
	      flush ();
	      SendLeaderInfo();
	      printf ( "\n\n" );
	      printf ( "-------------------------------------------------------------------\n" );
	      printf ( "    Leiter-Informationen gesendet \n");
	      printf ( "-------------------------------------------------------------------\n" );
	      printf ( "\n\n" );
	      flush ();
		
	      Input = br_Receivers[WinnerHost].Sock;
	      
	      if( br_OpenRecvBroadcast() == -1 )
		  br_PrintError();
	    }
        }
        else
        {
            Slave (); /* Slave does PCL_OPEN ! */

	    if ( MasterHost != WinnerHost )
	    {
	      if( ThisHost == WinnerHost ){
		printf ( "\n\n" );
		printf ( "-------------------------------------------------------------------\n" );
		printf ( "    Empfangen der Leiter-Informationen von Rechner %d an Rechner %d\n", MasterHost, WinnerHost );
		printf ( "-------------------------------------------------------------------\n" );
		printf ( "\n\n" );
		flush ();
		ReceiveLeaderInfo();
		printf ( "\n\n" );
		printf ( "-------------------------------------------------------------------\n" );
		printf ( "    Leiter-Informationen empfangen \n");
		printf ( "-------------------------------------------------------------------\n" );
		printf ( "\n\n" );
		flush ();
		    
		if( br_OpenSendBroadcast() == -1 )
		    br_PrintError();

	      } /* Ende von if( ThisHost == WinnerHost ) */
	      else{
		  if( br_OpenRecvBroadcast() == -1 )
		      br_PrintError();

		  Input = br_Receivers[WinnerHost].Sock;
	      } /* Ende von else */
	    } /* Ende von if ( MasterHost != WinnerHost ) */
        } /* Ende von else --> slave liegt vor */

#ifdef PCL
        if(MasterHost == ThisHost)
        {
           PCL_COMMENT("Gute Einzelergebnisse Master",true);
        }
        else
        {
           PCL_COMMENT("Gute Einzelergebnisse Slave",true);
        }
        for (i = 0; i < referee_report.rule_count; i++)
        {
           PCL_INTERMED(referee_report.rule[i]); 
        }
        for (i = 0; i < referee_report.equ_count; i++)
        {
           PCL_INTERMED(referee_report.equ[i]); 
        }
        for (i = 0; i < referee_report.goal_count; i++)
        {
           PCL_INTERMED(referee_report.goal[i]); 
        }
#endif

        MasterHost = WinnerHost;

        if (MasterHost == ThisHost)
        {
            referee_report.rule_count =
            referee_report.equ_count  =
            referee_report.goal_count = 0;
	    PCL_COMMENT("Empfangen der Ergebnisse",true);
            ReceiveResults ();
            PCL_COMMENT("Empfangen Ende",true);
        }
        else
        {
            PCL_COMMENT("Senden der Ergebnisse",true);
            SendResults ();
            PCL_COMMENT("Senden Ende",true);
        }
        PCL_CLOSE();
        PCL_EXTRACT(MasterHost == ThisHost);
    } /* while (true) */
} /* Ende von StartCompletion */

