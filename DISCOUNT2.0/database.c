/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          database
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Einlesen einer Aufgabenstellung
//-----------------------------------------------------------------------------
//      $Log: database.c,v $
//      Revision 1.1  1991/09/25  12:02:10  pitz
//      Initial revision
//
//
//=============================================================================
*/

#include    <stdio.h>
#include    <sys/param.h>
#include    "defines.h"
#include    "error.h"
#include    "memory.h"
#include    "list.h"
#include    "vartree.h"
#include    "polynom.h"
#include    "term.h"
#include    "termpair.h"
#include    "subst.h"
#include    "order.h"
#include    "cpweight.h"
#include    "reduce.h"
#include    "pcl.h"
#include    "buildcp.h"
#include    "complet.h"
#include    "systime.h"
#include    "domain.h"
#include    "referee.h"
#include    "database.h"
#include    "expert.h"
#include    "exp_set_next_config.h"
#include    "exp_def_next_cycletime.h"
#include    "team.h"


/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
               /* Nummer der Domaene, die zur Zeit vom Data-   */
	       /* base-Experten betrachtet wird                */
               /************************************************/
short      AktDomNr;

               /************************************************/
	       /* SFIND_ALL oder SFIND_FIRST, je nachdem, ob   */
	       /* alle Dom"anen gefunden werden sollen oder ob */
	       /* nach der ersten gefundenen gestoppt werden   */
	       /* soll.                                        */
               /************************************************/
Symbol            SuchModus;

               /************************************************/
	       /* Termpaar, mit dem die Regel und Gleichungs-  */
	       /* menge interreduziert wird.                   */
               /************************************************/
static   termpair   *global_termpair;

               /************************************************/
               /* Hier werden die bei der Dom"ane angegebenen  */
	       /* Ergebnisse nach Anwenden der gefundenen      */
	       /* Substitution abgelegt.                       */
               /************************************************/
static pairset      DatabaseResult = EmptySet;

               /************************************************/
	       /* Nach Aufruf der Funktion rule_and_equ_empty  */
	       /* gibt diese Variable an, ob die Menge der Re- */
	       /* geln und Gleichungen zu Beginn leer gewesen  */
	       /* ist.                                         */
	       /* true  -> Menge war leer.                     */
	       /* false -> Menge war nicht leer.               */
               /************************************************/
bool               no_rule_equ = false;


               /************************************************/
               /* Z"ahlt die Anzahl der versuchten Matches     */
	       /* pro Dom"ane.                                 */
               /************************************************/
static int          match_count = 0;

/*****************************************************************************/
/*                                                                           */
/*               Variablen, die auch extern verwendet werden                 */
/*                                                                           */
/*****************************************************************************/
               /************************************************/
               /* Anzahl der in diesem Zyklus betrachteten     */
	       /* Domaenen.                                    */
               /************************************************/
short           LookedDomains = 0;

      /*****************************************************************/
      /* Die folgenden Variablen dienen zur Verwaltung der Funktions-  */
      /* symbole aus der Aufagbenstellung beim Aufstellen aller moeg-  */
      /* lichen Substitutionen zweiter Ordnung mit den Funktionssym-   */
      /* bolen aus einer Domaenenspezifikation. Sie werden beide nach  */
      /* dem Parsen der Aufgabenstellung direkt versorgt!              */
      /*****************************************************************/

               /************************************************/
               /* In der folgenden Matrix werden nach Stellig- */
	       /* keiten sortiert die Funktionssymbole aus der */
	       /* Aufgabenstellung abgelegt. Der erste Index   */
	       /* gibt zugleich die Stelligkeit der darunter   */
	       /* abgespeicherten Funktionssymbole an.         */
               /************************************************/
SubstituierendesFSymb    SubstMatrix[MAXARITY][MAXFUNCTION];

               /************************************************/ 
	       /* In dem folgenden Feld wird fuer jede Stellig-*/
	       /* keit abgelegt, wieviele Funktionssymbole es  */
	       /* in der Aufgabenstellung eben mit dieser      */
	       /* Stelligkeit gibt.                            */
               /************************************************/
short                    ArityLength[MAXARITY];


               /************************************************/
               /* Wenn der Database-Experte eingesetzt wird,   */
	       /* setzt dieser diese Variable auf true.        */
	       /* Falls der Experte, der nach Erkennen einer   */
	       /* Dom"ane auf dem Prozessor des Database-Exper-*/
	       /* ten weiterarbeitet, die Beweisaufgabe l"osen */
	       /* kann, zeigt diese Variable an, da"s f"ur den */
	       /* Reproduktionslauf als erstes der Database-   */
	       /* Experte eingesetzt werden mu"s.              */
	       /* Dies wird in der Funktion Completed in       */
	       /* complet.c abgefragt.                         */
	       /* Dies ist notwendig, da die Variable          */
	       /* OwnConfig "uberschrieben wird.               */
               /************************************************/
bool                     database_config = false;

/*****************************************************************************/
/*                                                                           */
/*                          Konstanten                                       */
/*                                                                           */
/*****************************************************************************/
/* Diese Konstanten stehen auch in term.c -> bei Zeit vereinheitlichen */
static char      varstring[] = "Vxyzuvw";   /* Variablen fuer Pretty-Print  */
#define          varstrlen      6           /* Anzahl der Variablen         */

/*****************************************************************************/
/*                                                                           */
/*                Makros in der Rolle von Funktionsaufrufen                  */
/*                                                                           */
/*****************************************************************************/

               /************************************************/
/* Belegen eines Funktionssymbols, mit dem ein anderes substituiert wird     */
/* "arity" ist dabei die Stelligkeit der Funktionssymbole, und nr ist die    */
/* Nummer des Funktionssymbols in der Domaenenspezifikation. Beachte, dass   */
/* die Komponente "akt_match_index" bei diesem Funktionssymbol sinnvoll be-  */
/* legt sein muss, sonst ist das Ergebnis nicht vorhersagbar. Dieses Makro   */
/* sollte nur nach erfolgreichem Aufruf der Funktion NextFktSymbMatch erfol- */
/* gen.                                                                      */
               /************************************************/
#define subst_belegen(arity,nr) \
        SubstMatrix[arity][DomainGlobalInfo[AktDomNr].dom_funktion[nr].akt_match_index].belegt = true;

