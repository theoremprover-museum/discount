/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : com_phase                                                 */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : Kommunikation zwischen Leiter und anderen Experten zu     */
/*                 waehrend eines Teammeatings.                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <string.h>
#include "defines.h"
#include "error.h"
#include "termpair.h"
#include "cpweight.h"
#include "complet.h"
#include "socket.h"
#include "domain.h"
#include "scanner.h"
#include "database.h"
#include "expert.h"
#include "exp_def_next_cycletime.h"
#include "exp_set_next_config.h"
#include "team.h"
#include "referee.h"
#include "transfer.h"
#include "history.h"

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

/* Endemarkierung fuer gesendete kritische Paare und kritische Ziele         */
#define END_OF_TP -1

/* Wert von WinnerHost, solange noch kein neuer Leiter f"ur den n"achsten    */
/* Zyklus gefunden ist                                                       */
#define NO_WINNER      -1

/*****************************************************************************/
/*                                                                           */
/*                             lokale Variablen                              */
/*                                                                           */
/*****************************************************************************/
                  /*****************************************/
                  /* Socket des neuen Siegers              */
                  /* ben"otogt bei einem Leiterwechsel     */
                  /*****************************************/
static int    NewWinnerSock;

static long   CompletionFirstSteps[MAXEXPERT];
static long   CompletionSecondSteps[MAXEXPERT];


/*****************************************************************************/
/*                                                                           */
/*                            externe Variablen                              */
/*                                                                           */
/*****************************************************************************/
                  /***************************/
                  /* Broadcast-Informationen */
                  /***************************/
extern tReceiver br_Receivers[];

/*****************************************************************************/
/*                                                                           */
/*             Forward-Deklarationen interner Funktionen                     */
/*                                                                           */
/*****************************************************************************/

static void expert_kzg_senden ( void );

static void experten_daten_senden ( void );

static void spezialisten_daten_senden ( );

static void domaene_kzg_senden( short dom_nr );

static void domaenen_daten_senden ( void );

static void zyklus_zeit_daten_senden ( void );

static void expert_kzg_empfangen ( void );

static void experten_daten_empfangen ( void );

static void spezialisten_daten_empfangen ( void );

static void domaene_kzg_empfangen ( short dom_nr );

static void zyklus_zeit_daten_empfangen ( void );

static void domaenen_daten_empfangen ( void );
 
