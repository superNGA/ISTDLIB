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
#include <stdbool.h>

#define ENABLE_ASSERTIONS 1
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



/* Get VectorHeader_t pointer from given vector. */
#define Vector_GetHeader(Container) (&((VectorHeader_t*)Container)[-1])


/* Index of the last stored element + 1. Zero in case no elements. */
#define Vector_Len(Container) (Container == nullptr ? 0 : Vector_GetHeader(Container)->m_iSize)


/* Gets vector's capacity, i.e. number of elements that it can hold. */
#define Vector_Capacity(Container) (Container == nullptr ? 0 : Vector_GetHeader(Container)->m_iCapacity)


/* returns zero if vector is empty, else non-zero value. */
#define Vector_Empty(Container) Vector_Len(Container)


/* returns pointer to last element in the vector. Make sure vector is valid and not empty. */
#define Vector_Back(Container) (&Container[Vector_Len(Container) - 1])


/* returns pointer to first element in the vector. Make sure vector is valid and not empty. */
#define Vector_Front(Container) (&Container[0])


/* Insert element at any index. */
#define Vector_Insert(Container, Index, Data) do{                                           \
    Vector_AssertInit((void**)&Container, sizeof(*Container));                              \
    int             iIndex       = (Index);                                                 \
    VectorHeader_t* pHeader      = Vector_GetHeader(Container);                             \
    int             iAssertIndex = iIndex > pHeader->m_iSize ? iIndex : pHeader->m_iSize;   \
    Vector_MayGrowToIndex((void**)&Container, sizeof(*Container), iAssertIndex);            \
    Vector_VerifyRequest ((void**)&Container, sizeof(*Container), iIndex, true);            \
    uintptr_t pData     = ((uintptr_t)(Container));                                         \
    pHeader             = Vector_GetHeader(Container);                                      \
    uintptr_t pMoveAdrs = pData + (sizeof(*Container) * (uintptr_t)iIndex);                 \
    if(iIndex < pHeader->m_iSize)                                                           \
        memmove((void*)(pMoveAdrs + sizeof(*Container)), ((void*)pMoveAdrs),                \
                (pHeader->m_iSize - iIndex) * sizeof(*Container));                          \
    Container[iIndex] = (Data);                                                             \
    pHeader->m_iSize  = iIndex <= pHeader->m_iSize ? pHeader->m_iSize + 1 : iIndex + 1;     \
} while(0)


/* Erase element at any index. */
#define Vector_Erase(Container, Index) do{                                                        \
    Vector_AssertInit   ((void**)&Container, sizeof(*Container));                                 \
    Vector_VerifyRequest((void**)&Container, sizeof(*Container), (Index), true);                  \
    int             iIndex    = (Index);                                                          \
    VectorHeader_t* pHeader   = Vector_GetHeader(Container);                                      \
    uintptr_t       pMoveAdrs = (uintptr_t)Container + (sizeof(*Container) * (uintptr_t)iIndex);  \
    assertion(pHeader->m_iSize > 0      && "Empty vector");                                       \
    assertion(iIndex < pHeader->m_iSize && "Trying to erase out of bound entry");                 \
    memmove((void*)(pMoveAdrs), (void*)(pMoveAdrs + sizeof(*Container)),                          \
            (pHeader->m_iSize - iIndex - 1) * sizeof(*Container));                                \
    pHeader->m_iSize -= 1;                                                                        \
} while(0)


/* Verify & Free this vector. */
#define Vector_Free(Container) do{                                               \
    if(Container != nullptr)                                                     \
    {                                                                            \
        Vector_VerifyRequest((void**)&Container, sizeof(*Container), 0, false);  \
        free(Vector_GetHeader(Container));                                       \
    }                                                                            \
    Container = nullptr;                                                         \
} while(0)


/* Set element count as 0 for this vector. */
#define Vector_Clear(Container) do{                \
    if(Container != nullptr)                       \
        Vector_GetHeader(Container)->m_iSize = 0;  \
} while(0)


/* Simply put an element @ the end of the vector. */
#define Vector_PushBack(Container, Data) Vector_Insert(Container, Vector_Len(Container), Data)


