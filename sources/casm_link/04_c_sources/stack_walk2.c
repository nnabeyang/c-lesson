#include <stdio.h>

void print_address(int address) {
    printf("address: %x\n", address);
}

void print_msg(char *str) {
    printf("We get (%s)\n", str);
}
//#define INVESTIGATE 1
#undef INVESTIGATE

void func3 () {
    char *target;
#ifdef INVESTIGATE
   printf("target address = %x\n", &target);
    print_address(*(int*)((int)&target + 4)); // func2's r11    
    print_address(*(int*)(*(int*)((int)&target + 4))); // func1's r11  
    print_address(*(int*)(*(int*)(*(int*)((int)&target + 4)))); // main's r11
#else
    printf("we got main_msg value, \"%s\"\n", *(char**)(*(int*)(*(int*)(*(int*)((int)&target + 4))) - 8));
#endif
    printf("We are in func3\n");
}

void func2() {
    char *msg = "func2 message.";
#ifdef INVESTIGATE    
    printf("func1's r11 address = %x\n", *(int*)((int)&msg + 4));
#endif    
    printf("We are in func2, %s\n", msg);
    func3();
}

void func1() {
    char *msg = "func1 msg";    
#ifdef INVESTIGATE    
    printf("main's r11 address = %x\n", *(int*)((int)&msg + 4));
#endif    
    func2();
}


int main() {
    char *main_msg = "We are in main.";
#ifdef INVESTIGATE     
    printf("main main_msg's address = %x\n", &main_msg);
#endif
    printf("We are in main, %s\n", main_msg);
    func1();
    return 0;
}

