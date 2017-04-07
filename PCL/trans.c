
/*************************************************************************/
/*                                                                       */
/*   File:        trans.c                                                */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Uebersetzer fuer tc-Ausgaben in pcl                    */
/*                                                                       */
/*   Aenderungen: <1> 9.2.1991  neu                                      */
/*                                                                       */
/*************************************************************************/

#include "pcl_defs.h"
#include "parse.h"


#define CPRINT {if(printcomment) fprintf(out,"            %s %s\n",aktstep.comment,ViewString(&newcomment));}



/*----------------------------------------------------------------------------*/
/*               Forward-Deklarationen interner Funktionen                    */
/*----------------------------------------------------------------------------*/

void main(int argc,char* argv[]);
void PrintInfo();
void Translate();
Id_p GetAnchor(tc_IdType type);
long GetPclNum(tc_IdType type,long numval);
void PrintPclStep(Id_p des,char* lside,char* rside, char* just);


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/



FILE *out = stdout,
     *in = stdin;

BOOL WriteToFile = FALSE,
     ReadFromFile = FALSE,
     printcomment = FALSE,
     Verbose = FALSE,
     Help = FALSE;

long pcl_idnum = 0;

IdCell CP_anchor = {-1,FALSE,NoPair,-1,&CP_anchor,&CP_anchor},
       R_anchor  = {-1,FALSE,NoPair,-1,&R_anchor,&R_anchor},
       E_anchor  = {-1,FALSE,NoPair,-1,&E_anchor,&E_anchor},
       G_anchor  = {-1,FALSE,NoPair,-1,&G_anchor,&G_anchor};

tc_IdType lasttc_type = U;
long      lasttc_num  = -1;

StringCell newcomment = {NULL,0,0},
           juststring = {NULL,0,0};


/*----------------------------------------------------------------------------*/
/*                       Funktionen                                           */
/*----------------------------------------------------------------------------*/



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void main(int argc,char* argv[])                        */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                                                                            */
/* Beschreibung     : Uebernimmt die IO-Zuordnung und ruft die eigentliche    */
/*                    Funktion Translate mit den passenden Filepointern auf.  */
/*                                                                            */
/* Globale Variable : out, in, WriteToFile, ReadFromFile                      */
/*                                                                            */
/* Seiteneffekte    : Oeffnen und Schliessen der Files, Abwicklung des        */
/*                    Programmlaufs.                                          */
/*                                                                            */
/* Aenderungen      : <1> 14.2.1991  neu                                      */
/*                                                                            */
/******************************************************************************/


