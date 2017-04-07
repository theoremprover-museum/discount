/*-------------------------------------------------------------------------

File        : s_scanner.c

Autor       : Stephan Schulz

Inhalt      : This file deals with  the implementation of a simple,
              general purpose scanner for use with new extensions to
	      the DISCOUNT systems. DISCOUNT's original scanner is
	      rather unwieldy and inflexible - this is intended as a
	      base for all further extensions. It will replace the
	      scanner in parseprk.[ch]. This file is, in part, based
	      on pcl_scan.c and the old scanner in parseprk.c

Aenderungen : <1> 5.12.1994 neu
              <2> 8.3.1998 '\0' in scan_string eingefuegt

-------------------------------------------------------------------------*/

#include "s_scanner.h"




/*-----------------------------------------------------------------------*/
/*                        Lokale Typen                                   */
/*-----------------------------------------------------------------------*/

/* This record is used to save all the information pertaining to a single */
/* opened file. It will be used on a stack to allow suspending the scanning */
/* of one file while (completly) reading another */

typedef struct scannerstatecell
{
   TokenType AktToken;
   char*     AktLiteral;
   long      AktNum;
   
   FILE      *infile;   
   char*     file_store;
   
   char      aktchar;
   char      look;  
   long      column;
   long      line;    
   bool      ColonFollows;
   struct scannerstatecell* next;
}ScannerStateCell, *ScannerState_p;

#define AllocScannerStateCell()\
   (ScannerStateCell*)SizeMalloc(sizeof(ScannerStateCell)) 
#define FreeScannerStateCell(junk)\
   SizeFree(junk, sizeof(ScannerStateCell))

char accumulate_data[3*MAXLITERAL+10]; /* Space to accumulate any */
                                       /* possible representation of a */
                                       /* real: 3 fields of uncertain */
                                       /* length and 3 single */
                                       /* characters, allowing for 1 */
                                       /* byte \0 and 6 bytes just in */
                                       /* case */

/*-----------------------------------------------------------------------*/
/*                       Globale Variable                                */
/*-----------------------------------------------------------------------*/


TokenType AktToken           = NoToken;
char      AktLiteral[LENGTH] = "";
long      AktNum             = 0;

static FILE  *infile;    /* Input stream pointer */
static char* file_store; /* Name of input file for reference */

static char aktchar,  /* Current character and */
            look;     /* look-ahead            */
static long column,   /* Current position in the source file - */
            line;     /* Used for error messages (this is a simple */
		      /* hack and might be slightly off - a better way */
		      /* would be to store the beginning of each token */
		      /* in seperate variables - well, I'm not Niklaus */
		      /* Wirth :-)           */

char ErrorSpace[120+LENGTH]; /* Space for sprintf used in connection */
			     /* with error messages - enough space */
			     /* for the messages generated, including */
			     /* up to one ident read just now.*/

bool ColonFollows = false;

static ScannerState_p state_stack = NULL;

char* token_print_table[] = /* This variable gives the output */
			    /* representation of each token, to be */
			    /* used for error-messages or similar */
			    /* tasks */
{
   "No valid Token (probably EOF)",             /* NoToken */
   "Arbitrary identifier",                      /* identifier */
   "Identifier not terminating in a number",    /* ident */
   "Identifier terminating in a number",        /* idnum */
   "Positive number (Sequence of digits)",      /* cardinal */
   "Negative number (- followes by a sequence of digits)", /* negative */
   "Number: sequence of digits, may be lead by a -", /* number */
   "String, enclosed in double quotes \"\"",    /* String */
   "=",
   "(",
   ")",
   ":",
   ",",
   ";",
   "-",
   ".",
   "Comment (enclosed in /* ... */ or beginning with #)"
};
     
   



/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

static void push_scanner_state();
static void pop_scanner_state();

static char next_char();
static void scan_ident();
static void scan_number();
static void scan_string();
static void scan_line_comment();
static void scan_C_comment();


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : static void push_scanner_state()

Beschreibung     : This function will save the complete state of the
                   scanner onto a stack. This allows the scanning of other
		   files and the restoration of the old state to finish
		   dealing with the original file.

Globale Variable : AktToken, AktLiteral, AktNum, infile, file_store,
                   aktchar, look, column, line, ColonFollows, state_stack

Seiteneffekte    : Saves scanner state, memory operations

Aenderungen      : <1> 15.12.1994 neu

-------------------------------------------------------------------------*/

