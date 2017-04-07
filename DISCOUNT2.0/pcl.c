/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         pcl
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Ausgabe fuer PCL-Umsetzung
//-----------------------------------------------------------------------------
//      $Log: pcl.c,v $
//      Revision 0.1  1991/08/19  09:50:06  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/


#include    "pcl.h"


#ifdef PCL

/*
//-----------------------------------------------------------------------------
//      Sichtbare Variablen
//-----------------------------------------------------------------------------
*/

ExtractType     extract  = NOT_SELECTED;
ExtractType     fextract = NOT_SELECTED;
bool            f_async  = false;
char            *tmpdir = NULL;

termpair        *pclpair        = NULL;
termpair        *pcl_red_pair   = NULL;
bool            pcl_red_left    = false;

termpair        *pcl_parent1, *pcl_parent2;
bool            pcl_cp_left,  pcl_cp_eleft;

PclIdCell       pcl_aktid = {0,0,-1};
PclIdCell       pcl_storeid = {0,0,0};

char            pcl_filebase[MAXPATHLEN];/* Werden von pcl_open */
					 /* gesetzt, pcl_filebase */
					 /* enth"alt den aktuellen */
					 /* PCL-Dateinamen ohne */
					 /* Endung, */
char            problem_base[MAXPATHLEN];/* problem_base den Namen der */
					 /* Problemdatei */

/*
//-----------------------------------------------------------------------------
//      Lokale Variablen
//-----------------------------------------------------------------------------
*/

#define     PCL_POSLEN  1000


static FILE     *pclfile = stderr;
static char     position [PCL_POSLEN];


#endif 

/*
//-----------------------------------------------------------------------------
//      Lokale Funktionsdefinition
//-----------------------------------------------------------------------------
*/

     /* Diese Funktion wird wieder vom normalen DISCOUNT gebraucht... */
/*    static void     pcl_fprettyprint ( FILE *stream, 
                                        term *t, vartree **vars, variable *counter );
*/


#ifdef PCL
    static bool     myequal ( term *t1, term *t2 );  /* Nur zum */
						     /* testen... StS */
    static void     tpppair ( termpair *ptr );
    static void     tptype ( termpair *ptr ); /* Neu von StS, gibt */
					      /* Termpaar-Typ in */
					      /* echtem PCL aus */
    static void     pcl_cp    ( termpair *cp );
    static void     pcl_final ( termpair *ptr );

    static void     FindPos ( term *t, term *pos );
    static bool     findpos ( term *t, term *pos );
#endif

/*-------------------------------------------------------------------------
   Funktions-Implementierung: Die erste Funktion
   (pcl_fprettyprint() wird von pcl_fprinttpair() verwendet, diese
   wird auch vom normalen DISCOUNT verwendet (Aufruf in parse.c).
   Diese Funktionen sind deshalb nicht mit #ifdef geklammert! 
-----------------------------------------------------------------------------*/

