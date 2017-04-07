/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          termpair
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung von Termpaaren
//                      Es werden die geordneten Termpaarmengen
//                        SetOfRules[]
//                        SetOfEquations
//                        SetOfCriticalPairs
//                        SetOfGoals
//                        SetOfCriticalGoals
//                        SetOfAxioms        (neu)
//                      verwaltet.
//-----------------------------------------------------------------------------
//      $Log: termpair.c,v $
//      Revision 0.2  1991/08/26  13:50:27  pitz
//      special_flag eingefuehrt.
//
//      Revision 0.1  1991/08/19  09:50:41  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/


#include    "subst.h" /* Includiert auch termpair.h */


/******************************************************************/
/*                                                                */
/*            Variablen, die auch extern verwendet werden         */
/*                                                                */
/******************************************************************/

pairset     SetOfRules [MAXFUNCTION] = { EmptySet };
pairset     SetOfEquations           =   EmptySet;     /* [MAXFUNCTION]; */
pairset     SetOfCriticalPairs       =   EmptySet;
pairset     SetOfGoals               =   EmptySet;
pairset     SetOfCriticalGoals       =   EmptySet;
long        CPsLimit               =   MAXLONG;

pairset     SetOfAxioms              =   EmptySet;

/******************************************************************/
/*                                                                */
/*                          Konstante                             */
/*                                                                */
/******************************************************************/
#define     CACHESIZE                   16


/*
//-----------------------------------------------------------------------------
//      Modulinterne Daten
//-----------------------------------------------------------------------------
*/


static int         cpcacheptr                 = 0;
static termpair    *cpcache[CACHESIZE]      = { NULL };

static int         cgcacheptr                 = 0;
static termpair    *cgcache[CACHESIZE]      = { NULL };

static termpair *freelist   = NULL;
static long     TPcounter   = 0;            /* Zaehler fuer Termpaare         */


/*
//-----------------------------------------------------------------------------
//      Modulinterne Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/


static bool    hequal    ( term *t1, term *t2, vartree **v1, vartree **v2 );
static bool    hsubsum   ( term *left1,  term *left2, 
			  term *right1, term *right2 );

bool    ClearCPs ( bool msg );          /* Upcall in complet.c          */




/*
//-----------------------------------------------------------------------------
//  Funktion:       inittermpair
//
//  Parameter:      -keine-
//
//  Beschreibung:   Loeschen der Freiliste.
//-----------------------------------------------------------------------------
*/

