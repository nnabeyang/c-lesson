#include <stdio.h>

int sum_range(int beg, int end) {
    int sum=0;
    for(int i = beg; i <= end; i++) {
        sum+=i;
    }
    return sum;
}

int sum_range_inline(int beg, int end) {
    int res;
    /*
    Use r2 as sum.
    Use r3 as loop counter.
    */
    asm("mov r2, #0");
    asm("mov r3, r0");
    asm("loop:");
    asm("cmp r3, r1");
    asm("bgt end");
    asm("add r2, r2, r3");// sum+=i
    asm("add r3, r3, #1");// i++
    asm("b loop");
    asm("end:");
    asm("mov %0, r2" :"=r"(res));

    return res;
}


/*
test code
*/


void assert_true(int boolflag) {
    if(!boolflag) {
        printf("assert fail\n");
    }
}


int main() {
    int res;
    
    res = sum_range(3, 10);
    assert_true(res == 52);

    // TODO: fix sum_range_inline to pass test.
    res = sum_range_inline(3, 10);
    assert_true(res == 52);

    return 0;
}

