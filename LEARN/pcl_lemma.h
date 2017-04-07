

/*************************************************************************/
/*                                                                       */
/*   File:        pcl_lemma.h                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen fuer Lemma-Funktionen                    */
/*                                                                       */
/*   Aenderungen: <1> 18.10.1991 neu                                     */
/*                                                                       */
/*************************************************************************/

#include "pcl_buildtree.h"
#include "pcl_doio.h"

#ifndef _pcl_lemma

#define _pcl_lemma

/* Die hier definierten Werte werden sowohl in pcl_lemma.c als
   auch in lemma.c zur Initialisierung verwendet                */

#define S_AVERAGE_SIZE (long)1
#define S_MAX_SIZE (long)2
#define S_MIN_FAK 5.0
#define S_MAX_FAK 0.0
#define I_LEMMA_WEIGHT (long)11
#define O_MIN_USED (long)4
#define T_WEIGHT_FACTOR 0.5
#define T_OFFSET (long)2
#define C_INIT_WEIGHT (long)1
#define C_HYPO_WEIGHT (long)0
#define C_ORIENT_WEIGHT (long)0
#define C_CP_WEIGHT (long)3
#define C_REDU_WEIGHT (long)2
#define C_INST_WEIGHT (long)0
#define C_QUOT_WEIGHT (long)0
#define C_LEMMA_WEIGHT (long)15
#define P_MAX_LENGTH (long)10
#define U_MIN_LENGTH (long)2
#define U_MIN_USED (long)1

/* Die folgenden Defines sind die defaults fuer alle Funktionen, die 
   FindLemmas() ohne zusaetzliche Konfiguration aufrufen wollen      */

#define ITERATE FALSE
#define CRITERIA "iotp"  /* Maximal: "sotcpi"  */

/*----------------------------------------------------------------------------*/
/*                        Deklaration exportierter Funktionen                 */
/*----------------------------------------------------------------------------*/

void SetSyntacticParams(long average_size, long max_size,          
                        double min_fak, double max_fak);

void SetOftenParams(long min_used);

void SetImportantParams(long lemma_weight);

void SetTreeParams(double weight_factor,long offset);

void SetCompletionParams(long init_weight,   long hypo_weight,
                         long orient_weight, long cp_weight,
                         long redu_weight,   long inst_weight,
                         long quot_weight,   long lemma_weight);

void SetPartitionParams(long max_length);

void SetNegativeParams(long min_length, long min_used); 


long          JustWeight(Just_p just);

long          ChainLen(Just_p just);

NumListList_p GetExits(Just_p just,Step_p step);

NumListList_p GetUsed(Just_p just);

long          ValueExits(NumListList_p exits);

BOOL          IsLemma(Step_p step,char* criteria, long criterium);

void          ForgetLemmas(Step_p anchor);

void          FindLemmas(Step_p anchor,char* criteria,BOOL iterate);

#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/









