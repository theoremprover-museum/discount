/*

  Modul:        br_error.h

  benutzt:      -

  includes:     br_types.h

                stdio.h

  exportiert:   ErrorMsgs

  privat:       -

  Beschreibung: enthaelt alle Fehlermeldungen

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: -

  17.03.1993  Fehlermeldung BF (Buffer flushed) entfernt

  24.04.1995 Aufnahme der Funktionskoepfe aus br_error.c, um ANSI Standard zu erreichen
             (MK mit Unterstuetzung von T. L.)
  
*/
#ifndef __ERRORS__
#define __ERRORS__


/*
  Include-Dateien
*/
#include <stdio.h>
#include "br_types.h"

/* T.L. */
typedef void (*Terminatefunction)(void);

/* extern const char* ErrorMsgs[]; */
extern tErrorcode br_Errorcode;

/* T.L. */
extern Terminatefunction Terminate;

void br_Error (char *text, tErrorcode code);
void br_PrintError (void);


#endif
