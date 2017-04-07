/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          systime
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Bestimmung der Systemzeit
//-----------------------------------------------------------------------------
//      $Log: systime.c,v $
//
//      Revision 0.2  1993/03/03  14:36:12  lind
//      Brodcast-Unit in quit_handler ordnungsgemaess beendet 
//      (br_CloseRecvBroadcast( ... ))
//      
//      Revision 0.1  1993/03/03  10:06:12  lind
//      bedingte Compilierung fuer Broadcast-Statistic ergaenzt
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/


#include    <signal.h> 
#include    <sys/time.h>

#include    "systime.h"
#include    "br_types.h"
#include    "pcl.h"
#include    "complet.h"
#include    "exp_def_next_cycletime.h"

#ifdef STATISTICS

#include          "br_stat.h"
extern tString    br_Hostname;
extern tStatistic br_BroadcastStatistic;

#endif

/*
//-----------------------------------------------------------------------------
//      lokale Datenvereinbarungen
//-----------------------------------------------------------------------------
*/

       long                     cputime;

static bool                     *timerflag;
static struct itimerval         timer, value, cputimer;


/*
//-----------------------------------------------------------------------------
//      lokale Funktionvereinbarungen
//-----------------------------------------------------------------------------
*/

#ifdef SVR4
static void  kill_handler();
static void  ti_handler();
static void  hangup ();
#else
static void  kill_handler ( int sig, int code, 
			   struct sigcontext *scp, char *addr );

static void  ti_handler ( int sig, int code, 
			 struct sigcontext *scp, char *addr );

static void  hangup ( int sig, int code, 
		     struct sigcontext *scp, char *addr );
#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       interrupt_handler
//
//  Parameter:      ...
//
//  Beschreibung:   Diese Funktion wird asynchron aufgerufen, falls
//                  das Signal SIGINT gesendet wird
//-----------------------------------------------------------------------------
*/

#ifndef SVR4
void  interrupt_handler ( int sig, int code, struct sigcontext *scp, char *addr )
#else
void  interrupt_handler (  )
#endif
{
  printf("\n\n");
  printf ( "----------------------------------------------------------------\n" );
  printf ( "        Programm terminiert durch INTERRUPT-Signal.\n" );
  printf ( "----------------------------------------------------------------\n\n" );
  flush ();
  printf("\n\n");
  itime   = systime ()-itime;
  printf ( "Bisherige Laufzeit              : %ld.%03ld s\n\n", itime/1000, itime%1000 );
  flush();

  br_CloseRecvBroadcast();

  if (!(SilentMode || NullMode))
  {
    getchar ();
  }

  exit(0);

} /* Ende von  interrupt_handler */





/*
//-----------------------------------------------------------------------------
//  Funktion:       quit_handler
//
//  Parameter:      ...
//
//  Beschreibung:   Diese Funktion wird asynchron aufgerufen, falls 
//                  das Signal SIGQUIT gesendet wird, d.h. anderer Rechner hat den
//                  Beweis gefunden.
//-----------------------------------------------------------------------------
*/