/* Freigeben eines Funktionssymbols, mit dem ein anderes substituiert wurde  */
/* Die Bedeutung von arity und nr ist wie bei subst_belegen, wobei auch hier */
/* garantiert sein sollte, dass akt_match_index einen sinnvollen Wert ent-   */
/* haelt!                                                                    */
#define subst_freigeben(arity,nr) \
        SubstMatrix[arity][DomainGlobalInfo[AktDomNr].dom_funktion[nr].akt_match_index].belegt = false;


/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static void      local_prettyprint ( term *t, vartree **vars, variable *counter );

static void      local_printtpair ( termpair *pair );

static void      print_aktuelle_substitution( void );

static void      print_subst_zustand( void );

static void      match_eintragen( void );

static bool      fkt_not_used( function fkt_sym_aufgabe, function fkt_sym_domaene );

static bool      rule_and_equ_empty ( void );

static void      interRedRule ( termpair *ptr );

static void      interRedEqu ( termpair *ptr );

static void      interRed (termpair *tp);

static void      BuildResults ( void );

static term     *buildterm ( term *old_term );

static termpair *buildtermpair ( termpair *old_termpair );

static short     NextFktSymbMatch ( short fkt_nr );

static bool      restliche_subst_best ( short nr );

static bool      NextMatches ( short nr );


/*****************************************************************************/
/*                                                                           */
/*  Funktionen zum Ausgeben von Termpaaren innerhalb einer Domaene.          */
/*  Beachte : Sie koennen nicht mit printtpair ausgegeben werden, weil       */
/*            dort eine andere Zuordnung der Funktionssymbole erfolgt.       */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  local_prettyprinty                                       */
/*                                                                           */
/*  Parameter    :  Term, der ausgegeben werden soll                         */
/*                  Variablenbaum                                            */
/*                  Anzahl der bisherigen Variablen                          */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Der angegebene Term wird gem"a"s der durch AktDomNr      */
/*                  gegebenen Dom"ane ausgegeben                             */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static void    local_prettyprint ( term *t, vartree **vars, variable *counter )
{
    short       i = 0;
    variable    v;

    if (varp(t))
    {
        if ((v = VTfind (*vars, t->fcode)) == 0)
	{
	  VTadd ( vars, t->fcode, v = ++*counter );
	}

        if (v > varstrlen)
	{
	  printf ( "x%d", v-varstrlen );
	}
        else
	{
	  printf ( "%c", varstring[v] );
	}
    }
    else
    {
	/* Hier ist die Aenderung zu dem globalen prettyprint !! */
        printf ( DomainGlobalInfo[AktDomNr].dom_funktion[t->fcode].ident );
        if (t->arity > 0)
        {
            printf ( " (" );
            while ( i+1 < t->arity )
            {
                local_prettyprint ( t->argument[i++], vars, counter );
                printf ( "," );
            }
            local_prettyprint ( t->argument[i], vars, counter );
            printf ( ")" );
        }
    }
} /* Ende von local_prettyprint */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  local_printtpair                                         */
/*                                                                           */
/*  Parameter    :  Termpaar, das ausgegeben werden soll                     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Das Termpaar wird ausgegeben.                            */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  FuncCount                                                */
/*                  SetOfRules                                               */
/*                  SetOfEquations                                           */
/*                                                                           */
/*****************************************************************************/

static void    local_printtpair ( termpair *pair )
{
    vartree     *vars    = NULL;
    variable    counter  = 0;

    local_prettyprint ( pair->left,  &vars, &counter );

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

    local_prettyprint ( pair->right, &vars, &counter );
    printf ( "\n" );
    flush();

    VTclear ( &vars );
} /* Ende von local_printtpair */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  print_aktuelle_substitution                              */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion dient fuer Debug-Zwecke. mIt wird die     */
/*                  aktuell gefundene Substitution( Match ) ausgegeben.      */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                  Function                                                 */
/*                                                                           */
/*****************************************************************************/
static void print_aktuelle_substitution( void )
{
  int i;

  printf("  Symbol in der Domaenenspec | Symbol in der Spezifikation \n");
  for( i=1;i<=DomainGlobalInfo[AktDomNr].dom_fkt_anz; i++ )
  {
    printf("%28s <- %22s\n", DomainGlobalInfo[AktDomNr].dom_funktion[i].ident,
	   Function[DomainGlobalInfo[AktDomNr].dom_funktion[i].fmatch].ident );
  }
  flush();
} /* Ende von print_aktuelle_substitution */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  print_subst_zustand                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion dient fuer Debug-Zwecke. Mit ihr wird der */
/*                  aktuelle Belegungszustand der Funktionssymbole, wie er   */
/*                  in der Variablen SubstMatrix bzw. ArityLength enthalten  */
/*                  ist, ausgegeben.                                         */
/*                                                                           */
/*  Globale Var. :  ArityLength                                              */
/*                  SubstMatrix                                              */
/*                                                                           */
/*  Externe Var. :  Function                                                 */
/*                                                                           */
/*****************************************************************************/
static void print_subst_zustand( void )
{
  int i,j;

  /* Es werden nur Funktionssymbole bis zur Stelligkeit 5 ausgegeben */
  printf("Anzahl der Funktionssymbole bis zur Stelligkeit 5 : \n");
  for( i=0; i<5; i++ )
  {
    printf("%d Funktionssymbole der Stelligkeit %d\n",ArityLength[i], i );
  }

  /* Auch hier werden nur die Funktionssymbole bis zur Stelligkeit 5 betrachtet */
  printf("\nAktueller Zusatnd der SubstMatrix : \n");
  for( i=0; i<5; i++ )
  {
    for( j=0; j<ArityLength[i]; j++)
    {
      printf("Position %d bei Fktssymbolen der Stelligkeit %d ist %s belegt.",
	     j, i, SubstMatrix[i][j].belegt ? "" : "nicht" );
      if( SubstMatrix[i][j].belegt )
      {
	printf("Fktssymb : %s", Function[SubstMatrix[i][j].fktnr].ident );
      } /* Ende von if */
      printf("\n");
    } /* Ende von for j */
  } /* Ende von for i */

  flush();
} /* Ende von print_subst_zustand */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  match_eintragen                                          */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur versorgt die externen Variablen           */
/*                  FoundDomain und FoundDomainMatchFunc, d.h. eine ge-      */
/*                  fundene Dom'ane wird dort abgelegt.                      */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                  FoundDomain                                              */
/*                  FoundDomainCount                                         */
/*                  FoundDomainMatchFunc                                     */
/*                                                                           */
/*****************************************************************************/