/*****************************************************************************/
/*                                                                           */
/*             Senden und Empfangen der Gutachterbeurteilungen               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReceiveJudges                                            */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente wird nur vom Leiter des letzten Zyklus  */
/*                  aufgerufen. in ihr werden die Beurteilungen aller Gut-   */
/*                  achter empfangen und dabei in einem ersten Schritt ver-  */
/*                  arbeitet, wobei u.a. der Sieger bestimmt wird.           */
/*                  Es werden auch die relativen Beurteilungen ermittelt,    */
/*                  nachdem der Sieger des letzten Zyklus fessteht.          */
/*                                                                           */
/*                  Beachte, dass die Spezialisten nicht beurteilt werden    */
/*                  und daher auch keine Beurteilungen versenden, die hier   */
/*                  empfangen werden muessten.                               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  WinnerHost                                               */
/*                  ExpertGlobalInfo                                         */
/*                  CycleCount                                               */
/*                  ThisHost                                                 */
/*                  TeamExpCount                                             */
/*                                                                           */
/*****************************************************************************/
void ReceiveJudges ( void )
{
  short i;

  int   in; /* Socket zum Empfangen der Beurteilungen */

  /* Die folgende Variable empfaengt einen Wert des Gutachters, der da-       */
  /* rueber entscheidet, ob der Rechner(Experte) Leiter werden kann oder nicht*/
  int   LeiterMoeglich;

  /*******************************************************/
  /* Initilasierung des Siegers mit dem aktuellen Leiter */
  /* --> aktuelle Leiter wird auch beruecksichtigt.      */
  /* Es sei denn, der bisherige Leitr kann im n"achsten  */
  /* Zyklus kein Leiter mehre werden.                    */
  /*******************************************************/
  if (referee_report.master)
  {
    WinnerHost = ThisHost;
  }
  else
  {
    WinnerHost = NO_WINNER;
  }

  if (PlanDocum)
  {
    fprintf ( plan_doc, "Leiter %s auf Rechner %d bewertet sein aktuelles System mit %d.\n",
              get_exp_name(OwnConfig.exp_nr), ThisHost, referee_report.result);
  }

  if (!DemoMode)
  {
    printf ( "Leiter %s auf Rechner %d bewertet sein aktuelles System mit %d.\n",
              get_exp_name(OwnConfig.exp_nr), ThisHost, referee_report.result);
  }
  
  /***************************************/
  /* Ablegen der Beurteilung des Leiters.*/
  /***************************************/
  ExpertGlobalInfo[OwnConfig.exp_nr].abs_bewertung[CycleCount] = referee_report.result; 
    

  /****************************************************/
  /* Empfangen der Beurteilungen der anderen Experten */
  /****************************************************/
  for ( i=0; i<TeamExpCount; i++ )
  {
    /* Der aktuelle Leiter wird nicht beachtet. */
    if ( i != ThisHost )
    {
      /*************************/
      /* Zum Synchronisieren : */
      /*************************/
      WriteInt ( br_Receivers[i].Sock, SYNCCODE );

      in = br_Receivers[i].Sock;

      /***************************************************************/
      /* Bei Repro-L"aufen : Empfangen der verwendeten Konfiguration */
      /* Der Leiter kennt sie sonst nicht !!                         */
      /***************************************************************/

      if(ReproMode)
      {
         ExpertTeamInfo[i].exp_nr = ReadInt( in );  
      }
      
      /*******************************************************************************/
      /* Bei Spezialisten interessieren nur die Anzahl der durchgef"uhreten Schritte */
      /*******************************************************************************/
      if( !exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) )
      {
        /*****************************/
        /* Empfangen des Leiterflags */
        /*****************************/
        LeiterMoeglich   = ReadInt ( in );

        /*******************************/
        /* Empfangen der Beurteilungen */
        /*******************************/
        ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount] = 
           ReadInt( in );
      } /* Ende von if( !exp_ist_spezialist... */
        

      /********************************************/
      /* Empfangen der Anzahl der Arbeitsschritte */
      /********************************************/
      CompletionFirstSteps[i] = ReadInt(in);
      CompletionSecondSteps[i] = ReadInt(in);

      /*******************************************************************/
      /* Dokumentation der Beurteilungen, falls kein Spezialist vorliegt */
      /*******************************************************************/
      if( !exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) )
      {
        if (PlanDocum)
        {
          fprintf ( plan_doc, "Experte %s auf Rechner %d bewertet sein aktuelles System mit %d.\n",
             get_exp_name( ExpertTeamInfo[i].exp_nr ),i,
             ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount] );
        }

        if (!DemoMode)
        {
          printf ( "Experte %s auf Rechner %d bewertet sein aktuelles System mit %d.\n",
             get_exp_name( ExpertTeamInfo[i].exp_nr ),i,
             ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount] );
        }
      } /* Ende von !exp_ist_spezialist(... */

      
      /********************/
      /* Siegerbestimmung */
      /********************/
      if ( !exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) && LeiterMoeglich && 
         ( ExpertGlobalInfo[ExpertTeamInfo[WinnerHost].exp_nr].abs_bewertung[CycleCount] <
           ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount] ||
           WinnerHost == NO_WINNER) )   
      {
        WinnerHost = i;
      }
    } /* Ende von if (i != ThisHost && !exp_ist_spezialis... */
    else
    {
       if(ReproMode)
       {
          ExpertTeamInfo[i].exp_nr = OwnConfig.exp_nr;
       }
      CompletionFirstSteps[i] = FirstStepsDone;
      CompletionSecondSteps[i] = SecondStepsDone;
    } /* Ende von else */
  } /* Ende von for */

  /********************************************************/
  /* "Uberpr"ufen ob ein leiterf"ahiger Experte dabei war */
  /********************************************************/
  /*  @@ Ist das hier notwendig ??????*/
  if ( WinnerHost == NO_WINNER )
  {
      printf("\n*** FEHLER: keiner der Experten kann Master werden.\n");
      printf("        Programm wird abgebrochen.\n\n");
      TerminateTeam();
      exit(1);
  } 

  /***************************************/
  /* Aenderung von StS: Protokollausgabe */
  /***************************************/

  if(ProtocolMode)
  {
     if (!(log_f = fopen (logfile, "a")))
        Error ( __FILE__ ": "  "NewMaster", "LOGFILE-Fehler.");

     fprintf(log_f,"\ncycle %d\n\n",CycleCount-1);
     fprintf(log_f,"master:%d\n",ThisHost);
     for (i = 0; i < TeamExpCount; i++)
     {
        fprintf(log_f,"process %d using configuration %d (%s) did %d steps and %d steps\n",
                    i,ExpertTeamInfo[i].exp_nr,
                    get_exp_name( ExpertTeamInfo[i].exp_nr ),
                    CompletionFirstSteps[i], CompletionSecondSteps[i]);
     }
     fprintf(log_f,"\n");
     fclose(log_f);
  }
  /* Ende der Protokollausgabe */

  if (!DemoMode)
  {
    printf ( "Neuer Master %d mit %d.\n", WinnerHost,
         ExpertGlobalInfo[ExpertTeamInfo[WinnerHost].exp_nr].abs_bewertung[CycleCount]);
  }

  if(ReproMode)
  {
     WinnerHost = MasterPerCycle[CycleCount];
     printf("Reproduction overwrites: New Master will be %d.\n",
            WinnerHost);
  }


  /****************************************************/
  /* Berechnung der relativen Beurteilungen           */
  /* Fallunterscheidung je nach dem, ob die absolute  */
  /* Beurteilung des Siegers positiv oder negativ ist */
  /****************************************************/
  if( ExpertGlobalInfo[ExpertTeamInfo[WinnerHost].exp_nr].abs_bewertung[CycleCount] > 0 )
  {
    for ( i=0; i<TeamExpCount; i++ )
    {
      /* ausser Spezialisten werden alle betrachtet */
      if ( !exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) ) 
      {
        ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].rel_bewertung[CycleCount] =
          ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount] /
          ExpertGlobalInfo[ExpertTeamInfo[WinnerHost].exp_nr].abs_bewertung[CycleCount] ;
      } /* Ende von if */
    } /* Ende von for */
  } /* Ende von if */
  else
  {
    for ( i=0; i<TeamExpCount; i++ )
    {
      /* ausser Spezialisten werden alle betrachtet */
      if ( !exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) ) 
      {
        ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].rel_bewertung[CycleCount] =
          ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount] /
          ExpertGlobalInfo[ExpertTeamInfo[WinnerHost].exp_nr].abs_bewertung[CycleCount] ;
      } /* Ende von if */
    } /* Ende von for */
  } /* Ende von else */

  flush ();
} /* Ende von ReceiveJudges */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SendJudges                                               */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente wird nur von Experten aufgerufen, die   */
/*                  nicht Leiter des letzten Zyklus waren und ist das pas-   */
/*                  sende Gegenstueck zu ReceiveJudges. In dieser Komponente */
/*                  werden die Beurteilungen der Gutachter der letzten Runde */
/*                  an den Leiter gesendet.                                  */
/*                  Beachte : Wird von Spezialisten nicht aufgerufen.        */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  Hostname[]                                               */
/*                  Hostport[]                                               */
/*                  Input                                                    */
/*                                                                           */
/*****************************************************************************/
void SendJudges ( void )
{
  int out; /* Ausgabe-Socket */
  
  /**************************/
  /* Synchronisierung lesen */
  /**************************/
  (void)ReadInt ( Input );

  out = br_Receivers[MasterHost].Sock;

  /******************************************************************************/
  /* Bei Reprol"aufen: Versenden der eigenen Konfigurationsnummer an den Leiter */
  /* Ansonsten kennt der Leiter sie nicht, mu"s sie aber kennen, um das korrekte*/
  /* Protokoll zu verwenden.                                                    */
  /******************************************************************************/
  if(ReproMode)
  {
     WriteInt( out, OwnConfig.exp_nr );
  }

  /******************************************************************************/
  /* Bei spezialisten interessieren nur die Anzahl der durchgef"uhrten Schritte */
  /******************************************************************************/
  if( !exp_ist_spezialist( OwnConfig.exp_nr ) )
  {
    WriteInt ( out, referee_report.master );
    WriteInt ( out, referee_report.result );
  }

  WriteInt ( out, FirstStepsDone);
  /* FirstStepsDone darf nicht in ParallelCompletion auf 0 gesetzt werden,   */
  /* da sonst evtl. die Schritte des Database-Experten verlorengehen         */
  FirstStepsDone = 0;
  WriteInt ( out, SecondStepsDone);

} /* Ende von SendJudges */

/*****************************************************************************/
/*                                                                           */
/*             Kommunikation, falls Leiterwechsel notwendig                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  expert_kzg_senden                                        */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  In dieser Komponente werden alle Daten aus dem KZG ueber */
/*                  die Experten an den angegebenen Socket gesendet. Dies    */
/*                  sollte in der Regel der Leiter des naechsten Zyklus sein.*/
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  ExpertGlobalInfo                                         */
/*                  ExpertCount                                              */
/*                  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static void expert_kzg_senden ( void )
{
  short i;
  short j;

  /* Es werden nur die Experten betrachtet, die in der Konfigurationsdatei   */
  /* angegeben wurden.                                                       */
  for ( i=0; i<ExpertCount; i++ )
  {
    WriteInt( NewWinnerSock, ExpertGlobalInfo[i].anz_einsaetze );

    for ( j=0; j<ExpertGlobalInfo[i].anz_einsaetze; j++ )
    {
      WriteInt( NewWinnerSock, ExpertGlobalInfo[i].einsaetze[j] );
    }

    /* Beachte: Das erste Feld bei den Beurteilungen ist stets unbesetzt */
    for( j=1; j<=CycleCount; j++ )
    {
      WriteInt( NewWinnerSock, ExpertGlobalInfo[i].abs_bewertung[j] );
      WriteDouble( NewWinnerSock, &(ExpertGlobalInfo[i].rel_bewertung[j]) );
    }
    
    WriteInt( NewWinnerSock, ExpertGlobalInfo[i].wartekonto );
    WriteInt( NewWinnerSock, ExpertGlobalInfo[i].beurteilt );
  }
  
} /* Ende von expert_kzg_senden */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  experten_daten_senden                                    */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Kmponente versendet im Falle eines Leiterwechsels  */
/*                  alle Daten ueber die Experten, die der neue Leiter be-   */
/*                  noetigt. Dies sind die Daten des KZG und zwar die aller  */
/*                  in der Konfigurationsdatei angegebenen Experten.         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  TeamExpCount                                             */
/*                  Hostname[]                                               */
/*                  Hostport[]                                               */
/*                  WinnerHost                                               */
/*                  ExpertTeamInfo                                           */
/*                                                                           */
/*****************************************************************************/