#ifndef SVR4
static void  quit_handler ( int sig, int code, struct sigcontext *scp, char *addr )
#else
static void  quit_handler ( )
#endif
{
#ifdef STATISTICS

   tString filename;

#endif

   if(ReproMode)
   {
      PCL_COMMENT("Schlafender Prozess terminiert",true);
      PCL_CLOSE();
      PCL_EXTRACT(true);
   }
   else
   {
      PCL_CLOSE();
      PCL_CLEAN();
      PCL_EXTRACT(true);
   }
   printf ( "\n\n" );
   printf ( "----------------------------------------------------------------\n" );
   printf ( "        Programm terminiert durch QUIT-Signal.\n" );
   printf ( "----------------------------------------------------------------\n\n" );
   flush ();
   
   br_CloseRecvBroadcast();
   
#ifdef STATISTICS
   
   strcpy( filename, br_Hostname );
   strcat( filename, ".stat" );
   stat_ShowStatistic( &br_BroadcastStatistic, filename );
   
#endif
   
   if (!(SilentMode || NullMode))
      getchar ();
   
   exit (0);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ti_handler
//
//  Parameter:      ...
//
//  Beschreibung:   Diese Funktion wird asynchron aufgerufen, falls 
//                  der Timer ablaeuft.
//-----------------------------------------------------------------------------
*/

#ifndef SVR4
static void  ti_handler ( int sig, int code, struct sigcontext *scp, char *addr )
#else
static void  ti_handler (  )
#endif
{
    *timerflag = true;

    getitimer ( ITIMER_VIRTUAL, &value );
    cputime = 1000000000 - 
              (value.it_value.tv_sec*1000 + (value.it_value.tv_usec+500) /1000);

    timer.it_interval.tv_sec  =  1000000;
    timer.it_interval.tv_usec =  0;

    timer.it_value.tv_sec  =  1000000;
    timer.it_value.tv_usec =  0;

    setitimer ( ITIMER_REAL,    &timer, NULL );
    setitimer ( ITIMER_VIRTUAL, &timer, NULL );
}

#ifndef SVR4
static void  hangup ( int sig, int code, struct sigcontext *scp, char *addr )
#else
static void  hangup ( )
#endif
{
    printf ( "\n\nProgrammabbruch\n\n" );
    exit ( 1 );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       InitKillHandler
//
//  Parameter:      -keine-
//
//  Beschreibung:   Initialisere Kill-Handler
//-----------------------------------------------------------------------------
*/

void    InitKillHandler ( void )
{
    signal ( SIGQUIT, quit_handler );
}




/*
//-----------------------------------------------------------------------------
//  Funktion:       InitInterruptHandler
//
//  Parameter:      -keine-
//
//  Beschreibung:   Initialisere Interrupt-Handler
//-----------------------------------------------------------------------------
*/

void    InitInterruptHandler ( void )
{
    signal ( SIGINT, interrupt_handler );
} /* Ende von InitInterruptHandler */






/*
//-----------------------------------------------------------------------------
//  Funktion:       settimer ( long sec, bool *bptr )
//
//  Parameter:      sec     Laufzeit in Sekunden
//
//  Beschreibung:   Bestimmung einer Systemzeit in Millisekunden
//                  In Abh"angigkeit des Wertes von timeunit wird der Parameter
//                  als Angabe in Sekunden oder in Millisekunden interpretiert
//                  ("Anderung von MK )
//
//  Rueckgabewert:  Ein Zeitzaehler in ms
//-----------------------------------------------------------------------------
*/

void    settimer ( long sec, bool *bptr )
{
    if( timeunit == SECONDS )
    {
      timer.it_interval.tv_sec  =  sec;
      timer.it_interval.tv_usec =  0;

      timer.it_value.tv_sec  =  sec;
      timer.it_value.tv_usec =  0;
    }
    else
    {
      timer.it_interval.tv_sec  =  sec/1000;
      timer.it_interval.tv_usec =  sec%1000;

      timer.it_value.tv_sec  =  sec/1000;
      timer.it_value.tv_usec =  sec%1000;
    } /* Ende von else */

    timerflag = bptr;
    *timerflag = false;

    cputimer.it_interval.tv_sec  =  1000000;
    cputimer.it_interval.tv_usec =  0;

    cputimer.it_value.tv_sec  =  1000000;
    cputimer.it_value.tv_usec =  0;

    setitimer ( ITIMER_REAL,    &timer,    NULL );
    setitimer ( ITIMER_VIRTUAL, &cputimer, NULL );

    signal ( SIGALRM,   ti_handler );
    signal ( SIGVTALRM, ti_handler );

    signal ( SIGHUP,    hangup );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       systime
//
//  Parameter:      -keine-
//
//  Beschreibung:   Bestimmung einer Systemzeit in Millisekunden
//
//  Rueckgabewert:  Ein Zeitzaehler in ms
//-----------------------------------------------------------------------------
*/

long    systime ( void )
{
    struct timeval   TimeVal  = { 0L, 0L };
    struct timezone  TimeZone = { 0,  DST_MET };

#ifndef SVR4
    gettimeofday ( &TimeVal, &TimeZone );
#else
    gettimeofday ( &TimeVal, NULL);
#endif

    return (TimeVal.tv_sec & 0xfffffL) * 1000L + (TimeVal.tv_usec / 1000L);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       fprinttime ( FILE *log_f );
//
//  Parameter:      log_f      Ausgabestream
//
//  Beschreibung:   Ausgabe der Systemzeit 
//-----------------------------------------------------------------------------
*/

void    fprinttime ( FILE *log_f )
{
     time_t     clock;
     struct tm  *loctime;
     char       buffer[40];

     time ( &clock );
     loctime = localtime ( &clock );
     strftime ( buffer, 40, "%a", loctime );
     fprintf ( log_f, "%s", buffer );
}

