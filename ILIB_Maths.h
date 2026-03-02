//=========================================================================
//                      ILIB Maths
//=========================================================================
// by      : INSANE
// created : 02/03/2026
//
// purpose : Small utility math functions.
//-------------------------------------------------------------------------
#ifndef ILIB_MATH_H
#define ILIB_MATH_H

#include <stdlib.h>
#include <stdint.h>

#include "ILIB_Assertion.h"



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static int32_t Maths_RoundTowardZero(int32_t iNum, int32_t iRoundTo)
{
    if(iRoundTo == 0)
        return iNum;

    return (iNum / iRoundTo) * iRoundTo;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static int32_t Maths_Round(int32_t iNum, int32_t iRoundTo)
{
    if(iRoundTo == 0)
        return iNum;

    int32_t iSign   = (iNum < 0) ? -1    : 1;
    int32_t iAbsNum = (iNum < 0) ? -iNum : iNum;

    return iSign * ((iAbsNum + (iRoundTo - 1)) / iRoundTo) * iRoundTo;
}



#endif
