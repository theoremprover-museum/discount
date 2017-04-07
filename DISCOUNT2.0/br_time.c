/*

  Modul:        br_time.c

  benutzt:      -

  includes:     br_time.h

  exportiert:   br_StartWatch ( ... )
                br_StoppWatch ( ... )

  privat:       -

  Beschreibung: enthaelt Funktionen zur Zeitmessung

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: -

*/


/*
  Include-Dateien
*/
#include "br_time.h"



/*
	Funktion:      br_StartWatch ( watch )

	Parameter:     tTime* watch      Stoppuhr die gestartet wird

	Rueckgabewert: -1: Fehler
			0: kein Fehler

	benutzt:       -

	Beschreibung:  -startet eine Stoppuhr

*/

int br_StartWatch ( watch )

	tTime* watch;

{	
	struct timeval   TimeVal; 
	struct timezone  TimeZone;

	TimeVal.tv_sec = 0L;
	TimeVal.tv_usec = 0L;
	TimeZone.tz_minuteswest = 0;
	TimeZone.tz_dsttime = DST_MET;
	
	gettimeofday ( &TimeVal, &TimeZone );
	
	watch->start = (TimeVal.tv_sec & 0xfffffL) * 1000L + (TimeVal.tv_usec / 1000L);

};



/*
	Funktion:      br_StoppWatch ( watch )

	Parameter:     tTime* watch     Stoppuhr die angehalten wird

	Rueckgabewert: -1: Fehler
			0: kein Fehler

	benutzt:       -

	Beschreibung:  -haelt eine Stoppuhr an und schreibt die seit dem 
	                Start verstrichene Zeit nach watch.elapsed
*/

int br_StoppWatch ( watch )

	tTime* watch;

{
	struct timeval   TimeVal; 
	struct timezone  TimeZone;
	
	TimeVal.tv_sec = 0L;
	TimeVal.tv_usec = 0L;
	TimeZone.tz_minuteswest = 0;
	TimeZone.tz_dsttime = DST_MET;
	
	gettimeofday ( &TimeVal, &TimeZone );

	watch->elapsed = ((TimeVal.tv_sec & 0xfffffL) * 1000L + (TimeVal.tv_usec / 1000L)) - watch->start;

};



