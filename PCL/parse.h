

/*************************************************************************/
/*                                                                       */
/*   File:        parse.h                                                */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Parser fuer tc-pcl                                     */
/*                                                                       */
/*   Aenderungen: <1> 11.2.1991  neu                                     */
/*                <2> 25.2.1991  NextRealToken() exportieren             */
/*                                                                       */
/*************************************************************************/

#include "nameadmin.h"

typedef enum
{
   I,
   CP,
   R,
   E,
   U,
   G,
   F
} tc_IdType;

typedef enum
{
   s_noop,
   s_reduce,
   s_buildcp,
   s_delete,
   s_clear,
   s_subsum,
   s_initial,
   s_assign,
   s_swap
} tc_OpType;




typedef struct tc_stepdata
{
   tc_IdType tc_id_type;
   long      tc_id_numval;
   tc_OpType tc_operation;

   tc_IdType arg1_type;
   long      arg1_numval;
   char*     place1;

   tc_IdType arg2_type;
   long      arg2_numval;
   char*     place2;

   char*     res_lside;
   char*     res_rside;
   TokenType eq_or_r;

   char*     comment;

   StepType  pcl_type;
   long      pcl_numval;
} tc_StepData, *tc_Step_p;




/*----------------------------------------------------------------------------*/
/*                 Forward-Deklarationen exportierter Funktionen              */
/*----------------------------------------------------------------------------*/

extern tc_StepData aktstep;
extern char* tc_IdPrint[];

void Parse_tc_Step();
void NextRealToken();
void Print_tc_Step();


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