static void push_scanner_state()
{
   ScannerState_p handle;

   handle = AllocScannerStateCell();
   
   handle->AktToken     = AktToken;
   handle->AktLiteral   = SecureStrdup(AktLiteral);
   handle->AktNum       = AktNum;
   handle->infile       = infile;
   handle->file_store   = file_store;
   handle->aktchar      = aktchar;
   handle->look         = look;
   handle->column       = column;
   handle->line         = line;
   handle->ColonFollows = ColonFollows;

   handle->next = state_stack;
   
   state_stack = handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : static void pop_scanner_state();

Beschreibung     : Restores the scanner's state from the stack (if
                   notempty).

Globale Variable : AktToken, AktLiteral, AktNum, infile, file_store,
                   aktchar, look, column, line, ColonFollows, state_stack

Seiteneffekte    : Restores Scanner, Memory operations (as above)

Aenderungen      : <1> 15.12.1994 neu

-------------------------------------------------------------------------*/

static void pop_scanner_state()
{
   ScannerState_p handle;

   if(!state_stack)
   {
      fprintf(stderr,
	      "There is a bug in the program - someone tried to restore the\n" 
             "scanner from a non-existent state!\n");
      exit(-1);
   }

   handle = state_stack;
   state_stack = handle->next;
   
   AktToken     = handle->AktToken;
   strcpy(AktLiteral,handle->AktLiteral);
   free(handle->AktLiteral);
   AktNum       = handle->AktNum;
   infile       = handle->infile;
   file_store   = handle->file_store;
   aktchar      = handle->aktchar;
   look         = handle->look;
   column       = handle-> column;
   line         = handle->line;
   ColonFollows = handle->ColonFollows;

   FreeScannerStateCell(handle);
}



