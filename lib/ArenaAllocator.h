//=========================================================================
//                      Arena Allocator
//=========================================================================
// by      : INSANE
// created : 22/02/2026
//
// purpose : Simple Arena Allocator in C.
//-------------------------------------------------------------------------
#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H



#include <stdint.h>
#include <malloc.h>
#include <stdbool.h>

#include "Assertion.h"
#include "Vector.h"


#define nullptr        ((void*)0)
#define STD_ARENA_SIZE (1024 * 4)


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
typedef struct Arena_t
{
    void*    m_pMemory;   // malloc-ed memory.
    uint32_t m_iSize;     // Total size of arena in bytes.
    uint32_t m_iUsedTill; // Bytes used in arena.
    
} Arena_t;


/* Allocate memory to arena and set up internal variables. Using an uninitialized arena
   WILL cause errors. */
static bool Arena_Initialize(Arena_t* pArena, uint32_t iArenaSize);

/* Get memory of a fixed size from arena. Returns 0 on failure. */
static void* Arena_Allocate(Arena_t* pArena, size_t nBytes);

/* Mark the arena as empty, but don't free memory. */
static void Arena_Clear(Arena_t* pArena);

/* Free arena's memory, and invalid all internal varibles. */
static void Arena_Free(Arena_t* pArena);

/* How much of this arena is used. */
static size_t Arena_GetSize(Arena_t* pArena);

/* Total capacity of this arena in bytes. ( used + unused ) */
static size_t Arena_Capacity(Arena_t* pArena);



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
typedef struct AAMallocInfo_t
{ 
    void*  m_pMemory;  // Pointer to malloc-ed memory.
    size_t m_iSize;    // Size of malloc-ed region.

} AAMallocInfo_t;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
typedef struct ArenaAllocator_t
{
    Arena_t* m_pArenas;    // ISTDLIB vector of Arena_t objects
    size_t   m_iArenaSize; // Common arena capacity in bytes.

    struct AAMallocInfo_t* m_pMallocs;

} ArenaAllocator_t;


/* Initialie ArenaAllocator with NARENA number of arenas with IARENASIZE capacity of each arena. */
static bool ArenaAllocator_Initialize(ArenaAllocator_t* pArenaAlloc, int nArenas, size_t iArenaSize);

/* Allocate NBYTES bytes in the first arena that can allcoate.
 * If NBYTES is more than arena capacity, than malloc and store its information.
 * If NBYTES is less than arena capacity but no arena can allocate NBYTES, push back new arena & allocate. */
static void* ArenaAllocator_Allocate(ArenaAllocator_t* pArenaAlloc, size_t nBytes);

/* Handles allocation requests where NBYTES is more than arena capacity. Mallocs NBYTES. */
static void* ArenaAllocator_Malloc(ArenaAllocator_t* pArenaAlloc, size_t nBytes);

/* Mark all arenas as empty. Doesn't modify malloc-ed entries in any way. */
static void ArenaAllocator_Clear(ArenaAllocator_t* pArenaAlloc);

/* Free all arenas & malloc-ed memories and uninitialize this ArenaAllocator. */
static void ArenaAllocator_Free(ArenaAllocator_t* pArenaAlloc);

/* Combined bytes consumed across all arenas in this ArenaAllocator. */
static size_t ArenaAllocator_Size(ArenaAllocator_t* pArenaAlloc);

/* Combined bytes consumed across all arenas + malloc-ed entries in this ArenaAllocator. */
static size_t ArenaAllocator_SizeAll(ArenaAllocator_t* pArenaAlloc);

/* Combined capacity across all arenas in this ArenaAllocator. */
static size_t ArenaAllocator_Capacity(ArenaAllocator_t* pArenaAlloc);

/* Size of one arena in bytes. All arenas are of same size. */
static size_t ArenaAllocator_ArenaCapacity(ArenaAllocator_t* pArenaAlloc);