static void experten_daten_senden ( void )
{
  short i;

  /*****************************************************/
  /* Senden der Teamzusammensetzung des letzten Zyklus */
  /*****************************************************/
  for ( i=0; i<TeamExpCount; i++ )
  {
    WriteInt( NewWinnerSock, ExpertTeamInfo[i].exp_nr );
  }

  expert_kzg_senden();

} /* Ende von experten_daten_senden */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  spezialisten_daten_senden                                */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Mit dieser Komponente empfaengt der Sieger, also der neue*/
/*                  Leiter des naechsten Zyklus die relevanten Daten der     */
/*                  Spezialisten.                                            */
/*                  Es wird zunaechst getestet, ob der Spezialist in der     */
/*                  Konfigurationsdatei angegeben wurde. Nur dann wird       */
/*                  gesendet.                                                */
/*                  Ist der Datbase-Spezialist angegeben, so ist die         */
/*                  Komponente "spezialist" auf true gesetzt.                */
/*                  Ist ein Reduktionsspezialist angegeben, so ist die       */
/*                  Variable RedSpecCount ungleich 0.                        */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void spezialisten_daten_senden ( void )
{
  short i, j;

  /**************************************/
  /* Behandeln des Datbase-Spezialisten */
  /**************************************/
  if( Database.spezialist )
  {
    WriteInt( NewWinnerSock, StartDomNr );
    WriteInt( NewWinnerSock, LookedDomains );

    WriteInt( NewWinnerSock, Database.anz_einsaetze );
    WriteInt( NewWinnerSock, Database.wartekonto );
    for ( i=0; i<Database.anz_einsaetze; i++ )
    {
      WriteInt( NewWinnerSock, Database.einsaetze[i] );
    }
  }

  /****************************************/
  /* Behandeln des Reduktionsspezialisten */
  /****************************************/
  if( RedSpecCount > 0 )
  {
    for( i=0; i<RedSpecCount; i++ )
    {
      WriteInt( NewWinnerSock, RedSpecInfo[i].anz_einsaetze );
      for ( j=0; j<RedSpecInfo[i].anz_einsaetze; j++ )
      {
        WriteInt( NewWinnerSock, RedSpecInfo[i].einsaetze[j] );
      } /* Ende des inneren for */
    } /* Ende des "au"seren for */
  } /* Ende von if( RedSpecCount > 0 ) */

} /* spezialisten_daten_senden */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  domaene_kzg_senden                                       */
/*                                                                           */
/*  Parameter    :  Domaenennumer, derne KZG-Daten gesendet werden sollen    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden die KZG-Daten der angegebenen Domaene an den   */
/*                  angegebenen Socket gesendet.                             */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void domaene_kzg_senden( short dom_nr )
{
  short i;

  WriteInt( NewWinnerSock, DomainGlobalInfo[dom_nr].anz_gef_matches );
  for( i=0; i<DomainGlobalInfo[dom_nr].anz_gef_matches; i++ )
  {
    WriteStream( NewWinnerSock, (void *)&(DomainGlobalInfo[dom_nr].DomMatches[i]),
                 sizeof( KZGDomainFrame ) );
  }
} /* Ende von domaene_kzg_senden */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  domaenen_daten_senden                                    */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Im Falle eine Leiterwechsels sendet der alte Leiter mit  */
/*                  dieser Komponente dem neuen Leiter die notwendigen In-   */
/*                  formationen ueber die bis dahin erkannten Domaenen.      */
/*                  Es werden zur Zeit die Informationen der Domaenen ge-    */
/*                  sendet, die irgendwann einmal eingelesen wurden, unbe-   */
/*                  ruecksichtigt der Tatsache, ob die Domaene noch als in-  */
/*                  teressant eingestuft ist oder nicht. Die Liste der in-   */
/*                  teressanten Domaenen wird am Ende extra gesendet.        */
/*                  Vielleicht geht es effiezienter, wenn man nur            */
/*                  die interessanten noch versendet?!                       */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void domaenen_daten_senden ( void )
{
  short i;

  WriteInt( NewWinnerSock, DomainCount );
  for( i=0; i<DomainCount; i++ )
  {
    WriteStream( NewWinnerSock, (void *)&(InterestDomain[i]), sizeof( DomMatchSpec ) );
  }

  for ( i=0; i<DGICount; i++ )
  {
    domaene_kzg_senden( i );
  }

} /* Ende von domaenen_daten_senden */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  zyklus_zeit_daten_senden                                 */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Im Falle eine Leiterwechsels sendet der alte Leiter mit  */
/*                  dieser Komponente dem neuen Leiter die notwendigen In-   */
/*                  formationen ueber die Berechnung der naechsten Zyklus-   */
/*                  zeit. Dies ist momentan nur der Wert der Variablen       */
/*                  timebase, die sich im Falle einer erkannten Domaene aen- */
/*                  dern kann.                                               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void zyklus_zeit_daten_senden ( void )
{
  WriteInt( NewWinnerSock, timebase );
} /* Ende von zyklus_zeit_daten_senden */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SendLeaderInfo                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente wird nur vom Leiter des letzten Zyklus  */
/*                  aufgerufen. Falls ein Leiterwechsel vollzogen werden     */
/*                  muss, so fuehrt diese Komponente den notwendigen Daten-  */
/*                  austausch mit dem neuen Leiter durch.                    */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
void SendLeaderInfo ( void )
{
  NewWinnerSock = br_Receivers[WinnerHost].Sock;

  experten_daten_senden();

  spezialisten_daten_senden();

  domaenen_daten_senden();

  zyklus_zeit_daten_senden();

} /* Ende von SendLeaderInfo */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  expert_kzg_empfangen                                     */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden die Daten des KZG ueber alle Experten empfangen*/
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  ExpertGlobalInfo                                         */
/*                  Input                                                    */
/*                  ExpertCount                                              */
/*                  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static void expert_kzg_empfangen ( void )
{
  short i;
  short j;

  /* Es werden nur die Experten betrachtet, die in der Konfigurationsdatei   */
  /* angegeben wurden.                                                       */
  for ( i=0; i<ExpertCount; i++ )
  {
    ExpertGlobalInfo[i].anz_einsaetze = ReadInt( Input );

    for ( j=0; j<ExpertGlobalInfo[i].anz_einsaetze; j++ )
    {
      ExpertGlobalInfo[i].einsaetze[j] = ReadInt( Input );
    }

    /* Beachte: Das erste Feld bei den Beurteilungen ist stets unbesetzt */
    for ( j=1; j<=CycleCount; j++ )
    {
      ExpertGlobalInfo[i].abs_bewertung[j] = ReadInt( Input );
      ExpertGlobalInfo[i].rel_bewertung[j] = ReadDouble( Input );
    }

    ExpertGlobalInfo[i].wartekonto = ReadInt( Input);
    ExpertGlobalInfo[i].beurteilt = ReadInt( Input);
  }

} /* Ende von expert_kzg_empfangen */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  experten_daten_empfangen                                 */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Mit dieser Komponente empfaengt der Sieger, also der neue*/
/*                  Leiter des naechsten Zyklus die relevanten Daten der Ex- */
/*                  perten.                                                  */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  Hostname[]                                               */
/*                  Hostport[]                                               */
/*                  MasterHost                                               */
/*                  TeamExpCount                                             */
/*                  Input                                                    */
/*                  ThisHost                                                 */
/*                  ExpertGlobalInfo                                         */
/*                  ExpertTeamInfo                                           */
/*                                                                           */
/*****************************************************************************/

