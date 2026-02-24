#include <stdio.h>
#include "../lib/ArenaAllocator.h"


static void TestArena();


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void CapacityPerArena(ArenaAllocator_t* pAA)
{
    for(int i = 0; i < Vector_Len(pAA->m_pArenas); i++)
    {
        size_t iUsed = Arena_GetSize(&pAA->m_pArenas[i]);
        size_t iCapacity = Arena_Capacity(&pAA->m_pArenas[i]);
        printf("Arena %d is filled %.2f used %zu / %zu bytes\n", i, (double)iUsed / (double)iCapacity, iUsed, iCapacity);
    }
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int main(int nArgs, char** szArgs)
{
    ArenaAllocator_t arenaAlloc;
    if(ArenaAllocator_Initialize(&arenaAlloc, 2, 1000) == false)
        return 1;


    size_t nArenas = ArenaAllocator_ArenaCount(&arenaAlloc);
    for(int i = 0; i < 10; i++)
    {
        void* pMemory = ArenaAllocator_Allocate(&arenaAlloc, 200);
        printf("Allcoated memory @ %p\n", pMemory);

        if(ArenaAllocator_ArenaCount(&arenaAlloc) != nArenas)
            break;
    }
    printf("size : %zu, capacity : %zu, Arena Count : %zu\n", 
            ArenaAllocator_SizeAll(&arenaAlloc), ArenaAllocator_Capacity(&arenaAlloc),
            ArenaAllocator_ArenaCount(&arenaAlloc));

    CapacityPerArena(&arenaAlloc);

    ArenaAllocator_Clear(&arenaAlloc);

    printf("size : %zu, capacity : %zu, Arena Count : %zu\n", 
            ArenaAllocator_SizeAll(&arenaAlloc), ArenaAllocator_Capacity(&arenaAlloc),
            ArenaAllocator_ArenaCount(&arenaAlloc));

    ArenaAllocator_Free(&arenaAlloc);
    return 0;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static void TestArena()
{
    Arena_t arena;
    Arena_Initialize(&arena, STD_ARENA_SIZE);
    printf("Arena initialized\n");


    printf("Arena allcoated @ %p\n", arena.m_pMemory);
    while(1)
    {
        void* pMemory = Arena_Allocate(&arena, 100);
        printf("Allocated @ %p\n", pMemory);
        if(pMemory == nullptr)
            break;
    }
    printf("Arena size : %zu, Capacity : %zu\n", Arena_GetSize(&arena), Arena_Capacity(&arena));

    // clearing and reusing.
    Arena_Clear(&arena);
    printf("Arena size : %zu, Capacity : %zu\n", Arena_GetSize(&arena), Arena_Capacity(&arena));
    while(1)
    {
        void* pMemory = Arena_Allocate(&arena, 100);
        printf("Allocated @ %p\n", pMemory);
        if(pMemory == nullptr)
            break;
    }
    printf("Arena size : %zu, Capacity : %zu\n", Arena_GetSize(&arena), Arena_Capacity(&arena));


    Arena_Free(&arena);
    printf("Arena uninitialized\n");
}