/*-------------------------------------------------------------------------

FUNCTION         : static char next_char()

Beschreibung     : Reads the next character from the input file,
                   updates look, line and column

Globale Variable : aktchar,look,infile,line,column

Seiteneffekte    : Changes in the global variable accessed, takes a
                   character from the input file

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

static char next_char()
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
   look = getc(infile);

   return aktchar;
}


/*-------------------------------------------------------------------------

FUNCTION         : static void scan_ident()

Beschreibung     : Scans an ident (any subtype of identifier) and
                   sets AktToken, AktLiteral and AktNum according to
		   its value. 

Globale Variable : aktchar, AktToken, AktLiteral, AktNum

Seiteneffekte    : By using next_char()

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

static void scan_ident()
{
   int numstart = -1; /* If the identifier scanned so far terminates */
		      /* with a number this points to the position of */
		      /* the first digit. It is -1 otherwise */
   int pos = 0;       /* Position next character will be stored in */
   
   /* First character cannot be a digit - scan_number() catches this */
   /* case! */

   AktLiteral[pos++] = aktchar;

   while(isidchar(look))
   {
      next_char();

      if(pos == LENGTH-2) /* AktLiteral is indexed from 0 to LENGTH-1, */
			  /* last character position is reserved for */
			  /* string terminator '\0' */
      {
	 ScannerError("Identifier to long (recompile DISCOUNT with\n\
LENGTH #define'd to a greater value in defines.h");
      }

      if(!isdigit(aktchar))
      {
         numstart = -1;
      }
      else
      {
         if(numstart == -1) 
         {
            numstart = pos;
         }
      }
      AktLiteral[pos++] = aktchar;
   }

   ColonFollows = (look == ':') ? true : false;

   AktLiteral[pos] = '\0';
   
   if(numstart != -1)
   {
      AktToken = idnum; 
      
      AktNum = atol(&(AktLiteral[numstart]));
   }
   else
   {
      AktToken = ident;
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : static void scan_number()

Beschreibung     : Scannt Ganzzahl, Ergebnis in AktToken und AktNum

Globale Variable : AktToken, AktLiteral, aktchar, look

Seiteneffekte    : Durch next_char()

Aenderungen      : <1> 7.12..1994 neu

-------------------------------------------------------------------------*/

static void scan_number()
{
   int pos = 0;
 
   AktLiteral[pos++] = aktchar;

   while(isdigit(look))
   {
      next_char();

      if(pos == LENGTH-2) /* AktLiteral is indexed from 0 to LENGTH-1, */
			  /* last character position is reserved for */
			  /* string terminator '\0' */
      {
	 ScannerError("Number to long (you might want to recompile\n\
DISCOUNT with LENGTH #define'd to a greater value in defines.h, but\n\
will get a long integer overflow, anyways");
      }
      AktLiteral[pos++] = aktchar;
   }
   AktLiteral[pos] = '\0';

   AktNum = atol(AktLiteral);
   AktToken = AktNum >= 0? positive : negative;
}


/*-------------------------------------------------------------------------
//
// Function: void scan_string();
//
//  Scannt einen String (eingeschlossen in ""). Die Quotes sind
//  _nicht_ Teil des Strings, sie werden ueberlesen.
//
// Globale Variable : AktToken, AktLiteral
//
// Seiteneffekte    : Durch Aufruf von nextchar   
//
//-----------------------------------------------------------------------*/

void scan_string()
{
   int pos = 0;

   next_char();
   while(aktchar!='"')
   {
      if(!ischar(aktchar) || (aktchar == '\n'))
      {
	 ScannerError("Unexpected end of line or file while scanning string");
      }
      if(pos == LENGTH-2) /* AktLiteral is indexed from 0 to LENGTH-1, */
			  /* last character position is reserved for */
			  /* string terminator '\0' */
      {
	 ScannerError("String to long (you might want to recompile\n\
DISCOUNT with LENGTH #define'd to a greater value in defines.h, but\n\
the concept of arbitrarily limited strings is broken anyways\n\
('historical reasons')");
      }
      AktLiteral[pos++] = aktchar;
      next_char();
   }
   AktLiteral[pos] = '\0'; /* WURDE DAS WIRKLICH VERGESSEN (Felix) */
   AktToken = string;
}


/*-------------------------------------------------------------------------

FUNCTION         : static void scan_line_comment()

Beschreibung     : Scans a comment starting with # (terminated by \n)

Globale Variable : AktToken

Seiteneffekte    : By using next_char()

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

static void scan_line_comment()
{
   while(ischar(aktchar) && (aktchar!= '\n'))
   {
      next_char();
   }
   AktToken = comment;
}


/*-------------------------------------------------------------------------

FUNCTION         : static void scan_C_comment()

Beschreibung     : Scans a C-Style comment (delimited by slash * *
                   slash (the evil symbols I cannot use inside
		   this comment :-))

Globale Variable : AktToken

Seiteneffekte    : By using next_char()

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

static void scan_C_comment()
{
   while(ischar(aktchar) && (!((aktchar=='*') && (look=='/'))))
   {
      next_char();
   }
   if(ischar(aktchar))
   {
      next_char();
      AktToken = comment;
   }
   else
   {
      ScannerError("Unexpected end of source file while scanning comment");
   }
}


/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
//
// Function: FileExists()
//
//   Returns true, if the file exists and can be opened for
//  reading. If !silent, warns if file does not exist.
//
// Global Variables: -
//
// Side Effect     : Attempts to open file
//
//-----------------------------------------------------------------------*/

bool FileExists(char* filename, bool silent)
{
   FILE* tmp;
   bool  result=false;

   if((tmp=fopen(filename,"r")))
   {
      result=true;
      fclose(tmp);
   }
   else if(!silent)
   {
      printf("Warning: Cannot open file %s.\n", filename);
      perror("fopen()");
   }
   return result;
}


/*-------------------------------------------------------------------------

FUNCTION         : void InitScanner(char* filename)

Beschreibung     : Opens input stream, assigns it to infile and stores
                   name (filename) in file_store, gets first real
		   token. 

Globale Variable : infile, file_name

Seiteneffekte    : As above

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

void InitScanner(char* filename)
{
   push_scanner_state();
   
   file_store = SecureStrdup(filename);
   infile = fopen(filename,"r");
   if(!infile)
   {
      fprintf(stderr,"ERROR: Cannot open file %s.\n",filename);
      perror("       fopen");
      exit(1);
   }
   
   line = 1;
   column = 0;
   
   next_char();
   next_char();
   
   NextRealToken();
}


/*-------------------------------------------------------------------------

FUNCTION         : void EndScanner()

Beschreibung     : Ends an input operation, closes the stream, frees
                   file_name 

Globale Variable : infile, file_name

Seiteneffekte    : As above

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

void EndScanner()
{
   fclose(infile);
   if(file_store)
   {
      free(file_store);
   }
   pop_scanner_state(); /* Checks for erroes itself... */
}



