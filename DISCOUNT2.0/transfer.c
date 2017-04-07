/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          transfer
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Datenuebertragung zwischen Experten
//-----------------------------------------------------------------------------
//      $Log: transfer.c,v $
//      Revision 0.3  1992/03/25  12:56:47  pitz
//      Goals werden jetzt vor den ersten kritischen Paaren uebertragen.
//      Dies ist fuer GOAL*** erforderlich !
//
//      Revision 0.2  1991/09/05  14:43:15  pitz
//      Uebertragung von special_flag bei Termpaaren und kritischen Paaren.
//      Anpassung der Bewertung von kritischen Paaren bei speziellen
//      kritischen Paaren.
//
//      Revision 0.1  1991/08/19  09:50:44  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/


#include    <stdio.h>
#include    <string.h>

#include    <sys/param.h>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <netdb.h>

#ifdef    ATARI
    #include    <stdlib.h>
    #include    <ext.h>
#endif

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
#include    "complet.h"
#include    "socket.h"
#include    "domain.h"
#include    "referee.h"
#include    "transfer.h"
#include    "pcl.h"


/*
//-----------------------------------------------------------------------------
//      lokale Datenbereiche
//-----------------------------------------------------------------------------
*/

static int      lsock;
static byte     buffer[BUFSIZE];

/* Anzahl der kritischen Ziele, die bei der Aufl"osung der Verzeigerung */
/* gel"oscht werden                                                     */
static long xcg = 0;
/* Anzahl der kritischen Paare, die bei der Aufl"osung der Verzeigerung */
/* gel"oscht werden                                                     */
static long xcp = 0;


/*
//-----------------------------------------------------------------------------
//      Lokale Funktionsdefinitionen
//-----------------------------------------------------------------------------
*/

static void       termput  ( byte **ptr, long *bufsize,
			     term *t, vartree **vars, variable *counter );
static term       *termget ( byte **ptr );

static void       sendtermpair  ( termpair *tptr );
static termpair   *recvtermpair ( int sock );

static void       sendcritpair  ( termpair *tptr );
static void       sendcritgoal  ( termpair *tptr );
static termpair   *recvcrittermpair ( int sock );

static void       getcpparents  ( termpair *cp );

/*
//-----------------------------------------------------------------------------
//      Uebermitteln von Polynomen
//-----------------------------------------------------------------------------
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       SendPolynom ( int sock, polynom *poly )
//
//  Parameter:      sock        Kennung des Sockets
//                  poly        Zeiger auf das zu versendende Polynom
//
//  Beschreibung:   Ein Polynom wird versendet.
//-----------------------------------------------------------------------------
*/