static void experten_daten_empfangen ( void )
{
  short i;

  /* Empfangen der Teamzusammensetzung des letzten Zyklus */
  for ( i=0; i<TeamExpCount; i++ )
  {
            /* Expertennummern empfangen */
    ExpertTeamInfo[i].exp_nr = ReadInt( Input );
  }

  expert_kzg_empfangen( );

} /* experten_daten_empfangen */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  spezialisten_daten_empfangen                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Mit dieser Komponente empfaengt der Sieger, also der neue*/
/*                  Leiter des naechsten Zyklus die relevanten Daten der     */
/*                  Spezialisten.                                            */
/*                  Es wird zunaechst getestet, ob der Spezialist in der     */
/*                  Konfigurationsdatei angegeben wurde. Nur dann wird       */
/*                  gesendet.                                                */
/*                  Ist der Datbase-Spezialist angegeben, so ist die         */
/*                  Komponente "spezialist" auf true gesetzt.                */
/*                  Ist ein Reduktionsspezialist angegeben, so ist die       */
/*                  Variable RedSpecCount ungleich 0.                        */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void spezialisten_daten_empfangen ( void )
{
  short i, j;

  /**************************************/
  /* Behandeln des Datbase-Spezialisten */
  /**************************************/
  if( Database.spezialist )
  {
    StartDomNr = ReadInt( Input );
    LookedDomains = ReadInt( Input );

    Database.anz_einsaetze = ReadInt( Input );
    Database.wartekonto    = ReadInt( Input );
    for ( i=0; i<Database.anz_einsaetze; i++ )
    {
      Database.einsaetze[i] = ReadInt( Input );
    }
  }

  /****************************************/
  /* Behandeln des Reduktionsspezialisten */
  /****************************************/
  if( RedSpecCount > 0 )
  {
    for( i=0; i<RedSpecCount; i++ )
    {
      RedSpecInfo[i].anz_einsaetze = ReadInt( Input );
      for ( j=0; j<RedSpecInfo[i].anz_einsaetze; j++ )
      {
        RedSpecInfo[i].einsaetze[j] = ReadInt( Input );
      } /* Ende des innerenm for */
    } /* Ende des "au"seren for */
  } /* Ende von if( RedSpecCount > 0 ) */

} /* spezialisten_daten_empfangen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  domaene_kzg_empfangen                                    */
/*                                                                           */
/*  Parameter    :  Domaenennumer, deren KZG-Daten empfangen werden sollen   */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden die KZG-Daten der angegebenen Domaene an dem1  */
/*                  angegebenen Socket empfangen.                            */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void domaene_kzg_empfangen ( short dom_nr )
{
  short i;

  DomainGlobalInfo[dom_nr].anz_gef_matches = ReadInt( Input );
  for( i=0; i<DomainGlobalInfo[dom_nr].anz_gef_matches; i++ )
  {
    ReadStream( Input, (void *)&(DomainGlobalInfo[dom_nr].DomMatches[i]),
                sizeof( KZGDomainFrame ) );
  }
} /* Ende von domaene_kzg_empfangen */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  domaenen_daten_empfangen                                 */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente wird bei einem Leiterwechsel vom Sieger,*/
/*                  also vom neuen Leiter des naechsten Zyklus aufgerufen.   */
/*                  Hier empfaengt er alle Informationen ueber die Domaenen. */
/*                  Es werden nur die Domaenen gesendet, die irgendwann ein- */
/*                  mal gelesen wurden, und dem neuen Leiter noch nicht be-  */
/*                  kannt sind.                                              */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  Input                                                    */
/*                                                                           */
/*****************************************************************************/

static void domaenen_daten_empfangen ( void )
{
  short i;

  DomainCount= ReadInt( Input );
  for( i=0; i<DomainCount; i++ )
  {
    ReadStream( Input, (void *)&(InterestDomain[i]), sizeof( DomMatchSpec ) );
  }

  for( i=0; i<DGICount; i++ )
  {
    domaene_kzg_empfangen( i );
  }

} /* Ende von domaenen_daten_empfangen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  zyklus_zeit_daten_empfangen                              */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Im Falle eines Leiterwechsels empfaengt der neue Leiter  */
/*                  mit dieser Komponente alle notwendigen Daten zur Berech- */
/*                  nung der naechsten Zyklusdauer. Momentan ist dies nur    */
/*                  der Wert der Variablen timebase, die sich im Falle einer */
/*                  erkannten Domaene aendern kann.                          */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  timebase                                                 */
/*                  Input                                                    */
/*                                                                           */
/*****************************************************************************/

static void zyklus_zeit_daten_empfangen ( void )
{
  timebase = ReadInt( Input );
} /* Ende von zyklus_zeit_daten_empfangen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReceiveLeaderInfo                                        */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente wird vom Sieger der letzten Runde, also */
/*                  vom Leiter des naechsten Zyklus ausgefuehrt. In dieser   */
/*                  Komponente empfaengt er vom letzten Leiter die Leiter-   */
/*                  spezifischen Daten. Diese Komponente ist das passende    */
/*                  Gegenstueck zur Komponente SendLeaderInfo.               */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/
void ReceiveLeaderInfo ( void )
{
  experten_daten_empfangen();

  spezialisten_daten_empfangen();

  domaenen_daten_empfangen();

  zyklus_zeit_daten_empfangen();

} /* Ende von ReceiveLeaderInfo */