/* Number of arenas in this ArenaAllocator. */
static size_t ArenaAllocator_ArenaCount(ArenaAllocator_t* pArenaAlloc);



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static bool Arena_Initialize(Arena_t* pArena, uint32_t iArenaSize)
{
    pArena->m_iSize     = iArenaSize == 0 ? STD_ARENA_SIZE : iArenaSize;
    pArena->m_iUsedTill = 0;
    pArena->m_pMemory   = malloc(iArenaSize);

    return pArena->m_pMemory != nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void* Arena_Allocate(Arena_t* pArena, size_t nBytes)
{
    uint32_t iCapacity = pArena->m_iSize - pArena->m_iUsedTill; // Bytes left.

    if(nBytes > iCapacity)
        return nullptr;

    void* pOutput = (void*)((uintptr_t)pArena->m_pMemory + (uintptr_t)pArena->m_iUsedTill);
    
    pArena->m_iUsedTill += nBytes;

    return pOutput;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Arena_Clear(Arena_t* pArena)
{
    pArena->m_iUsedTill = 0;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void Arena_Free(Arena_t* pArena)
{
    free(pArena->m_pMemory);

    pArena->m_iSize     = 0;
    pArena->m_iUsedTill = 0;
    pArena->m_pMemory   = nullptr;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static size_t Arena_GetSize(Arena_t* pArena)
{
    return (size_t)pArena->m_iUsedTill;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static size_t Arena_Capacity(Arena_t* pArena)
{
    return (size_t)pArena->m_iSize;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static bool ArenaAllocator_Initialize(ArenaAllocator_t* pArenaAlloc, int nArenas, size_t iArenaSize)
{
    // Arena count must be valid.
    assertion(nArenas > 0 && "Invalid Arena count");
    if(nArenas <= 0)
        return false;


    pArenaAlloc->m_pArenas    = NULL;
    pArenaAlloc->m_iArenaSize = iArenaSize;
    pArenaAlloc->m_pMallocs   = NULL;

    
    // PushBack and iniitalize arenas.
    bool bArenaInitFailed = false;
    for(int iArenaIndex = 0; iArenaIndex < nArenas; iArenaIndex++)
    {
        Arena_t iTempArena;
        Vector_PushBack(pArenaAlloc->m_pArenas, iTempArena);

        if(Arena_Initialize(Vector_Back(pArenaAlloc->m_pArenas), iArenaSize) == false)
        {
            bArenaInitFailed = true;
            break;
        }
    }


    // Incase Arena init failed, clean up and leave.
    if(bArenaInitFailed == true)
    {
        // if we have failed, the last entry must be bad.
        for(int iArenaIndex = 0; iArenaIndex < Vector_Len(pArenaAlloc->m_pArenas); iArenaIndex++)
        {
            Arena_t* pArena = &pArenaAlloc->m_pArenas[iArenaIndex];

            if(pArena->m_pMemory != nullptr)
                Arena_Free(pArena);
        }

        Vector_Free(pArenaAlloc->m_pArenas);
        return false;
    }


    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void* ArenaAllocator_Allocate(ArenaAllocator_t* pArenaAlloc, size_t nBytes)
{
    // Arena allcoator must be initialized.
    assertion(pArenaAlloc->m_pArenas != nullptr && "ArenaAllcator is uninitialized");
    if(pArenaAlloc->m_pArenas == nullptr)
        return nullptr;


    // nBytes is more than max arena capacity, malloc nBytes and return it.
    if(nBytes > pArenaAlloc->m_iArenaSize)
        return ArenaAllocator_Malloc(pArenaAlloc, nBytes);


    // Allocate from the first arena that can allocate.
    for(int iArenaIndex = 0; iArenaIndex < Vector_Len(pArenaAlloc->m_pArenas); iArenaIndex++)
    {
        Arena_t* pArena  = &pArenaAlloc->m_pArenas[iArenaIndex];
        void*    pMemory = Arena_Allocate(pArena, nBytes);

        if(pMemory != nullptr)
            return pMemory;
    }


    // In case no arena is capable of allocating, pushback new arena.
    Vector_PushBack(pArenaAlloc->m_pArenas, (Arena_t){0});
    if(Arena_Initialize(Vector_Back(pArenaAlloc->m_pArenas), pArenaAlloc->m_iArenaSize) == false)
        return nullptr;

    return Arena_Allocate(Vector_Back(pArenaAlloc->m_pArenas), nBytes);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void* ArenaAllocator_Malloc(ArenaAllocator_t* pArenaAlloc, size_t nBytes)
{
    AAMallocInfo_t mallocInfo = {0};

    mallocInfo.m_pMemory = malloc(nBytes);
    mallocInfo.m_iSize   = nBytes;

    if(mallocInfo.m_pMemory == nullptr)
        return nullptr;

    Vector_PushBack(pArenaAlloc->m_pMallocs, mallocInfo);
    return mallocInfo.m_pMemory;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void ArenaAllocator_Clear(ArenaAllocator_t* pArenaAlloc)
{
    for(int iArenaIndex = 0; iArenaIndex < Vector_Len(pArenaAlloc->m_pArenas); iArenaIndex++)
    {
        Arena_Clear(&pArenaAlloc->m_pArenas[iArenaIndex]);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void ArenaAllocator_Free(ArenaAllocator_t* pArenaAlloc)
{
    // Free all arenas.
    for(int iArenaIndex = 0; iArenaIndex < Vector_Len(pArenaAlloc->m_pArenas); iArenaIndex++)
    {
        Arena_Free(&pArenaAlloc->m_pArenas[iArenaIndex]);
    }

    // Free malloc-ed entries.
    for(int iMallocIndex = 0; iMallocIndex < Vector_Len(pArenaAlloc->m_pMallocs); iMallocIndex++)
    {
        free(pArenaAlloc->m_pMallocs[iMallocIndex].m_pMemory);
    }

    Vector_Free(pArenaAlloc->m_pArenas);
    Vector_Free(pArenaAlloc->m_pMallocs);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static size_t ArenaAllocator_Size(ArenaAllocator_t* pArenaAlloc)
{
    size_t iTotalSize = 0;

    for(int iArenaIndex = 0; iArenaIndex < Vector_Len(pArenaAlloc->m_pArenas); iArenaIndex++)
    {
        iTotalSize += Arena_GetSize(&pArenaAlloc->m_pArenas[iArenaIndex]);
    }

    return iTotalSize;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static size_t ArenaAllocator_SizeAll(ArenaAllocator_t* pArenaAlloc)
{
    size_t iTotalSize = ArenaAllocator_Size(pArenaAlloc);

    for(int iMallocIndex = 0; iMallocIndex < Vector_Len(pArenaAlloc->m_pMallocs); iMallocIndex++)
    {
        iTotalSize += pArenaAlloc->m_pMallocs[iMallocIndex].m_iSize;
    }

    return iTotalSize;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static size_t ArenaAllocator_Capacity(ArenaAllocator_t* pArenaAlloc)
{
    size_t iTotalCapacity = 0;

    for(int iArenaIndex = 0; iArenaIndex < Vector_Len(pArenaAlloc->m_pArenas); iArenaIndex++)
    {
        iTotalCapacity += Arena_Capacity(&pArenaAlloc->m_pArenas[iArenaIndex]);
    }

    return iTotalCapacity;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static size_t ArenaAllocator_ArenaCapacity(ArenaAllocator_t* pArenaAlloc)
{
    return pArenaAlloc->m_iArenaSize;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static size_t ArenaAllocator_ArenaCount(ArenaAllocator_t* pArenaAlloc)
{
    return Vector_Len(pArenaAlloc->m_pArenas);
}


#endif
