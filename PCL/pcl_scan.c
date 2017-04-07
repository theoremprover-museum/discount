/*************************************************************************/
/*                                                                       */
/*   File:        pcl_scan.c                                             */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Scanner fuer pcl                                       */
/*                                                                       */
/*   Aenderungen: <1> 2.4.1991 Uebernagme von scan.c                     */
/*                                                                       */
/*************************************************************************/


#include "pcl_scan.h"



/*----------------------------------------------------------------------------*/
/*               Forward-Deklarationen interner Funktionen                    */
/*----------------------------------------------------------------------------*/

char nextchar();

void scan_ident();
void scan_string();
void scan_line_comment();
void scan_C_comment();
void setliteral(Token_p tok,char* lit);
void settextval(Token_p tok, char* txt, long len);



/*----------------------------------------------------------------------------*/
/*                 Globale Variable                                           */
/*----------------------------------------------------------------------------*/


char       aktchar,  /* aktuelle Zeichen  */
           look;


Token      akttoken = {NoToken,NULL,0,NULL,0,0};   /* aktuelles Token  */

StringCell aktliteral = {NULL,0,0};  /* Zusammengesetzter String  */

long       column,   /* Position im Sourcefile  */
           line;

FILE*      in;     /* Eingabefile  */
char*      InFile; /* Name des Eingabefiles - fuer Fehlermeldung  */

#ifdef LEARN_VERSION

char* colon_ids[]=
{
   "domain",
   "type",
   "signature",
   "examples",
   "specification",
   "goals",
   "lemmas",
   "facts",
   "brain",
   ""
};

#endif

/*----------------------------------------------------------------------------*/
/*           Exportierte Funktionen                                           */
/*----------------------------------------------------------------------------*/



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : InitScanner(FILE *S_in,char* S_name)                    */
/*                                                                            */
/* Beschreibung     : Initialisiert Scanner, setzt aktchar, look, akttoken,   */
/*                    line ,column, in, InFile, aktliteral.                   */ 
/*                                                                            */
/* Globale Variable : aktchar, look, akttoken, line, column, in, InFile,      */
/*                    aktliteral                                              */
/*                                                                            */
/* Seiteneffekte    : s.o.                                                    */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                    <2> 02.4.1991 aktchar ist "aktuell"                     */
/*                                                                            */
/******************************************************************************/