/*-------------------------------------------------------------------------

FUNCTION         : void ScannerError(char* message)

Beschreibung     : Produces error message, exits DISCOUNT

Globale Variable : line, column, file_name

Seiteneffekte    : Terminates the process (should (perhaps) terminate
                   the team...)

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

void ScannerError(char* message)
{
   fprintf(stderr, 
	   "ERROR during parsing %s at line %ld, column %ld: %s\n",
	   file_store, line, column, message);
   exit(-1);
}


/*-------------------------------------------------------------------------

FUNCTION         : TokenType NextToken()

Beschreibung     : Scans and returns the next Token.

Globale Variable : AktToken, AktLiteral, AktNum

Seiteneffekte    : By calling next_char()

Aenderungen      : <1> 5.12.1994 neu

-------------------------------------------------------------------------*/

TokenType NextToken()
{      
   AktToken = NoToken;
   AktNum = 0;
   AktLiteral[0] = '\0';

   while(isspace(aktchar))
   {
      next_char(); 
   }

   if(aktchar!=EOF)
   {
      if(isdigit(aktchar) || ((aktchar == '-') && (isdigit(look))))
      {
	 scan_number();
      }
      else if(isidchar(aktchar))
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
	 case '=': 
	    AktToken = equal_sign;
	    break;
	 case '(':  
	    AktToken = openbracket;
	    break;
	 case ')': 
	    AktToken = closebracket;
	    break;
	 case ':': 
	    AktToken = colon;
	    break;
	 case ',':  
	    AktToken = comma;
	    break;
	 case ';':
	    AktToken = semicolon;
	    break;
	 case '-':
	    AktToken = hyphen; /* negative numbers are catched above! */
	    break;
	 case '.':
	    AktToken = dot;
	    break;
	 case '#':  
	    scan_line_comment();
	    break;
	 case '/': 
	    if(next_char()=='*') 
	    {
	       scan_C_comment();
	    }
	    else
	    {
	       ScannerError("/* incomplete");
	    }
	    break;
	 default:
	    ScannerError("Unexpected character in input stream");
	    break;
	 }
      }
      next_char();
   }
   return AktToken;
}
	    

/*-------------------------------------------------------------------------

FUNCTION         : TokenType NextRealToken()

Beschreibung     : Skips comments and returns next non-comment-token.
                   Sets AktLiteral, AktNum and AktToken using
		   NextToken() 

Globale Variable : -

Seiteneffekte    : By calling NextToken() 

Aenderungen      : <1> 6.12.1994 neu

-------------------------------------------------------------------------*/

TokenType NextRealToken()
{
   NextToken();
   while(AktToken == comment)
   {
      NextToken();
   }
/*   printf("NextRealToken() retuns (%s) %s...\n",
	  AktLiteral, token_print_table[AktToken]);*/
   
   return AktToken;
}

/*-------------------------------------------------------------------------

FUNCTION         : bool TestToken(TokenType token)

Beschreibung     : Returns true if AktToken is compatible with the
                   argument, false else. 'identifier' is compatible
		   with all three subtypes (ident, idnum, number).

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 6.12.1994 neu

-------------------------------------------------------------------------*/

bool TestToken(TokenType token)
{
   if(token == identifier) 
   {
      return (AktToken == idnum) || (AktToken == ident) || 
             (AktToken == positive) || (AktToken == identifier);
   }
   else if(token == number)
   {
      return (AktToken == positive) || (AktToken == negative) || 
             (AktToken == number);
   }  
   return AktToken == token;
}


/*-------------------------------------------------------------------------

FUNCTION         : bool TestIdent(TokenType token, char* literal)

Beschreibung     : Returns true if token is compatible with
                   AktToken (checked using TestToken()) and if literal
		   equals AktLiteral (case-insensitive), false in all
		   other cases. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 6.12.1994 neu

-------------------------------------------------------------------------*/

bool TestIdent(TokenType token, char* literal)
{
   if(TestToken(token))
   {
      return (bool)(!strcasecmp(literal, AktLiteral));
   }
   return false;
}



/*-------------------------------------------------------------------------

FUNCTION         : void CheckToken(TokenType token)

Beschreibung     : Checks if the curren token matches the expected one
                   (using TestToken()), produces an "... expected"
		   error if not. No effect otherwise.

Globale Variable : AktToken, token_print_table, ErrorSpace 

Seiteneffekte    : Might terminate the program (using
                   ScannerError()). In this case ErrorSpace will also
		   be written (not that it matters :-).

Aenderungen      : <1> 6.12.1994 neu

-------------------------------------------------------------------------*/