/* Remove an element from the end of this vector. */
#define Vector_PopBack(Container) do{                                       \
    Vector_VerifyRequest((void**)&Container, sizeof(*Container), 0, true);  \
    VectorHeader_t* pHeader = Vector_GetHeader(Container);                  \
    assertion(pHeader->m_iSize > 0 && "Empty vector");                      \
    pHeader->m_iSize -= 1;                                                  \
} while(0)


/* Increase capacity to hold SIZE elements. */
#define Vector_Reserve(Container, Size) do{                                    \
    Vector_AssertInit   ((void**)&Container, sizeof(*Container));              \
    Vector_VerifyRequest((void**)&Container, sizeof(*Container), 0, false);    \
    if(Size > 0)                                                               \
        Vector_GrowToIndex((void**)&Container, sizeof(*Container), Size - 1);  \
} while(0)


/* Increase capacity to SIZE elements and set vector size to SIZE. */
#define Vector_Resize(Container, Size) do{         \
    Vector_Reserve(Container, Size);               \
    Vector_GetHeader(Container)->m_iSize = Size;   \
} while(0)


/* Reallocate and set capacity to vector size. */
#define Vector_ShrinkToFit(Container) do{                        \
    Vector_AssertInit  ((void**)&Container, sizeof(*Container)); \
    Vector_ShrinkToSize((void**)&Container, sizeof(*Container)); \
} while(0)



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
typedef struct VectorHeader_t
{
    uint32_t m_iSize;        // Biggest index at which an element is stored. + 1.
    uint32_t m_iCapacity;    // Total elements that can be stored. ( malloced size / element size )
    uint32_t m_iElementSize; // Size of one element.
    uint32_t m_iSignature;   // Signature to help us prevent mistakes.

} VectorHeader_t;


/* If container (*ppContainer) is nullptr, initialize it. */
static void Vector_AssertInit(void** ppContainer, size_t iElementSize);

/* Grow Container capacity by growth rate untill we can accomodate IASSERTTOINDEX index. */
static void Vector_MayGrowToIndex(void** ppContainer, size_t iElementSize, int iAssertToIndex);

/* Realloc this Contianer to it can accomodate IINDEX index. */
static void Vector_GrowToIndex(void** ppContainer, size_t iElementSize, int iIndex);

/* Makes sure Container is not nullptr. 
 * Makes sure Container is Initialized correctly and its header's signature matches. 
 * Makes sure vector has capacity to accomodate IWANTINDEX index.
 * Makes sure IWANTINDEX is not negative. */
static void Vector_VerifyRequest(void** ppContainer, size_t iElementSize, int iWantIndex, bool bAssertCapacity);

/* Realloc this Container so new capacity = size. */
static void Vector_ShrinkToSize(void** ppContainer, size_t iElementSize);



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

    Vector_VerifyRequest(ppContainer, iElementSize, 0, false);

    // Reallcoate to fit up to "iAssertToIndex" size.
    while(pHeader->m_iCapacity <= iAssertToIndex)
    {
        // Old size & Capacity
        uint32_t iOldCapacity      = pHeader->m_iCapacity;
        size_t   iOldSize          = sizeof(VectorHeader_t) + iElementSize * pHeader->m_iCapacity;

        // why 0 capacity tho?
        if(pHeader->m_iCapacity == 0)
            pHeader->m_iCapacity = STD_VECTOR_CAPACITY;

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

    Vector_VerifyRequest(ppContainer, iElementSize, 0, false);

    // We already have requested capacity.
    if(pHeader->m_iCapacity > iIndex)
        return;

    // Old size & new size.
    size_t iOldSize      = sizeof(VectorHeader_t) + iElementSize * pHeader->m_iCapacity;
    pHeader->m_iCapacity = iIndex + 1;
    size_t iNewSize      = sizeof(VectorHeader_t) + iElementSize * pHeader->m_iCapacity;
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
static void Vector_VerifyRequest(void** ppContainer, size_t iElementSize, int iWantIndex, bool bAssertCapacity)
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
    if(bAssertCapacity == true)
        assertion(iWantIndex >= 0 && iWantIndex < (int32_t)pHeader->m_iCapacity && "Out of bound");
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Vector_ShrinkToSize(void** ppContainer, size_t iElementSize)
{
    Vector_VerifyRequest(ppContainer, iElementSize, 0, true);

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
