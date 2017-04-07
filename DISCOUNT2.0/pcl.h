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
//      $Log: pcl.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/


/*-----------------------------------------------------------------------------

Kommentare von StS:

Die hier definierten Funktionen werden zum Teil (pcl_fprinttpair() und
von dieser gerufenen Funktionen) auch in dem normalen DISCOUNT-System
verwendet, deshalb habe ich nicht den kompletten Code mit #ifdef PCL
f"ur die bedingte Compilation auskommentiert. Ich habe aber versucht,
den kompletten Code, der nur f"ur PCL ben"otigt wird, soweit m"oglich
vom Rest  zu trennen. Ich habe auch die #ifdef ANSI-Direktiven soweit
als m"oglich entfernt. Da das System wohl nur noch auf ANSI-Compilern
"ubersetzt wird, machen sie die #ifdef-Struktur nur unn"otig
kompliziert... 

Einige zus"atzliche Kommentare zu den "Anderungen:

- Es gibt eine neue M"oglichkeit zur bedingten Compilation: Ist bei
  der "Ubersetzung das Macro PCL_ALL_CPS definiert, so werden alle
  kritischen Paare im PCL-Protokoll gemeldet. Ist das nicht der Fall,
  so werden sofort triviale Kritische Paare in der Ausgabe
  unterdr"uckt. 
- Ich habe mich bem"uht, die vorhanden Funktion zu vereinfachen, dabei
  die Auffrufformate aber beizubehelten. Insbesondere hat sich sehr
  wenig in anderen Modulen ge"andert, lediglich PCL_SWAP wurde durch
  PCL_ORIENT ersetzt und PCL_INSERT konnte entfallen. 
- Dieses Modul ist nur sehr lose an den Rest der Vervollst"andigung
  gekoppelt, macht aber viele Annahmen "uber ihren Ablauf.
  Insbesondere werden Identifier eigentlich zu sp"at vergeben, die
  Orientierung eines Termpaares aber schon beim Vergleich und damit
  vor der Aufnahme in die Regelmenge. Dies kann m"oglicherweise bei
  der parallelen Vervollst"andigung mit EquationsOnly zu unerwarteten
  Ergebnissen f"uhren, ist aber notwendig, da PCL keine andere
  M"oglichkeit kennt, das Vertauschen der Seiten eines Termpaares
  darzustellen. Auch wird erwartet, das nur Ziele subsummiert werden
  und das subsummierte und auf triviale Gleichungen reduzierte Ziele
  automatisch tes-finals sind. Diese Konventionen wurden auch bei
  TRANS vorausgesetzt.
- Eine weiterer Compile-Time-Switch: PCL_ECHO_SYSTEM sorgt daf"ur, das
  alle SYTEM-Aufrufe zu Kontrollzwecken ausgegeben wird. Im Augenblick
  wird er direkt vor der Definition von SYSTEM definiert.

-----------------------------------------------------------------------------*/

#ifndef     __PCL
#define     __PCL


#include    "subst.h"
#include    "complet.h"


/*-----------------------------------------------------------------------------
           Typen
-----------------------------------------------------------------------------*/

#ifdef PCL

typedef enum extracttype
{
   NOT_SELECTED,
   NO_EXTRACT,
   MEXTRACT,
   REV_REXTRACT,
   TAC_REXTRACT
}ExtractType;

#endif

/*
//-----------------------------------------------------------------------------
//      Sichtbare Variablen
//-----------------------------------------------------------------------------
*/

#ifdef PCL

extern ExtractType extract;
extern ExtractType fextract;

extern bool        f_async; /* Gibt an, ob die letzte Extraktion */
                            /* asynchron durchgef"uhrt werden */
                	    /* soll... */
extern char *tmpdir;

extern termpair *pclpair;

extern termpair *pcl_red_pair;
extern bool     pcl_red_left;

extern termpair *pcl_parent1, *pcl_parent2;
extern bool     pcl_cp_left,  pcl_cp_eleft;

#endif

/*
//-----------------------------------------------------------------------------
//      Makros
//-----------------------------------------------------------------------------
*/

#ifdef PCL

#define PCL_ECHO_SYSTEM 

#ifdef PCL_ECHO_SYSTEM 

#define SYSTEM(str)                                        \
{                                                          \
   int res;                                                \
                                                           \
   printf("## system(%s)\n",str);                          \
   flush();                                                \
   if((res = system(str)))                                 \
   {                                                       \
      printf("Warning: system(%s) returned %d - this may be a problem...\n\
Typical is an out of memory error from the extraction program.\n" \
	     ,str,res);                                    \
      flush();                                             \
   }                                                       \
}

#else 

