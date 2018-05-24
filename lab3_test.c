#include "types.h"
#include "user.h"

int test(int n) { 
    int x = n + 1;
    return x;
}

int test2(int n) { 
    int a[1100] ={0};
    a[1099]=n;
    printf(1, "Test 2: array a is at %p\n", a);
    printf(1, "Test 2: array a is at %p\n", &a[1099]);
    return a[1099];
}

int main(int argc, char *argv[]) {
    printf(1, "####################################################\n");
    printf(1, "# This program tests the correctness of your lab #3\n");
    printf(1, "####################################################\n");

    printf(1, "Test 1: Simple test\n");
    int pid = 0;
    pid = fork();
    if(pid == 0){
        test(1);
        exit();
    }
    wait();

    printf(1, "Test 2: Stack growth test.\n");
    
    printf(1, "Test 2: trap handled return %d\n", test2(2));
    exit();
}