/*****************************************************************************/
/*                                                                           */
/*             Senden und Empfangen der Resultate der Teammitglieder         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReceiveResults                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente wird vom Leiter des naechsten Zyklus    */
/*                  aufgerufen. Er empfaengt in dieser Komponente die von den*/
/*                  Gutachtern ausgewaehlten Regeln und Gleichungen.         */
/*                  Nachdem er die Berichte der Gutachter empfangen hat,     */
/*                  sendet er sie alle an die anderen Teammitglieder, damit  */
/*                  auch diese wie er die Berichte in ihrer "History" able-  */
/*                  gen koennen.                                             */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  ExpertTeamInfo                                           */
/*                  TeamExpCount                                             */
/*                  ThisHost                                                 */
/*                  SetOfCriticalPairs                                       */
/*                  ReceivedEq                                               */
/*                                                                           */
/*****************************************************************************/
void ReceiveResults ( void )
{
   termpair    *tp;
   int         in;
   short       i, j, k;
   short       red_spec_idx;
   long        tpnum;
   long        clr_counter = 0;
   long        counter;
   long        oldsize;
   report      saverep[MAXEXPERT];
   
   /*******************************************/
   /* Ablegen des eigenen Gutachter-Berichtes */
   /*******************************************/
   bcopy ( &referee_report, &(saverep[ThisHost]), sizeof (report) );
   
   /************************************************************/
   /* Empfangen und Ablegen der Berichte der anderen Gutachter */
   /************************************************************/
   for ( i=0; i<TeamExpCount; i++ )
   {
      
      if ( i == ThisHost )    
      {
         /* Die Berechnung des Korrekturfaktors erfolgt in */
         /* Abh"angigkeit von der Laufzeit und ist daher bei */
         /* Repro-L"aufen nicht berechenbar ( die Variable */
         /* saverep[i].runtime ist auf 0 gesetzt ) --> Deswegen f"allt */
         /* die gesamte if-Struktur bei Reprol"aufen weg */ 
         if(!ReproMode)
         {
            /*******************************************************************/
            /* Es wird der Korrekturfaktor zur Relativierung der Ergebnisse an */
            /* Prozessorauslastung, Zyklusdauer und Anzahl der Termpaare be-   */
            /* rechnet.                                                        */
            /*******************************************************************/
            ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].korrektur_fac[CycleCount] = 
               100.0*(double)CycleTime[CycleCount-1]*(double)saverep[i].cputime/((double)saverep[i].runtime * (double)GesamtGroesse);
            
            /********************************************************/
            /* Es wird die normierte absolute Beurteilung berechnet */
            /********************************************************/
            ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].norm_abs_bew[CycleCount] =
               ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].korrektur_fac[CycleCount] *
                  (double)ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount];
         }
         continue;
      }
      
      printf ( "Ergebnisse von Experte %d:\n", i );
      flush ();
      
      /*******************/
      /* Synchronisation */
      /*******************/
      WriteInt ( br_Receivers[i].Sock, SYNCCODE );
      
      /***********************************************************************/
      /* Da der Leiter zum Senden eingerichtet wurde, muss hier sein Em-     */
      /* pfangssocket geoeffnet werden.                                      */
      /***********************************************************************/
      in = br_Receivers[i].Sock;
      
      /*****************************************/
      /* Empfangen des Berichts des Gutachters */
      /*****************************************/
      ReceiveReport ( in, &(saverep[i]) );

     
      /***********************************************************************/
      /* Jetzt erfolgt je nachdem, ob der Experte ein Spezialist oder keiner */
      /* war, eine Sonderbehandlung.                                         */
      /***********************************************************************/
      if ( ( ExpertTeamInfo[i].exp_nr == REDUCE_1 ) ||
          ( ExpertTeamInfo[i].exp_nr == REDUCE_2 ) ||
          ( ExpertTeamInfo[i].exp_nr == REDUCE_3 ) )
      {
         red_spec_idx = GetRedSpecIdx( ExpertTeamInfo[i].exp_nr );
         
         /********************************************************/
         /* Zuerst der Reduktionsspezialist f"ur kritische Paare */
         /********************************************************/
         if( RedSpecInfo[red_spec_idx].red_typ == CP )
         {
            /* Zunaechst werden die Nummern der kritischen Paare empfangen, die  */
            /* geloescht werden sollen.                                          */
            counter = 0;
            oldsize = SizeOf (SetOfCriticalPairs);
            while ( (tpnum = ReadInt (in)) != END_OF_TP )
            {
               counter++;
               if ((tp = CPByNumber (tpnum)))
               {
                  clr_counter++;
                  DeleteCP   ( tp );
                  deleteterm ( tp->left );
                  deleteterm ( tp->right );
                  deletepair ( tp );
               }
            } /* Ende von while */
            /*TPClearCPCache();*/ /* MK 14.4.1993 : als notwendig angesehen */
            /* DeleteCP korrigiert den Cache bereits --> TPClearCPCache nicht mehr n"otig */
            /* MK, 20.4.1994 */
            printf ( "%ld/%ld kritische Paare geloescht.\n", clr_counter, oldsize );
            
            k  = ReadInt ( in ); /* Anzahl der kritischen Paare, die gesendet werden */
            for ( j=0; j<k; j++)
            {
               tp = ReceivePair (in);
               tp->weight = CPWeight (tp);
               InsertCP ( tp );
            }
            if ( k )
            {
               printf ( "%d kritische Paare empfangen.\n",k );
            }
         } /* if RedSpecInfo...red_typ = CP */
         
         /*******************************************************/
         /* Jetzt der Reduktionsspezialist f"ur kritische Ziele */
         /*******************************************************/
         if( RedSpecInfo[red_spec_idx].red_typ == CG )
         {
            counter = 0;
            oldsize = SizeOf (SetOfCriticalGoals);
            while ( (tpnum = ReadInt (in)) != END_OF_TP )
            {
               counter++;
             if ((tp = CGByNumber (tpnum)))
             {
                clr_counter++;
                DeleteCritGoal ( tp );
                deleteterm ( tp->left );
                deleteterm ( tp->right );
                deletepair ( tp );
             }
            } /* Ende von while */
            /*TPClearCGCache();*/ /* DeleteCritGoal korrigiert den Cache bereits */
            
          printf ( "%ld/%ld kritische Ziele geloescht.\n", clr_counter, oldsize );
            
            k  = ReadInt ( in ); /* Anzahl der kritischen Ziele, die gesendet werden */
            for ( j=0; j<k; j++)
            {
               tp = ReceivePair (in);
               tp->weight = CGWeight (tp);
               InsertCritGoal ( tp );
            }
            if ( k )
            {
               printf ( "%d kritische Ziele empfangen.\n",k );
            }
         } /* Ende von if( (RedSpecInfo ... red_typ = CG */
         
         continue;
      } /* Ende von if --> Reduktionsspezialist liegt vor */
      

      if (ExpertTeamInfo[i].exp_nr == DATABASE)
      {         
         /* Empfangen der Daten, die der Leiter benoetigt, um sein Domaenen-*/
         /* wissen zu aktualisieren.                                        */
         /*******************************************/
         /* Empfangen der neu gefundenen Domaenen : */
         /*******************************************/
         FoundDomainCount = ReadInt( in );
         printf("  Es wurden %d Domaenen gefunden:\n", FoundDomainCount );
         for ( j=0; j<FoundDomainCount; j++ )
         {
            FoundDomain[j] = ReadInt( in ); /* Nummer der gefundenen Domaene */
            printf("    %s\n", DomainGlobalInfo[FoundDomain[j]].dom_name );
            /************************************/
            /* Empfangen des gefundenen Matches */
            /************************************/
            for ( k=1; k<=DomainGlobalInfo[FoundDomain[j]].dom_fkt_anz; k++ )
            {
               FoundDomainMatchFunc[j][k] = ReadInt( in );
            }
         }
         printf("\n");
         
         /********************************************************/
         /* Empfangen der Anzahl Domaenen, die betrachtet wurden */
         /********************************************************/
         LookedDomains = ReadInt( in );
         
      } /* if ... DATABASE */
      
      /**********************************************************************/
      /* Es werden die Ergebnisse eines Experten bzw. des Database-Experten */
      /* integriert.                                                        */
      /**********************************************************************/
      /*************************************************************/
      /* Die empfangenen Gleichungen und Regeln werden aufgenommen */
      /*************************************************************/
      for (j = 0; j < saverep[i].rule_count; j++)
      {
         tp = saverep[i].rule[j];
         tp->type = CRITPAIR;
         printf("  ");
         printtpair ( tp );
         Add ( &ReceivedEq, tp );
      }
      
      for (j = 0; j < saverep[i].equ_count; j++)
      {
         tp = saverep[i].equ[j];
         tp->type = CRITPAIR;
         printf("  ");
         printtpair ( tp );
         Add ( &ReceivedEq, tp );
      }
      
      /********************************************/
      /* Die empfangenen Ziele werden aufgenommen */
      /********************************************/
      if( Paramodulation )
      {
         for (j = 0; j < saverep[i].goal_count; j++)
         {
            tp = saverep[i].goal[j];
            tp->type = CRITGOAL;
            printf("  ");
            printtpair ( tp );
            Add ( &ReceivedGoals, tp );
         }
      } /* Ende von if ( Paramodulation ) */
      
      /* Die Berechnung des Korrekturfaktors erfolgt in Abh"angigkeit */
      /* von der Laufzeit und ist daher bei Repro-L"aufen nicht */
      /* berechenbar ( die Variable saverep[i].runtime ist auf 0 */
      /* gesetzt ) --> Deswegen f"allt die gesamte if-Struktur bei */
      /* Reprol"aufen weg */
      if(!ReproMode)
      {
         if ( ExpertTeamInfo[i].exp_nr != DATABASE )
         {
            /*******************************************************************/
            /* Es wird der Korrekturfaktor zur Relativierung der Ergebnisse an */
            /* Prozessorauslastung, Zyklusdauer und Anzahl der Termpaare be-   */
            /* rechnet.                                                        */
            /*******************************************************************/
            ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].korrektur_fac[CycleCount] = 
               100.0*(double)CycleTime[CycleCount-1]*(double)saverep[i].cputime/((double)saverep[i].runtime * (double)GesamtGroesse);
            
            /********************************************************/
            /* Es wird die normierte absolute Beurteilung berechnet */
            /********************************************************/
            ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].norm_abs_bew[CycleCount] =
               ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].korrektur_fac[CycleCount] *
                  (double)ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].abs_bewertung[CycleCount];
         }
      }
   } /* Ende von for */
   
   flush();
   
   /****************************************************************************/
   /* Es werden nun noch die Berichte aller an alle anderen Rechner verschickt */
   /****************************************************************************/
   for ( i=0; i<TeamExpCount; i++ )
   {
      AddReport ( i, &(saverep[i]) );
      
      if ( i != ThisHost )
      {
         for ( j=0; j<TeamExpCount; j++ )
         {
            SendReport (  br_Receivers[i].Sock, &(saverep[j]) );
         } /* Ende des inneren for */
      } /* Ende von if */
   } /* Ende des aeusseren for */
} /* Ende von ReceiveResults */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SendResults                                              */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente ist das passende Gegenstueck zur        */
/*                  Komponente ReceiveResults und wird von den Experten auf- */
/*                  gerufen, die nicht Leiter des naechsten Zyklus geworden  */
/*                  sind. In ihr werden die von den Gutachtern ausgewaehlten */
/*                  Regeln und Gleichungen an den neuen Leiter gesendet.     */
/*                  Vorher gibt der Experte die ausgewaehlten Termpaare noch */
/*                  aus und nach dem Senden empfaengt er die Berichte aller  */
/*                  Gutachter im Team, um sie in seiner History abzuspeichern*/
/*                                                                           */
/*                  Bei den Spezialisten erfolgt eine Sonderbehandlung!      */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  Input                                                    */
/*                  referee_report                                           */
/*                  Hostname                                                 */
/*                  Hostport                                                 */
/*                  SetOfCriticalPairs                                       */
/*                  OwnConfig                                                */
/*                  TeamExpCount                                             */
/*                                                                           */
/*****************************************************************************/
void SendResults ( void )
{
  termpair    *tp;
  int         out, i, j;
  int         counter = 0;
  report      rep;
  short       red_spec_idx;

  /**************************/
  /* Synchronisierung lesen */
  /**************************/
  (void)ReadInt ( Input );

  /******************************************************/
  /* Ausgeben der vom Gutachter ausgewaehlten Termpaare */
  /* wenn es kein Reduktionsspezialist ist.             */
  /******************************************************/
  if ( ( OwnConfig.exp_nr != REDUCE_1 ) || ( OwnConfig.exp_nr != REDUCE_2 ) ||
       ( OwnConfig.exp_nr != REDUCE_3 ) )
  {
    /*********************************************************************/
    /* Ergebnisse des Database-Experten nur wenn Dom"anen erkannt wurden */
    /*********************************************************************/
    if( (OwnConfig.exp_nr != DATABASE) || FoundDomainCount )
    {
      printf ( "Ausgewaehlte Resultate:\n\n" );
      printf("Regeln:\n");
      for (i = 0; i < referee_report.rule_count; i++)
      {
        printtpair ( referee_report.rule[i] );
      }

      printf("Gleichungen:\n");
      for (i = 0; i < referee_report.equ_count; i++)
      {
        printtpair ( referee_report.equ[i] );
      }

      if( Paramodulation )
      {
        printf("Ziele:\n");
        for (i = 0; i < referee_report.goal_count; i++)
        {
          printtpair ( referee_report.goal[i] );
        }
      } /* Ende von if( Paramodulation ) */
    } /* Ende von if( OwnConfig.exp_nr != DATABASE... */
  } /* Ende von if OwnConfig.exp_nr != REDUCE_1,  ...REDUCE_2, ...REDUCE_3 */

  /************************/
  /* Senden des Berichtes */
  /************************/
  out = br_Receivers[MasterHost].Sock;

  SendReport ( out, &referee_report );

  /**********************************************************/
  /* Sonderbehandlung, falls der Experte ein Spezialist war */
  /**********************************************************/
  if ( ( OwnConfig.exp_nr == REDUCE_1 ) || ( OwnConfig.exp_nr == REDUCE_2 ) ||
       ( OwnConfig.exp_nr == REDUCE_3 ) )
  {
    red_spec_idx = GetRedSpecIdx( OwnConfig.exp_nr );
    /********************************************************/
    /* Zuerst der Reduktionsspezialist f"ur kritische Paare */
    /********************************************************/
    if( RedSpecInfo[red_spec_idx].red_typ == CP )
    {
      /**************************************************************************/
      /* Senden der Nummern der kritischen Paare, die geloescht werden koennen. */
      /**************************************************************************/
      tp = SetOfCriticalPairs.first;
      while ( tp )
      {
        if (tp->state == CLEARED) /* Dieses CP wird endgueltig geloescht */
        {
          WriteInt ( out, tp->number ); 
        }

        if (tp->state == MARKED)
        {  /* Diese CP werden spaeter wieder gesendet */
          WriteInt ( out, tp->number );
          counter++;
        }
        
        tp = tp->next;
      } /* Ende von while */

      /**********************************************/
      /* Als Endemarkierung wird END_OF_TP gesendet */
      /**********************************************/
      WriteInt( out, END_OF_TP );

      WriteInt ( out, counter );

      tp = SetOfCriticalPairs.first;
      while (tp)
      {
        if (tp->state == MARKED)
        {
          SendPair ( out, tp );
        }
        
        tp = tp->next;
      } /* Ende von while */
    } /* Ende von if(RedSpecInfo...red_typ == CP */

    /*******************************************************/
    /* Jetzt der Reduktionsspezialist f"ur kritische Ziele */
    /*******************************************************/
    if( RedSpecInfo[red_spec_idx].red_typ == CG )
    {
      /********************************************************/
      /* Senden der krit. Ziele, die gel"oscht werden k"onnen */
      /********************************************************/
      tp = SetOfCriticalGoals.first;
      while ( tp )
      {
        if (tp->state == CLEARED) /* Dieses CG wird endgueltig geloescht */
        {
          WriteInt ( out, tp->number ); 
        }

        if (tp->state == MARKED)
        {  /* Diese CG werden spaeter wieder gesendet */
          WriteInt ( out, tp->number );
          counter++;
        }
        
        tp = tp->next;
      } /* Ende von while */
      /**********************************************/
      /* Als Endemarkierung wird END_OF_TP gesendet */
      /**********************************************/
      WriteInt( out, END_OF_TP );

      WriteInt ( out, counter );

      tp = SetOfCriticalGoals.first;
      while (tp)
      {
        if (tp->state == MARKED)
        {
          SendPair ( out, tp );
        }
        
        tp = tp->next;
      } /* Ende von while */
    } /* Ende von if( (RedSpecInfo ... red_typ = CG */
  } /* Ende von if --> Reduktionsspezialist liegt vor */

  else if ( OwnConfig.exp_nr == DATABASE )
  {
    /* Im folgenden verschickt der Database-Experte noch die Informationen,   */
    /* die der Leiter benoetigt, um sein Domaenenwissen zu aktualisieren.     */
    /****************************************/
    /* Senden der neu gefundenen Domaenen : */
    /****************************************/
    printf("Es wurde%s %d Domaene%s gefunden.\n",
            FoundDomainCount == 1 ? "" : "n", FoundDomainCount,
            FoundDomainCount == 1 ? "" : "n" );
    WriteInt( out, FoundDomainCount ); /* Zuerst die Anzahl */
    for ( i=0; i<FoundDomainCount; i++ )
    {
      printf("Domaene: %s\n", DomainGlobalInfo[FoundDomain[i]].dom_name );
      WriteInt( out, FoundDomain[i] );   /* Nummer der gefundenen Domaene     */
      /************************************/
      /* Versenden des gefundenen Matches */
      /************************************/
      for ( j=1; j<=DomainGlobalInfo[FoundDomain[i]].dom_fkt_anz; j++ )
      {
        WriteInt( out, FoundDomainMatchFunc[i][j] );
      }
    }

    /******************************************************/
    /* Senden der Anzahl Domaenen, die betrachtet wurden. */
    /******************************************************/
    WriteInt( out, LookedDomains );

    /*********************************************************/
    /* Zuruecksetzen aller erforderlichen globalen Variablen */
    /*********************************************************/
    ResetDomainVariables();

  } /* Ende von if...DATABASE */

  /**************************************************************************/
  /* Empfangen der Berichte aller Gutachter, also auch des eigenen nochmal  */
  /* und Ablegen in der eigenen History.                                    */
  /**************************************************************************/
  for ( i=0; i<TeamExpCount; i++ )
  {
    ReceiveReport ( Input, &rep );
    AddReport ( i, &rep );
  }
} /* Ende von SendResults */