#define SYSTEM(str)                                        \
{                                                          \
   int res;                                                \
                                                           \
   if((res = system(str)))                                 \
   {                                                       \
      printf("Warning: system(%s) returned %d - this may be a problem...\n\
Typical is an out of memory error from the extraction program.\n" \
	     ,str,res);                                    \
      flush();                                             \
   }                                                       \
}
#endif



    #define PCL_INIT()                  pcl_init ()
    #define PCL_EXIT(mode)              pcl_exit (mode)
    #define PCL_OPEN(problempath,mode,cycle,host)\
                                        pcl_open(problempath,mode,cycle,host)
    #define PCL_COMMENT(txt,mark)       pcl_comment(txt,mark)
    #define PCL_CLOSE()                 pcl_close()
    #define PCL_EXTRACT(ismaster)       pcl_extract(ismaster)
    #define PCL_FEXTRACT()              pcl_fextract()
    #define PCL_CLEAN()                 pcl_clean()

    #define PCL_ORIENT(pair,dir)        pcl_orient (pair,dir)
    #define PCL_SUBSUM(equ,pair)        pcl_subsum (equ, pair)

    #define PCL_REDUCE_LEFT(pair)       pcl_red_pair = pair;        \
                                        pcl_red_left = true
    #define PCL_REDUCE_RIGHT(pair)      pcl_red_pair = pair;        \
                                        pcl_red_left = false

    #define PCL_R_REDUCE(rule,pos)      pcl_rreduce (rule, pos)
    #define PCL_E_REDUCE(equ,pos,side)  pcl_ereduce (equ, pos, side)

    #define PCL_SAVE(pair)              StorePclId(&(pair->pclid))

    #define PCL_INIT_CP(ptr1,ptr2,flag) pcl_parent1  = ptr1;        \
                                        pcl_parent2  = ptr2;        \
                                        pcl_cp_left  = flag;        \
                                        pcl_cp_eleft = true
    #define PCL_INIT_ECP(flag)          pcl_cp_eleft = flag
    #define PCL_NEWCP(cp,t,pos,sigma)   pcl_newcp (cp,t,pos,sigma)
    #define PCL_INTERMED(cp)            pcl_intermed(cp)
#else


    #define PCL_INIT()
    #define PCL_EXIT(mode)
    #define PCL_OPEN(problempath,mode,cycle,host)
    #define PCL_COMMENT(txt,mark)   
    #define PCL_CLOSE()
    #define PCL_EXTRACT(ismaster)   
    #define PCL_FEXTRACT()
    #define PCL_CLEAN()

    #define PCL_ORIENT(pair,dir)  
    #define PCL_SUBSUM(equ,pair)

    #define PCL_REDUCE_LEFT(pair)
    #define PCL_REDUCE_RIGHT(pair)

    #define PCL_R_REDUCE(rule,pos)
    #define PCL_E_REDUCE(equ,pos,side)

    #define PCL_SAVE(pair)

    #define PCL_INIT_CP(ptr1,ptr2,flag)
    #define PCL_INIT_ECP(flag)
    #define PCL_NEWCP(cp,t,pos,sigma)
    #define PCL_INTERMED(cp,doit)

#endif


/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/


   /* Diese Funktion wird auch im normalen DISCOUNT gebraucht ! */

void    pcl_fprinttpair ( FILE *stream, termpair *pair, char *string );

#ifdef PCL

   void    pcl_open    ( char *filename,char* mode, int cycle,int host );
   void    pcl_comment (char* txt,bool mark);
   void    pcl_close   ( void );
   void    pcl_extract ( bool ismaster);
   void    pcl_fextract();
   void    pcl_clean   ();
   void    pcl_init    ();
   void    pcl_exit    ( bool proof );

   void    pcl_swap    ( termpair *pair );
   void    pcl_subsum  ( termpair *equ, termpair *pair );
   void    pcl_clear   ( termpair *pair );
   void    pcl_delete  ( termpair *pair );
   void    pcl_rreduce ( termpair *rule, term *pos );
   void    pcl_ereduce ( termpair *equ,  term *pos, bool side );
   void    pcl_orient(termpair *pair, char dir);
   void    pcl_newcp   ( termpair *cp, term *t, term *pos, subst *sigma );
   void    pcl_intermed( termpair *cp );

/* Was folgt sind Funktionen zu echten PCL-Identifiern, von StS */

   void    InitPclIds(int cycle,int host,int count);
   void    StorePclId(PclId_p id);
   PclId_p RetrievePclId();
   PclId_p NextPclId();
   PclId_p CurrentPclId();
   void    fPrintPclId(FILE *out, PclId_p id,bool lineid);

#endif


#endif   /* __PCL */

/*--------------------------------------------------------------
     Ende der Datei
--------------------------------------------------------------*/