static void match_eintragen( void )
{
  short i;

  FoundDomain[FoundDomainCount] = AktDomNr;

  for( i=1; i<=DomainGlobalInfo[AktDomNr].dom_fkt_anz; i++ )
  {
    FoundDomainMatchFunc[FoundDomainCount][i] = DomainGlobalInfo[AktDomNr].dom_funktion[i].fmatch;
  }

  FoundDomainCount++;

} /* Ende von match_eintragen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  fkt_not_used                                             */
/*                                                                           */
/*  Parameter    :  Nummer eines Funktionssymbols aus der Aufgabenstellung   */
/*                  Nummer des Funktionssymbols der aktuellen Dom"ane, bis   */
/*                  zu dem die Funktionssymbole bereits substituiert sind.   */
/*                                                                           */
/*  Returnwert   :  true, falls das durch den ersten Parameter gegebene      */
/*                        Funktionssymbol noch nicht als substituierendes    */
/*                        Funktionssymbol bei einem Funktionssymbol der ak-  */
/*                        tuellen Dom"ane vorkommt.                          */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es werden alle bereits durchgef"uhrten Funktions-        */
/*                  symbolsubstitutionen "uberpr"uft.                        */
/*                  BEACHTE: Diese Funktion garantiert die, dass die         */
/*                           Gesamtsubstitution schlie"slich injektiv ist.   */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                  FuncCount                                                */
/*                  Function                                                 */
/*                                                                           */
/*****************************************************************************/
static bool fkt_not_used( function fkt_sym_aufgabe, function fkt_sym_domaene )
{
  function i;

  for( i=fkt_sym_domaene-1; i>0; i-- )
  {
    if( DomainGlobalInfo[AktDomNr].dom_funktion[i].fmatch == fkt_sym_aufgabe )
    {
      return false;
    }
  } /* Ende von for */

  return true;
} /* Ende von fkt_not_used */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  rule_and_equ_empty                                       */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls es keine Regeln oder Gleichungen gibt        */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es wird "uberpr"uft, ob die Regelmengen und die Glei-    */
/*                  chungsmenge leer ist.                                    */
/*                  Es wird die globale Variable no_rule_equ gesetzt: auf    */
/*                  true, wenn keine Regeln oder Gleichungen vorliegen, sonst*/
/*                  auf false ( Das ist ihr Initialisierungswert. )          */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  FuncCount                                                */
/*                  SetOfRules                                               */
/*                  SetOfEquations                                           */
/*                  no_rule_equ                                              */
/*                                                                           */
/*****************************************************************************/
static bool rule_and_equ_empty ( void )
{
  function i;

  /*********************/
  /* Testen der Regeln */
  /*********************/
  for (i=0;i<=FuncCount;i++)
  {
    if (SetOfRules[i].first)
    {
      return false;
    }
  }

  /***************************/
  /* Testen der Gleichungen  */
  /***************************/
  return ( no_rule_equ = !(bool)SetOfEquations.first );
} /* Ende von rule_and_equ_empty */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  interRedRule                                             */
/*                                                                           */
/*  Parameter    :  Regel, die mit dem Termpaar global_termpair reduziert    */
/*                  werden soll                                              */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird versucht die Regel, mit dem in der globalen      */
/*                  Variablen global_termpair abgelegten Termpaar zu re-     */
/*                  duzieren. Handelt es sich bei dem Termpaar in            */
/*                  global_termpair um eine Regel, so darf sie nat"urlich    */
/*                  nicht auf sich selbst angewendet werden, daher die erste */
/*                  if-Abfrage.                                              */
/*                                                                           */
/*                                                                           */
/*  Globale Var. :  global_termpair                                          */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/
static void interRedRule ( termpair *ptr )
{
  bool red; /* true, wenn Reduktion m"oglich war */
  if( ptr == global_termpair )
  {
    return;
  }

  /*************************************/
  /* zuerst die rechte Seite der Regel */
  /*************************************/
  red = (global_termpair->type == RULE) ? Rreduce(global_termpair, &(ptr->right))
					: Ereduce(global_termpair, &(ptr->right));
  if( red )
  {
    reduce( &(ptr->right) );
  }

  /***********************************/
  /* jetzt die linke Seite der Regel */
  /***********************************/
  red = (global_termpair->type == RULE) ? Rreduce(global_termpair, &(ptr->left))
					: Ereduce(global_termpair, &(ptr->left));
  if( red )
  {
    reduce( &(ptr->left) );
    if( equal(ptr->left, ptr->right) )
    {
      DeleteRule( ptr );
      deleteterm( ptr->right );
      deleteterm( ptr->left );
      deletepair( ptr );
    }
    else
    {
      switch( Compare( ptr->left, ptr->right ) )
      {
	case TLESS  : SwapSides( ptr );
		      break;
	case TANY   :
	case TEQUAL : DeleteRule( ptr );
		      ptr->type=EQUATION;
		      InsertEqu( ptr );
		      break;
	default       : printf("undefined result of function 'Compare'\n");
			exit(1);
      } /* Ende von switch */
    } /* Ende von else */
  } /* Ende von if( red ) */
} /* Ende von interRedRule */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  interRedEqu                                              */
/*                                                                           */
/*  Parameter    :  Gleichung, die mit dem Termpaar global_termpair reduziert*/
/*                  werden soll                                              */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird versucht die Gleichung, mit dem in der globalen  */
/*                  Variablen global_termpair abgelegten Termpaar zu re-     */
/*                  duzieren. Handelt es sich bei dem Termpaar in            */
/*                  global_termpair um eine Gleichung, so darf sie nat"urlich*/
/*                  nicht auf sich selbst angewendet werden, daher die erste */
/*                  if-Abfrage.                                              */
/*                                                                           */
/*  Beschreibung :                                                           */
/*                                                                           */
/*  Globale Var. :  global_termpair                                          */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/
static void interRedEqu ( termpair *ptr )
{
  bool red = false; /* true, wenn Reduktion m"oglich war */

  if( ptr == global_termpair )
  {
    return;
  }

  if( global_termpair->type == RULE )
  {
    if ( Rreduce( global_termpair, &(ptr->left) ) )
    {
      red = true;
      reduce( &(ptr->left) );
    }
    if ( Rreduce( global_termpair, &(ptr->right) ) )
    {
      red = true;
      reduce( &(ptr->right) );
    }
  } /* Ende von if ...->type == RULE ) */
  else
  {
    if ( Ereduce( global_termpair, &(ptr->left) ) )
    {
      red = true;
      reduce( &(ptr->left) );
    }
    if ( Ereduce( global_termpair, &(ptr->right) ) )
    {
      red = true;
      reduce( &(ptr->right) );
    }
  } /* Ende von else */

  /************************************************/
  /* Testen, ob Gleichung reduziert werden konnte */
  /************************************************/
  if( red )
  {
    if( equal( ptr->left, ptr->right ) )
    {
      DeleteEqu( ptr );
      deleteterm( ptr->left );
      deleteterm( ptr->right );
      deletepair( ptr );
    }
    else
    {
      switch( Compare( ptr->left, ptr->right ) )
      {
	case TGREATER : DeleteEqu( ptr );
			ptr->type=RULE;
			InsertRule( ptr );
			break;
	case TLESS    : DeleteEqu( ptr );
			ptr->type=RULE;
			SwapSides( ptr );
			InsertRule( ptr );
			break;
	default       : printf("undefined result of function 'Compare'\n");
			exit(1);
      } /* Ende von switch */
    } /* Ende von else */
  } /* Ende von if ( red ) */
} /* Ende von interRedEqu */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  interRed                                                 */
/*                                                                           */
/*  Parameter    :  Termpaar, mit dem interreduziert werden soll             */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird zun"achst versucht, alle Regeln mit dem          */
/*                  Termpaar zu reduzieren; anschlie"send alle Gleichungen   */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/
static void interRed (termpair *tp)
{
  global_termpair = tp;

  ForAllRulesDo( interRedRule );
  ForAllEquDo( interRedEqu );

} /* Ende von interRed */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  FindRules                                                */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls alle bei der aktuellen Domaene angegebenen   */
/*                        Gleichungen in der Regel bzw. Gleichungsmenge ge-  */
/*                        funden werden konnten.                             */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :                                                           */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/
bool   FindRules ( void )
{
  termpair    *ptr, *new_tp, *subsum_tp, *next;
  short       i;
  bool        domaene_gefunden = false;

  if ( !DemoMode )
  {
    printf("\n");
  }

  /*************************************************/
  /* Testen, ob Regeln oder Gleichungen vorliegen. */
  /* Sonst Betrachten der kritischen Paare.        */
  /* Interessant vor allem im ersten Zyklus        */
  /*************************************************/
  if ( rule_and_equ_empty() )
  {
    printf("\nKeine Regeln oder Gleichungen vorhanden.\n");
    printf("Benutze kritische Paare...\n\n");

    ptr=SetOfCriticalPairs.first;
    while (ptr)
    {
      next=ptr->next;
      if (!DemoMode)
      {
        printf("kritisches Paar: ");
        printtpair(ptr);
      }
      reduce(&(ptr->left));
      reduce(&(ptr->right));
      if (!DemoMode)
      {
        printf("nach Reduktion: ");
        printtpair(ptr);
      }
      if (equal(ptr->left,ptr->right))
      {
        if (!DemoMode)
          printf("ist trivial und wird deshalb geloescht.\n");
        deleteterm(ptr->left);
        deleteterm(ptr->right);
        deletepair(ptr);
      }
      else
      {
        switch (Compare(ptr->left,ptr->right))
        {
          case TGREATER : InsertRule(ptr);
                          ptr->type=RULE;
                          if (!DemoMode)
                          {
                            printf("Regel: ");
                            printtpair(ptr);
                          }
                          break;
          case TLESS    : SwapSides(ptr);
                          InsertRule(ptr);
                          ptr->type=RULE;
                          if (!DemoMode)
                          {
                            printf("Regel: ");
                            printtpair(ptr);
                          }
                          break;
	  case TANY     :
          case TEQUAL   : InsertEqu(ptr);
                          ptr->type=EQUATION;
                          if (!DemoMode)
                          {
                            printf("Gleichung: ");
                            printtpair(ptr);
                          }
                          break;
          default       : printf("undefined result of function 'Compare'\n");
                          exit(1);
        }
        interRed(ptr);
       }
      if (!DemoMode)
      {
        printf("\n");
      }
      ptr=next;
    }
    SetOfCriticalPairs.first=SetOfCriticalPairs.last=NULL;
    SetOfCriticalPairs.count=SetOfCriticalPairs.setcount=0;
  } /* Ende von if --> keine Gleichungen oder Regeln vorhanden */

  /**********************************************************/
  /* Testen, ob die bei der Dom"ane angegebenen Gleichungen */
  /* in der aktuellen Problemstellung enthalten sind.       */
  /**********************************************************/
  /*if( !DemoMode )
  {
    printf("%2d. MATCH:\n\n", ++match_count);
    print_aktuelle_substitution();
  }*/

  for( i=0; i<DomainGlobalInfo[AktDomNr].def_anz && !domaene_gefunden; i++ )
  {
    domaene_gefunden = true;

    if( !DemoMode )
    {
      printf("%2d. Menge von definierenden Gleichungen:\n\n", i+1);
    }

    for( ptr=DomainGlobalInfo[AktDomNr].def_gleichungen[i].first;
	 ptr;
	 ptr=ptr->next )
    {
      if (!DemoMode)
      {
        printf("Voraussetzung der Datenbasis: ");
	local_printtpair( ptr );
	printf("nach Anwendung des Signatur-Matches: ");
      }

      new_tp = buildtermpair( ptr );
      if (!DemoMode)
      {
	printtpair( new_tp );
      }

      reduce( &(new_tp->left) );
      reduce( &(new_tp->right) );
      if (!DemoMode)
      {
	printf("Nach Reduktion: ");
	printtpair( new_tp );
      }

      /***********************************************************/
      /* Gleichung konnte auf identische Seiten reduziert werden */
      /***********************************************************/

      if( equal( new_tp->left, new_tp->right ) )
      {
	if (!DemoMode)
	{
	  printf("-- identische Seiten : ok --\n");
	}
      }
      /****************************************************************/
      /* Gleichung kann von einer anderen Gleichung subsumiert werden */
      /****************************************************************/
      else if( (subsum_tp = FindSubsum(&SetOfEquations, new_tp)) != NULL )
      {
	if (!DemoMode)
	{
	  printf("subsumiert durch ");
	  printtpair( subsum_tp );
	}
      } /* Ende von else if */
      /*************************************************************/
      /* Gleichung kann in der aktuellen Problemstellung nicht als */
      /* g"ultig nachgewiesen werden.                              */
      /*************************************************************/
      else
      {
	deleteterm(new_tp->left);
	deleteterm(new_tp->right);
	deletepair(new_tp);
	if (!DemoMode)
	{
	  printf("\n");
	}
	domaene_gefunden = false;
	break;
      }
    } /* Ende des inneren for */
  } /* Ende des "au"seren for */

  if (!DemoMode)
  {
    printf ( "\n" );
  }

  return domaene_gefunden;
} /* Ende von FindRules */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  buildterm                                                */
/*                                                                           */
/*  Parameter    :  Pointer auf Term, der umgebaut werden soll               */
/*                                                                           */
/*  Returnwert   :  Pointer auf den substituierten Term                      */
/*                                                                           */
/*  Beschreibung :  Der uebergebene Term wird gemaess des Matchs, das fuer   */
/*                  die aktuell betrachtete Domaene gefunden wurde umge-     */
/*                  schrieben, wobei auch neue Variablen kreiert werden.     */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