void InitScanner(FILE *S_in,char* S_name)
{

   akttoken.token = NoToken;

   FREE(akttoken.literal);
   akttoken.literal = NULL;
   akttoken.litmem = 0;
   FREE(akttoken.textval);
   akttoken.textval = NULL;
   akttoken.textmem = 0;
   akttoken.numval = 0;

   in = S_in;
   InFile = S_name;

   ResetString(&aktliteral);

   line = 1;
   column = 0;

   nextchar();
   nextchar();
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void NextToken()                                        */
/*                                                                            */
/* Beschreibung     : Schreibt in akttoken das naechste Token. Der von        */
/*                    akttoken referenzierte Stringspeicher wird vorher frei- */
/*                    gegeben.                                                */
/*                                                                            */
/* Globale Variable : aktchar,akttoken                                        */
/*                                                                            */
/* Seiteneffekte    : akttoken wird veraendert, Zeichen werden gelesen        */
/*                                                                            */
/* Aenderungen      : <1> 15.2.1991 neu                                       */
/*                    <2> 02.4.1991 aktchar ist "aktuell", assign entfaellt,  */
/*                                  hyphen wird neu eingefuehrt               */
/*                                                                            */
/******************************************************************************/


void NextToken()
{
   
   akttoken.token = NoToken;
   akttoken.numval = 0;

   while(isspace(aktchar))
   {
      nextchar(); 
   }

   if(aktchar!=EOF)
   {
      if(isidchar(aktchar))
      {
         scan_ident();
      }
      else
      {
         switch(aktchar)
         {
	 case '"':
	    scan_string();
	    break;
	 case '(':
	    akttoken.token = openbracket;
	    setliteral(&akttoken,"(");
	    break;
	 case ')':
	    akttoken.token = closebracket;
	    setliteral(&akttoken,")");
	    break;
	 case '=':
	    akttoken.token = equ;
	    setliteral(&akttoken,"=");
	    break;
	 case '-':
	    if(look=='>')
	    {   
	       nextchar();
	       akttoken.token = r_arrow;
	       setliteral(&akttoken,"->");
	    }
                       else
                       {
                          akttoken.token = hyphen;
                          setliteral(&akttoken,"-");
                       }
	    break;
	 case ',': 
	    akttoken.token = comma;
	    setliteral(&akttoken,",");
	    break;
	 case '.':
	    akttoken.token = fullstop;
	    setliteral(&akttoken,".");
	    break;
	 case '#':
	    scan_line_comment();
	    break;
	 case '/':
	    if(nextchar()=='*') 
	    {
	    scan_C_comment();
	    }
	    else
	    {
	       RdErr("/* incomplete");
	    }
	    break;
	 case ':':
	    akttoken.token = colon;
	    setliteral(&akttoken,":");
	    break;
	 default:
	    AppendString(&aktliteral,"Unexpected Character '");
	    AppendChar(&aktliteral,aktchar);
	    AppendChar(&aktliteral,'\'');
	    RdErr(ViewString(&aktliteral));
	    ResetString(&aktliteral); /* Eigentlich ueberfluessig - */
	    break;                    /* RdErr terminiert Programm  */
         }
         nextchar();
      }
   }
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintToken(Token_p tok)                            */
/*                    IN     Token_p tok                                      */
/*                                                                            */
/* Beschreibung     : Gibt Token mit relevanten Werten aus.                   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 21.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintToken(Token_p tok)
{
   printf("Token: %ld  Literal: %s\n",(long)tok->token,tok->literal);
   if((tok->token == idnum)||(tok->token == Identifier))
   {
      printf("Textval: %s  ",tok->textval);
      printf("Numval: %ld",tok->numval);
   }
   printf("\n");
}
      


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void RdErr(char* message)                               */
/*                                                                            */
/* Beschreibung     : Gibt Fehlermeldung aus und bricht Programm ab.          */
/*                                                                            */
/* Globale Variable : line,column,InFile                                      */
/*                                                                            */
/* Seiteneffekte    : Programmabbruch                                         */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

#ifdef __GNUC__
volatile
#endif
void RdErr(char* message)
{
   fprintf(stderr,"ERROR: %s while reading %s at line %ld and column %ld ...\n",
           message,InFile,line,column);
   exit(SYNTAX_ERROR);
}


/*----------------------------------------------------------------------------*/
/*                 Interne Funktionen                                         */
/*----------------------------------------------------------------------------*/



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char nextchar()                                         */
/*                                                                            */
/* Beschreibung     : Setzt look auf das naechste Zeichen, aktchar auf look   */
/*                    Rueckgabewert ist ebenfalls aktchar. Die gelesene       */
/*                    Position wird aktualisiert.                             */
/*                                                                            */
/* Globale Variable : aktchar,look,in,line,column                             */
/*                                                                            */
/* Seiteneffekte    : Ein Zeichen des Eingabestreams wird gelesen.            */
/*                                                                            */
/* Aenderungen      : <1> 15.2.1991 neu                                       */
/*                    <2> 18.2.1991 Zeichenposition mitfuehren.               */
/*                                                                            */
/******************************************************************************/


char nextchar()
{
   if(aktchar == '\n')
   {
      line++;
      column = 1;
   }
   else
   {
      column++;
   }
   aktchar = look;
   look = getc(in);

   return aktchar;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void scan_ident()                                       */
/*                                                                            */
/* Beschreibung     : Scannt Identifier, Ergebnis in akttoken.                */
/*                                                                            */
/* Globale Variable : akttoken,aktliteral                                     */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von setliteral,settextval,nextchar         */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/


void scan_ident()
{
   int numstart = -1;
#ifdef LEARN_VERSION
   int f;
   BOOL iscolonid = FALSE;
#endif

   AppendChar(&aktliteral,aktchar);

   if(!isdigit(aktchar))
   {
      numstart = -1;
   }
   else
   {
      if(numstart == -1)
      {
         numstart = aktliteral.len-1;
      }
   }

   nextchar();
   while(isidchar(aktchar))
   {
      AppendChar(&aktliteral,aktchar);
      
      if(!isdigit(aktchar))
      {
         numstart = -1;
      }
      else
      {
         if(numstart == -1)
         {
            numstart = aktliteral.len-1;
         }
      }
      nextchar();
   }
#ifdef LEARN_VERSION
   if(aktchar == ':')
   {
      LOWER_STR(ViewString(&aktliteral));      

      for(f=0; (!iscolonid) && strcmp(colon_ids[f],""); f++)
      {
	 if(!strcmp(colon_ids[f],ViewString(&aktliteral)))
	 {
	    iscolonid = TRUE;
	 }
      }
   }
   if(iscolonid)
   {
      AppendChar(&aktliteral,aktchar);
      nextchar();
      akttoken.token = colonident;
      setliteral(&akttoken,ViewString(&aktliteral));
   }
   else
#endif
   if(numstart != -1)
   {
      akttoken.token = idnum;
      setliteral(&akttoken,ViewString(&aktliteral));
      akttoken.numval = atol(ViewString(&aktliteral)+numstart);
      settextval(&akttoken,ViewString(&aktliteral),numstart);
   }
   else
   {
      akttoken.token = ident;
      setliteral(&akttoken,ViewString(&aktliteral));
   }
   ResetString(&aktliteral);
}

/*-------------------------------------------------------------------------
//
// Function: void scan_string();
//
//  Scannt einen String (eingeschlossen in ""). Die Quotes sind
//  _nicht_ Teil des Strings, sie werden ueberlesen.
//
// Globale Variable : akttoken,aktliteral
//
// Seiteneffekte    : Durch Aufruf von setliteral,nextchar   
//
//-----------------------------------------------------------------------*/

void scan_string()
{
   nextchar();
   while(aktchar!='"')
   {
      if(!ischar(aktchar) || (aktchar == '\n'))
      {
	 RdErr("Unexpected end of line or file while scanning string");
      }
      AppendChar(&aktliteral,aktchar);
      nextchar();
   }
   akttoken.token = string;
   setliteral(&akttoken, ViewString(&aktliteral));
   
   ResetString(&aktliteral);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void scan_line_comment()                                */
/*                                                                            */
/* Beschreibung     : Scannt mit # beginnenden Kommentar                      */
/*                                                                            */
/* Globale Variable : akttoken,aktliteral                                     */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von setliteral,nextchar                    */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void scan_line_comment()
{
   AppendChar(&aktliteral,aktchar);
   while(ischar(aktchar) && (aktchar!='\n'))
   {
      nextchar();
      AppendChar(&aktliteral,aktchar);
   }
   akttoken.token = comment;
   setliteral(&akttoken,ViewString(&aktliteral));

   ResetString(&aktliteral);
}


      

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void scan_C_comment()                                   */
/*                                                                            */
/* Beschreibung     : Scannt Kommentar mit C-Syntax.                          */
/*                                                                            */
/* Globale Variable : akttoken,aktliteral                                     */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von setliteral,nextchar                    */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void scan_C_comment()
{
   AppendString(&aktliteral,"/*");
   while(ischar(aktchar) && (!((aktchar=='*') && (look=='/'))))
   {
      nextchar();
      AppendChar(&aktliteral,aktchar);
   }
   if(ischar(aktchar))
   {
      nextchar();
      AppendChar(&aktliteral,aktchar);
      nextchar();

      akttoken.token = comment;
      setliteral(&akttoken,ViewString(&aktliteral));
   }
   else
   {
      RdErr("Unexpected end of source file while scanning comment");
   }

   ResetString(&aktliteral);
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void setliteral(Token_p tok,char* lit)                  */
/*                                                                            */
/* Beschreibung     : Setzt das Literal im Token auf den angegebenen String.  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Eventuell werden Speicheroperationen durchgefuehrt.     */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/


void setliteral(Token_p tok,char* lit)
{
   int insize;

   insize = strlen(lit);

   if((tok->litmem)>insize)
   {
      strcpy(tok->literal,lit);
   }
   else
   {
      FREE(tok->literal);
      tok->literal = secure_strdup(lit);
      tok->litmem = insize+1;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void settextval(Token_p tok,char* lit,long len)         */
/*                                                                            */
/* Beschreibung     : Setzt das Literal im Token auf die ersten len Zeichen   */
/*                    des angegebenen Strings.                                */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Eventuell werden Speicheroperationen durchgefuehrt.     */
/*                                                                            */
/* Aenderungen      : <1> 18.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/


void settextval(Token_p tok,char* txt,long len)
{
   if((tok->textmem)>len)
   {
      strncpy(tok->textval,txt,len);
      (tok->textval)[len] = '\0';
   }
   else
   {
      FREE(tok->textval);
      tok->textval = secure_malloc(len+1);
      strncpy(tok->textval,txt,len);
      (tok->textval)[len] = '\0';
      tok->textmem = len+1;
   }
}





/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