void      SendPolynom ( int sock, polynom *poly )
{
    WriteInt (sock, (long)(poly->opc));
    switch (poly->opc)
    {
       case op_const:   
       case op_var:     WriteInt (sock, (long)(poly->value));
                        break;
       case op_add:     
       case op_sub:     
       case op_mul:     SendPolynom (sock, poly->left);
                        SendPolynom (sock, poly->right);
                        break;
       case op_pot:     SendPolynom (sock, poly->left);
                        WriteInt (sock, (long)(poly->value));
                        break;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceivePolynom ( int sock )
//
//  Parameter:      sock        Kennung des Sockets
//
//  Rueckgabewert:  Zeiger auf das empfangene Polynom.
//
//  Beschreibung:   Ein Polynom wird empfangen.
//-----------------------------------------------------------------------------
*/

polynom   *ReceivePolynom ( int sock )
{
    polynom     *poly;

    poly = newpoly ((opcode)ReadInt (sock), 0);

    switch (poly->opc)
    {
       case op_const:
       case op_var:     poly->value = ReadInt (sock);
                        break;
       case op_add:
       case op_sub:
       case op_mul:     poly->left  = ReceivePolynom (sock);
                        poly->right = ReceivePolynom (sock);
                        break;
       case op_pot:     poly->left  = ReceivePolynom (sock);
                        poly->value = ReadInt (sock);
                        break;
    }

    return poly;
}

/*
//-----------------------------------------------------------------------------
//      Uebermitteln von Termen unde Termpaaren
//-----------------------------------------------------------------------------
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       termput ( byte **ptr, long *bufsize,
//                            term *t, vartree **vars, variable *counter )
//
//  Parameter:      ptr         *Zeiger auf Pufferbereich fuer Ausgabe
//                  bufsize     verbleibende Puffergroesse
//                  t           Term
//                  vars        *Variablenbaum, der die Umsetzung der bisher
//                              aufgetreten Variablen enthaelt.
//                  counter     *Zaehler fuer Variablen
//
//  Beschreibung:   Ein Term wird in einen Puffer zur Versendung geschrieben.
//-----------------------------------------------------------------------------
*/

static void     termput ( byte **ptr, long *bufsize,
                          term *t, vartree **vars, variable *counter )
{
    short       i = 0;
    variable    v;

    if (*bufsize <= 0)
        Error ( __FILE__ ": "  "termput", "Ungenuegender Pufferbereich" );

    if (varp(t))
    {
        if ((v = VTfind (*vars, t->fcode)) == 0)
            VTadd ( vars, t->fcode, v = --(*counter) );

        if (v > -128)
        {
            (*bufsize)--; 
            **ptr = (byte)(v);          (*ptr)++;
        }
        else
        {
            (*bufsize) -= 3; 
            **ptr = (byte)(-128);               (*ptr)++;
            **ptr = (byte)((v >> 8));           (*ptr)++;
            **ptr = (byte)(v & 0xff);           (*ptr)++;
        }
    }
    else
    {
        (*bufsize)--;
        **ptr = (byte)(t->fcode);
        (*ptr)++;

        for (i = 0; i < t->arity; 
            termput (ptr, bufsize, t->argument[i++], vars, counter) );
    }
}


static term   *termget ( byte **ptr )
{
    term        *tptr;
    short       i;
    function    fcode;

    fcode = (function)(**ptr);  (*ptr)++;
    if (fcode == -128)
    {
        fcode = (function)(**ptr) << 8;         (*ptr)++;
        fcode = fcode | (function)(**ptr);      (*ptr)++;
    }
    tptr = newterm ( fcode );
    
    for (i = 0; i < tptr->arity; 
         tptr->argument[i++] = termget (ptr) );

    lgetweight (tptr);
    return tptr;
}


/*
//-----------------------------------------------------------------------------
//      Versenden von Termpaaren
//-----------------------------------------------------------------------------
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       sendtermpair  ( termpair *tptr )
//
//  Parameter:      t           Term
//
//  lokaler Para.:  lsock       Kennung des Sockets auf den geschrieben 
//                              werden soll.
//
//  Beschreibung:   Uebermitteln eines Termpaars
//-----------------------------------------------------------------------------
*/

static void  sendtermpair  ( termpair *tptr )
{
    byte        *bptr    = buffer;
    long        bufsize  = BUFSIZE;
    vartree     *vars    = NULL;
    variable    counter  = 0;

    termput ( &bptr, &bufsize, tptr->left,  &vars, &counter );
    termput ( &bptr, &bufsize, tptr->right, &vars, &counter );

    WriteInt    ( lsock, BUFSIZE-bufsize );
    WriteStream ( lsock, (char *)buffer, BUFSIZE-bufsize );
    WriteInt    ( lsock, tptr->number );
    WriteInt    ( lsock, tptr->special_flag );
    WriteDouble ( lsock, &(tptr->quality) );
#ifdef PCL
    WriteInt    ( lsock, tptr->pclid.cycle);
    WriteInt    ( lsock, tptr->pclid.host);
    WriteInt    ( lsock, tptr->pclid.count);
#endif
    VTclear ( &vars );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       recvtermpair  ( int sock )
//
//  Parameter:      sock        Kennung des Sockets von dem gelesen
//                              werden soll.
//
//  Rueckgabe:      Zeiger auf empfangenes Termpaar
//
//  Beschreibung:   Empfangen eines Termpaars
//-----------------------------------------------------------------------------
*/

static termpair  *recvtermpair ( int sock )
{
    termpair    *tp;
    byte        *bptr    = buffer;
    term        *left, *right;
    long        bufsize;
    
    bufsize = ReadInt ( sock );
    ReadStream ( sock, (char *)buffer, bufsize );

    left  = termget ( &bptr );
    right = termget ( &bptr );
    tpnewvars ( left, right );

    tp = newpair ( left, right, NULL, NULL );
    tp->number       = ReadInt ( sock );
    tp->special_flag = ReadInt ( sock );
    tp->quality      = ReadDouble ( sock );
#ifdef PCL
   tp->pclid.cycle = ReadInt ( sock );
   tp->pclid.host  = ReadInt ( sock );
   tp->pclid.count = ReadInt ( sock );
#endif
    return tp;
}



/*
//-----------------------------------------------------------------------------
//      Versenden von kritischen Termpaaren
//-----------------------------------------------------------------------------
*/

/*****************************************************************************/
/*                                                                           */
/*                     Versenden von kritischen Paaren                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  sendcritpair                                             */
/*                                                                           */
/*  Parameter    :  krit. Paar, das verschickt werden soll                   */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Uebermitteln eines kritischen Paares an den socket       */
/*                  lsock.                                                   */
/*                                                                           */
/*  Globale Var. :  lsock                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void  sendcritpair  ( termpair *tptr )
{
    byte        *bptr    = buffer;
    long        bufsize  = BUFSIZE;
    vartree     *vars    = NULL;
    variable    counter  = 0;
    int         coparent1, coparent2;

    if (IsAct (tptr))
    {
        termput ( &bptr, &bufsize, tptr->left,  &vars, &counter );
        termput ( &bptr, &bufsize, tptr->right, &vars, &counter );

        WriteInt    ( lsock, BUFSIZE-bufsize );
        WriteStream ( lsock, (char *)buffer, BUFSIZE-bufsize );
        WriteInt    ( lsock, tptr->number );
        WriteInt    ( lsock, tptr->special_flag );
#ifdef PCL
        WriteInt    ( lsock, tptr->pclid.cycle);
        WriteInt    ( lsock, tptr->pclid.host);
        WriteInt    ( lsock, tptr->pclid.count);
#endif
	if (tptr->parent1)
	{
	    if (tptr->parent1->type == RULE)
	    {
		coparent1 = tptr->parent1->number;
	    }
	    else
	    {
		coparent1 = -tptr->parent1->number; 
	    }
	}
	else
	{
	    coparent1 = 0;
	}

	if (tptr->parent2)
	{
	    if (tptr->parent2->type == RULE)
	    {
		coparent2 = tptr->parent2->number;
	    }
	    else
	    {
		coparent2 = -tptr->parent2->number;
	    }
	}
	else
	{
	    coparent2 = 0;
	}

        VTclear ( &vars );
        WriteInt(lsock,coparent1);
        WriteInt(lsock,coparent2);
    }
    else   /* krit. Paar ist nicht mehr aktuell -> L"oschen */
    {
        WriteInt ( lsock, 0 );

        (SetOfCriticalPairs.setcount)--;
        Delete ( &SetOfCriticalPairs, tptr );
	TPClearCPCachePtr( tptr );
	TPClearCGCachePtr( tptr );
        deleteterm ( tptr->left );
        deleteterm ( tptr->right );
        deletepair ( tptr );
    }
} /* Ende von sendcritpair */

/*****************************************************************************/
/*                                                                           */
/*                     Versenden von kritischen Zielen                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  sendcritgoal                                             */
/*                                                                           */
/*  Parameter    :  krit. Ziel, das verschickt werden soll                   */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Uebermitteln eines kritischen Zieles an den socket       */
/*                  lsock.                                                   */
/*                                                                           */
/*  Globale Var. :  lsock                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void  sendcritgoal  ( termpair *tptr )
{
    byte        *bptr    = buffer;
    long        bufsize  = BUFSIZE;
    vartree     *vars    = NULL;
    variable    counter  = 0;
    int         coparent1, coparent2;

    if (IsAct (tptr))
    {
        termput ( &bptr, &bufsize, tptr->left,  &vars, &counter );
        termput ( &bptr, &bufsize, tptr->right, &vars, &counter );

        WriteInt    ( lsock, BUFSIZE-bufsize );
        WriteStream ( lsock, (char *)buffer, BUFSIZE-bufsize );
        WriteInt    ( lsock, tptr->number );
        WriteInt    ( lsock, tptr->special_flag );
#ifdef PCL
        WriteInt    ( lsock, tptr->pclid.cycle);
        WriteInt    ( lsock, tptr->pclid.host);
        WriteInt    ( lsock, tptr->pclid.count);
#endif
	if (tptr->parent1)
	{
	    if (tptr->parent1->type == GOAL)  /* Erster Elternteil mu"s ein Ziel sein */
	    {
		coparent1 = tptr->parent1->number;
	    }
	    else
	    {
	        Error ( __FILE__ ": "  "sendcritgoal", 
			"Erster Elternteil muss ein Ziel sein" );
	    }
	}
	else /* Ein kritisches Ziel mu"s zwei Eltern besitzen */
	{
	    Error ( __FILE__ ": "  "sendcritgoal", "Krit. Ziel fehlt ein Elternteil." );
	}

	if (tptr->parent2)
	{
	    if (tptr->parent2->type == RULE)
	    {
		coparent2 = tptr->parent2->number;
	    }
	    else  /* Gleichung ist der zweite Elternteil */
	    {
		coparent2 = -tptr->parent2->number;
	    }
	}
	else
	{
	    Error ( __FILE__ ": "  "sendcritgoal", "Krit. Ziel fehlt ein Elternteil." );
	}

        VTclear ( &vars );
        WriteInt(lsock,coparent1);
        WriteInt(lsock,coparent2);
    }
    else
    {
        WriteInt ( lsock, 0 );

        (SetOfCriticalGoals.setcount)--;
        Delete ( &SetOfCriticalGoals, tptr );
	TPClearCPCachePtr( tptr );
	TPClearCGCachePtr( tptr );
        deleteterm ( tptr->left );
        deleteterm ( tptr->right );
        deletepair ( tptr );
    }
} /* sendcritgoal */