static term  *buildterm ( term *old_term )
{
    term    *new_term;
    short   i;

    /********************************/
    /* Testen, ob Variable vorliegt */
    /********************************/
    if (old_term->fcode < 0)
    {
      new_term = newterm ( old_term->fcode );
      new_term->weight = old_term->weight;

      return new_term;
    }

    /**********************************************/
    /* Funktionssymbol liegt vor --> wird ersetzt */
    /**********************************************/
    new_term = newterm ( DomainGlobalInfo[AktDomNr].dom_funktion[old_term->fcode].fmatch );
    new_term->weight = old_term->weight;
    for (i = 0; i < old_term->arity; i++)
    {
      new_term->argument[i] = buildterm ( old_term->argument[i] );
    }

    return new_term;
} /* Ende von buildterm */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  buildtermpair                                            */
/*                                                                           */
/*  Parameter    :  Pointer auf Termpaar, das umgebaut werden soll           */
/*                                                                           */
/*  Returnwert   :  Pointer auf das substituierte Termpaar                   */
/*                                                                           */
/*  Beschreibung :  Das uebergebene Termpaar wird gemaess des Matchs, das    */
/*                  fuer die aktuell betrachtete Domaene gefunden wurde umge-*/
/*                  schrieben, wobei auch neue Variablen kreiert werden.     */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

