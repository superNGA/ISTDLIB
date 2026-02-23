//=========================================================================
//                      Arena Allocator
//=========================================================================
// by      : INSANE
// created : 22/02/2026
//
// purpose : Simple Arena Allocator in C.
//-------------------------------------------------------------------------
#ifndef ARENA_ALLCOATOR_H
#define ARENA_ALLCOATOR_H



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
    uint32_t m_iSize;     // Total size of arena.
    uint32_t m_iUsedTill; // Bytes used in arena.
    
} Arena_t;


/* Allocate memory to arena and set up internal variables. Using an uninitialized arena
   WILL cause errors. */
static bool Arena_Initialize(Arena_t* pArena, uint32_t iArenaSize = STD_ARENA_SIZE);

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
typedef struct ArenaAllocator_t
{
    Arena_t* m_pArenas;
    int32_t  m_nArenas;

} ArenaAllocator_t;


static bool ArenaAllocator_Initalize(ArenaAllocator_t* pArenaAlloc, int nArenas = 1);
static void* ArenaAllocator_Allocate(ArenaAllocator_t* pArenaAlloc, size_t nBytes);
static bool ArenaAllocator_Clear();
static bool ArenaAllocator_Free();
static size_t ArenaAllocator_Size();
static size_t ArenaAllocator_Capacity();



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static bool Arena_Initialize(Arena_t* pArena, uint32_t iArenaSize)
{
    pArena->m_iSize     = iArenaSize;
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

    void* pOutput = (void*)((uintptr_t)pArena->m_pMemory + (uint64_t)pArena->m_iUsedTill);
    
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
static bool ArenaAllocator_Initalize(ArenaAllocator_t* pArenaAlloc, int nArenas)
{
    assertion(nArenas >= 0 && "Invalid arena count");

    pArenaAlloc->m_pArenas = (Arena_t*)malloc(sizeof(Arena_t) * nArenas);
    if(pArenaAlloc->m_pArenas == nullptr)
        return false;

    pArenaAlloc->m_nArenas = nArenas;


    int nArenasDone = 0; // Number of arenas initialized successfully.

    for(int iArenaIndex = 0; iArenaIndex < pArenaAlloc->m_nArenas; iArenaIndex++)
    {
        if(Arena_Initialize(&pArenaAlloc->m_pArenas[iArenaIndex]) == false)
            break;

        nArenasDone++;
    }


    // In case we failed to initialize any arena, free all arenas and fail.
    if(nArenasDone != pArenaAlloc->m_nArenas)
    {
        for(int iArenaIndex = 0; iArenaIndex < nArenasDone; iArenaIndex++)
        {
            Arena_Free(&pArenaAlloc->m_pArenas[iArenaIndex]);
        }

        free(pArenaAlloc->m_pArenas);
        pArenaAlloc->m_nArenas = 0;

        return false;
    }


    return true;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void* ArenaAllocator_Allocate(ArenaAllocator_t* pArenaAlloc, size_t nBytes)
{
    void* pMem = nullptr;

    for(int iArenaIndex = 0; iArenaIndex < pArenaAlloc->m_nArenas; iArenaIndex++)
    {
        pMem = Arena_Allocate(&pArenaAlloc->m_pArenas[iArenaIndex], nBytes);
        if(pMem != nullptr)
            break;
    }

    // If we managed to find some memory in current arenas, return it.
    if(pMem != nullptr)
        return pMem;

    // else we need to get some more arenas.
    uint32_t iOldArenaCount = pArenaAlloc->m_nArenas;
    pArenaAlloc->m_nArenas *= 2;
    pArenaAlloc->m_pArenas  = (Arena_t*)realloc(pArenaAlloc->m_pArenas, pArenaAlloc->m_nArenas * sizeof(Arena_t));
    if(pArenaAlloc->m_pArenas == nullptr) // Failed to allocate more memory.
        return nullptr;

    // Now we need to initialize all new arenas.
    int nArenasDone = 0; // Number of arenas initialized successfully.

    for(int iArenaIndex = iOldArenaCount; iArenaIndex < pArenaAlloc->m_nArenas; iArenaIndex++)
    {
        if(Arena_Initialize(&pArenaAlloc->m_pArenas[iArenaIndex]) == false)
            break;

        nArenasDone++;
    }


    // In case we failed to initialize any arena, free all arenas and fail.
    if(nArenasDone != pArenaAlloc->m_nArenas - iOldArenaCount)
    {
        for(int iArenaIndex = iOldArenaCount; iArenaIndex < iOldArenaCount + nArenasDone; iArenaIndex++)
        {
            Arena_Free(&pArenaAlloc->m_pArenas[iArenaIndex]);
        }

        free(pArenaAlloc->m_pArenas);
        pArenaAlloc->m_nArenas = 0;

        return nullptr;
    }


    return pMem;
}


static bool ArenaAllocator_Clear();
static bool ArenaAllocator_Free();
static size_t ArenaAllocator_Size();
static size_t ArenaAllocator_Capacity();


#endif