/*****************************************************************************/
/*                                                                           */
/*                     Empfangen der kritischen Termpaare                    */
/*                                                                           */
/*****************************************************************************/

/*
//-----------------------------------------------------------------------------
//  Funktion:       recvcrittermpair  ( int sock )
//
//  Parameter:      sock        Kennung des Sockets von dem gelesen
//                              werden soll.
//
//  Rueckgabe:      Zeiger auf empfangenes Termpaar
//
//  Beschreibung:   Empfangen eines kritischen Termpaars
//-----------------------------------------------------------------------------
*/

static termpair  *recvcrittermpair ( int sock )
{
    termpair    *tp;
    byte        *bptr    = buffer;
    term        *left, *right;
    long        bufsize;
    
    if (!(bufsize = ReadInt (sock)))
    {
        return NULL;
    }

    ReadStream ( sock, (char *)buffer, bufsize );

    left  = termget ( &bptr );
    right = termget ( &bptr );
    tpnewvars ( left, right );

    tp = newpair ( left, right, NULL, NULL );
    tp->number       = ReadInt ( sock );
    tp->special_flag = ReadInt ( sock );
#ifdef PCL
    tp->pclid.cycle = ReadInt ( sock );
    tp->pclid.host  = ReadInt ( sock );
    tp->pclid.count = ReadInt ( sock );
#endif
    tp->coparent1 = ReadInt ( sock );
    tp->coparent2 = ReadInt ( sock );

     return tp;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       getcpparents ( termpair *cp )
//
//  Parameter:      cp      Zeiger auf ein empfangenes Kritisches Paar
//
//  Beschreibung:   Aufloesen der Verzeigerung eines empfangenen 
//                  kritischen Paares.
//-----------------------------------------------------------------------------
*/


static void  getcpparents ( termpair *cp )
{
    bool        spec = false;

    if (cp->coparent1)
        if (cp->coparent1 > 0)
            if ((cp->parent1 = RuleByNumber (cp->coparent1)))
            {
                cp->coparent1 = cp->parent1->count;
                spec = spec || cp->parent1->special_flag;
            }
            else
            {
                xcp++;
                DeleteCP   ( cp );
                deleteterm ( cp->left );
                deleteterm ( cp->right );
                deletepair ( cp );
                return;
            }
        else
            if ((cp->parent1 = EquByNumber (-cp->coparent1)))
            {
                cp->coparent1 = cp->parent1->count;
                spec = spec || cp->parent1->special_flag;
            }
            else
            {
                xcp++;
                DeleteCP   ( cp );
                deleteterm ( cp->left );
                deleteterm ( cp->right );
                deletepair ( cp );
                return;
            }

    if (cp->coparent2)
        if (cp->coparent2 > 0)
            if ((cp->parent2 = RuleByNumber (cp->coparent2)))
            {
                cp->coparent2 = cp->parent2->count;
                spec = spec || cp->parent2->special_flag;
            }
            else
            {
                xcp++;
                DeleteCP   ( cp );
                deleteterm ( cp->left );
                deleteterm ( cp->right );
                deletepair ( cp );
                return;
            }
        else
            if ((cp->parent2 = EquByNumber (-cp->coparent2)))
            {
                cp->coparent2 = cp->parent2->count;
                spec = spec || cp->parent2->special_flag;
            }
            else
            {
                xcp++;
                DeleteCP   ( cp );
                deleteterm ( cp->left );
                deleteterm ( cp->right );
                deletepair ( cp );
                return;
            }

    if (spec && special_factor)
    {
        DeleteCP ( cp );
        SpecialWeight ( cp );
        InsertCP ( cp );
    }
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  getcgparents                                             */
/*                                                                           */
/*  Parameter    :  Zeiger auf ein empfangenes Kritisches Ziel               */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Aufloesen der Verzeigerung eines empfangenen kritischen  */
/*                  Zieles.                                                  */
/*                                                                           */
/*  Globale Var. :  xcg                                                      */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/


static void  getcgparents ( termpair *cg )
{
  bool        spec = false;

  if (cg->coparent1)
  {
    if ((cg->parent1 = GoalByNumber (cg->coparent1)))
    {
      cg->coparent1 = cg->parent1->count;
      spec = spec || cg->parent1->special_flag;
    }
    else
    {
       xcg++;
       DeleteCritGoal   ( cg );
       deleteterm ( cg->left );
       deleteterm ( cg->right );
       deletepair ( cg );
       return;
    }
  } /* Ende von if (cg->coparent1) */
  else /* Jedes krit Ziel mu"s ein Ziel als Elternteil haben */
  {
     Error ( __FILE__ ": "  "getcgparents", "Krit. Ziel fehlt ein Elternteil." );
  } /* Ende von else */

  if (cg->coparent2)
  {
    if (cg->coparent2 > 0)
    {
      if ((cg->parent2 = RuleByNumber (cg->coparent2)))
      {
	cg->coparent2 = cg->parent2->count;
	spec = spec || cg->parent2->special_flag;
      }
      else
      {
	xcg++;
	DeleteCritGoal   ( cg );
	deleteterm ( cg->left );
	deleteterm ( cg->right );
	deletepair ( cg );
	return;
      }
    }
    else
    {
      if ((cg->parent2 = EquByNumber (-cg->coparent2)))
      {
	  cg->coparent2 = cg->parent2->count;
	  spec = spec || cg->parent2->special_flag;
      }
      else
      {
	xcg++;
	DeleteCritGoal   ( cg );
	deleteterm ( cg->left );
	deleteterm ( cg->right );
	deletepair ( cg );
	return;
      }
    }
  } /* Ende von if (cg->coparent2) */
  else /* Jedes kritische Ziel muss einen zweiten Elternteil haben */
  {
     Error ( __FILE__ ": "  "getcgparents", "Krit. Ziel fehlt ein Elternteil." );
  } /* Ende von else */

  if (spec && special_factor)
  {
      DeleteCP ( cg );
      SpecialWeight ( cg );
      InsertCP ( cg );
  }
} /* Ende von getcgparents */

/*
//-----------------------------------------------------------------------------
//  Funktion:       GetCpParents 
//
//  Parameter:      -keine-
//
//  Beschreibung:   Aufschluesseln der Verzeigerung anhand der Nummern
//-----------------------------------------------------------------------------
*/

void      GetCpParents ( void )
{
    xcp = 0;
    ForAllCPsDo ( getcpparents );
    if (!DemoMode)
        printf ( "%d CPs geloescht.\n", xcp );
}

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  GetCgParents                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Aufschluesseln der Verzeigerung der kritischen Ziele     */
/*                  anhand der Nummern.                                      */
/*                                                                           */
/*  Globale Var. :  xcg                                                      */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void      GetCgParents ( void )
{
    xcg = 0;
    ForAllCritGoalsDo ( getcgparents );
    if (!DemoMode)
    {
      printf ( "%d CPs geloescht.\n", xcg );
    }
} /* Ende von GetCgParents */



/*
//-----------------------------------------------------------------------------
//      Exportierte Funktionen
//-----------------------------------------------------------------------------
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       SendPair ( int sock, termpair *tp )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//                  tp          Zeiger auf Termpaar
//
//  Beschreibung:   Versenden eines Termpaares
//-----------------------------------------------------------------------------
*/

void      SendPair ( int sock, termpair *tp )
{
    lsock = sock;
    sendtermpair ( tp );
    WriteInt (sock, tp->type);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceivePair ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf von dem die Daten
//                              empfangen werden sollen.
//
//  Rueckgabe:      Zeiger auf das empfangene Termpaar
//
//  Beschreibung:   Empfangen eines Termpaares
//-----------------------------------------------------------------------------
*/

termpair  *ReceivePair ( int sock )
{
    termpair    *tp;

    tp = recvtermpair ( sock );
    tp->type = ReadInt ( sock );

    return tp;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SendRules ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Senden aller aktuellen Regeln
//-----------------------------------------------------------------------------
*/

void      SendRules ( int sock )
{
    lsock = sock;
    WriteInt ( sock, SizeOf (SetOfRules[0]) );
    ForAllRulesDo ( sendtermpair );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SendEquations ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Senden aller aktuellen Gleichungen
//-----------------------------------------------------------------------------
*/
void      SendEquations ( int sock )
{
    lsock = sock;
    WriteInt ( sock, SizeOf (SetOfEquations) );
    ForAllEquDo ( sendtermpair );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SendCriticalPairs ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Senden aller aktuellen kritischen Paare
//-----------------------------------------------------------------------------
*/

void      SendCriticalPairs ( int sock )
{
    lsock = sock;

    if (!DemoMode)
    {
        printf ( "Kritische Paare %ld senden.\n", SizeOf (SetOfCriticalPairs) );
        flush ();
    }

    WriteInt ( sock, SizeOf (SetOfCriticalPairs) );
    ForAllCPsDo ( sendcritpair );

    if (!DemoMode)
    {
        printf ( "Kritische Paare %ld gesendet.\n", SizeOf (SetOfCriticalPairs) );
        flush ();
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SendGoals ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Senden aller aktuellen Ziele
//-----------------------------------------------------------------------------
*/

void      SendGoals ( int sock )
{
    lsock = sock;
    WriteInt ( sock, SizeOf (SetOfGoals) );
    ForAllGoalsDo ( sendtermpair );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SendCriticalGoals ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Senden aller paramodulierten Ziele
//-----------------------------------------------------------------------------
*/

void      SendCriticalGoals ( int sock )
{
    lsock = sock;

    if (!DemoMode)
    {
        printf ( "Kritische Ziele %ld senden.\n", SizeOf (SetOfCriticalGoals) );
        flush ();
    }
    
    WriteInt ( sock, SizeOf (SetOfCriticalGoals) );
    ForAllCritGoalsDo ( sendcritgoal );
    
    if (!DemoMode)
    {
        printf ( "Kritische Ziele %ld gesendet.\n", SizeOf (SetOfCriticalGoals) );
        flush ();
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceiveRules ( int sock, bool neworder )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//                  neworder    true    achte auf die Ordnung
//
//  Beschreibung:   Empfangen von Regeln
//-----------------------------------------------------------------------------
*/

void      ReceiveRules ( int sock, bool neworder )
{
    int         setsize, i;
    termpair    *ptr;

    setsize = ReadInt ( sock );
    if (!neworder)
        for (i = 0; i < setsize; i++ )
            NInsertRule ( recvtermpair (sock) );
    else
        for (i = 0; i < setsize; i++ )
        {
            ptr = recvtermpair (sock);
            if (GreaterThan (ptr->left, ptr->right))
                NInsertRule ( ptr );
            else
            {
                ptr->weight = -10;
                InsertCP (ptr);
            }
        }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceiveEquations ( int sock, bool neworder )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//                  neworder    true    achte auf die Ordnung
//
//  Beschreibung:   Empfangen von Gleichungen
//-----------------------------------------------------------------------------
*/

void      ReceiveEquations ( int sock, bool neworder )
{
    int         setsize, i;
    termpair    *ptr;

    setsize = ReadInt ( sock );
    if (!neworder)
        for (i = 0; i < setsize; i++ )
            NInsertEqu ( recvtermpair (sock) );
    else
        for (i = 0; i < setsize; i++ )
        {
            ptr = recvtermpair (sock);
            if (Compare (ptr->left, ptr->right) == TEQUAL)
                NInsertEqu ( ptr );
            else
            {
                ptr->weight = -10;
                InsertCP (ptr);
            }
        }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceiveCriticalPairs ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Empfangen von kritischen Paaren
//-----------------------------------------------------------------------------
*/

void      ReceiveCriticalPairs ( int sock )
{
    termpair    *tp;
    int         setsize, i, oldsetsize;

    setsize = ReadInt ( sock ); 
    oldsetsize = SizeOf(SetOfCriticalPairs); 
    /* "Anderung von StS, fr"uher fehlerhafte Angabe der empfangenen */
    /* kritischen Paare beim zweiten Senden (nach der Interreduction), */
    /* es wurden nicht die neuen, sondern alle kritischen Paare */
    /* angezeigt.. */

    if (!DemoMode)
    {
        printf ( "Kritische Paare %ld erwartet.\n", setsize );
        flush ();
    }
    
    for (i = 0; i < setsize; i++ )
    {
        if ((tp = recvcrittermpair (sock)))
        {
            tp->weight = CPWeight ( tp );
            NInsertCP ( tp );
        }
    }
    if (!DemoMode)
    {
       printf ( "Kritische Paare %ld empfangen.\n", 
                SizeOf(SetOfCriticalPairs)-oldsetsize );
       flush ();
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceiveGoals ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Empfangen von Zielen
//-----------------------------------------------------------------------------
*/

void      ReceiveGoals ( int sock )
{
    int         setsize, i;

    setsize = ReadInt ( sock );
    for (i = 0; i < setsize; i++ )
        NInsertGoal ( recvtermpair (sock) );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceiveCriticalGoals ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Empfangen von kritischen Zielen
//-----------------------------------------------------------------------------
*/

void      ReceiveCriticalGoals ( int sock )
{
    termpair    *tp;
    int         setsize, i, oldsetsize;
     
    oldsetsize = SizeOf( SetOfCriticalGoals );

    setsize = ReadInt ( sock );
    if (!DemoMode)
    {
        printf ( "Kritische Ziele %ld erwartet.\n", setsize );
        flush ();
    }

    for (i = 0; i < setsize; i++ )
    {
        if ((tp = recvcrittermpair (sock)))
        {
            tp->weight = CGWeight ( tp );
            NInsertCritGoal ( tp );
        }
    }
    if (!DemoMode)
    {
        printf ( "Kritische Ziele %ld empfangen.\n", SizeOf (SetOfCriticalGoals) - oldsetsize );
        flush ();
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SendAllPairs ( int sock )
//
//  Parameter:      sock        Kennung des Sockets auf den die Daten
//                              geschickt werden sollen.
//
//  Beschreibung:   Senden aller Termpaare
//-----------------------------------------------------------------------------
*/

void    SendAllPairs ( int sock )
{
    /* Die Ziele werden nicht verschickt, da sie in BuildAndSendNextStartsystem */
    /* bereits vor den kritischen Paaren verschickt werden. Genauere Begruen-   */
    /* dung siehe dort!                                                         */
    SendCriticalPairs ( sock );
    if( Paramodulation )
    {
      SendCriticalGoals ( sock );
    }
    SendRules         ( sock );
    SendEquations     ( sock );
/*  SendGoals         ( sock ); */

    WriteInt ( sock, RuleCounter );
    WriteInt ( sock, EquCounter  );
    WriteInt ( sock, CPCounter   );
    WriteInt ( sock, GoalCounter );
    WriteInt ( sock, CGCounter   );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceiveAllPairs ( int sock, bool neworder )
//
//  Parameter:      sock        Kennung des Sockets auf dem die Daten
//                              gelesen werden sollen.
//                  neworder    true    achte auf die Ordnung
//
//  Beschreibung:   Empfangen aller Termpaare
//-----------------------------------------------------------------------------
*/

void    ReceiveAllPairs ( int sock, bool neworder )
{
    /* Die Ziele werden nicht empfangen, da sie in ReceiveNextStartsystem       */
    /* bereits vor den kritischen Paaren empfangen  werden. Genauere Begruen-   */
    /* dung siehe dort!                                                         */
    ReceiveCriticalPairs ( sock );
    if( Paramodulation )
    {
      ReceiveCriticalGoals ( sock );
    }
    ReceiveRules         ( sock, neworder );
    ReceiveEquations     ( sock, neworder );
/*  ReceiveGoals         ( sock );      */

    RuleCounter = ReadInt ( sock );
    EquCounter  = ReadInt ( sock );
    CPCounter   = ReadInt ( sock );
    GoalCounter = ReadInt ( sock );
    CGCounter   = ReadInt ( sock );
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       SendReport
//
//  Parameter:      sock    Kennung des Sockets
//                  rep     Pufferbereich des Reports
//  
//        Erledigt: Ausgabe der Intermeds wurde nach
//                  team.c::StartCompletion() verlagert.
//
//                sense   Ist true, falls die Versendung Sinn macht
//                          (d.h. f"ur die Vervollst"andigung
//                          notwendig ist), false, falls es nur um die
//                          history geht. Der Parameter wird nur an
//                          PCL_INTERMED weitergereicht.
//
//
//
//  Beschreibung:   Versenden eines Referee-Reports
//-----------------------------------------------------------------------------
*/

void    SendReport ( int sock, report *rep )
{
    int         i;

    WriteStream ( sock, rep, sizeof (report) );
    for (i = 0; i < rep->rule_count; i++)
    {
       SendPair ( sock, rep->rule[i] );
    }
    for (i = 0; i < rep->equ_count; i++)
    {
       SendPair ( sock, rep->equ[i] );
    }
    for (i = 0; i < rep->goal_count; i++)
    {
       SendPair ( sock, rep->goal[i] );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReceiveReport
//
//  Parameter:      sock    Kennung des Sockets
//                  rep     Pufferbereich fuer Reports
//
//  Beschreibung:   Empfangen eines Referee-Reports
//-----------------------------------------------------------------------------
*/

void    ReceiveReport ( int sock, report *rep )
{
   int         i;
 
   ReadStream ( sock, rep, sizeof (report) );
  
   for (i = 0; i < rep->rule_count; i++)
   {
      rep->rule[i] = ReceivePair ( sock );
   }

   
   for (i = 0; i < rep->equ_count; i++)
   {
      rep->equ[i] = ReceivePair ( sock );
   }

   for (i = 0; i < rep->goal_count; i++)
   {
      rep->goal[i] = ReceivePair ( sock );
   }
}