static termpair *buildtermpair ( termpair *old_termpair )
{
  termpair *new_termpair;

  new_termpair = newpair( buildterm( old_termpair->left ),
			  buildterm( old_termpair->right ),
			  NULL,
			  NULL );
  new_termpair->type = EQUATION;

  return new_termpair;
} /* Ende von buildtermpair */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  BuildResults                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die bei der aktuell betrachteten Domaene als bekannt an- */
/*                  gegebenen Termpaare werden zuertst daraufhin untersucht, */
/*                  ob sie bereits in der aktuellen Regel- bzw Gleichungsmen-*/
/*                  ge enthalten sind, wenn nicht werden die beiden Terme des*/
/*                  Termpaares mit buildterm umgeschrieben, reduziert und    */
/*                  falls dann ungleich in die Ergebnismenge des Database-   */
/*                  Experten aufgenommen.                                    */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                  Result                                                   */
/*                  pair                                                     */
/*                  DatabaseResult                                           */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

static void   BuildResults ( void )
{
    termpair    *ptr, *pair;
    term        *left, *right;

    ptr = DomainGlobalInfo[AktDomNr].bekannte_res.first;
    while( ptr )
    {
      getweight ( left  = buildterm ( ptr->left  ));
      getweight ( right = buildterm ( ptr->right ));
      reduce ( &left );
      reduce ( &right );
      if (!equal (left,right))
      {
	pair = newpair ( left, right, NULL, NULL );
	if (!FindSubsum(&SetOfEquations,pair))
	{
	  Insert( &DatabaseResult, pair );
	  if( !DemoMode )
	  {
	    printf("Reduziertes Resultat des Database-Experten : ");
	    printtpair ( pair );
	    flush();
	  }
	}
      }
      else
      {
       deleteterm ( left  );
       deleteterm ( right );
      }

      ptr = ptr->next;
    } /* Ende von while */

    if (!DemoMode && !DatabaseResult.setcount )
    {
      printf ( "Keine neuen Ergebnisse !\n\n" );
    }
} /* Ende von BuildResults */