void    inittermpair ( void )
{
    freelist = NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       newpair ( term    *left,     term     *right,
//                            termpair *parent1, termpair *parent2 )
//
//  Parameter:      left        Zeiger auf linke Seite des Termpaares
//                  right       Zeiger auf rechte Seite des Termpaares
//                  parent1     Zeiger auf erste Quelle
//                  parent2     Zeiger auf zweite Quelle
//
//  Rueckgabewert:  Pointer auf Termpaarzelle
//
//  Beschreibung:   Pointer auf Termpaar
//                  Allokiert werden immer mehrere Zellen, die dann
//                  in eine Freiliste in der Tabelle der Funktionssymbole
//                  eingekettet werden.
//-----------------------------------------------------------------------------
*/

termpair    *newpair ( term     *left,     term     *right,
                       termpair *parent1,  termpair *parent2 )
{
    register termpair   *ptr, *next;
    register short      i;
             long       size;

    if (!freelist)
    {
        size = PAIRALLOC * sizeof (termpair);
        ptr = freelist = Allocate ( size );
        if (!ptr)
        {
            if (ClearCPs ( true ))
                return newpair ( left, right, parent1, parent2 );

            Error ( __FILE__ ": "  "newpair", "Zuwenig Speicher." );
        }

        for ( i = 1; i < PAIRALLOC; i++ )
        {
            next = ptr;
            #ifdef MEMDEBUG
               ptr->debug = 0;
            #endif
            ptr = ptr->next = ++next;
        }
        ptr->next = NULL;
    }

    ptr      = freelist;
    freelist = freelist->next;

/*    ptr = SizeMalloc(sizeof(termpair));*/

    ptr->left         = left;
    ptr->right        = right;
    ptr->type         = UNKNOWN;
    ptr->count        = ++TPcounter;
    ptr->number       = 0;
    ptr->weight       = 0;
    ptr->quality      = 1;
    ptr->state        = NEW;
    ptr->special_flag = false;
    ptr->parent1      = parent1;
    ptr->parent2      = parent2;
    ptr->coparent1    = (parent1) ? parent1->count
                                  : 0;
    ptr->coparent2    = (parent2) ? parent2->count
                                  : 0;
    ptr->next         = NULL;
    ptr->prev         = NULL;
    
    ClrReferee ( ptr );

    #ifdef MEMDEBUG
       if (ptr->debug)
           Error ( __FILE__ ": "  "newpair", "MEMORY-ERROR." );
       ptr->debug++;
    #endif

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletepair ( termpair *pair )
//
//  Parameter:      pair        freizugebendes Termpaar
//
//  Beschreibung:   Termpaar wird in die Freiliste eingekettet.
//-----------------------------------------------------------------------------
*/

void    deletepair ( termpair *pair )
{
    pair->left      = NULL;
    pair->right     = NULL;
    pair->type      = UNKNOWN;
    pair->count     = 0;
    pair->number    = 0;
    pair->weight    = 0;
    pair->quality   = 0;
    pair->parent1   = NULL;
    pair->parent2   = NULL;
    pair->coparent1 = 0;
    pair->coparent2 = 0;
    pair->prev      = NULL;

    pair->next  = freelist;
    freelist    = pair;

    #ifdef MEMDEBUG
       pair->debug--;
      if (pair->debug)
           Error ( __FILE__ ": "  "deletepair", "MEMORY-ERROR." );
    #endif

/*    SizeFree(pair, sizeof(termpair)); */
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       printtpair ( termpair *pair )
//
//  Parameter:      pair        Termpaar
//
//  Beschreibung:   Termpaar wird ausgegeben.
//-----------------------------------------------------------------------------
*/

void    printtpair ( termpair *pair )
{
    vartree     *vars    = NULL;
    variable    counter  = 0;

    prettyprint ( pair->left,  &vars, &counter );

    switch (pair->type)
    {
    case RULE:      printf ( " --> "  );
                    break;
    case CRITPAIR:  printf ( " = " );
                    break;
    case EQUATION:  printf ( " = "    );
                    break;
    case GOAL:      printf ( " = "  );
                    break;
    default:        printf ( " <-> "  );
                    break;
    }

    prettyprint ( pair->right, &vars, &counter );
    printf ( "\n" );

    VTclear ( &vars );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       fprinttpair ( FILE *stream, termpair *pair, char *string )
//
//  Parameter:      stream      Ausgabestream
//                  pair        Termpaar
//                  string      Zeichenkette zwischen linker und rechter
//                              Seite
//
//  Beschreibung:   Termpaar wird auf stream ausgegeben.
//-----------------------------------------------------------------------------
*/

void    fprinttpair ( FILE *stream, termpair *pair, char *string )
{
    vartree     *vars    = NULL;
    variable    counter  = 0;

    fprettyprint ( stream, pair->left,  &vars, &counter );
    fprintf      ( stream, "%s", string  );
    fprettyprint ( stream, pair->right, &vars, &counter );
    fprintf      ( stream, "\n" );
    VTclear      ( &vars );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sprinttpair ( termpair *pair )
//
//  Parameter:      pair        Termpaar
//
//  Rueckgabewert:  Pointer auf Zeichenkette, die die Ausgabe des Termpaares
//                  enthaelt.
//
//  Beschreibung:   Termpaar wird in String ausgegeben.
//-----------------------------------------------------------------------------
*/

char    *sprinttpair ( termpair *pair )
{
    vartree     *vars    = NULL;
    variable    counter  = 0;
    long        size;
    char        *buffer, *save;

    size =   sprettylen (pair->left, &vars, &counter) 
           + 5 
           + sprettylen (pair->right, &vars, &counter);

    buffer = save = SecureMalloc ( size+1 );
    sprettyprint ( &buffer, pair->left, &vars );

    switch (pair->type)
    {
    case RULE:      sprintf ( buffer, " --> "  );
                    buffer += 5;
                    break;
    case CRITPAIR:  sprintf ( buffer, " = " );
                    buffer += 3;
                    break;
    case EQUATION:  sprintf ( buffer, " = "    );
                    buffer += 3;
                    break;
    case GOAL:      sprintf ( buffer, " = "  );
                    buffer += 3;
                    break;
    default:        sprintf ( buffer, " <-> "  );
                    buffer += 5;
                    break;
    }

    sprettyprint ( &buffer, pair->right, &vars );
    printf ( "\n" );

    VTclear ( &vars );
    return save;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SwapSides ( termpair *pair )
//
//  Parameter:      pair        freizugebendes Termpaar
//
//  Beschreibung:   Linke und rechte Seite werden vertauscht.
//-----------------------------------------------------------------------------
*/

void    SwapSides ( termpair *pair )
{
    term    *help;

    help  = pair->left;
    pair->left  = pair->right;
    pair->right = help;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       hequal ( term *t1, term *t2, vartree **v1, vartree **v2 )
//
//  Parameter:      t1, t2      zu Vergleichende zweier Terme
//                  v1, v2      Variablenbaeume fuer v1, v2
//
//  Rueckgabewert:  true        t1 und t2 sind bis auf Variablenumbenennung
//                              gleich
//                  false       sonst
//
//  Beschreibung:   Hilfsfunktion fuer tpequal
//-----------------------------------------------------------------------------
*/

static bool     hequal ( term *t1, term *t2, vartree **v1, vartree **v2 )
{
    short   i;

    if (t1->weight != t2->weight)
        return false;

    if (varp (t1))
    {
        if (!varp (t2))
            return false;

        if ((i = VTfind (*v1, t1->fcode)) != 0)
            return (i == t2->fcode);

        if (VTfind (*v2, t2->fcode))
            return false;

        VTadd ( v1, t1->fcode, t2->fcode );
        VTadd ( v2, t2->fcode, t1->fcode );
        return true;
    }

    if (t1->fcode != t2->fcode)
        return false;

    for ( i = 0; i < t1->arity; i++ )
        if (!hequal (t1->argument[i], t2->argument[i], v1, v2))
            return false;

    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       tpequal ( termpair *pair1, termpair *pair2 )
//
//  Parameter:      pair1       Zwei zu vergleichende Terme
//                  pair2
//
//  Rueckgabewert:  true        Paare sind bis auf Variablenumbenennung gleich
//                  false       sonst
//
//  Beschreibung:   Test zweier Termpaare auf Gleichheit
//                  Seiten werden dabei auch ueber Kreuz geprueft
//-----------------------------------------------------------------------------
*/

bool    tpequal ( termpair *pair1, termpair *pair2 )
{
    vartree     *v1     = NULL;
    vartree     *v2     = NULL;
    bool        result;

    result = hequal ( pair1->left, pair2->left, &v1, &v2 );
    if (result)
        result = hequal ( pair1->right, pair2->right, &v1, &v2 );

    VTclear ( &v1 );
    VTclear ( &v2 );

    if (result)
        return true;

    result = hequal ( pair1->right, pair2->left, &v1, &v2 );
    if (result)
        result = hequal ( pair1->left, pair2->right, &v1, &v2 );

    VTclear ( &v1 );
    VTclear ( &v2 );

    return result;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       hsubsum ( term *left1,  term *left2, 
//                            term *right1, term *right2 )
//
//  Parameter:      left1       4 Terme
//                  left2
//                  right1
//                  right2
//
//  Rueckgabewert:  true        (left1,right1) ueberdeckt (left2,right2)
//                  false       sonst
//
//  Beschreibung:   
//-----------------------------------------------------------------------------
*/

static bool    hsubsum ( term *left1, term *left2, term *right1, term *right2 )
{
    subst    *sigma = NULL;
    short    i, j;

    if ((left1->weight <= left2->weight) && (right1->weight <= right2->weight))
    {
        if (match (left1, left2, &sigma) && match (right1, right2, &sigma))
        {
            deletematch ( sigma );
            return true;
        }
        sigma = NULL;
    }

    if ((right1->weight <= left2->weight) && (left1->weight <= right2->weight))
    {
        if (match (right1, left2, &sigma) && match (left1, right2, &sigma))
        {
            deletematch ( sigma );
            return true;
        }
        sigma = NULL;
    }

    if (left2->fcode != right2->fcode)
        return false;

    for (i = 0; i < left2->arity; i++)
        if (!equal (left2->argument[i], right2->argument[i]))
        {
            for (j = i+1; j < left2->arity; j++)
                if (!equal (left2->argument[j], right2->argument[j]))
                    return false;

            return hsubsum ( left1,  left2->argument[i], 
                             right1, right2->argument[i] );
        }

    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       tpsubsum ( termpair *pair1, termpair *pair2 )
//
//  Parameter:      pair1       zwei Termpaare
//                  pair2
//
//  Rueckgabewert:  true        pair2 wird von pair1 subsumiert
//                  false       sonst
//
//  Beschreibung:   Ueberpruefung, ob pair1 von pair2 subsumiert wird.
//-----------------------------------------------------------------------------
*/

bool    tpsubsum ( termpair *pair1, termpair *pair2 )
{
    return hsubsum ( pair1->left, pair2->left, pair1->right, pair2->right );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       tpnewvars ( term *t1, term *t2 )
//
//  Parameter:      left, right     zwei Terme
//
//  Beschreibung:   Variablenumbennenung in beiden Termen.
//                  Alle Variablen in den Termen werden durch neu erzeugte
//                  Variablen ersetzt.
//-----------------------------------------------------------------------------
*/

void    tpnewvars ( term *left, term *right )
{
    vartree     *vars    = NULL;

    newvars ( left,  &vars );
    newvars ( right, &vars );
    VTclear ( &vars );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       tpcopypnew  ( termpair *pair, term **left, term **right )
//
//  Parameter:      pair        ein Termpaar
//                  left, right Referenzen auf Termpointer
//
//  Rueckgabewert:  left, right Termkopien der beiden Seiten von pair
//                              mit neuen Variablen
//
//  Beschreibung:   Die beiden Seiten eines Termpaares werden kopiert und
//                  mit neuen Variablen belegt.
//-----------------------------------------------------------------------------
*/

void    tpcopynew  ( termpair *pair, term **left, term **right )
{
    vartree     *vars    = NULL;

    *left  = copynew ( pair->left,  &vars );
    *right = copynew ( pair->right, &vars );
    VTclear ( &vars );
}



/*
//=============================================================================
//      allgemeine Mengenoperationen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       Add ( pairset *set, termpair *pair )
//
//  Parameter:      pairset     Menge von Termpaaren
//                  pair        Termpaar
//
//  Beschreibung:   Termpaar wird an die Menge set angefuegt.
//-----------------------------------------------------------------------------
*/

void    Add ( pairset *set, termpair *pair )
{
    pair->count  = ++TPcounter;
    pair->next   = NULL;

    if (set->last)
    {
        pair->prev = set->last;
        set->last  = set->last->next = pair;
    }
    else
    {
        pair->prev = NULL;
        set->first = set->last = pair;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Insert ( pairset *set, termpair *pair )
//
//  Parameter:      pairset     Menge von Termpaaren
//                  pair        Termpaar
//
//  Beschreibung:   Termpaar wird gemaess weight eingefuegt.
//-----------------------------------------------------------------------------
*/

void    Insert ( pairset *set, termpair *pair )
{
    termpair    *ptr;

    pair->count      = ++TPcounter;

    if (set->last)
    {
        if (pair->weight < set->first->weight)
        {
            pair->prev = NULL;
            pair->next = set->first;
            set->first = set->first->prev = pair;
        }
        else
        if (pair->weight >= set->last->weight)
        {
            pair->prev = set->last;
            pair->next = NULL;
            set->last  = set->last->next = pair;
        }
        else
        {
            ptr = set->first;
            while (pair->weight >= ptr->weight)
                ptr = ptr->next;

            pair->prev = ptr->prev;
            pair->next = ptr;
            ptr->prev  = ptr->prev->next = pair;
        }
    }
    else
    {
        pair->prev = NULL;
        pair->next = NULL;
        set->first = set->last = pair;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Delete ( pairset *set, termpair *pair )
//
//  Parameter:      pairset     Menge von Termpaaren
//                  pair        Termpaar
//
//  Beschreibung:   Termpaar wird aus der Menge set entfernt.
//-----------------------------------------------------------------------------
*/

void    Delete ( pairset *set, termpair *pair )
{
    pair->type       = UNKNOWN;

    if (pair->prev)
        pair->prev->next = pair->next;
    else
        set->first = pair->next;

    if (pair->next)
        pair->next->prev = pair->prev;
    else
        set->last = pair->prev;

    pair->prev = pair->next = NULL;
}



/*
//=============================================================================
//      exportierte Mengenoperationen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       AddRule, AddEqu, AddGoal, AddCP
//
//  Beschreibung:   Hinzufuegen eines Termpaares in die entsprechende Menge
//-----------------------------------------------------------------------------
*/

void      AddRule ( termpair *rule )
{
    rule->type   = RULE;
    rule->number = ++(SetOfRules[0].count);
    SetOfRules[0].setcount++;
    rule->setcode = (varp (rule->left)) ? 0 : rule->left->fcode;
    Add ( &(SetOfRules[rule->setcode]), rule );
}


void      AddEqu ( termpair *equ )
{
    equ->type   = EQUATION;
    equ->number = ++(SetOfEquations.count);
    SetOfEquations.setcount++;
    Add ( &(SetOfEquations), equ );
}


void      AddCP ( termpair *cp )
{
    cp->type   = CRITPAIR;
    cp->number = ++(SetOfCriticalPairs.count);
    SetOfCriticalPairs.setcount++;
    Add ( &SetOfCriticalPairs, cp );
}


void      AddGoal ( termpair *goal )
{
    goal->type   = GOAL;
    goal->number = ++(SetOfGoals.count);
    SetOfGoals.setcount++;
    Add ( &SetOfGoals, goal );
}


void      AddCritGoal ( termpair *cgoal )
{
    cgoal->type   = CRITGOAL;
    cgoal->number = ++(SetOfCriticalGoals.count);
    SetOfCriticalGoals.setcount++;
    Add ( &SetOfCriticalGoals, cgoal );
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  use_cache                                                */
/*                                                                           */
/*  Parameter    :  Typ des Termpaares, das eingef"ugt werden soll           */
/*                  Termpaar, das eingef"ugt werden soll                     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Das Termpaar wird unter Ausnutzung eines Caches in die   */
/*                  entsprechende Termpaarmenge eingef"ugt.                  */
/*                  ( Zur Zeit nur f"ur kritische Paare und kritische Ziele) */
/*                                                                           */
/*  Globale Var. :  cgcache                                                  */
/*                  cgcacheptr                                               */
/*                  cpcache                                                  */
/*                  cpcacheptr                                               */
/*                                                                           */
/*  Externe Var. :  SetOfCriticalPairs                                       */
/*                  SetOfCriticalGoals                                       */
/*                                                                           */
/*****************************************************************************/

void use_cache( short tp_type, termpair *tp )
{
  termpair   *ptr;
  pairset    *set;
  termpair   **cache;
  int         i, *cacheptr;

  switch( tp_type )
  {
    case RULE     : Error( __FILE__ ": " "use_cache", "Ein Cache fuer Regeln existiert nicht." );
                    break;
    case EQUATION : Error( __FILE__ ": " "use_cache", "Ein Cache fuer Gleichungen existiert nicht." );
                    break;
    case CRITPAIR : set = &SetOfCriticalPairs;
		    cache = cpcache;
		    cacheptr = &cpcacheptr;
		    break;
    case GOAL     : Error( __FILE__ ": " "use_cache", "Ein Cache fuer Ziele existiert nicht." );
		    break;
    case CRITGOAL : set = &SetOfCriticalGoals;
		    cache = cgcache;
		    cacheptr = &cgcacheptr;
		    break;
    default       : Error( __FILE__ ": " "use_cache", "Unbekannter Termpaartyp." );
  } /* Ende von switch */

  if( set->last )
  {
    /*********************************/
    /* Einf"ugen am Anfang der Menge */
    /*********************************/
    if( tp->weight < set->first->weight )
    {
      tp->prev = NULL;
      tp->next = set->first;
      set->first = set->first->prev = tp;
    }
    /*******************************/
    /* Einf"ugen am Ende der Menge */
    /*******************************/
    else if( tp->weight >= set->last->weight )
    {
      tp->prev = set->last;
      tp->next = NULL;
      set->last = set->last->next = tp;
    }
    /***************************************************/
    /* Einf"ugeposition mit Hilfe des Caches bestimmen */
    /***************************************************/
    else
    {
      ptr = set->first;
      for( i=0; i<CACHESIZE; i++)
      {
	/****************************************************************/
	/* Termpaar im Cache finden, das das n"achst kleinere zu tp ist */
	/****************************************************************/
	if( cache[i] && (cache[i]->type == tp_type) && cache[i]->next &&
	    (tp->weight >= cache[i]->weight) && (cache[i]->weight >= ptr->weight) )
	{
	  ptr = cache[i];
	}

      } /* Ende von for */

      /********************************************************************/
      /* Termpaar in der Menge finden, vor das tp eingetragen werden mu"s */
      /********************************************************************/
      while( tp->weight >= ptr->weight )
      {
	ptr = ptr->next;
      }

      /*****************************/
      /* tp in den Cache eintragen */
      /*****************************/
      cache[*cacheptr] = tp;
      *cacheptr = ((*cacheptr) +1) % CACHESIZE;

      /*****************************/
      /* tp in die Menge eintragen */
      /*****************************/
      tp->prev = ptr->prev;
      tp->next = ptr;
      ptr->prev  = ptr->prev->next = tp;
    } /* Ende von else --> Position mit Cache bestimmen */
  } /* Ende von if( set->last ) */
  else /* Menge ist noch leer */
  {
    tp->prev = NULL;
    tp->next = NULL;
    set->first = set->last = tp;
  } /* Ende von else */
} /* Ende von use_cache */
/*
//-----------------------------------------------------------------------------
//  Funktion:       InsertRule, InsertEqu, InsertGoal, InsertCP,
//
//  Beschreibung:   Einfuegen eines Termpaares in die entsprechende Menge
//-----------------------------------------------------------------------------
*/

void    InsertRule ( termpair *rule )
{
    rule->type   = RULE;
    rule->number = ++(SetOfRules[0].count);
    SetOfRules[0].setcount++;
    rule->setcode = (varp (rule->left)) ? 0 : rule->left->fcode;
    rule->weight = rule->left->weight;
    Insert ( &(SetOfRules[rule->setcode]), rule );
}


void      InsertEqu ( termpair *equ )
{
    equ->type   = EQUATION;
    equ->number = ++SetOfEquations.count;
    SetOfEquations.setcount++;
    equ->weight = min (equ->left->weight,equ->right->weight);
    Insert ( &(SetOfEquations), equ );
}


void      InsertCP ( termpair *cp )
{
    termpair    *ptr;
    /* pairset     *set;
    int         i;*/

    cp->type   = CRITPAIR;
    cp->number = ++SetOfCriticalPairs.count;
    cp->count      = ++TPcounter;
    SetOfCriticalPairs.setcount++;
    use_cache( CRITPAIR, cp );

    if(SetOfCriticalPairs.setcount > CPsLimit)
    {
       ptr = DelLast(&SetOfCriticalPairs);
       deleteterm(ptr->left);
       deleteterm(ptr->right);
       deletepair(ptr);
    } 

/*    Insert ( &SetOfCriticalPairs, cp ); 
    return; */

    /*set = &SetOfCriticalPairs;

    if (set->last)
    {
        if (cp->weight < set->first->weight)
        {
            cp->prev = NULL;
            cp->next = set->first;
            set->first = set->first->prev = cp;
        }
        else
        if (cp->weight >= set->last->weight)
        {
            cp->prev = set->last;
            cp->next = NULL;
            set->last  = set->last->next = cp;
        }
        else
        {
            ptr = set->first;
            for (i = 0; i < CACHESIZE; i++)
                if ( (cpcache[i]) && 
                     (cpcache[i]->type == CRITPAIR) && 
                     (cpcache[i]->next) &&
                     (cp->weight >= cpcache[i]->weight) &&
                     (cpcache[i]->weight >= ptr->weight)   )
                {
                    ptr = cpcache[i];
                }

            while (cp->weight >= ptr->weight)
                ptr = ptr->next;

            cpcache[cpcacheptr] = cp;
            cpcacheptr = (cpcacheptr+1) % CACHESIZE;

            cp->prev = ptr->prev;
            cp->next = ptr;
            ptr->prev  = ptr->prev->next = cp;
        }
    }
    else
    {
        cp->prev = NULL;
        cp->next = NULL;
        set->first = set->last = cp;
    }*/
}


void      InsertGoal ( termpair *goal )
{
    goal->type   = GOAL;
    goal->number = ++SetOfGoals.count;
    SetOfGoals.setcount++;
    Insert ( &SetOfGoals, goal );
}


void      InsertCritGoal ( termpair *cgoal )
{
    cgoal->type   = CRITGOAL;
    cgoal->number = ++SetOfCriticalGoals.count;
    SetOfCriticalGoals.setcount++;
    /*Insert ( &SetOfCriticalGoals, cgoal );*/
    cgoal->count = ++TPcounter;

    use_cache( CRITGOAL, cgoal );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       NInsertRule, NInsertEqu, NInsertGoal, NInsertCP,
//
//  Beschreibung:   Einfuegen eines Termpaares in die entsprechende Menge
//                  Dabei wird KEINE neue Nummer vergeben
//-----------------------------------------------------------------------------
*/

void    NInsertRule ( termpair *rule )
{
    rule->type   = RULE;
    SetOfRules[0].setcount++;
    rule->setcode = (varp (rule->left)) ? 0 : rule->left->fcode;
    rule->weight = rule->left->weight;
    Insert ( &(SetOfRules[rule->setcode]), rule );
}


void    NInsertEqu ( termpair *equ )
{
    equ->type   = EQUATION;
    SetOfEquations.setcount++;
    equ->weight = min (equ->left->weight,equ->right->weight);
    Insert ( &(SetOfEquations), equ );
}


void    NInsertCP ( termpair *cp )
{
    /*termpair    *ptr;
    pairset     *set;
    int         i;*/

    cp->type   = CRITPAIR;
    SetOfCriticalPairs.setcount++;
    cp->count      = ++TPcounter;
    use_cache( CRITPAIR, cp );

/*  Insert ( &SetOfCriticalPairs, cp ); 
    return; */

    /*set = &SetOfCriticalPairs;

    if (set->last)
    {
        if (cp->weight < set->first->weight)
        {
            cp->prev = NULL;
            cp->next = set->first;
            set->first = set->first->prev = cp;
        }
        else
        if (cp->weight >= set->last->weight)
        {
            cp->prev = set->last;
            cp->next = NULL;
            set->last  = set->last->next = cp;
        }
        else
        {
            ptr = set->first;
            for (i = 0; i < CACHESIZE; i++)
                if ( (cpcache[i]) &&
                     (cpcache[i]->type == CRITPAIR) &&
                     (cpcache[i]->next) &&
                     (cp->weight >= cpcache[i]->weight) &&
                     (cpcache[i]->weight >= ptr->weight)   )
                {
                    ptr = cpcache[i];
                }

            while (cp->weight >= ptr->weight)
                ptr = ptr->next;

            cpcache[cpcacheptr] = cp;
            cpcacheptr = (cpcacheptr+1) % CACHESIZE;

            cp->prev = ptr->prev;
            cp->next = ptr;
            ptr->prev  = ptr->prev->next = cp;
        }
    }
    else
    {
        cp->prev = NULL;
        cp->next = NULL;
        set->first = set->last = cp;
    } */
}


void    NInsertGoal ( termpair *goal )
{
    goal->type   = GOAL;
    SetOfGoals.setcount++;
    Insert ( &SetOfGoals, goal );
}


void    NInsertCritGoal ( termpair *cgoal )
{
    cgoal->type   = CRITGOAL;
    SetOfCriticalGoals.setcount++;
    cgoal->count = ++TPcounter;

    use_cache( CRITGOAL, cgoal );
    /*Insert ( &SetOfCriticalGoals, cgoal );*/
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReInsertRule ( termpair *rule )
//
//  Beschreibung:   Falls eine Regel durch Cancellation ein neues Topsymbol
//                  auf der linken Seite erhaelt wird hier geprueft, ob
//                  die Regel in eine neue Klasse der Regelmenge eingeordnet
//                  werden muss. Dies wird gegebenenfalls durchgefuehrt.
//-----------------------------------------------------------------------------
*/

void    ReInsertRule ( termpair *rule )
{
    Delete ( &(SetOfRules[rule->setcode]), rule );

    rule->type   = RULE;
    rule->weight = rule->left->weight;

    rule->setcode = (varp (rule->left)) ? 0 : rule->left->fcode;
    Insert ( &(SetOfRules[rule->setcode]), rule );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReInsertCP ( termpair *cp )
//
//  Beschreibung:   Ein kritisches Paar wird anhand seines (neuen) Gewichts
//                  erneut in die Menge der kritischen Paare eingefuegt.
//                  Wird (momentan) nur bei PCL-Version benoetigt.
//-----------------------------------------------------------------------------
*/

void    ReInsertCP ( termpair *cp )
{
    Delete ( &SetOfCriticalPairs, cp );
    Insert ( &SetOfCriticalPairs, cp );
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       DeleteRule, DeleteEqu, DeleteGoal, DeleteCP
//
//  Beschreibung:   Loeschen eines Termpaares aus der entsprechende Menge
//-----------------------------------------------------------------------------
*/

void      DeleteRule ( termpair *rule )
{
    SetOfRules[0].setcount--;
    Delete ( &(SetOfRules[rule->setcode]), rule );
}


void      DeleteEqu ( termpair *equ )
{
    SetOfEquations.setcount--;
    Delete ( &SetOfEquations, equ );
}


void      DeleteCP ( termpair *cp )
{
    SetOfCriticalPairs.setcount--;
    Delete ( &SetOfCriticalPairs, cp );

    TPClearCPCachePtr ( cp );
}


void      DeleteGoal ( termpair *goal )
{
    SetOfGoals.setcount--;
    Delete ( &SetOfGoals, goal );
}


void      DeleteCritGoal ( termpair *cgoal )
{
    SetOfCriticalGoals.setcount--;
    Delete ( &SetOfCriticalGoals, cgoal );

    TPClearCGCachePtr ( cgoal );
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       RuleByNumber ( long number )
//
//  Parameter:      number  Nummer der gesuchten Regel
//
//  Rueckgabewert:  Pointer auf Regel
//                  (NULL falls nicht gefunden)
//
//  Beschreibung:   Es wird eine Regel anhand der Nummer gesucht.
//-----------------------------------------------------------------------------
*/

termpair  *RuleByNumber ( long number )
{
    termpair    *ptr;
    int         i;

    for (i = 0; i <= FuncCount; i++)
    {
        ptr = SetOfRules[i].first;
        while (ptr)
        {
            if (ptr->number == number)
                return ptr;
            ptr = ptr->next;
        }
    }
    return NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       EquByNumber ( long number )
//
//  Parameter:      number  Nummer der gesuchten Gleichung
//
//  Rueckgabewert:  Pointer auf Gleichung
//                  (NULL falls nicht gefunden)
//
//  Beschreibung:   Es wird eine Gleichung anhand der Nummer gesucht.
//-----------------------------------------------------------------------------
*/

termpair  *EquByNumber ( long number )
{
    termpair    *ptr;

    ptr = SetOfEquations.first;
    while (ptr)
    {
        if (ptr->number == number)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CPByNumber ( long number )
//
//  Parameter:      number  Nummer des gesuchten kritischen Paares
//
//  Rueckgabewert:  Pointer auf kritisches Paar
//                  (NULL falls nicht gefunden)
//
//  Beschreibung:   Es wird ein kritisches Paar anhand der Nummer gesucht.
//-----------------------------------------------------------------------------
*/

termpair  *CPByNumber ( long number )
{
    termpair    *ptr;

    ptr = SetOfCriticalPairs.first;
    while (ptr)
    {
        if (ptr->number == number)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  GoalByNumber                                             */
/*                                                                           */
/*  Parameter    :  Nummer des gesuchten Ziels                               */
/*                                                                           */
/*  Returnwert   :  Pointer auf das Ziel, falls es gefunden wurde            */
/*                  sonst NULL                                               */
/*                                                                           */
/*  Beschreibung :  Es wird das Ziel anhand seiner Nummer gesucht.           */
/*                                                                           */
/*  Globale Var. :  SetOfCriticalGoals                                       */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
termpair  *GoalByNumber ( long number )
{
    termpair    *ptr;

    ptr = SetOfGoals.first;
    while (ptr)
    {
        if (ptr->number == number)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
} /* Ende von GoalByNumber */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  CGByNumber                                               */
/*                                                                           */
/*  Parameter    :  Nummer des gesuchten kritischen Ziels                    */
/*                                                                           */
/*  Returnwert   :  Pointer auf das kritische Ziel, falls es gefunden wurde  */
/*                  sonst NULL                                               */
/*                                                                           */
/*  Beschreibung :  Es wird das kritische Ziel anhand seiner Nummer gesucht. */
/*                                                                           */
/*  Globale Var. :  SetOfCriticalGoals                                       */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
termpair  *CGByNumber ( long number )
{
    termpair    *ptr;

    ptr = SetOfCriticalGoals.first;
    while (ptr)
    {
        if (ptr->number == number)
            return ptr;
        ptr = ptr->next;
    }
    return NULL;
} /* Ende von CGByNumber */

/*
//-----------------------------------------------------------------------------
//  Funktion:       Empty ( int type )
//
//  Parameter:      pairset     Menge von Termpaaren
//
//  Rueckgabewert:  true    falls set leer ist
//                  false   sonst
//
//  Beschreibung:   Es wird geprueft, ob die Menge set leer ist.
//-----------------------------------------------------------------------------
*/

bool    Empty ( int type )
{
    switch (type)
    {
    case RULE:      return (0 == SetOfRules[0].setcount);
    case EQUATION:  return (0 == SetOfEquations.setcount);
    case CRITPAIR:  return (0 == SetOfCriticalPairs.setcount);
    case GOAL:      return (0 == SetOfGoals.setcount);
    case CRITGOAL:  return (0 == SetOfCriticalGoals.setcount);
    }
    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       DelFirst ( pairset *set )
//
//  Parameter:      pairset     Menge von Termpaaren
//
//  Rueckgabewert:  Forderstes Element in set.
//                  NULL    Menge war leer.
//
//  Beschreibung:   Diese Funktion liefert das erste Element in set.
//                  Dieses wird gleichzeitig aus der Menge entfernt.
//-----------------------------------------------------------------------------
*/

termpair  *DelFirst ( pairset *set )
{
    termpair    *ptr = NULL;

    if (set->first)
    {
        set->setcount--;

        ptr = set->first;
        set->first = set->first->next;
        if (set->first)
            set->first->prev = NULL;
        else
            set->last = NULL;

        ClrReferee ( ptr );

	TPClearCPCachePtr ( ptr );
	TPClearCGCachePtr ( ptr );
    }

    return ptr;
}
/*
//-----------------------------------------------------------------------------
//  Funktion:       DelLast ( pairset *set )
//
//  Parameter:      pairset     Menge von Termpaaren
//
//  Rueckgabewert:  Letztes Element in set.
//                  NULL    Menge war leer.
//
//  Beschreibung:   Diese Funktion liefert das letzte Element in set.
//                  Dieses wird gleichzeitig aus der Menge entfernt.
//-----------------------------------------------------------------------------
*/

termpair  *DelLast ( pairset *set )
{
    termpair    *ptr = NULL;

    if (set->last)
    {
        set->setcount--;

        ptr = set->last;
        set->last = set->last->prev;
        if (set->last)
            set->last->next = NULL;
        else
            set->first = NULL;

        ClrReferee ( ptr );

	TPClearCPCachePtr ( ptr );
	TPClearCGCachePtr ( ptr );
    }

    return ptr;
}




/*
//-----------------------------------------------------------------------------
//  Funktion:       FindPair ( termpair *pair )
//
//  Parameter:      set     Zu durchsuchende Gleichung
//                  pair    Gesuchtes Paar
//
//  Rueckgabewert:  Nummer des Termpaares, das gleich pair ist
//                  (0 sonst)
//
//  Beschreibung:   Sucht eine Gleichung in der Menge der Gleichungen.
//                  Dabei werden Variablenpermutationen und Seitenvertauschung
//                  beachtet !
//-----------------------------------------------------------------------------
*/

short   FindPair ( pairset *set, termpair *pair )
{
    register termpair   *ptr;

    ptr = set->first;
    while (ptr)
    {
        if ((ptr != pair) && (tpequal (ptr, pair)))
            return ptr->number;
        ptr = ptr->next;
    }

    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       FindSubsum ( pairset *set, termpair *pair )
//
//  Parameter:      set     Zu durchsuchende Gleichung
//                  pair    Gesuchtes Paar
//
//  Rueckgabewert:  Pointer auf das Termpaares, das pair ueberdeckt
//                  (0 sonst)
//
//  Beschreibung:   Ueberprueft, ob ein Paar von einem Paar in einer Menge
//                  subsumiert wird.
//-----------------------------------------------------------------------------
*/

termpair    *FindSubsum ( pairset *set, termpair *pair )
{
    register termpair   *ptr;

    ptr = set->first;
    while (ptr)
    {
        if (tpsubsum (ptr, pair))
            return ptr;
        ptr = ptr->next;
    }

    return NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       InititalSet ( termpair *pair )
//
//  Parameter:      set     Zu initialisierende Menge
//
//  Beschreibung:   Alle Paare in der Menge set werden als Initial
//                  gekennzeichnet.
//-----------------------------------------------------------------------------
*/

void    InititalSet ( pairset *set )
{
    register termpair   *ptr;

    ptr = set->first;
    while (ptr)
    {
        Initial ( ptr );
        ptr = ptr->next;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ClearSet
//
//  Parameter:      set     Die zu loeschende Menge von Termpaaren
//
//  Beschreibung:   Loeschen aller Termpaare in einer Menge
//-----------------------------------------------------------------------------
*/

void    ClearSet ( pairset *set )
{
    termpair    *ptr, *next;

    ptr = set->first;
    while (ptr)
    {
        next = ptr->next;
        deleteterm ( ptr->left );
        deleteterm ( ptr->right );
        deletepair ( ptr );
        ptr = next;
    }
    set->first    = NULL;
    set->last     = NULL;
    set->count    = 0;
    set->setcount = 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       TPClearData
//
//  Parameter:      -keine-
//
//  Beschreibung:   Loeschen aller Termpaare in allen Mengen.
//-----------------------------------------------------------------------------
*/

void    TPClearData ( void )
{
    short       i;

    for (i = 0; i <= FuncCount; i++)
        ClearSet ( &(SetOfRules[i]) );

    ClearSet ( &SetOfEquations     );
    ClearSet ( &SetOfCriticalPairs );
    ClearSet ( &SetOfGoals         );
    ClearSet ( &SetOfCriticalGoals );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       TPClearCPCache
//
//  Parameter:      -keine-
//
//  Beschreibung:   Loeschen des CPcaches.
//-----------------------------------------------------------------------------
*/

void    TPClearCPCache ( void )
{
    short       i;

    for (i = 0; i < CACHESIZE; i++)
        cpcache[i] = NULL;

    cpcacheptr = 0;
}


void    TPClearCPCachePtr ( termpair *ptr )
{
    short       i;

    for (i = 0; i < CACHESIZE; i++)
        if (cpcache[i] == ptr)
        {
            cpcache[i] = NULL;
            return;
        }
}

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  TPClearCGCache                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Der Cache f"ur kritische Ziele wird gel"oscht.           */
/*                                                                           */
/*  Globale Var. :  cgcache                                                  */
/*                  cgcacheptr                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void    TPClearCGCache ( void )
{
    short       i;

    for (i = 0; i < CACHESIZE; i++)
    {
      cgcache[i] = NULL;
    }

    cgcacheptr = 0;
} /* Ende von TPClearCGCache */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  TPClearCGCachePtr                                        */
/*                                                                           */
/*  Parameter    :  Pointer auf das Termpaar, das entfernt werden soll       */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird "uberpr"uft, ob das angegebene Termpaar in dem   */
/*                  Cache f"ur kritische Ziele enthalten ist, wenn ja, so    */
/*                  wird es aus dem Cache entfernt.                          */
/*                                                                           */
/*  Globale Var. :  cgcache                                                  */
/*                  cgcacheptr                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void    TPClearCGCachePtr ( termpair *ptr )
{
  short       i;

  for (i = 0; i < CACHESIZE; i++)
  {
    if (cgcache[i] == ptr)
    {
      cgcache[i] = NULL;
      return;
    }
  }
} /* Ende von TPClearCGCachePtr */


/*
//=============================================================================
//      Reorganisation
//=============================================================================
*/

REORG_FUNCTION(reorg,termpair,next)

void ReorgTermpair ( bool silence )
{
    long   Itime = systime (); /* von itime in Itime umgewandelt, da itime jetzt
				  eine global Variable ist.  Martin, 6.9.94 */

    if (!silence)
    {
        printf ( "REORGANISATION %ld.%02ld MB\n\n", memory_size/1000000,
                                                    (memory_size%1000000)/10000 );
        printf ( "Reorganisiere Termpaare\n" );
    }
    freelist = reorg ( freelist, STARTSHIFT );
    ReorgTerm ( silence );
    ReorgSubst ( silence );

    Itime = systime()-Itime;
    if (!silence)
    {
       printf ( "\nDauer der Reorganistation: %5ld.%03ld s\n\n", Itime/1000, Itime%1000 );
       flush ();
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ForAllRulesDo ( void (*proc)(termpair *ptr) )
//
//  Parameter:      proc    Operation, die auf alle Regeln angewendet
//                          werden soll.
//
//  Beschreibung:   Alle Regeln werden als Argument an die Operation proc
//                  uebergeben.
//                  Dabei kann innerhalb dieser Prozedur der Pointer
//                  veraendert werden.
//-----------------------------------------------------------------------------
*/

void  ForAllRulesDo ( void (*proc)(termpair *ptr) )
{
    termpair    *ptr, *next;
    int         i;

    for (i = 0; i <= FuncCount; i++)
    {
        ptr = SetOfRules[i].first;
        while (ptr)
        {
            next = ptr->next;
            proc ( ptr );
            ptr = next;
        }
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ForAllFRulesDo (function fcode, void (*proc)(termpair *ptr))
//
//  Parameter:      fcode   Ausgewaehltes Funktionssymbol
//                  proc    Operation, die auf alle Regeln angewendet
//                          werden soll.
//
//  Beschreibung:   Alle Regeln mit linkem Topsymbol fcode werden als Argument
//                  an die Operation proc uebergeben.
//                  Dabei kann innerhalb dieser Prozedur der Pointer
//                  veraendert werden.
//-----------------------------------------------------------------------------
*/

void  ForAllFRulesDo ( function fcode, void (*proc)(termpair *ptr) )
{
    termpair    *ptr, *next;

    ptr = SetOfRules[fcode].first;
    while (ptr)
    {
        next = ptr->next;
        proc ( ptr );
        ptr = next;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ForAllEquDo ( void (*proc)(termpair *ptr) )
//
//  Parameter:      proc    Operation, die auf alle Regeln angewendet
//                          werden soll.
//
//  Beschreibung:   Alle Gleichungen werden als Argument an die Operation proc
//                  uebergeben.
//                  Dabei kann innerhalb dieser Prozedur der Pointer
//                  veraendert werden.
//-----------------------------------------------------------------------------
*/

void  ForAllEquDo ( void (*proc)(termpair *ptr) )
{
    termpair    *ptr, *next;

    ptr = SetOfEquations.first;
    while (ptr)
    {
        next = ptr->next;
        proc ( ptr );
        ptr = next;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ForAllCPsDo ( void (*proc)(termpair *ptr) )
//
//  Parameter:      proc    Operation, die auf alle Regeln angewendet
//                          werden soll.
//
//  Beschreibung:   Alle kritschen Paare werden als Argument an die Operation
//                  proc uebergeben.
//                  Dabei kann innerhalb dieser Prozedur der Pointer
//                  veraendert werden.
//-----------------------------------------------------------------------------
*/

void  ForAllCPsDo ( void (*proc)(termpair *ptr) )
{
    termpair    *ptr, *next;

    ptr = SetOfCriticalPairs.first;
    while (ptr)
    {
        next = ptr->next;
        proc ( ptr );
        ptr = next;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ForAllGoalsDo ( void (*proc)(termpair *ptr) )
//
//  Parameter:      proc    Operation, die auf alle Regeln angewendet
//                          werden soll.
//
//  Beschreibung:   Alle Ziele werden als Argument an die Operation proc
//                  uebergeben.
//                  Dabei kann innerhalb dieser Prozedur der Pointer
//                  veraendert werden.
//-----------------------------------------------------------------------------
*/

void  ForAllGoalsDo ( void (*proc)(termpair *ptr) )
{
    termpair    *ptr, *next;

    ptr = SetOfGoals.first;
    while (ptr)
    {
        next = ptr->next;
        proc ( ptr );
        ptr = next;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ForAllCritGoalsDo ( void (*proc)(termpair *ptr) )
//
//  Parameter:      proc    Operation, die auf alle Regeln angewendet
//                          werden soll.
//
//  Beschreibung:   Alle kritschen Ziele werden als Argument an die Operation
//                  proc uebergeben.
//                  Dabei kann innerhalb dieser Prozedur der Pointer
//                  veraendert werden.
//-----------------------------------------------------------------------------
*/

void  ForAllCritGoalsDo ( void (*proc)(termpair *ptr) )
{
    termpair    *ptr, *next;

    ptr = SetOfCriticalGoals.first;
    while (ptr)
    {
        next = ptr->next;
        proc ( ptr );
        ptr = next;
    }
}


/*
//=============================================================================
//      Ausgabe von Mengen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintSet ( pairset *set )
//
//  Parameter:      pairset     Menge von Termpaaren
//
//  Beschreibung:   Ausgabe einer Menge von Termpaaren.
//-----------------------------------------------------------------------------
*/

void    PrintSet ( pairset *set )
{
    termpair    *ptr;

    ptr = set->first;
    while (ptr)
    {
        printf ( "(%4ld)  ", ptr->number );
        printtpair ( ptr );
        ptr = ptr->next;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintRules
//
//  Parameter:      -keine-
//
//  Beschreibung:   Ausgabe der Regelmenge.
//-----------------------------------------------------------------------------
*/

void    PrintRules ( void )
{
    termpair    *ptr;
    int         i;

    for (i = 0; i <= FuncCount; i++)
    {
        ptr = SetOfRules[i].first;

        while (ptr)
        {
            printf ( "(%4ld)  ", ptr->number );
            printtpair ( ptr );
            ptr = ptr->next;
       }
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintEquations
//
//  Parameter:      -keine-
//
//  Beschreibung:   Ausgabe der Gleichungsmenge.
//-----------------------------------------------------------------------------
*/

void    PrintEquations ( void )
{
    PrintSet ( &SetOfEquations );
}


/*
//=============================================================================
//      weitere Mengenoperationen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       MoveSet  ( pairset *dest, pairset *source )
//
//  Parameter:      dest    Kopf der Zielmenge
//                  source  Kopf der Quelle
//
//  Beschreibung:   Eine Gleichungsmenge wird verschoben.
//                  d.h. der Mengenkopf wird kopiert und der
//                       urspruengliche Inhalt geloescht.
//
//  Beachte:        Der Inhalt von dest geht verloren. Insbesondere
//                  werden die Speicherbereiche nicht freigegeben.
//-----------------------------------------------------------------------------
*/

void      MoveSet  ( pairset *dest, pairset *source )
{
    dest->first    = source->first;
    dest->last     = source->last;
    dest->count    = source->count;
    dest->setcount = source->setcount;
    
    source->first = source->last     = NULL;
    source->count = source->setcount = 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       MergeCPs ( pairset *source )
//
//  Parameter:      source  Mengenkopf
//
//  Beschreibung:   Die in der Menge source enthaltenen Termpaare 
//                  werden in die Menge der kritischen Paare aufgenommen.
//-----------------------------------------------------------------------------
*/

void      MergeCPs ( pairset *source )
{
    termpair    *tp;

    while ((tp = DelFirst (source)) != NULL)
        InsertCP ( tp );

    source->first = source->last     = NULL;
    source->count = source->setcount = 0;
}
    
/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  MergeCGs                                                 */
/*                                                                           */
/*  Parameter    :  Pointer auf den Mengenkopf                               */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die in der uebergebenen Menge enthaltenen Termpaare wer- */
/*                  den in die Menge der kritischen Ziele aufgenommen.       */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void MergeCGs ( pairset *source )
{
  termpair *tp;

  while ((tp = DelFirst (source)) != NULL)
  {
    InsertCritGoal( tp );
  }

  source->first = source->last     = NULL;
  source->count = source->setcount = 0;
} /* Ende von MergeCGs */

/*
//=============================================================================
//      Cancellation
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       Cancellation ( termpair *pair )
//
//  Parameter:      pair    Zeiger auf ein Termpaar
//
//  Rueckgabewert:  true    Cancellation war erfolgreich
//                  false   sonst
//
//  Beschreibung:   Es wird ein Termpaar auf Cancellation geprueft.
//                  Ist dies moeglich, wird diese durchgefuehrt, und
//                  Cancellation erneut aufgerufen, um auch ein tiefer
//                  Schachtelung aufzuloesen.
//-----------------------------------------------------------------------------
*/

bool      Cancellation ( termpair *pair )
{
    term    *left  = pair->left;
    term    *right = pair->right;
    short   i, j;
    bool    flag;

    if ((left->fcode) != (right->fcode))
        return false;

    if ((varp (left)) || (constp(left)))
        return false;

    if (!(Function [left->fcode].cancellation))
        return false;

    for (i = 0; i < left->arity; i++)
    if (CheckCancel (left->fcode, i))
    {
        flag = true;
        for (j = 0; flag && (j < left->arity); j++)
            flag = (i == j) ||
                   (equal (left->argument[j], right->argument[j]));

        if (flag)
        {
            pair->left  = left->argument[i];
            pair->right = right->argument[i];
            for (j = 0; flag && (j < left->arity); j++)
            if (i != j)
            {
                deleteterm (left->argument[j]);
                deleteterm (right->argument[j]);
            }
            deletet (left);
            deletet (right);
            Cancellation ( pair );
            return true;
        }
    }
    return false;
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       SumWeightCP ( int n )
//
//  Parameter:      n       Anzahl der zu summierenden Kritischen Paare
//
//  Rueckgabewert:  Gewichtsumme der ersten n kritsichen Paare
//
//  Beschreibung:   Summiert die Gewichte der ersten n kritischen Paare
//-----------------------------------------------------------------------------
*/

long      SumWeightCP ( int n )
{
     termpair   *cp;
     long       sum     = 0;

     cp = SetOfCriticalPairs.first;
     while (cp && n)
     {
        if (cp->weight >= -20000)
        {
            sum += cp->weight;
            n--;
        }
        cp = cp->next;
     }

     return sum;
}



/*
//=============================================================================
//      
//=============================================================================
*/

void    CheckSet ( pairset *set )
{
    termpair    *ptr;

    ptr = set->first;
    while (ptr)
    {
        printf ( "Teste: (%4ld)  ", ptr->number );
        printtpair ( ptr );
        if (checkweight (ptr->left) || checkweight (ptr->right))
        {
            printf ( "Fehler in Struktur: (%4ld)  ", ptr->number );
            printtpair ( ptr );
        }
        ptr = ptr->next;
    }
}


void     CheckPairs ( void )
{
    short   i;

    for (i = 0; i <= FuncCount;
        CheckSet ( &(SetOfRules[i++]) ));
    CheckSet ( &SetOfEquations );
    CheckSet ( &SetOfCriticalPairs );
    CheckSet ( &SetOfGoals );
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  akt_termpaare_anz_best                                   */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  Anzahl aller dem System zu dem Zeitpunkt bekannten Term- */
/*                  paare; d.h. die Summe der Maechtigkeiten der globalen    */
/*                  Termpaarmengen SetOfRules[], SetOfEquations,             */
/*                  SetOfCriticalPairs, SetOfGoals, SetOfCriticalGoals.      */
/*                                                                           */
/*  Beschreibung :  Mit des SizeOf-Makros werden die Maechtigkeiten bestimmt.*/
/*                  Bei den Regeln steht die Maechtigkeit in der nullten     */
/*                  Komponente des Feldes SetOfRules[].                      */
/*                                                                           */
/*  Globale Var. :  SetOfRules[]                                             */
/*                  SetOfEquations                                           */
/*                  SetOfCriticalPairs                                       */
/*                  SetOfGoals                                               */
/*                  SetOfCriticalGoals                                       */
/*                                                                           */
/*  Externe Var. :  FuncCount                                                */
/*                                                                           */
/*****************************************************************************/

long akt_termpaare_anz_best ( void )
{
    return (SizeOf( SetOfRules[0] ) + SizeOf(SetOfEquations) +
            SizeOf( SetOfCriticalPairs) + SizeOf( SetOfGoals ) +
            SizeOf( SetOfCriticalGoals ) );

} /* Ende von akt_termpaare_anz_best */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  PrintAllPairs                                            */
/*                                                                           */
/*  Parameter    :  File-Deskriptor, in den geschrieben werden soll          */
/*                                                                           */
/*  Returnwert   :  kein                                                     */
/*                                                                           */
/*  Beschreibung :  Alle Termpaarmengen des aktuellen Systems werden in die  */
/*                  angegebene Datei geschrieben.                            */
/*                                                                           */
/*  Globale Var. :  SetOfCriticalPairs                                       */
/*                  SetOfCriticalGoals                                       */
/*                  SetOfEquations                                           */
/*                  SetOfRules                                               */
/*                  SetOfGoals                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void PrintAllPairs ( FILE  *file )
{
  termpair    *ptr;
  int         i;

  fprintf( file, "Menge der kritischen Paare : \n");
  ptr = SetOfCriticalPairs.first;
  while (ptr)
  {
    fprinttpair ( file, ptr, " --> ");
    ptr = ptr->next;
  }

  fprintf( file, "Menge der kritischen Ziele : \n");
  ptr = SetOfCriticalGoals.first;
  while (ptr)
  {
    fprinttpair ( file, ptr, " --> ");
    ptr = ptr->next;
  }

  fprintf( file, "Menge der Gleichungen : \n");
  ptr = SetOfEquations.first;
  while (ptr)
  {
    fprinttpair ( file, ptr, " --> ");
    ptr = ptr->next;
  }

  fprintf( file, "Menge der Regeln : \n");
  for (i = 0; i <= FuncCount; i++)
  {
    ptr = SetOfRules[i].first;
    while (ptr)
    {
      fprinttpair ( file, ptr, " --> ");
      ptr = ptr->next;
    }
  }

  fprintf( file, "Menge der Ziele : \n");
  ptr = SetOfGoals.first;
  while (ptr)
  {
    fprinttpair ( file, ptr, " --> ");
    ptr = ptr->next;
  }
} /* PrintAllPairs */