/*****************************************************************************/
/*                                                                           */
/*             Senden und Empfangen der neuen Siegernummer                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SendWinnerNo                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Senden der Rechnernummer des Siegers.                    */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  WinnerHost                                               */
/*                  TeamExpCount                                             */
/*                  ThisHost                                                 */
/*                                                                           */
/*****************************************************************************/

void SendWinnerNo ( void )
{
  short i;

  for ( i=0; i<TeamExpCount; i++ )
  {
    if ( i != ThisHost )
    {
      WriteInt( br_Receivers[i].Sock, WinnerHost );
    } /* Ende von if */
  } /* Ende von for */

  if( br_SetMaster( br_Receivers[WinnerHost].Host ) == -1 )
        br_PrintError();
} /* Ende von SendWinnerNo */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReceiveWinnerNo                                          */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Empfangen der Rechnernummer des Siegers.                 */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  WinnerHost                                               */
/*                  Input                                                    */
/*                                                                           */
/*****************************************************************************/

void ReceiveWinnerNo ( void )
{
  WinnerHost = ReadInt( Input );

  if (WinnerHost == ThisHost)
  {
    if (!DemoMode)
    {
      printf ( "Ich werde Master (%d).\n", WinnerHost );
    }
  } /* Ende von if */
  else
  {
    printf ( "Naechster Master %d.\n", WinnerHost );
  }
  flush();
  
  if( br_SetMaster( br_Receivers[WinnerHost].Host ) == -1 )
        br_PrintError();

} /* Ende von ReceiveWinnerNo */