static void print_subst_matrix()
{
  int i,j;

  for(i=0;i<ArityLength[0];i++)
  {
    for( j=0;j<5;j++ )
    {
      printf("%3s", SubstMatrix[i][j].belegt == false ? "f" : "b" );
    }
    printf("\n");
  }
} /* Ende von print_subst_matrix */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  NextFktSymbMatch                                         */
/*                                                                           */
/*  Parameter    :  Nummer eines Funktionssymbols aus der durch AktDomNr     */
/*                  bestimmten Domaenenspezifikation.                        */
/*                                                                           */
/*  Returnwert   :  Falls es noch ein Funktionssymbol selber Stelligkeit in  */
/*                  der Signatur der Aufgabenstellung gibt, das noch nicht   */
/*                  belegt ist, so wird das naechst moegliche zurueckgegeben.*/
/*                  Gibt es kein freies Funktionssymbol mehr, so wird 0 zu-  */
/*                  rueckgegeben, was mit Sicherheit keine Nummer eines Funk-*/
/*                  tionssymbols ist.                                        */
/*                  Es wird also so garantiert, da"s die Substitution        */
/*                  injektiv ist.                                            */
/*                                                                           */
/*  Beschreibung :  Die Matrix SubstMatrix wird fuer die durch die uebergebe-*/
/*                  ne Nummer bestimmte Stelligkeit durchsucht; begonnen     */
/*                  wird dabei bei dem Feldindex, der durch next_match_index */
/*                  vorgegeben ist und die Suche erfolgt zyklisch.           */
/*                  Mit Hilfe eines Zaehlers wird ueberprueft, ob man bereits*/
/*                  alle moeglichen Funktionssymbole betrachtet hat.         */
/*                  Wurde eine Substitution gefunden wird auch noch          */
/*                  next_match_index veraendert.                             */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static short NextFktSymbMatch ( short fkt_nr )
{
  short zaehler = 0;
  short arity;
  short *next;
  short match_fkt_symb_nr;

  /**************************************************/
  /* Stelligkeit des zu matchenden Funktionssymbols */
  /**************************************************/
  arity = DomainGlobalInfo[AktDomNr].dom_funktion[fkt_nr].arity;

  /*******************************************************/
  /* Position in SubstMatrix[arity], ab der gesucht wird */
  /*******************************************************/
  next = &(DomainGlobalInfo[AktDomNr].dom_funktion[fkt_nr].next_match_index);

  while ( zaehler < ArityLength[arity] )
  {
    if ( SubstMatrix[arity][*next].belegt )
    {
      /****************************************************/
      /* Diese Stelle ist bereits belegt -> weiter suchen */
      /****************************************************/
      *next = ( (*next) + 1 ) % ArityLength[arity];
      zaehler++;
    }
    else /* Substitution gefunden */
    {
      match_fkt_symb_nr = SubstMatrix[arity][*next].fktnr;

      /***************************/
      /* Aktuellen Index ablegen */
      /***************************/
      DomainGlobalInfo[AktDomNr].dom_funktion[fkt_nr].akt_match_index = *next;

      /*****************************************/
      /* Festsetzen des naechsten Startpunktes */
      /*****************************************/
      *next = ( (*next) + 1 ) % ArityLength[arity];

      return match_fkt_symb_nr;
    }
  } /* Ende von while */

  /****************************************/
  /* Es konnte kein Match gefunden werden */
  /****************************************/
  return 0;
      
} /* Ende von NextFktSymbMatch */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  restliche_subst_best                                     */
/*                                                                           */
/*  Parameter    :  Nummer eines Funktionssymbols : im folgenden "nr"        */
/*                                                                           */
/*  Returnwert   :  true, wenn eine Dom"ane gefunden werden konnte und       */
/*                        der Suchmodus SFIND_FIRST ist.                     */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es werden alle moeglichen Substitutionen fuer die Funk-  */
/*                  tionssymbole nr bis ...dom_fkt_anz betrachtet unter Be-  */
/*                  ruecksichtigung der durch die Funktionssymbole 1 bis nr-1*/
/*                  bereits belegten Funktionssymbole.                       */
/*                  Ist nr == ...dom_fkt_anz, so wurde die naechste Substi-  */
/*                  tution gefunden und sie wird bearbeitet.                 */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

static bool restliche_subst_best ( short nr )
{
  short start_match;

  /* BEACHTE : Die bisherige Substitution muss an dieser Stelle nicht freige-*/
  /*           geben werden, da dies bereits in NextMatches geschehen ist,   */
  /*           bzw. nach dem Bearbeiten  eines Matches in dieser Funktion    */
  /*           hier selbst geschieht.                                        */
  /*           Es muss also nur der Startindex auf 0 gesetzt werden.         */
  DomainGlobalInfo[AktDomNr].dom_funktion[nr].next_match_index = 0;

  /***************************************************************************/
  /* Bestimmen der ersten Substitution ( eine muss es geben!!! ), sonst haet-*/
  /* te bereits vorher ein Abbruch erfolgen muessen!                         */
  /***************************************************************************/
  start_match = DomainGlobalInfo[AktDomNr].dom_funktion[nr].fmatch = 
		NextFktSymbMatch( nr );
  
  do
  {
    /***************************************/
    /* Ablegen erfolgreicher Matchversuche */
    /***************************************/
    subst_belegen( DomainGlobalInfo[AktDomNr].dom_funktion[nr].arity, nr );

    if ( nr == DomainGlobalInfo[AktDomNr].dom_fkt_anz )
    {
      /************************/
      /* Neuer Match gefunden */
      /************************/
      if( FindRules() )
      {
	BuildResults();
	if( !DemoMode )
	{
	  printf("Gefundener Match:\n");
	  print_aktuelle_substitution();
	}
	match_eintragen();
	if( SuchModus == SFIND_FIRST )
	{
	  return true;
	}
      }
    } /* Ende von if */
    else
    {
      /***********************************/
      /* Alle weiteren Symbole versorgen */
      /***********************************/
      if( restliche_subst_best( nr+1 ) && ( SuchModus == SFIND_FIRST ) )
      {
	return true;
      }
    } /* Ende von else */
    
    /*************************************************************************/
    /* Die betrachtete Substitution muss freigegeben werden, damit wenigstens*/
    /* eine Substitution fuer nr gefunden werden kann!!                      */
    /*************************************************************************/
    subst_freigeben( DomainGlobalInfo[AktDomNr].dom_funktion[nr].arity, nr );

  } while ( start_match != 
	   (DomainGlobalInfo[AktDomNr].dom_funktion[nr].fmatch = NextFktSymbMatch( nr )));

  return false;
} /* Ende von restliche_subst_best */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  NextMatches                                              */
/*                                                                           */
/*  Parameter    :  Nummer eines Funktionssymbols : im folgenden "nr"        */
/*                                                                           */
/*  Returnwert   :  true, wenn eine Dom"ane gefunden werden konnte und       */
/*                        der Suchmodus SFIND_FIRST ist                      */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Diese Funktion bestimmt alle moeglichen Substitutionen,  */
/*                  bei denen die Substitutionen fuer die Funktionssymbole   */
/*                  aus der Domaenenspezifikation mit den Nummern 1 bis nr-1 */
/*                  unveraendert bleiben. Die aktuelle Substitution fuer nr  */
/*                  wird als abgeschlossen bearbeitet betrachtet. Es werden  */
/*                  also alle anderen Moeglichkeiten in dem Rahmen, den die  */
/*                  ersten (nr-1) Substitutionen vorgeben, betrachtet.       */
/*                  In einer Schleife werden alle moeglichen Substitutionen  */
/*                  fuer nr betrachtet und dann jeweils alle Kombinationen   */
/*                  fuer nr+1 bis ...dom_fkt_anz betrachtet.                 */
/*                  Ist nr = ...dom_fkt_anz, also alle Funktionssymbole      */
/*                  substituiert, so wird das Match bearbeitet.              */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

