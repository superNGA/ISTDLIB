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
#include <string.h>
#include "Assertion.h"


#define nullptr              ((void*)0)
#define STD_VECTOR_CAPACITY  (10)
#define STD_VECTOR_SIGNATURE (0xBED0DECA)
#define STD_VECTOR_GROWTH    (2)

/*

Vector Structure : 
    [ Header ][ Entry 0 ][ Entry 1 ][ Entry 2 ]...

Header holds metadata about this Vector, like size, capacity n stuff.

*/

// erase @ index
// pop back
// Back()
// Front()
// Empty()


/* Get VectorHeader_t pointer from given vector. */
#define Vector_GetHeader(Container) (&((VectorHeader_t*)Container)[-1])

/* Index of the last stored element + 1. Zero in case no elements. */
#define Vector_Len(Container) (Container == nullptr ? 0 : Vector_GetHeader(Container)->m_iSize)

/* Gets vector's capacity, i.e. number of elements that it can hold. */
#define Vector_Capacity(Container) (Container == nullptr ? 0 : Vector_GetHeader(Container)->m_iCapacity)

/* Insert element at any index. */
#define Vector_Insert(Container, Index, Data) {                                      \
    Vector_AssertInit((void**)&Container, sizeof(*Container));                       \
    int             iIndex  = (Index);                                               \
    Vector_MayGrowToIndex((void**)&Container, sizeof(*Container), iIndex);           \
    Vector_VerifyRequest ((void**)&Container, sizeof(*Container), iIndex);           \
    uintptr_t       pData     = ((uintptr_t)(Container));                            \
    VectorHeader_t* pHeader   = Vector_GetHeader(Container);                         \
    uintptr_t       pMoveAdrs = pData + (sizeof(*Container) * (uintptr_t)iIndex);    \
    if(iIndex < pHeader->m_iSize)                                                    \
        memmove((void*)(pMoveAdrs + sizeof(*Container)), ((void*)pMoveAdrs),         \
                (pHeader->m_iSize - iIndex) * sizeof(*Container));                   \
    Container[iIndex] = (Data);                                                      \
    pHeader->m_iSize  = iIndex <= pHeader->m_iSize ? pHeader->m_iSize + 1 : iIndex;  \
}

/* Erase element at any index. */
#define Vector_Erase(Container, Index) {\
}

/* Verify & Free this vector. */
#define Vector_Free(Container) {                                       \
    if(Container != nullptr)                                           \
    {                                                                  \
        Vector_VerifyRequest((void**)&Container, sizeof(*Container));  \
        free(Vector_GetHeader(Container));                             \
    }                                                                  \
    Container = nullptr;                                               \
}

/* Set element count as 0 for this vector. */
#define Vector_Clear(Container) {                  \
    if(Container != nullptr)                       \
        Vector_GetHeader(Container)->m_iSize = 0;  \
}

/* Simply put an element @ the end of the vector. */
#define Vector_PushBack(Container, Data) Vector_Insert(Container, Vector_Len(Container), Data)

/* Remove an element from the end of this vector. */
#define Vector_PopBack(Container) {                                   \
    Vector_VerifyRequest((void**)&Container, sizeof(*Container), 0);  \
    VectorHeader_t* pHeader = Vector_GetHeader(Container);            \
    assertion(pHeader->m_iSize > 0 && "Empty vector");                \
    pHeader->m_iSize -= 1;                                            \
}

/* Increase capacity to hold SIZE elements. */
#define Vector_Reserve(Container, Size) {                                      \
    Vector_AssertInit   ((void**)&Container, sizeof(*Container));              \
    Vector_VerifyRequest((void**)&Container, sizeof(*Container), -1);          \
    if(Size > 0)                                                               \
        Vector_GrowToIndex((void**)&Container, sizeof(*Container), Size - 1);  \
}

/* Increase capacity to SIZE elements and set vector size to SIZE. */
#define Vector_Resize(Container, Size) {           \
    Vector_Reserve(Container, Size);               \
    Vector_GetHeader(Container)->m_iSize = Size;   \
}

/* Reallocate and set capacity to vector size. */
#define Vector_ShrinkToFit(Container) {                          \
    Vector_AssertInit  ((void**)&Container, sizeof(*Container)); \
    Vector_ShrinkToSize((void**)&Container, sizeof(*Container)); \
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
typedef struct VectorHeader_t
{
    uint32_t m_iSize;        // Biggest index at which an element is stored. + 1.
    uint32_t m_iCapacity;    // Total elements that can be stored. ( malloced size / element size )
    uint32_t m_iElementSize; // Size of one element.
    uint32_t m_iSignature;   // Signature to help us prevent mistakes.

} VectorHeader_t;



static void Vector_AssertInit    (void** ppContainer, size_t iElementSize);
static void Vector_MayGrowToIndex(void** ppContainer, size_t iElementSize, int iAssertToIndex);
static void Vector_GrowToIndex   (void** ppContainer, size_t iElementSize, int iIndex);

/* Makes sure Container is not nullptr. 
 * Makes sure Container is Initialized correctly and its header's signature matches. 
 * Makes sure vector has capacity to accomodate IWANTINDEX index. */
