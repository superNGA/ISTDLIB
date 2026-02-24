#include "../lib/Vector.h"
#include <stdio.h>


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
int main(int nArgs, char** szArgs)
{
    int* v = NULL;

    // push_back test.
    for(int i = 0; i < 100; i++)
    {
        Vector_PushBack(v, i);
    }

    printf("Vector size : %u, Capacity : %u\n", Vector_Len(v), Vector_Capacity(v));

    for(int i = 0; i < Vector_Len(v); i++)
        printf("%d, ", v[i]);
    printf("\n\n");


    // Insert test.
    Vector_Insert(v, 5, 69);

    printf("Vector size : %u, Capacity : %u\n", Vector_Len(v), Vector_Capacity(v));

    for(int i = 0; i < Vector_Len(v); i++)
        printf("%d, ", v[i]);
    printf("\n\n");

    
    // Shrink test.
    Vector_ShrinkToFit(v);

    printf("Vector size : %u, Capacity : %u\n", Vector_Len(v), Vector_Capacity(v));
    for(int i = 0; i < Vector_Len(v); i++)
        printf("%d, ", v[i]);
    printf("\n\n");


    // Erase test.
    Vector_Erase(v, 5);

    printf("Vector size : %u, Capacity : %u\n", Vector_Len(v), Vector_Capacity(v));
    for(int i = 0; i < Vector_Len(v); i++)
        printf("%d, ", v[i]);
    printf("\n\n");


    // Insert test again.
    for(int i = 0; i <= 10; i++)
    {
        Vector_Insert(v, i * 10, i * 100);
        printf("Inserted @ index : %d, Vector size : %u, Capacity : %u\n", i * 10, Vector_Len(v), Vector_Capacity(v));
    }
    for(int i = 0; i < Vector_Len(v); i++)
        printf("%d, ", v[i]);
    printf("\n\n");


    // erase test again.
    for(int i = 10; i >= 0; i--)
    {
        Vector_Erase(v, i * 10);
        printf("Erased @ index : %d, Vector size : %u, Capacity : %u\n", i * 10, Vector_Len(v), Vector_Capacity(v));
    }
    for(int i = 0; i < Vector_Len(v); i++)
        printf("%d, ", v[i]);
    printf("\n\n");


    // Back & front.
    printf("Back : %d, Front : %d\n", *Vector_Back(v), *Vector_Front(v));


    // Empty & clear
    printf("Empty : %d\n", Vector_Empty(v));
    Vector_Clear(v);
    printf("Empty : %d\n", Vector_Empty(v));


    // Resize
    Vector_Resize(v, 100);
    printf("Size : %u, Capacity : %u\n", Vector_Len(v), Vector_Capacity(v));


    // Reserve
    Vector_Reserve(v, 200);
    printf("Size : %u, Capacity : %u\n", Vector_Len(v), Vector_Capacity(v));


    // Shrink
    Vector_ShrinkToFit(v);
    printf("Size : %u, Capacity : %u\n", Vector_Len(v), Vector_Capacity(v));


    // free
    printf("Container : %p\n", v);
    Vector_Free(v);
    printf("Container : %p\n", v);

    return 0;
}