static bool NextMatches ( short nr )
{
  short start_match; /* Match von nr beim Aufruf der Funtion */
  short arity;
  short i;

  /*******************************************************************/
  /* aktuelle match ablegen, um keine Substitution zweimal zu finden */
  /*******************************************************************/
  start_match = DomainGlobalInfo[AktDomNr].dom_funktion[nr].fmatch;

  /*********************************************************/
  /* Stelligkeit des Funktionssymbols, das betrachtet wird */
  /*********************************************************/
  arity = DomainGlobalInfo[AktDomNr].dom_funktion[nr].arity;

  while ( true )
  {
    /***********************************************************************/
    /* Es werden alle Substitutionen f"ur Funktionssymbole gr"o"ser als nr */
    /* freigegeben.                                                        */
    /***********************************************************************/

    for( i=nr; i<=DomainGlobalInfo[AktDomNr].dom_fkt_anz ; i++ )
    {
      subst_freigeben( DomainGlobalInfo[AktDomNr].dom_funktion[i].arity, i );
    }

    if ( ( DomainGlobalInfo[AktDomNr].dom_funktion[nr].fmatch = NextFktSymbMatch( nr ) )
	 == start_match )
    {
      /*****************************************************/
      /* Es gibt keine anderen Moeglichkeiten mehr fuer nr */
      /*****************************************************/
      return false;
    } /* Ende von if */
    
    /**********************************/
    /* Ablegen des gefundenen Matches */
    /**********************************/
    subst_belegen( arity, nr );

    if ( nr == DomainGlobalInfo[AktDomNr].dom_fkt_anz )
    {
      /************************/
      /* Neuer Match gefunden */
      /************************/
      if( FindRules() )
      {
	BuildResults();
	if( !DemoMode )
	{
	  printf("Gefundener Match:\n");
	  print_aktuelle_substitution();
	}
	match_eintragen();
	if( SuchModus == SFIND_FIRST )
	{
	  return true;
	}
      }
    } /* Ende von if */
    else
    {
      /****************************************************************/
      /* Die restlichen Funktionssymbole m"ussen noch versorgt werden */
      /****************************************************************/
      if( restliche_subst_best( nr+1 ) )
      {
	return true;
      }
    } /* Ende von else */
  } /* Ende von while */

  return false;
} /* Ende von NextMatches */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  FindAndTestAllMatches                                    */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls mindestens eine Domain gefunden wurde        */
/*                  und der Suchmodus SFIND_FIRST ist, false sonst           */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur ist die zentrale Steuereinheit beim Fin-  */
/*                  den aller Substitutionen zweiter Ordnung fuer die durch  */
/*                  die globale Variable AktDomNr spezifizierte Domaene.     */
/*                  Zunaechst erfolgt eine Initialisierung.                  */
/*                  Dann wird die erste Substitution aufgebaut, dabei wird   */
/*                  auch getestet, ob es ueberhaupt ein Substitution geben   */
/*                  kann; dies muss im folgenden dann nicht mehr getestet    */
/*                  werden.                                                  */
/*                  Mit diesem match werden dann die definierenden Regeln    */
/*                  und Gleichungen getestet, ob sie in der aktuellen Regel- */
/*                  und Gleichungsmenge enthalten sind und falls ja werden   */
/*                  die angegebenen Resultate der Domaene verarbeitet.       */
/*                  Zum Schluss werden dann noch alle anderen moeglichen     */
/*                  Substitutionen berechnet.                                */
/*                                                                           */
/*                  Wird eine Dom'ane erkannt und ist der Suchmodus          */
/*                  FindFirst, so wird die weitere Suche abgebrochen.        */
/*                                                                           */
/*  Globale Var. :  AktDomNr                                                 */
/*                  ArityLength                                              */
/*                  SubstMatrix                                              */
/*                                                                           */
/*  Externe Var. :  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

bool FindAndTestAllMatches ( void )
{
  short i,j;

  /*******************/
  /* Initialisierung */
  /*******************/
  for ( i=1; i<=DomainGlobalInfo[AktDomNr].dom_fkt_anz; i++ )
  {
    DomainGlobalInfo[AktDomNr].dom_funktion[i].next_match_index = 0;
  }
  
  for  ( i=0; i<MAXARITY; i++ )
  {
    for ( j=0; j<ArityLength[i]; j++ )
    {
      SubstMatrix[i][j].belegt = false;
    }
  }
  
  /*****************************/
  /* Erste Substitution finden */
  /*****************************/
  for ( i=1; i<=DomainGlobalInfo[AktDomNr].dom_fkt_anz; i++ )
  {
    if ( !(DomainGlobalInfo[AktDomNr].dom_funktion[i].fmatch = NextFktSymbMatch( i ) ) )
    {
       return false; /* Dann gibt es keine Substitution */
    } /* Ende von if */
    else
    {
      /**********************************/
      /* Ablegen des gefundenen Matches */
      /**********************************/
      subst_belegen( DomainGlobalInfo[AktDomNr].dom_funktion[i].arity, i );
    } /* Ende von else */
  } /* Ende von for */
  
  /**************************************/
  /* Erste Substitution wird bearbeitet */
  /**************************************/
  if( FindRules() )
  {
    BuildResults();
    if( !DemoMode )
    {
      printf("Gefundener Match:\n");
      print_aktuelle_substitution();
    }
    match_eintragen();;
    if( SuchModus == SFIND_FIRST )
    {
      return true;
    }
  }

  /******************************************************/
  /* Finden und Bearbeiten aller anderen Substitutionen */
  /******************************************************/
  for ( i=DomainGlobalInfo[AktDomNr].dom_fkt_anz; i>=1; i-- )
  {
    if( NextMatches( i ) )
    {
      return true;
    }
  } /* Ende von for */
  return false;
} /* Ende von FindAndTestAllMatches */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DatabaseExpert                                           */
/*                                                                           */
/*  Parameter    :  maximale Laufzeit in Sekunden                            */
/*                  Anzahl der zu betrachtenden Domaenen                     */
/*                  Pointer auf ein Feld mit den Nummern der zu betrachtenden*/
/*                  Domaenen                                                 */
/*                  Suchmodus                                                */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diesr Experte versucht bekannte Strukturen innerhalb     */
/*                  den bisher abgeleiteten Regeln und Gleichungen zu finden.*/
/*                  Die bekannten Strukturen entnimmt er dabei den Domaenen- */
/*                  spezifikationen in der globalen Variablen                */
/*                  DomainGlobalInfo.                                        */
/*                  Es wird vorausgesetzt, dass die benoetigten Domaenenspe- */
/*                  zifikationen auf dem Rechenr des Database-Experten be-   */
/*                  reits vorliegen.                                         */
/*                  Zun"achst versucht der Experte Dom"anen zu erkennen;     */
/*                  Ist der Suchmodus SFIND_FIRST, so stoppy er die Suche    */
/*                  nach dem ersten erfolgreichen Finden einer Dom"ane.      */
/*                  Beim Suchmodus SFIND_ALL versucht er, alle angegebenen   */
/*                  Dom"anen und alle m"oglichen Matches zu finden.          */
/*                  In beiden F"allen arbeitet der Database-Experte dann     */
/*                  als normaler Experte, wobei die bekannten Resultate      */
/*                  als Regeln oder Gleichungen aufgenommen wurden.          */
/*                                                                           */
/*  Globale Var. :  DatabaseResult                                           */
/*                  AktDomNr                                                 */
/*                                                                           */
/*  Externe Var. :  FoundDomainCount                                         */
/*                  DomainGlobalInfo                                         */
/*                                                                           */
/*****************************************************************************/