/*****************************************************************************/
/*                                                                           */
/*             Senden und Empfangen der naechsten Zyklusdauer                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SendCycleTime                                            */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Senden der Zykluszeit fuer den naechsten Zyklus.         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  TeamExpCount                                             */
/*                  ThisHost                                                 */
/*                  CycleTime                                                */
/*                  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

void SendCycleTime ( void )
{
  short i;

  for ( i=0; i<TeamExpCount; i++ )
  {
    if ( i != ThisHost )
    {
      WriteInt( br_Receivers[i].Sock, CycleTime[CycleCount] );
    } /* Ende von if */
  } /* Ende von for */
} /* Ende von SendCycleTime */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReceiveCycleTime                                         */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Empfangen der Zykluszeit fuer den naechsten Zyklus.      */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  CycleTime                                                */
/*                  CycleCount                                               */
/*                  Input                                                    */
/*                  referee_report                                           */
/*                                                                           */
/*****************************************************************************/

void ReceiveCycleTime ( void )
{
  referee_report.runtime = CycleTime[CycleCount] = ReadInt( Input );
} /* Ende von ReceiveCycleTime */



/*****************************************************************************/
/*                                                                           */
/*             Senden und Empfangen der naechsten Teamkonfiguration          */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DistributeTeam                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Dieses Modul wird nur vom Leiter aufgerufen. Er sendet   */
/*                  an alle anderen Teammitglieder die Konfigurationen der   */
/*                  Experten, die auf diesen Rechnern laufen sollen.         */
/*                  Diese Komponente ist das passende Gegenstueck zu         */
/*                  ReceiveNextConfiguration.                                */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  ExpertGlobalInfo                                         */
/*                  ExpertTeamInfo                                           */
/*                  ThisHost                                                 */
/*                  TeamExpCount                                             */
/*                                                                           */
/*****************************************************************************/