static char      varstring[] = "Vxyzuvw";   /* Variablen fuer Pretty-Print  */
#define          varstrlen      6           /* Anzahl der Variablen         */


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_fprettyprint ( FILE *stream,
//                                 term *t, vartree **vars, variable *counter )
//
//  Parameter:      stream  Ausgabestream
//                  t       Pointer auf Term
//                  vars    Pointer auf den aktuellen Variablenbaum
//                  counter Zaehler fuer Variablen
//
//  Beschreibung:   Verbesserte Ausgabe eines Terms.
//                  Hilsfunktion fuer prettyterm
//                  Wird auch fuer Ausgabe von Termpaaren exportiert
//                  Ausgabe aus Stream
//-----------------------------------------------------------------------------
*/

static void    pcl_fprettyprint ( FILE *stream, 
                                  term *t, vartree **vars, variable *counter )
{
    short       i = 0;
    variable    v;

    if (varp(t))
    {
       if ((v = VTfind (*vars, t->fcode)) == 0)
          VTadd ( vars, t->fcode, v = ++*counter );

       if (v > varstrlen)
          fprintf ( stream, "x%ld", v-varstrlen );
       else
          fprintf ( stream, "%c", varstring[v] );
    }
    else
    {
        fprintf ( stream, Function[t->fcode].ident );
        if (t->arity > 0)
        {
            fprintf ( stream, "(" ); /* Aenderung StS: "(" statt " ("  */
            while ( i+1 < t->arity )
            {
               pcl_fprettyprint ( stream, t->argument[i++], vars, counter );
               fprintf ( stream, "," );
            }
            pcl_fprettyprint ( stream, t->argument[i], vars, counter );
            fprintf ( stream, ")" );
        }
        else
            fprintf ( stream, "()" );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_fprintpair ( FILE *stream, termpair *pair, char *string )
//
//  Parameter:      stream      Ausgabestream
//                  pair        Termpaar
//                  string      Zeichenkette zwischen linker und rechter
//                              Seite
//
//  Beschreibung:   Termpaar wird auf stream ausgegeben.
//-----------------------------------------------------------------------------
*/

void    pcl_fprinttpair ( FILE *stream, termpair *pair, char *string )
{
    vartree     *vars    = NULL;
    variable    counter  = 0;
     pcl_fprettyprint ( stream, pair->left,  &vars, &counter );
    fprintf      ( stream, "%s", string  );
    pcl_fprettyprint ( stream, pair->right, &vars, &counter );
    VTclear      ( &vars );
}


/*-----------------------------------------------------------------------
Ab hier ist nur noch reiner PCL-Code, der nur bei -D PCL ben"otigt wird. 
-----------------------------------------------------------------------*/


#ifdef PCL

/*-----------------------------------------------------------------------------
                                                                           
Funktion        : static bool myequal ( term *t1, term *t2 )

Autor           : StS (geklaut von equal (term.c) von Werner)

Beschreibung    : Vergleicht zwei Terme, verwendet dabei NICHT die
                  Termgewichte (die sind beim Aufruf in der Regel noch
		  nicht auf dem aktuellen Stand...)

Globale Variable: ?

Seiteneffekte   : -

-----------------------------------------------------------------------------*/

bool   myequal ( term *t1, term *t2 )
{
    short   i;

    if ((t1->fcode != t2->fcode)||(t1->arity != t2->arity))
        return false;

    if (funcp (t1))
    {
        for (i = 0; i < t1->arity; i++ )
            if (!myequal (t1->argument[i], t2->argument[i]))
                return false;
    }

    return true;
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : static void tpppair ( termpair *ptr )

Autor           : StS (geklaut von tppair() von Werner)

Beschreibung    : Gibt Termpaar ohne \n aus.

Globale Variable: ?

Seiteneffekte   : Ausgabe

-----------------------------------------------------------------------------*/

static void     tpppair ( termpair *ptr )
{
    switch (ptr->type)
    {
      case RULE:      pcl_fprinttpair ( pclfile, ptr, " -> " );
                      break;
      case EQUATION:  pcl_fprinttpair ( pclfile, ptr, " = " );
                      break;
      case CRITPAIR:  pcl_fprinttpair ( pclfile, ptr, " = " );
                      break;
      case GOAL:      pcl_fprinttpair ( pclfile, ptr, " = " );
                      break;
      case CRITGOAL:  pcl_fprinttpair ( pclfile, ptr, " = " );
                      break;
      default:        pcl_fprinttpair ( pclfile, ptr, " = " );
                      break;
    }
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : static void     tptype ( termpair *ptr )

Autor           : StS

Beschreibung    : Gibt zu einem Termpaar den pcl-typ in : : geschachtelt
auf pclfile aus, also etwa : tes-eqn :, : tes-rule :.... Ich wei"s,
da"s das nicht sch"on ist, aber die Typen werden hier nur so verwendet

Globale Variable: pclfile 

Seiteneffekte   : Ausgabe 

-----------------------------------------------------------------------------*/

static void tptype ( termpair *ptr )
{
   switch(ptr->type)
   {
      case RULE:      fprintf(pclfile,": tes-rule :");
                      break;
       case EQUATION: 
       case UNKNOWN:  
       case CRITPAIR: /* PCL unterscheidet das nicht mehr... Ich */
		      /* hoffe, mit den UNKNOWN's gibt es keinen */
		      /* "Arger. */
                      fprintf ( pclfile," : tes-eqn : " );
                      break;
       case GOAL:     fprintf ( pclfile," : tes-goal : " );
                      break;
       case CRITGOAL: fprintf ( pclfile," : crit-goal : " );
                      break;
       default:       fprintf(stderr,"ERROR: Illegal type in tptype()...\n");
                      exit(2);
                      break;
   }
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : static void    pcl_cp ( termpair *cp )

Autor           : Werner, Kommentare und "Anderungen StS

Beschreibung    : Gibt initiale Gleichungen (und Ziele) aus

Globale Variable: ?

Seiteneffekte   : Ausgabe, ?

-----------------------------------------------------------------------------*/

static void    pcl_cp ( termpair *cp )
{
    cp->pclid = *NextPclId();
    fPrintPclId(pclfile,&(cp->pclid),true);
    tptype(cp);
    tpppair(cp);
    switch(cp->type)
    {
       case RULE:     
                      fprintf ( pclfile," : initial" );
                      break;
       case EQUATION: 
       case CRITPAIR: /* Nicht sauber, aber initiale kommen zuerst als */
		      /* CP's */
                      fprintf ( pclfile," : initial\n" );
                      break;
       case GOAL:   
                      fprintf ( pclfile," : hypothesis\n" );
                      break;
       case CRITGOAL: 
                      fprintf ( pclfile," : hypothesis\n" );
                      break;
       default:       fprintf(stderr,"ERROR: Illegal initial termpair...\n");
                      exit(2);
                      break;
    }
}


static void     pcl_final ( termpair *ptr )
{
   StorePclId(&(ptr->pclid));
   ptr->pclid = *NextPclId();
   fPrintPclId(pclfile,&(ptr->pclid),true);
   fprintf(pclfile, " : tes-final : ");
   tpppair ( ptr );
   fprintf(pclfile, " : ");
   fPrintPclId(pclfile,RetrievePclId(),false);
   fprintf(pclfile, "\n");
}




static void     FindPos ( term *t, term *pos )
{
    strcpy ( position, "" );
    findpos ( t, pos );
}


static bool     findpos ( term *t, term *pos )
{
    short           i;
    static char     lpos[PCL_POSLEN];

    if (pos == t)
        return true;

    for (i = 0; i < t->arity; i++)
        if (findpos (t->argument[i],pos))
        {
            strcpy  ( lpos, position );
            sprintf ( position, ".%ld", (long)(i+1) );
            strcat  ( position, lpos );
            return true;
        }

    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_open ( char *problempath,char *mode, int cycle, int host )
//
//  Parameter:      problempath: voller Name der Problem-Datei
//                  mode       : "a" oder "w"
//                  cycle,host : Informationen f"ur InitPclId() und
//                               das Zusammenbauen des Dateinamens
//
//  Beschreibung:   Oeffnen der PCL-Datei, Initialisieren der
//  Identifier, setzen von pcl_filebase (der sp"ater f"ur die
//  extract-aufrufe gebraucht wird...
//-----------------------------------------------------------------------------
*/

void pcl_open ( char *problempath,char *mode, int cycle, int host )
{
   char    PCLfile[MAXPATHLEN],help[MAXPATHLEN];

   strcpy ( problem_base, problempath );
   strcpy ( help, problempath );
   if(ParallelMode)
   {
      strcat(help,".%02d.%01d");
      sprintf(pcl_filebase,help,cycle,host);
   }
   else
   {
      strcpy (pcl_filebase, problempath );
   }
   strcpy ( PCLfile, pcl_filebase);
   strcat ( PCLfile, ".pcl" );

   if(mode[0]=='w')
   {
      InitPclIds(cycle,host,-1);
   }

    if (!(pclfile = fopen (PCLfile, mode)))
        Error ( __FILE__ ": "  "pcl_open", "PCL-Datei kann nicht geoeffnet werden." );
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : void pcl_comment(char* txt,bool mark)

Autor           : StS

Beschreibung    : Schreibt den "ubergebenen Text als Kommentar ins
PCL-File. Ist mark gesetzt, so wird es durch auff"allige Zeilen eingerahmt.

Globale Variable: pclfile

Seiteneffekte   : -

-----------------------------------------------------------------------------*/

void pcl_comment(char* txt,bool mark)
{
   if(mark)
   {
      fprintf(pclfile,
      "#=======================================================================\n");
   }
   fprintf(pclfile,"# %s\n",txt);
   if(mark)
   {
      fprintf(pclfile,
      "#=======================================================================\n");
   }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_close
//
//  Parameter:      -keine-
//
//  Beschreibung:   Schliessen der PCL-Datei
//-----------------------------------------------------------------------------
*/

void    pcl_close ( void )
{
   fclose ( pclfile );
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : void pcl_extract(bool ismaster)

Autor           : StS

Beschreibung    : F"uhrt je nach Zustand von extract die Extraktion
der .pcl-Dateien durch System-Aufrufe durch.

Globale Variable: extract, pcl_filebase

Seiteneffekte   : Eigentlich keine...im Prozess zumindest

-----------------------------------------------------------------------------*/

void pcl_extract(bool ismaster)
{
   char command[3*MAXPATHLEN];

   if(ismaster)
   {
      if(extract != NO_EXTRACT)
      {
	 sprintf(command,"mv %s.pcl %s.imd",pcl_filebase,pcl_filebase);
         SYSTEM(command);
      }
   }
   else
   {
      switch(extract)
      {
      case NO_EXTRACT: 
	                 break;
      case MEXTRACT:     sprintf(command,"mextract -i -m 1 %s.pcl -o %s.imd",
                                 pcl_filebase,pcl_filebase);
		         SYSTEM(command);
			 sprintf(command,"rm %s.pcl",pcl_filebase);
			 SYSTEM(command);
		         break;
      case REV_REXTRACT: sprintf(command,"revert %s.pcl|rextract -i -o %s.imd",
                                 pcl_filebase,pcl_filebase);
		         SYSTEM(command);
		         sprintf(command,"rm %s.pcl",pcl_filebase);
			 SYSTEM(command);
			 break;
      case TAC_REXTRACT: if(tmpdir)
                         {
			    sprintf(command,
				    "TMPDIR=%s;tac %s.pcl|rextract -i -o %s.imd",
				    tmpdir,pcl_filebase,pcl_filebase);
			 }
                         else
			 {
			    sprintf(command,
				    "tac %s.pcl|rextract -i -o %s.imd",
				    pcl_filebase,pcl_filebase);
			 }
		         SYSTEM(command);
		         sprintf(command,"rm %s.pcl",pcl_filebase);
			 SYSTEM(command);
		         break;
      default:           Error ( __FILE__ ": "  "pcl_extract", 
                                "Falscher Wert in extract" ); 
                         break;
      }
   }
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : void    pcl_fextract();

Autor           : StS

Beschreibung    : F"uhrt je nach Wert der Variablen fextract die
Gesamtextraction aller .pcl beziehungsweise  .imd -Dateien durch.

Globale Variable: fextract, extract, problem_base

Seiteneffekte   : Tja... Hauptspeicher voll, Systemabsturz... Au"ser
der Manipulation der Dateien ist keiner geplant...

-----------------------------------------------------------------------------*/

void    pcl_fextract()
{
   char command[3*MAXPATHLEN];
   char filename[MAXPATHLEN];
   char wildcards[]=".*";
   char bg;

   bg = f_async ? '&' :' ';

   if(!ParallelMode)
   {
      strcpy(wildcards,"");
   }

   if(extract == NO_EXTRACT)
   {
      sprintf(filename,"%s%s.pcl",problem_base,wildcards);
   }
   else
   {
      sprintf(filename,"%s%s.imd",problem_base,wildcards);
   }
   switch(fextract)
   {
   case NO_EXTRACT: 
	              break;
   case MEXTRACT:     sprintf(command,"(mextract -m 1 %s -o %s.xtr;rm %s)%c",
                              filename,problem_base,filename,bg);
		      SYSTEM(command);
		      break;
   case REV_REXTRACT: sprintf(command,"(revert %s|rextract -o %s.xtr;rm %s)%c",
                              filename,problem_base,filename,bg);
                      SYSTEM(command);
		      break;
   case TAC_REXTRACT: if(tmpdir)
                      {
			 sprintf(command,
				 "(TMPDIR=%s;cat %s|tac|rextract -o %s.xtr;rm %s)%c",
				 tmpdir,filename,problem_base,filename,bg);
		      }
		      else
		      {
			 sprintf(command,"(cat %s|tac|rextract -o %s.xtr;rm %s)%c",
				 filename,problem_base,filename,bg);
		      }
           	      SYSTEM(command);
		      break;
   default:           Error ( __FILE__ ": "  "pcl_extract", 
                             "Falscher Wert in extract" ); 
                      break;
   }
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : void    pcl_clean   ();

Autor           : StS

Beschreibung    : L"oscht das aktuelle PCL-File, genauer gesagt
erzeugt ein neues, das nur einen Kommentar enth"alt...damit mit quit
abgeschossenen Prozesse keine PCL-Dateinen mit unvollst"andigen
Schritten erzeugen k"onnen (f"uhrt zu Syntax-Fehlern). Nur, falls
fextract != NO_EXTRACT, sonst soll sich der User selbst darum
k"ummern...wird auch nur bei ndef REPRO aufgerufen, sonst ist die
Datei sowieso leer.

Globale Variable: fextract, pcl_filebase, pclfile

Seiteneffekte   : Datei wir ver"andert, ebenso pcl_filebase (das in
diesem Fall nicht mehr verwendet werden darf...und wohl auch nicht
wird, da der Prozess terminiert)

-----------------------------------------------------------------------------*/

void    pcl_clean()
{
   char PCLfile[MAXPATHLEN];

   if(fextract != NO_EXTRACT)
   {
      strcpy(PCLfile,pcl_filebase);
      strcat(PCLfile,".pcl");
      if (!(pclfile = fopen (PCLfile, "w")))
	 Error ( __FILE__ ": "  "pcl_clean", "PCL-Datei kann nicht geoeffnet werden." );
      PCL_COMMENT("Datei geloescht von pcl_clean()",true);
      fclose(pclfile);
   }
}
   





/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_init ()
//
//  Parameter:      filename    Dateiname der Ausgabe
//
//  Beschreibung:   Initialisiere pcl
//-----------------------------------------------------------------------------
*/

void    pcl_init ()
{
    ForAllCPsDo   ( pcl_cp );
    ForAllGoalsDo ( pcl_cp );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_exit
//
//  Parameter:      proof   true   Beweismodus
//                          false  Vervollstaendigung 
//
//  Beschreibung:   Beende pcl
//-----------------------------------------------------------------------------
*/

void    pcl_exit ( bool proof )
{
    if (!proof)
    {
        ForAllRulesDo ( pcl_final );
        ForAllEquDo   ( pcl_final );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_subsum ( termpair *equ, termpair *pair )
//
//  Parameter:      equ     ueberdeckende Gleichung
//                  pair    Termpaar
//                  pos     Stelle im Term  (noch zu Implementieren !)
//
//  Beschreibung:   equ subsumiert pair
//-----------------------------------------------------------------------------
*/

void    pcl_subsum ( termpair *equ, termpair *pair )
{
   StorePclId(&(pair->pclid));
   pair->pclid = *NextPclId();
   fPrintPclId(pclfile,&(pair->pclid),true);
   if((pair->type == GOAL)||(pair->type == CRITGOAL)) 
   {
      fprintf(pclfile," : tes-final : ");  /* At least in this version this is */
				       /* always true...I hope....*/
   }
   else
   {
      tptype(pair);
   }
   tpppair ( pair );
   fprintf(pclfile," : instance(");
   fPrintPclId(pclfile,RetrievePclId(),false);
   fprintf(pclfile,",");
   fPrintPclId(pclfile,&(equ->pclid),false);
   fprintf(pclfile,")\n");
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_rreduce ( termpair *rule, term *pos )
//
//  Parameter:      rule    reduzierende Regel
//                  pos     Position an der reduziert wurde
//
//  Globale Werte:  pcl_red_pair    reduziertes Termpaar
//                  pcl_red_left    Seite der Reduktion
//
//  Beschreibung:   Reduktion eines Termpaares mit einer Regel
//-----------------------------------------------------------------------------
*/

void    pcl_rreduce ( termpair *rule, term *pos )
{
   if (pcl_red_left) 
      FindPos ( pcl_red_pair->left,  pos );
   else  
      FindPos ( pcl_red_pair->right, pos );

   StorePclId(&(pcl_red_pair->pclid));
   pcl_red_pair->pclid = *NextPclId();
   fPrintPclId(pclfile,&(pcl_red_pair->pclid),true);

   if(((pcl_red_pair->type == GOAL)||  /* ziemlicher Hack...aber in */
				       /* TRANS lief es bisher */
				       /* genauso. */
       (pcl_red_pair->type == CRITGOAL)) && 
        myequal(pcl_red_pair->left,pcl_red_pair->right))
   {
      fprintf(pclfile," : tes-final : ");
   }
   else
   {
      tptype(pcl_red_pair);
   }

   tpppair(pcl_red_pair);
   fprintf ( pclfile, " : tes-red(");
   fPrintPclId(pclfile,RetrievePclId(),false);
   fprintf ( pclfile, ",%c%s,",(pcl_red_left) ? 'L' : 'R',position);
   fPrintPclId(pclfile,&(rule->pclid),false);
   fprintf ( pclfile, ",L)\n"); 
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_ereduce ( termpair *equ, term *pos, bool side )
//
//  Parameter:      equ     reduzierende Gleichung
//                  pos     Position an der reduziert wurde
//                  side    true    Gl. wird von links nach rechts verwendet
//                          false   Gl. wird von rechts nach links verwendet
//
//  Globale Werte:  pcl_red_pair    reduziertes Termpaar
//                  pcl_red_left    Seite der Reduktion
//
//  Beschreibung:   Reduktion eines Termpaares mit einer Gleichung
//-----------------------------------------------------------------------------
*/

void    pcl_ereduce ( termpair *equ, term *pos, bool side )
{
    if (pcl_red_left)  FindPos ( pcl_red_pair->left,  pos );
                 else  FindPos ( pcl_red_pair->right, pos );

   StorePclId(&(pcl_red_pair->pclid));
   pcl_red_pair->pclid = *NextPclId();
   fPrintPclId(pclfile,&(pcl_red_pair->pclid),true);

   if(((pcl_red_pair->type == GOAL)||  /* ziemlicher Hack...aber in */
				       /* TRANS lief es bisher */
				       /* genauso. */
       (pcl_red_pair->type == CRITGOAL)) && 
       myequal(pcl_red_pair->left,pcl_red_pair->right))
   {
      fprintf(pclfile," : tes-final : ");
   }
   else
   {
      tptype(pcl_red_pair);
   }

   tpppair(pcl_red_pair);
   fprintf ( pclfile, " : tes-red(");
   fPrintPclId(pclfile,RetrievePclId(),false);
   fprintf ( pclfile, ",%c%s,",(pcl_red_left) ? 'L' : 'R',position);
   fPrintPclId(pclfile,&(equ->pclid),false);
   fprintf ( pclfile, ",%c)\n",(side) ? 'L' : 'R'); 
}



/*-----------------------------------------------------------------------------
                                                                           
Funktion        : void pcl_orient(termpair *pair, char dir)

Autor           : StS

Beschreibung    : Ausgabe der Orientierung einer Gleichung zu einer
Regel, Vergabe der neuen Identifier.

Globale Variable: -

Seiteneffekte   : Neue Ids

-----------------------------------------------------------------------------*/

void pcl_orient(termpair *pair, char dir)
{
   StorePclId(&(pair->pclid));
   pair->pclid = *NextPclId();
   fPrintPclId(pclfile,&(pair->pclid),true);
   fprintf(pclfile," : tes-rule : ");
   pcl_fprinttpair ( pclfile, pair, " -> " ); /* Nicht ganz sauber, */
					      /* eigentlich wird der */
					      /* Typ erst in NewRule() */
					      /* umgesetzt */
   fprintf(pclfile," : orient(");
   fPrintPclId(pclfile,RetrievePclId(),false);
   fprintf(pclfile,",%c)\n",dir);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       pcl_newcp  ( termpair *cp, term *t, term *pos )
//
//  Parameter:      cp      neues kritisches Paar
//                  t, pos  pos in t spez. die Position an der das
//                          kritische Paar gebildet wird
//                  sigma   Unifikator
//
//  Autor:          Werner, aber vollst"andig neu StS
//
//  Beschreibung:   Melde ein neues kritisches Paar.
//-----------------------------------------------------------------------------
*/

void    pcl_newcp ( termpair *cp, term *t, term *pos, subst *sigma )
{
    FindPos ( t,  pos );

#ifndef PCL_ALL_CPS  /* Spare Ausgabe trivialer CP's, die sowieso */
		     /* gleich gel"oscht werden */
    if(equal(cp->left,cp->right))
    {
       return;
    }
#endif
    cp->pclid = *NextPclId();
    fPrintPclId(pclfile,&(cp->pclid),true);
    tptype(cp);
    tpppair ( cp );
    fprintf ( pclfile, " : cp(");
    fPrintPclId(pclfile,&(pcl_parent1->pclid),false);
    fprintf ( pclfile, ",%c%s,",(pcl_cp_eleft) ? 'L' : 'R', position);
    fPrintPclId(pclfile,&(pcl_parent2->pclid),false);
    fprintf ( pclfile, ",%c)\n",(pcl_cp_left) ? 'L' : 'R');
}

/*-----------------------------------------------------------------------------
                                                                           
Funktion        : void    pcl_intermed( termpair *cp )

Autor           : StS

Beschreibung : Zitiert ein Termpaar als tes-intermed beziehungsweise
tes-intermedgoal oder crit-subgoal und gibt ihm eine neue Nummer...mit
dieser Funktion werden die weitergemeldeten Zwischenergebnisse
ausgegeben.


Der frueher notwendige zus"atzliche Parameter doit entfaellt, da die
Aufrufe aus SendReport() verlagert wurden.

Globale Variable: -

Seiteneffekte   : Identifier in *cp neu, NextPclId() wird gerufen, Ausgabe

-----------------------------------------------------------------------------*/

void    pcl_intermed( termpair *cp )
{

   StorePclId(&(cp->pclid));
   cp->pclid = *NextPclId();
   fPrintPclId(pclfile,&(cp->pclid),true);
   if(cp->type == GOAL)
   {
      fprintf(pclfile," : tes-intermedgoal : ");
   }
   else if(cp->type == CRITGOAL)
   {
      fprintf(pclfile," : crit-intermedgoal : ");
   }
   else
   {
      fprintf(pclfile," : tes-intermed : ");
   }
   tpppair ( cp );
   fprintf(pclfile," : ");
   fPrintPclId(pclfile,RetrievePclId(),false);
   fprintf(pclfile,"\n");
}



/*---------------------------------------------------------------------------
      Neue Funktionen von StS zur Verwaltung von ECHTEN PCL-Id's
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------

 Funktion        : void    InitPclIds(int cycle,int host,int count)

 Autor           : StS

 Beschreibung    : Setzt die Z"ahler f"ur die internen Variablen auf
 die angegebenen Werte. In der Regel sollte die Funktion immer dann
 aufgerufen werden, wenn ein neues PCL-File ge"offnet wird und dabei
 als Parameter CycleCount, ThisHost und -1 erhalten.

 Globale Variable: pcl_aktid

 Seiteneffekte   : Setzt pcl_aktid

-----------------------------------------------------------------------------*/


void    InitPclIds(int cycle,int host,int count)
{
   pcl_aktid.cycle = cycle;
   pcl_aktid.host  = host;
   pcl_aktid.count = count;
}



/*-----------------------------------------------------------------------------

 Funktion        : void    StorePclId(PclId_p id)

 Autor           : StS

 Beschreibung    : Speichert EINEN PCl-Identifer intern in einer
 Kopie. RetrievePclId() gibt den Pointer auf diese Kopie.

 Globale Variable: pcl_storeid

 Seiteneffekte   : pcl_storeid wird ver"andert

-----------------------------------------------------------------------------*/

void    StorePclId(PclId_p id)
{
   pcl_storeid = *id;
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : PclId_p RetrievePclId()

Autor           : StS

Beschreibung    : Gibt einen Pointer auf den mit StorePclId()
gespeicherten Identifer zur"uck. Es wird KEINE Kopie angelegt, der
Wert dieses Identifiers wird also durch einen erneuten Aufruf von
StorePclId(PclId_p id) ver"andert. F"ur eine l"angerfristige
Speicherung muss anderweitig gesorgt werden.

Globale Variable: pcl_storeid

Seiteneffekte   : -

-----------------------------------------------------------------------------*/

PclId_p RetrievePclId()
{
   return &pcl_storeid;
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : PclId_p NextPclId()

Autor           : StS

Beschreibung    : Berechnet den n"achsten eindeutigen, gr"o"seren
PCL-Identifier und gibt einen Pointer auf diesen zur"uck. Es handelt
sich dabei nur um einen Pointer auf eine globale Variable, f"ur eine
dauerhafte Verwendung muss der Inhalt dieser Variable noch kopiert
werden. 

Globale Variable: pcl_aktid

Seiteneffekte   : pcl_aktid wird ver"andert.

-----------------------------------------------------------------------------*/

PclId_p NextPclId()
{
   pcl_aktid.count++;
   return &pcl_aktid;
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : PclId_p CurrentPclId(

Autor           : StS

Beschreibung    : Gibt einen Pointer auf die Variable zur"uck, die den
aktuellen PCL-Identifier beinhalted. Restruktionen wie oben (nur
Pointer). 

Globale Variable: pcl_aktid

Seiteneffekte   : -

-----------------------------------------------------------------------------*/

PclId_p CurrentPclId()
{
   return &pcl_aktid;
}


/*-----------------------------------------------------------------------------
                                                                           
Funktion        : void fPrintPclId(FILE *out, PclId_p id,bool lineid)

Autor           : StS

Beschreibung    : Gibt PCL-Identifier auf *out aus. Ist lineid = true,
so wird die erste Zahl einger"uckt. Ist ParallelMode = false, so
werden Zyklus und Host zur besseren "Ubersicht unterdr"uckt.

Globale Variable: -

Seiteneffekte   : Ausgabe

-----------------------------------------------------------------------------*/

void fPrintPclId(FILE *out, PclId_p id,bool lineid)
{
   if(ParallelMode)
   {
      if(lineid)
      {
         fprintf(out,"%2d.%01d.%05d",id->cycle,id->host,id->count);
      }
      else
      {
         fprintf(out,"%d.%01d.%05d",id->cycle,id->host,id->count);
      }
   }
   else
   {
      if(lineid)
      {
         fprintf(out,"%5d",id->count);
      }
      else
      {
         fprintf(out,"%d",id->count);
      }
   }
}


#endif /* PCL  */

/*-------------------------------------------------------------------
          Ende der Datei
-------------------------------------------------------------------*/
