#include "syscall.h" 
#define maxlen 255 
int main() 
{ 
    int id1;
    int length;
    char filename[50];
    char buffer[50];
    ReadString(filename,maxlen);
    id1=Open(filename,0);
    if(id1!=-1){
        Close(id1);
        Remove(filename);
    }
    else{
        Remove(filename);
    }

  
    Halt();
}