void DistributeTeam ( void )
{
  short i;
  short j;
  short red_spec_idx; /* Index eines Reduktionsspezialisten */

  int out; /* Socket, an den gesendet wird */

  for ( i=0; i<TeamExpCount; i++ )
  {

    /********************************/
    /* Sonderbehandlung beim Sieger */
    /********************************/
    /* Der Sieger versendet sein System natuerlich nicht, sondern setzt die */
    /* Variable OwnConfig direkt.                                           */
    if ( i == ThisHost )
    {
        if ( exp_ist_spezialist( OwnConfig.exp_nr = ExpertTeamInfo[i].exp_nr ) )
        {
          Error ( __FILE__ ": " "DistributeTeam", "Leiter darf kein Spezialist sein!");
        }

        OwnConfig.exp_nr = ExpertTeamInfo[i].exp_nr;

        for ( j=0; j<ExpertGlobalInfo[OwnConfig.exp_nr].cpparam_anz; j++ )
        {
          OwnConfig.cp_parameter[j] = ExpertTeamInfo[i].cp_parameter[j];
        }
    
        for ( j=0; j<ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].cgparam_anz; j++ )
        {
          OwnConfig.cg_parameter[j] = ExpertTeamInfo[i].cg_parameter[j];
        }

        continue;
    } /* Ende von if ( i== ThisHost ) */

    /*****************************************************/
    /* Senden der Konfiguration an das i-te Teammitglied */
    /*****************************************************/

    out = br_Receivers[i].Sock;

    /* Zuerst wird die Anzahl der im Team eingesetzten Experten transferiert;*/
    /* Dies muss jeder Experte wissen, um alle Ergebnisse in SendResults zu  */
    /* E M P F A N G E N!!!!                                                 */
    WriteInt( out, TeamExpCount );

    /*****************************/
    /* Senden der Expertennummer */
    /*****************************/
    WriteInt( out, ExpertTeamInfo[i].exp_nr );

    /* Senden der Parameter, beachte, dass alle Rechner denselben Aufbau von */
    /* ExpertGlobalInfo haben, so dass nur der Index und die aktuellen Para- */
    /* meterwerte gesendet werden muessen.( Auch wenn dies zur Zeit noch die */
    /* Default-Werte sind, meistens ).                                       */
    /* Alle Rechner haben auch denselben Aufbau bei den Spezialisten!        */
    /*************************************/
    /* Senden der CP-Funktion-Parameter  */
    /*************************************/
    /* Sonderbehandlung bei Spezialisten */
    /*************************************/
    if ( exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) )
    {
      switch ( ExpertTeamInfo[i].exp_nr )
      {
        case DATABASE  : /* Senden der Anzahl der Parameter */
                         WriteInt( out, ExpertTeamInfo[i].cp_parameter[0] );
                         
                         /* Senden der Parameter */
                         for ( j=1; j<=ExpertTeamInfo[i].cp_parameter[0]; j++ )
                         {
                           WriteInt( out, ExpertTeamInfo[i].cp_parameter[j] );
                         }
                        
                         /* Senden des Suchmodus */
                         WriteInt( out, ExpertTeamInfo[i].cg_parameter[0] );
                         break;

        case REDUCE_1  :
        case REDUCE_2  :
        case REDUCE_3  : red_spec_idx = GetRedSpecIdx(ExpertTeamInfo[i].exp_nr);
                         
                         if( RedSpecInfo[red_spec_idx].red_typ == CP )
                         {
                           for ( j=0; j<4; j++ )
                           {
                             WriteInt( out, ExpertTeamInfo[i].cp_parameter[j] );
                           }
                         }
                         if( RedSpecInfo[red_spec_idx].red_typ == CG )
                         {
                           for ( j=0; j<4; j++ )
                           {
                             WriteInt( out, ExpertTeamInfo[i].cg_parameter[j] );
                           }
                         }
                         break;

        default        : Error( __FILE__ ": " "DistributeTeam", "Unbekannter Spezialist." );
                         break;
      } /* Ende von switch */
    } /* Ende von if */
    else   /* Experte */
    {
      for ( j=0; j<ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].cpparam_anz; j++ )
      {
        WriteInt( out, ExpertTeamInfo[i].cp_parameter[j] );
      }

      /************************************/
      /* Senden der CG-Funktion-Parameter */
      /************************************/
      if (Paramodulation)
      {
        for ( j=0; j<ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].cgparam_anz; j++ )
        {
          WriteInt( out, ExpertTeamInfo[i].cg_parameter[j] );
        } /* Ende von for */
      } /* Ende von if */
    } /* Ende von else */

    /* Damit jeder Rechner weiss, wie lange die Planung gedauert hat, um    */
    /* bei einem erfolgreichem Abbruch ein sinnvolles Ergebnis zu liefern!  */
    WriteInt( out, GesamtPlanungszeit );

  } /* Ende der for-Schleife */

} /* DistributeTeam */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReceiveNextConfiguration                                 */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die Experten, die nicht Leiter sind empfangen in dieser  */
/*                  Komponente ihre Konfiguration fuer den Zyklus und legen  */
/*                  diese in der Variablen OwnConfig ab. Diese Komponente    */
/*                  ist das paasende Gegenstueck zu DistributeTeam.          */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  OwnConfig                                                */
/*                  Input                                                    */
/*                                                                           */
/*****************************************************************************/

void ReceiveNextConfiguration ( void )
{
  short j;
  short red_spec_idx; /* Index eines Reduktionsspezialisten */

  /* Zuerst wird die Anzahl der im Team eingesetzten Experten empfangen;   */
  /* Dies muss jeder Experte wissen, um alle Ergebnisse in SendResults zu  */
  /* E M P F A N G E N!!!!                                                 */
  TeamExpCount = ReadInt( Input );

  /********************************/
  /* Empfangen der Expertennummer */
  /********************************/
  OwnConfig.exp_nr = ReadInt( Input );

  /***************************************/
  /* Empfangen der CP-Funktion-Parameter */
  /***************************************/
  /* Sonderbehandlung bei Spezialisten   */
  /***************************************/
  if ( exp_ist_spezialist( OwnConfig.exp_nr ) )
  {
    switch ( OwnConfig.exp_nr )
    {
      case DATABASE  : /* Empfangen der Anzahl der Parameter */
                       OwnConfig.cp_parameter[0] = ReadInt ( Input );

                       /* Empfangen der Parameter */
                       for ( j=1; j<=OwnConfig.cp_parameter[0]; j++ )
                       {
                         OwnConfig.cp_parameter[j] = ReadInt ( Input );
                       }

                       /* Empfangen des Suchmodus */
                       OwnConfig.cg_parameter[0] = ReadInt ( Input );

                       break;

      case REDUCE_1  :
      case REDUCE_2  :
      case REDUCE_3  : red_spec_idx = GetRedSpecIdx(OwnConfig.exp_nr);

                       if( RedSpecInfo[red_spec_idx].red_typ == CP )
                       {
                         for ( j=0; j<4; j++ )
                         {
                           OwnConfig.cp_parameter[j] = ReadInt ( Input );
                         }
                       }
                       if( RedSpecInfo[red_spec_idx].red_typ == CG )
                       {
                         for ( j=0; j<4; j++ )
                         {
                           OwnConfig.cg_parameter[j] = ReadInt ( Input );
                         }
                       }
                       break;

      default        : Error( __FILE__ ": " "ReceiveNextConfiguration", "Unbekannter Spezialist." );
                       break;
    } /* Ende von switch */
  } /*Ende von if */
  else   
  /***********/
  /* Experte */
  /***********/
  {
    for ( j=0; j<ExpertGlobalInfo[OwnConfig.exp_nr].cpparam_anz; j++ )
    {
      OwnConfig.cp_parameter[j] = ReadInt ( Input );
    }

    /***************************************/
    /* Empfangen der CG-Funktion-Parameter */
    /***************************************/
    if (Paramodulation)
    {
      for ( j=0; j<ExpertGlobalInfo[OwnConfig.exp_nr].cgparam_anz; j++ )
      {
        OwnConfig.cg_parameter[j] = ReadInt ( Input );
      } /* for */
    } /* if */
  }/* else */

  /* Damit jeder Rechner weiss, wie lange die Planung gedauert hat, um    */
  /* bei einem erfolgreichem Abbruch ein sinnvolles Ergebnis zu liefern!  */
  GesamtPlanungszeit = ReadInt ( Input );

} /* ReceiveNextConfiguration */
