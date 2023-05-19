#include "syscall.h"
#include "copyright.h"
#define ConsoleInput 0
#define ConsoleOutput 1

void main(){
    SpaceId newProc1, newProc2;
    int ec1,ec2;
    newProc1 = Exec("a");
    newProc2 = Exec("b");
    ec1 = Join(newProc1);
    ec2 = Join(newProc2);
    Halt();
}

