//=========================================================================
//                      Vector
//=========================================================================
// by      : INSANE
// created : 22/02/2026
//
// purpose : std::vector<> rip-off in C. Inspired by nothings/stb_ds.h
//-------------------------------------------------------------------------
#ifndef VECTOR_H
#define VECTOR_H


#include <stdint.h>
#include <stdlib.h>
#include "Assertion.h"


#define nullptr              ((void*)0)
#define STD_VECTOR_CAPACITY  (10)
#define STD_VECTOR_SIGNATURE (0xBED0DECA)
#define STD_VECTOR_GROWTH    (2.0)

/*

Vector Structure : 
    [ Header ][ Entry 0 ][ Entry 1 ][ Entry 2 ]...

*/


// TODO: Reserve
// todo: Resize


/* Index of the last stored element + 1. Zero in case no elements. */
#define Vector_Len(Container) (Container == nullptr ? 0 : ((VectorHeader_t*)Container)[-1].m_iSize)

/* Insert element at any index with capacity. */
#define Vector_Insert(Container, Index, Data) {                                    \
    int iIndex = Index;                                                            \
    Vector_QualityCheck((void**)&Container, sizeof(*Container), -1, iIndex);       \
    VectorHeader_t* pHeader = &((VectorHeader_t*)Container)[-1];                   \
    for(int iVecIndex = pHeader->m_iSize; iVecIndex > iIndex; iVecIndex++)         \
        Container[iVecIndex] = Container[iVecIndex - 1];                           \
                                                                                   \
    Container[iIndex] = Data;                                                      \
    pHeader->m_iSize = iIndex > pHeader->m_iSize ? iIndex : pHeader->m_iSize + 1;  \
}

#define Vector_PushBack(Container, Data) Vector_Insert(Container, Vector_Len(Container), Data)
#define Vector_Resize(Container, Size)  { Vector_Reserve(Container, Size); ((VectorHeader_t*)Container)[-1].m_iSize = Size; }
#define Vector_Reserve(Container, Size) { Vector_QualityCheck(&Container, sizeof(*Container), -1, Size); }


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
typedef struct VectorHeader_t
{
    uint32_t m_iSize;        // Biggest index at which an element is stored. + 1.
    uint32_t m_iCapacity;    // Total elements that can be stored. ( malloced size / element size )
    uint32_t m_iElementSize; // Size of one element.
    uint32_t m_iSignature;   // Signature to help us prevent mistakes.

} VectorHeader_t;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Vector_QualityCheck(void** ppContainer, size_t iElementSize, int iWantIndex, int iAssertToIndex)
{
    if(*ppContainer == nullptr)
    {
        size_t          iDefaultSize = sizeof(VectorHeader_t) + (iElementSize * STD_VECTOR_CAPACITY);
        VectorHeader_t* pHeader      = (VectorHeader_t*)malloc(iDefaultSize);

        pHeader->m_iSignature   = STD_VECTOR_SIGNATURE;
        pHeader->m_iCapacity    = STD_VECTOR_CAPACITY;
        pHeader->m_iSize        = 0;
        pHeader->m_iElementSize = iElementSize;

        // set up the container's pointer.
        *ppContainer = &pHeader[1];
    }


    void*           pData   = *ppContainer;
    VectorHeader_t* pHeader = &((VectorHeader_t*)pData)[-1];

    // Container valid?
    if(pHeader->m_iSignature != STD_VECTOR_SIGNATURE || pHeader->m_iElementSize != iElementSize)
    {
        assertion(false && "Invalid container");
        return;
    }

    // Valid index ?
    assertion(iWantIndex < (int32_t)pHeader->m_iSize && "Out of bound");

    // Reallcoate to fit up to "iAssertToIndex" size.
    if(pHeader->m_iCapacity <= iAssertToIndex)
    {
        pHeader->m_iCapacity      *= STD_VECTOR_GROWTH;
        VectorHeader_t* pNewHeader = (VectorHeader_t*)realloc(pHeader, sizeof(VectorHeader_t) + iElementSize * pHeader->m_iCapacity);
        *ppContainer               = &pNewHeader[1];
    }
}


#endif
