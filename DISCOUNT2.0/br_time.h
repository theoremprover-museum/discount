/*

  Modul:        br_time.h

  benutzt:      -

  includes:     br_types.h
                br_stat.h
		sys/time.h

  exportiert:   tTime

  privat:       -

  Beschreibung: enthaelt den neuen Datentypen tTime

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: -

*/
#ifndef __BRTIME__
#define __BRTIME__

/*
  Include-Dateien
*/
#include <sys/time.h>
#include "br_types.h"
#include "br_stat.h"


typedef struct tme{
	long start;
	long elapsed;
} tTime;


#endif
