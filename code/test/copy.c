#include "syscall.h" 
#define maxlen 255 
int main() 
{ 
    int id1,id2;
    int length;
    char filename[50];
    char filename2[50];
    char buffer[50];
    // ReadString(filename,maxlen);
    // ReadString(filename2,maxlen);
    id1=Open("hello1.txt",1);
    id2=Open("hello2.txt",0);
    if(id1 != -1&& id2 != -1){
        length=Seek(-1,id1);
        Seek(0,id1);
        Read(buffer,length,id1);
        Write(buffer,length,id2);
    }
}
