#include <stdio.h>

int main() {    
    char *arr[3][4] = {
        {1, "Dhaval", 3, 4},
        {5, 6, "abc", 8},
        {1, 10, 11, 12}
    };

    printf( "%c", *(*(*(arr+0) + 1) + 2 ) );

    return 0;
}
