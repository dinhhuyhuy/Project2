#include "syscall.h"
#include "copyright.h"

void main(){
    SpaceId newProc1, newProc2;
    int ec1,ec2;
    newProc1 = Exec("cat");
    newProc2 = Exec("copy");
    ec1 = Join(newProc1);
    ec2 = Join(newProc2);
    Halt();
}