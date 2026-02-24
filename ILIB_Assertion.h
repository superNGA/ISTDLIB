//=========================================================================
//                      Assertion
//=========================================================================
// by      : INSANE
// created : 22/02/2026
//
// purpose : assertion.h rip-off so we have assertions in release mode.
//-------------------------------------------------------------------------
#ifndef ASSERTION_H
#define ASSERTION_H


#include <stdio.h>
#include <stdlib.h>


#ifndef ENABLE_ASSERTIONS
#define ENABLE_ASSERTIONS 1
#endif


#if (ENABLE_ASSERTIONS == 1)
#define assertion(expression) { if((expression) == false) Assertion(#expression, __FILE__, __LINE__); }
#elif
#define assertion(expression) 
#endif


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static inline void Assertion(const char* szExpression, const char* szFile, int iLine)
{
    printf("Assertion failed!\n");
    printf("Expression : %s\n", szExpression);
    printf("File       : %s\n", szFile);
    printf("Line       : %d\n", iLine);
    abort();
}


#endif