void main(int argc,char* argv[])
{
   int  f;
   BOOL ReadFromFile = FALSE;

   for(f = 1; f<argc; f++)
   {    /* Teste, ob -o Option gesetzt und korrekt, teste, ob illegale Optionen */
      if(strcmp(argv[f],"-o")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -o expects filename... \n");
            exit(OPTION_ERROR);
         }
         else if(WriteToFile)
         {
            fprintf(stderr,"ERROR: No more than one -o Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            out = fopen(argv[++f],"w");
            if(!out)
            {
               fprintf(stderr,"ERROR: unable to open file %s...\n",argv[f]);
               exit(IO_ERROR);
            }
            WriteToFile = TRUE;
         }
      }
      else if(strcmp(argv[f],"-v")==0)
      {
         Verbose = TRUE;
      }
      else if(strcmp(argv[f],"-h")==0)
      {
         Help = TRUE;
      }
      else if(strcmp(argv[f],"-c")==0)
      {
         printcomment = TRUE;
      }
      else if(*(argv[f])=='-')
      {
         fprintf(stderr,"ERROR: unknown option %s...\n",argv[f]);
         exit(OPTION_ERROR);
      }
   }
   if(Help)
   {
      PrintInfo();
      exit(NO_ERROR);
   }
   if(Verbose)
   {
      PrintInfo();
   }
   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      if (strcmp(argv[f],"-o")==0)
      {
         f++;
      }
      else if(*(argv[f])=='-')
      {   /* Optionen ueberspringen  */
      }
      else
      {
         in = fopen(argv[f],"r");
         if(!in)
         {
            fprintf(stderr,"ERROR: unable to open file %s...\n",argv[f]);
            exit(IO_ERROR);
         }
         ReadFromFile = TRUE;

         InitScanner(in,argv[f]);
         
         Translate();

         fclose(in);
      }
   }

   if(!ReadFromFile)
   {   /* lese Standard-Eingabe  */

      InitScanner(in,"stdin");

      Translate();
   }

   if(WriteToFile)
   {   /* schliese Ausgabefile  */ 
      fclose(out);
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintInfo()                                        */
/*                                                                            */
/* Beschreibung     : Gibt Informationen ueber das Programm aus.              */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 1.3.1991 neu                                        */
/*                                                                            */
/******************************************************************************/

void PrintInfo()
{
   fprintf(stderr,"\n\n   trans 1.1  vom 16.2.1992");
   fprintf(stderr,"\n\n   Usage: trans [-v] [-h] [-c] [- o outfile] [infile1 ... infileN]");
   fprintf(stderr,"\n\n   Geschrieben von Stephan Schulz");
   fprintf(stderr,"\n\n   trans uebersetzt die von DISCOUNT erzeugten Protokolle");
   fprintf(stderr,"\n   in echtes PCL-Format. Wird kein Eingabefile angegeben,");
   fprintf(stderr,"\n   so wird die Standard-Eingabe gelesen, mehrere Eingabe-");

   fprintf(stderr,"\n   files werden wie eine zusammenhaengende Datei behandelt.");
   fprintf(stderr,"\n\n    OPTIONEN:");
   fprintf(stderr,"\n\n    -v          Ausgabe der Programminformationen vor");
   fprintf(stderr,"\n                Beginn der Uebersetzung.");
   fprintf(stderr,"\n    -h          Ausgabe der Programminformationen, Abbruch.");
   fprintf(stderr,"\n    -c          Uebernahme von Kommentaren im Eingabefile,");
   fprintf(stderr,"\n                Ausgabe erlaeuternder Kommentare");
   fprintf(stderr,"\n    -o outfile  Schreibe Ausgabe in outfile. Entfaellt diese");
   fprintf(stderr,"\n                Option, so wird auf die Standard-Ausgabe ge-");
   fprintf(stderr,"\n                schrieben.\n\n\n");
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void Translate()                                        */
/*                                                                            */
/* Beschreibung     : Uebersetzt tc_pcl nach PCL                              */
/*                                                                            */
/* Globale Variable : aktstep,CP_anchor,R_anchor,E_anchor,G_anchor,pcl_idnum  */
/*                                                                            */
/* Seiteneffekte    : alle                                                    */
/*                                                                            */
/* Aenderungen      : <1> 26.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void Translate()
{
   Id_p     newid;
   long     arg1;
   long     arg2;
   long     oldnum;


   NextRealToken();
   while(akttoken.token != NoToken)
   {
      ResetString(&newcomment);
      ResetString(&juststring);
      Parse_tc_Step();
      switch(aktstep.tc_operation)
      {
         case s_noop:    CPRINT;
                         break;
         case s_reduce:  newid = AllocIdCell();
                         newid->swapped = FALSE;
                         newid->numval = aktstep.tc_id_numval;
                         newid->num = pcl_idnum++;
                         sprintf(ErrStr,"# %ld war %s%ld ",
                                 newid->num,
                                 tc_IdPrint[(long)aktstep.tc_id_type],
                                 aktstep.tc_id_numval);
                         AppendString(&newcomment,ErrStr);

                         arg1 = GetPclNum(aktstep.tc_id_type,aktstep.tc_id_numval);
                         sprintf(ErrStr,",%ld war %s%ld ",
                                 arg1,
                                 tc_IdPrint[(long)aktstep.tc_id_type],
                                 aktstep.tc_id_numval);
                         AppendString(&newcomment,ErrStr);

                         arg2 = GetPclNum(aktstep.arg1_type,aktstep.arg1_numval);
                         sprintf(ErrStr,",%ld war %s%ld ",
                                 arg2,
                                 tc_IdPrint[(long)aktstep.arg1_type],
                                 aktstep.arg1_numval);
                         AppendString(&newcomment,ErrStr);

                         AppendString(&juststring,"tes-red(");
                         sprintf(ErrStr,"%ld",arg1);
                         AppendString(&juststring,ErrStr);
                         AppendChar(&juststring,',');
                         AppendString(&juststring,aktstep.place2);
                         AppendChar(&juststring,',');
                         sprintf(ErrStr,"%ld",arg2);
                         AppendString(&juststring,ErrStr);
                         AppendChar(&juststring,',');
                         AppendString(&juststring,aktstep.place1);
                         AppendChar(&juststring,')');

                         if(aktstep.tc_id_type == G)
                         {
                            if(strcmp(aktstep.res_lside,aktstep.res_rside))
                            {
                               newid->type = tes_goal;
                            }
                            else
                            {
                               newid->type = (aktstep.eq_or_r==equ)?tes_final_e:tes_final_r; 
                            }
                         }
                         else if((*(aktstep.place2)=='L')||(aktstep.eq_or_r == equ))
                         {
                            newid->type = tes_eqn;
                         }
                         else
                         {
                            newid->type = tes_rule;
                         }

                         InsertIdentifier(GetAnchor(aktstep.tc_id_type),newid);

                         PrintPclStep(newid,aktstep.res_lside,aktstep.res_rside,
                                      ViewString(&juststring));

                         CPRINT;
                         break;
         case s_delete:  if(!EraseIdentifier(GetAnchor(aktstep.tc_id_type),
                                             aktstep.tc_id_numval))
                         {
                            fprintf(stderr,
                                    "Warning: Identifier-Number %d not matching...assuming CP\n",
                                    idnum);
                         
                            if(!EraseIdentifier(&CP_anchor,aktstep.tc_id_numval)) 
                            {
                               sprintf(ErrStr,"Sorry, it did not work");
                               RdErr(ErrStr);
                            }
                         }

                         sprintf(ErrStr,"\n            #STEP %s%ld deleted\n",
                                     tc_IdPrint[(long)aktstep.tc_id_type],
                                     aktstep.tc_id_numval);
                         AppendString(&newcomment,ErrStr);
                         CPRINT;
                         break;
         case s_clear:   if(!EraseIdentifier(GetAnchor(aktstep.tc_id_type),
                                             aktstep.tc_id_numval))
                         {
                            fprintf(stderr,
                                    "Warning: Identifier-Number %d not matching...assuming CP\n",
                                    idnum);

                            if(!EraseIdentifier(&CP_anchor,aktstep.tc_id_numval))
                            {
                               sprintf(ErrStr,"Sorry, it did not work");
                               RdErr(ErrStr);
                            }
                         }
 
                         sprintf(ErrStr,"\n           #STEP %s%ld cleared\n",
                                     tc_IdPrint[(long)aktstep.tc_id_type],
                                     aktstep.tc_id_numval);
                         AppendString(&newcomment,ErrStr);
                         CPRINT;
                         break;
         case s_buildcp: newid = AllocIdCell();
                         newid->swapped = FALSE;
                         newid->numval = aktstep.tc_id_numval;
                         newid->num = pcl_idnum++;
                         newid->type = tes_eqn;
                         sprintf(ErrStr,"# %ld war %s%ld",
                                        newid->num,
                                        tc_IdPrint[(long)aktstep.tc_id_type],
                                        aktstep.tc_id_numval);
                         AppendString(&newcomment,ErrStr);

                         arg1 = GetPclNum(aktstep.arg1_type,aktstep.arg1_numval);
                         arg2 = GetPclNum(aktstep.arg2_type,aktstep.arg2_numval);

                         sprintf(ErrStr,", %ld war %s%ld",
                                        arg1,
                                        tc_IdPrint[(long)aktstep.arg1_type],
                                        aktstep.arg1_numval);
                         AppendString(&newcomment,ErrStr);

                         sprintf(ErrStr,", %ld war %s%ld",
                                        arg2,
                                        tc_IdPrint[(long)aktstep.arg2_type],
                                        aktstep.arg2_numval);
                         AppendString(&newcomment,ErrStr);

                         AppendString(&juststring,"cp(");
                         sprintf(ErrStr,"%ld",arg1);
                         AppendString(&juststring,ErrStr);
                         AppendChar(&juststring,',');
                         AppendString(&juststring,aktstep.place1);
                         AppendChar(&juststring,',');
                         sprintf(ErrStr,"%ld",arg2);
                         AppendString(&juststring,ErrStr);
                         AppendChar(&juststring,',');
                         AppendString(&juststring,aktstep.place2);
                         AppendChar(&juststring,')');

                         InsertIdentifier(GetAnchor(aktstep.tc_id_type),newid);

                         PrintPclStep(newid,aktstep.res_lside,aktstep.res_rside,
                                      ViewString(&juststring));
                         CPRINT;
                         break;
         case s_subsum:  newid = RemoveIdentifier(GetAnchor(aktstep.tc_id_type),
                                                  aktstep.tc_id_numval);
                         if(aktstep.tc_id_type == G)
                         {
                            arg1 = newid->num;
                            arg2 = GetPclNum(aktstep.arg1_type,aktstep.arg1_numval);
                            sprintf(ErrStr,"instance(%ld,%ld)",arg1,arg2);
                            newid->num = pcl_idnum++;
                            newid->type = (aktstep.eq_or_r==equ)?tes_final_e:tes_final_r; 
                            PrintPclStep(newid,aktstep.res_lside,aktstep.res_rside,ErrStr);
                            sprintf(ErrStr,"# %ld war %s%ld, %ld war %s%ld, %ld war %s%ld",
                                         newid->num,
                                         tc_IdPrint[(long)aktstep.tc_id_type],
                                         aktstep.tc_id_numval,
                                         arg1,
                                         tc_IdPrint[(long)aktstep.tc_id_type],
                                         aktstep.tc_id_numval,
                                         arg2,
                                         tc_IdPrint[(long)aktstep.arg1_type],
                                         aktstep.tc_id_numval);
                            AppendString(&newcomment,ErrStr);

                         }
                         else
                         {
                         sprintf(ErrStr,"\n           #STEP %s%ld (%ld)  subsumed\n",
                                     tc_IdPrint[(long)aktstep.tc_id_type],
                                     aktstep.tc_id_numval,
                                     newid->num);
                         AppendString(&newcomment,ErrStr);
                         }
                         FreeIdCell(newid);
                         CPRINT;
                         break;
         case s_initial: newid = AllocIdCell();
                         newid->swapped = FALSE;
                         newid->numval = aktstep.tc_id_numval;
                         newid->num = pcl_idnum++;
                         sprintf(ErrStr,"# %ld war %s%ld",
                                        newid->num,
                                        tc_IdPrint[(long)aktstep.tc_id_type],
                                        newid->numval);
                         AppendString(&newcomment,ErrStr);
                         if(aktstep.tc_id_type == G)
                         {
                            newid->type = tes_goal;
                            sprintf(ErrStr,"hypothesis");
                         }
                         else
                         {
                            newid->type = (aktstep.eq_or_r==equ)?tes_eqn:tes_rule;
                            sprintf(ErrStr,"initial");
                         }
                         PrintPclStep(newid,aktstep.res_lside,aktstep.res_rside,ErrStr);

                         InsertIdentifier(GetAnchor(aktstep.tc_id_type),newid);
                         CPRINT;
                         break;
         case s_assign:  if(aktstep.tc_id_type == F)
                         {
                            arg1 = GetPclNum(aktstep.arg1_type,aktstep.arg1_numval);
                            newid = AllocIdCell();
                            newid->swapped = FALSE;
                            newid->numval = aktstep.tc_id_numval;
                            newid->num = pcl_idnum++;
                            newid->type = (aktstep.eq_or_r==equ)?tes_final_e:tes_final_r; 
                            sprintf(ErrStr,"%ld",arg1);
                            PrintPclStep(newid,aktstep.res_lside,
                                         aktstep.res_rside,ErrStr);
                            sprintf(ErrStr,"# %ld war %s%ld",
                                           newid->num,
                                           tc_IdPrint[(long)aktstep.tc_id_type],
                                           aktstep.tc_id_numval);
                            AppendString(&newcomment,ErrStr);

                            FreeIdCell(newid);
                            arg1 = GetPclNum(aktstep.arg1_type,aktstep.arg1_numval);
                            sprintf(ErrStr,", %ld war %s%ld",
                                           arg1,
                                           tc_IdPrint[(long)aktstep.arg1_type],
                                           aktstep.arg1_numval);
                            AppendString(&newcomment,ErrStr);

                         }
                         else
                         {
                            newid = RemoveIdentifier(GetAnchor(aktstep.arg1_type),
                                                     aktstep.arg1_numval);
                            if(aktstep.tc_id_type == R) 
                            {
                               newid->type = tes_rule;
                               newid->numval = aktstep.tc_id_numval;
                               if(newid->swapped)
                               {
                                  sprintf(ErrStr,"orient(%ld,x)",newid->num);
                               }
                               else
                               {
                                  sprintf(ErrStr,"orient(%ld,u)",newid->num);
                               }
                               oldnum = newid->num;
                               newid->numval = aktstep.tc_id_numval;
                               newid->num = pcl_idnum++;
                               newid->swapped = FALSE;

                               PrintPclStep(newid,aktstep.res_lside,
                                            aktstep.res_rside,ErrStr);

                               sprintf(ErrStr,"# %ld war %s%ld",
                                              newid->num,
                                              tc_IdPrint[(long)aktstep.arg1_type],
                                              aktstep.arg1_numval);
                               AppendString(&newcomment,ErrStr);

                               sprintf(ErrStr,", %ld war %s%ld",
                                              oldnum,
                                              tc_IdPrint[(long)aktstep.arg1_type],
                                              aktstep.arg1_numval);
                               AppendString(&newcomment,ErrStr);

                            }
                            else
                            {
                               newid->numval = aktstep.tc_id_numval;
                               sprintf(ErrStr,"\n            #STEP %ld war %s%ld\n",
                                              newid->num,
                                              tc_IdPrint[(long)aktstep.arg1_type],
                                              aktstep.arg1_numval);
                               AppendString(&newcomment,ErrStr);
                            }
                            InsertIdentifier(GetAnchor(aktstep.tc_id_type),newid);
                         }

                         
                         CPRINT; 
                         break;
         case s_swap:    newid = GetIdentifierAddr(GetAnchor(aktstep.tc_id_type),
                                                   aktstep.tc_id_numval);
                         newid->swapped = !newid->swapped;
                         sprintf(ErrStr,"\n            #STEP %s%ld (%ld) swapped\n",
                                     tc_IdPrint[(long)aktstep.tc_id_type],
                                     aktstep.tc_id_numval,
                                     newid->num);
                         AppendString(&newcomment,ErrStr);
                         CPRINT;
                         break;
         default:        fprintf(out,"Unknown Feature...\n");
                         Print_tc_Step(&aktstep);
                         break;
      }
      if(aktstep.tc_id_type != U)
      {
         lasttc_type = aktstep.tc_id_type;
         lasttc_num  = aktstep.tc_id_numval;
      }
   }
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Id_p GetAnchor(tc_IdType type)                          */
/*                    IN    tc_IdType type                                    */
/*                                                                            */
/* Beschreibung     : Liefert zu eingegebenem Typ die korrekte Speicherliste  */
/*                                                                            */
/* Globale Variable : CP_anchor,R_anchor,E_anchor,G_anchor,lasttc_typE        */
/*                                                                            */
/* Seiteneffekte    : Warnung bei falscher Eingabe                            */
/*                                                                            */
/* Aenderungen      : <1> 26.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Id_p GetAnchor(tc_IdType type)
{
   if(type == U)
   {
      type = lasttc_type;
   }
   switch(type)
   {
      case CP: return &CP_anchor;
               break;
      case R:  return &R_anchor;
               break;
      case E:  return &E_anchor;
               break;
      case G:  return &G_anchor;
               break;
      default: fprintf(stderr,"Warning: Wrong type in GetAnchor...\n");
               return NULL;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long GetPclNum(tc_IdType type,long numval)              */
/*                    IN    tc_IdType type                                    */
/*                    long  numval                                            */
/*                                                                            */
/* Beschreibung     : Gibt zu tc_Identifier die Nummer des pcl-Identifiers.   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 26.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

long GetPclNum(tc_IdType type,long numval)
{
   StepType dummy;
   long     res;

   if(!GetPclIdentifier(GetAnchor(type),numval,&dummy,&res))
   {
      sprintf(ErrStr,"%s%ld unknown",tc_IdPrint[(long)type],numval);
      RdErr(ErrStr);
   }
   return res;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintPclStep(Id_p des,char* lside,                 */
/*                                      char* rside, char* just)              */
/*                    IN    Id_p      des                                     */
/*                    IN    tc_IdType type                                    */
/*                    IN    char*     lside                                   */
/*                    IN    char*     rside                                   */
/*                    IN    char*     just                                    */
/*                                                                            */
/* Beschreibung     : Gibt pcl-Schritt aus                                    */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 26.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintPclStep(Id_p des,char* lside,char* rside, char* just)
{
   if(des->type == tes_goal)
   {
      fprintf(out,"%9ld : tes-goal : %s = %s : %s\n",
                   des->num,
                   lside,
                   rside,
                   just);
   }
   else if(des->type == tes_eqn)
   {
      fprintf(out,"%9ld : tes-eqn : %s = %s : %s\n",
                   des->num,
                   lside,
                   rside,
                   just);
   }
   else if(des->type == tes_final_r)
   {
      fprintf(out,"%9ld : tes-final : %s -> %s : %s\n",
                   des->num,
                   lside,
                   rside,
                   just);
   }
   else if(des->type == tes_final_e)
   {
      fprintf(out,"%9ld : tes-final : %s = %s : %s\n",
                   des->num,
                   lside,
                   rside,
                   just);
   }
   else
   {
      fprintf(out,"%9ld : tes-rule : %s -> %s : %s\n",
                   des->num,
                   lside,
                   rside,
                   just); 
   }
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