void    DatabaseExpert ( long sec, short dom_anz, long *dom_nr, Symbol such_modus )
{
    static bool tflag = false;
    short       i, j;
    bool        expert_found = false;
    long        startzeit, endezeit, restzeit;
    termpair    *equ;

    database_config = true;

    no_rule_equ = false;
    SuchModus = such_modus;

    DatabaseResult.first = NULL;
    DatabaseResult.last = NULL;
    DatabaseResult.count = 0;
    DatabaseResult.setcount = 0;

    FoundDomainCount = 0;

    LookedDomains = 0;


    if(!ReproMode)
    {
       settimer ( sec, &tflag );
    }

    startzeit = systime();
    /*************************************/
    /* Unterscheidung je nach Such-Modus */
    /*************************************/
    while( ((!ReproMode && !tflag) || 
	    ( ReproMode && 
	     (FirstStepsDone < FirstStepsPerCycle[CycleCount-1])))
	  && (LookedDomains < dom_anz) && 
	  ( (such_modus == SFIND_ALL) || !FoundDomainCount ) )
    {
       FirstStepsDone++;
       
       match_count = 0;
       
       AktDomNr = dom_nr[LookedDomains];
       
       if( DomainGlobalInfo[AktDomNr].dom_fkt_anz )
       {
	  printf("DOMAENE : %s\n", DomainGlobalInfo[AktDomNr].dom_name );
	  FindAndTestAllMatches();
       }
       
       LookedDomains++;
    } /* Ende von while */

    /**********************************************************************/
    /* Starten eines normalen Experten, falls eine Dom"aene erkannt wurde */
    /**********************************************************************/
    if ( FoundDomainCount )
    {
      /********************************************************************/
      /* Es wird versucht, einen Experten aus einem der Startteams der    */
      /* gefundenen Dom"anen zu finden.                                   */
      /* Falls keine Angaben da sind, wird der erste in der Konfigura-    */
      /* tionsdatei angegebene Experte genommen.                          */
      /********************************************************************/
      for( i=0; i<FoundDomainCount && !expert_found; i++ )
      {
	for( j=0; j<DomainGlobalInfo[FoundDomain[i]].start_team_groesse && !expert_found; j++ )
	{
	  if( (OwnConfig.exp_nr = get_exp_nr(DomainGlobalInfo[FoundDomain[i]].start_team_exp[j].name)) != NOEXPERT )
	  {
	    expert_found = true;
	  }
	} /* Ende des inneren for */
      } /* Ende des "au"seren for */

      /*************************************************/
      /* Falls kein Experte gefunden werden konnte     */
      /* --> erster Experte in der Konfigurationsdatei */
      /*************************************************/
      if( !expert_found )
      {
	OwnConfig.exp_nr = 0;
      }

      printf("\n\nStarten von Experte :\n");
      set_parameter( &OwnConfig );
      SetConfiguration( DemoMode );

      if (CPWeight==occnest)
      {
	init_goal_fn_m();
      }

      SetRefParams();

      /**********************************************************/
      /* Falls am Anfang keine Regeln oder Gleichungen vorlagen */
      /* wird wieder die alte Menge der kritischen Paare herge- */
      /* stellt.                                                */
      /**********************************************************/
      if( no_rule_equ )
      {
	TPClearCPCache();
	ForAllRulesDo( InsertCP );

	for( i=0; i <= FuncCount; i++ )
	{
	  SetOfRules[i].first = SetOfRules[i].last = NULL;
	} /* Ende von for */
	SetOfRules[0].setcount = SetOfRules[0].count = 0;


	ForAllEquDo(  InsertCP );
	SetOfEquations.first = SetOfEquations.last = NULL;
	SetOfEquations.setcount = SetOfEquations.count = 0;
      } /* Ende von if */

      /************************/
      /* Starten des Experten */
      /************************/
      endezeit = systime();
      if( timeunit == SECONDS )
      {
	restzeit = sec - (( endezeit - startzeit )/1000);
	printf("  ( Verbleibende Zeit : %4ld s )\n", restzeit );
      }
      else
      {
	restzeit = sec - ( endezeit - startzeit );
	printf("  ( Verbleibende Zeit : %4ld.%03ld s )\n", restzeit/1000, restzeit%1000 );
      }
      printf("\n");
      if( !tflag )
      {
	Interreduce( &DatabaseResult );
	ParallelCompletion( sec - (( endezeit - startzeit )/1000) );
	Referee( SR_NONE, NULL,
		 RefGlobalInfo[OwnConfig.exp_nr].resultate_ga, resultate_ga_param );
      }
      else
      {
	Referee( SR_NONE, NULL, SR_NONE, NULL );
	/* Wenn keine Zeit mehr f"ur ParallelCompletion war, werden alle */
	/* neuen Resultate an den Leiter gesendet!                       */
	/* maximal jedoch MAXREFEQU                                      */
	for( equ = DatabaseResult.first; 
	     equ && (referee_report.equ_count < MAXREFEQU);
	     equ = equ->next )
	{
	  referee_report.equ[referee_report.equ_count++] = equ;
	} /* Ende von for */
      } /* Ende von else */

      /************************************************/
      /* Zur"ucksetzen der Konfiguration auf DATABASE */
      /************************************************/
      OwnConfig.exp_nr = DATABASE;
    } /* Ende von if ( FoundDomainCount ) */
    else
    {
      Referee( SR_NONE, NULL, SR_NONE, NULL );
    }

    database_config = false;
} /* Ende von DatabaseExpert */
