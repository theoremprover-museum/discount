/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          polynom
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung von Polynomen
//-----------------------------------------------------------------------------
//      $Log: polynom.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __POLYNOM
#define     __POLYNOM

/*
//-----------------------------------------------------------------------------
//      Typdeklaration
//-----------------------------------------------------------------------------
*/

typedef enum opcodes    { op_const,
                          op_var,
                          op_add,
                          op_sub,
                          op_mul,
                          op_pot                        } opcode;

typedef struct polycell { opcode            opc;
                          long              value;
                          struct polycell   *left;
                          struct polycell   *right;
                          #ifdef MEMDEBUG
                            short   debug;
                          #endif
                                                        } polynom;
        

/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    polynom  *newpoly ( opcode opc, long value );
    void     deleteply ( polynom *poly );

    long     polyval     ( polynom *poly, long *var );
    polynom  *getpolynom ( function *fcode, bool newfunc, char *fptr, char *pptr );
    void     printpoly   ( polynom *poly );

#endif

#endif