void CheckToken(TokenType token)
{
   if(!TestToken(token))
   {
      sprintf(ErrorSpace, "%s expected, but %s read",
	      token_print_table[token], token_print_table[AktToken]);
      ScannerError(ErrorSpace);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void CheckIdent(TokenType token, char* literal)

Beschreibung     : CheckIdent() is to TestIdent() as CheckToken() is
                   to TestToken() :-). Tests if the expected
		   identifier is current, produces an error if not.

Globale Variable : AktToken, token_print_table, AktLiteral,
                   ErrorSpace 

Seiteneffekte    : 

Aenderungen      : <1> .1994 neu

-------------------------------------------------------------------------*/

void CheckIdent(TokenType token, char* literal)
{
   if(!TestIdent(token,literal))
   {
      if(TestToken(token)) /* At least it was an identifier... */
      {
	 sprintf(ErrorSpace, "%s (%s) expected, but %s (%s) read",
		 token_print_table[token], literal, 
		 token_print_table[AktToken], AktLiteral);
      }
      else
      {
	 sprintf(ErrorSpace, "%s (%s) expected, but %s read",
		 token_print_table[token], literal, 
		 token_print_table[AktToken]); 
      }
      ScannerError(ErrorSpace);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void AcceptToken(TokenType token)

Beschreibung     : Checks if the expected token is present and eats
                   it. Error otherwise.

Globale Variable : -

Seiteneffekte    : By calling CheckToken() and NextRealToken()

Aenderungen      : <1> 6.12.1994 neu

-------------------------------------------------------------------------*/

void AcceptToken(TokenType token)
{
   CheckToken(token);
   NextRealToken();
}


/*-------------------------------------------------------------------------

FUNCTION         : void AcceptIdent(TokenType token, char* literal)

Beschreibung     : Checks for the presence of the correct identifier.
                   Eats it if present, error otherwise.

Globale Variable : -

Seiteneffekte    : By calling CheckToken() and NextRealToken()

Aenderungen      : <1> 6.12.1994 neu

-------------------------------------------------------------------------*/

void AcceptIdent(TokenType token, char* literal)
{
   CheckIdent(token, literal);
   NextRealToken();
}


/*-------------------------------------------------------------------------
//
// Function: ParseBool()
//
//   Parses a boolean value (T/F) and returns true or false
//   accordingly. 
//
// Global Variables: -
//
// Side Effect     : Reads input
//
//-----------------------------------------------------------------------*/

bool ParseBool()
{
   if(TestIdent(ident,"T"))
   {
      NextRealToken();
      return true;
   }
   else if(TestIdent(ident,"F"))
   {
      NextRealToken();
      return false;
   }
   CheckIdent(ident, "Boolean Identifier ('T' or 'F') expected");
   return false;
}

/*-----------------------------------------------------------------------

FUNCTION        : double ParseReal()

Description     : Parses a real value and return it. A real is
                  composed of an optional "-", a sequence of digits,
                  an optional "." followed by one or more digits and
                  an optional "e" or "E" followed by another sequence
                  of digits (again with an optional "-").

Global Variables: Tok.*

Side Effects    : Input is read (and checked)

Changes         : <1> 23.7.1995 New

-----------------------------------------------------------------------*/

double ParseReal()
{
   accumulate_data[0]='\0';

   if(TestToken(hyphen))
   {
      strcat(accumulate_data, AktLiteral);
      NextRealToken();
   }
   strcat(accumulate_data,AktLiteral);
   NextToken();
#ifndef ALLOW_COMMA_AS_DECIMAL_DOT
   if(TestToken(dot))
#else
   if(TestToken(dot) || TestToken(comma))
#endif
   {
      strcat(accumulate_data,".");
      NextToken();
      strcat(accumulate_data,AktLiteral);
      CheckToken(positive);
      NextToken();
   }
   if(TestIdent(idnum,"e") || TestIdent(idnum,"E"))
   {
      strcat(accumulate_data,AktLiteral);
      NextToken();
   }
   else if(TestIdent(ident,"e") || TestIdent(ident,"E"))
   {
      strcat(accumulate_data,AktLiteral);
      NextToken();
      if(TestToken(hyphen))
      {
         strcat(accumulate_data, AktLiteral);
         NextToken();
      }
      strcat(accumulate_data,AktLiteral);
      CheckToken(positive);
      NextToken();
   }

   if(TestToken(comment))
   {
      NextRealToken();
   }

   return atof(accumulate_data);
}


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