static void Vector_VerifyRequest (void** ppContainer, size_t iElementSize, int iWantIndex);
static void Vector_ShrinkToSize  (void** ppContainer, size_t iElementSize);



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Vector_AssertInit(void** ppContainer, size_t iElementSize)
{
    if(*ppContainer == nullptr)
    {
        size_t          iDefaultSize = sizeof(VectorHeader_t) + (iElementSize * STD_VECTOR_CAPACITY);
        VectorHeader_t* pHeader      = (VectorHeader_t*)malloc(iDefaultSize);
        memset(pHeader, 0, iDefaultSize);

        pHeader->m_iSignature   = STD_VECTOR_SIGNATURE;
        pHeader->m_iCapacity    = STD_VECTOR_CAPACITY;
        pHeader->m_iSize        = 0;
        pHeader->m_iElementSize = iElementSize;

        // set up the container's pointer.
        *ppContainer = &pHeader[1];
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Vector_MayGrowToIndex(void** ppContainer, size_t iElementSize, int iAssertToIndex)
{
    void*           pData   = *ppContainer;
    VectorHeader_t* pHeader = &((VectorHeader_t*)pData)[-1];

    Vector_VerifyRequest(ppContainer, iElementSize, -1);

    // Reallcoate to fit up to "iAssertToIndex" size.
    while(pHeader->m_iCapacity <= iAssertToIndex)
    {
        // Old size & Capacity
        uint32_t iOldCapacity      = pHeader->m_iCapacity;
        size_t   iOldSize          = sizeof(VectorHeader_t) + iElementSize * pHeader->m_iCapacity;

        // Scaled size & Capacity.
        pHeader->m_iCapacity      *= (uint32_t)STD_VECTOR_GROWTH;
        size_t iNewSize            = sizeof(VectorHeader_t) + iElementSize * pHeader->m_iCapacity;

        // Reallcoate.
        VectorHeader_t* pNewHeader = (VectorHeader_t*)realloc(pHeader, iNewSize);
        *ppContainer               = &pNewHeader[1];

        // Refresh this in case we got a different memory region.
        pHeader                    = pNewHeader;
        pData                      = *ppContainer;

        assertion(pNewHeader != nullptr && "You made realloc() fail. Consider touching some grass now.");

        // Initialize new memory region to 0
        memset((void*)((uintptr_t)pNewHeader + iOldSize), 0, iNewSize - iOldSize);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Vector_GrowToIndex(void** ppContainer, size_t iElementSize, int iIndex)
{
    void*           pData   = *ppContainer;
    VectorHeader_t* pHeader = &((VectorHeader_t*)pData)[-1];

    Vector_VerifyRequest(ppContainer, iElementSize, -1);

    // We already have requested capacity.
    if(pHeader->m_iCapacity > iIndex)
        return;

    // Old size & new size.
    size_t iOldSize = sizeof(VectorHeader_t) + iElementSize * pHeader->m_iCapacity;
    size_t iNewSize = sizeof(VectorHeader_t) + iElementSize * (iIndex + 1);
    assertion(iNewSize > iOldSize && "New size is smaller than old size.");

    // realloc.
    VectorHeader_t* pNewHeader = (VectorHeader_t*)realloc(pHeader, iNewSize);
    assertion(pNewHeader != nullptr && "You made realloc() fail. Consider touching some grass now.");

    // Initialize new memory to 0.
    memset((void*)((uintptr_t)pNewHeader + iOldSize), 0, iNewSize - iOldSize);

    // set container to new memory.
    *ppContainer = &pNewHeader[1];
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Vector_VerifyRequest(void** ppContainer, size_t iElementSize, int iWantIndex)
{
    assertion(*ppContainer != nullptr && "Uninitialized container");

    void*           pData   = *ppContainer;
    VectorHeader_t* pHeader = &((VectorHeader_t*)pData)[-1];

    // Container valid?
    if(pHeader->m_iSignature != STD_VECTOR_SIGNATURE || pHeader->m_iElementSize != iElementSize)
    {
        assertion(false && "Invalid container");
        return;
    }

    // Container corrupted ( holds invalid metadata )?
    assertion(pHeader->m_iSize <= pHeader->m_iCapacity && "Vector has Corrupted Metadata");

    // Valid index ?
    assertion(iWantIndex < (int32_t)pHeader->m_iCapacity && "Out of bound");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Vector_ShrinkToSize(void** ppContainer, size_t iElementSize)
{
    Vector_VerifyRequest(ppContainer, iElementSize, -1);

    VectorHeader_t* pHeader    = Vector_GetHeader(*ppContainer);

    // Realloc to size.
    size_t          iNewSize   = sizeof(VectorHeader_t) + iElementSize * pHeader->m_iSize;
    VectorHeader_t* pNewHeader = (VectorHeader_t*)realloc(pHeader, iNewSize);

    assertion(pNewHeader != nullptr && "You made realloc() fail. Consider touching some grass now.");

    *ppContainer         = &pNewHeader[1];
    pHeader              = Vector_GetHeader(*ppContainer);
    pHeader->m_iCapacity = pHeader->m_iSize;
}


#endif
