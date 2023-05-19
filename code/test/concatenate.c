#include "syscall.h" 
#define maxlen 255 
int main() 
{ 
    int id1,id2;
    int length1,length2;
    char filename[50];
    char filename2[50];
    char buffer[50];
    char buffer2[50];
    ReadString(filename,maxlen);
    ReadString(filename2,maxlen);
    id1=Open(filename,1);
    id2=Open(filename2,0);
    if(id1 != -1&& id2 != -1){
        length1=Seek(-1,id1);
        Seek(0,id1);
        Read(buffer,length1,id1);

        length2=Seek(-1,id2);
        Seek(0,id2);
        Read(buffer2,length2,id2);
        
        Seek(0,id2);
        Write(buffer,length1,id2);
        Seek(-1,id2);
        Write(buffer2,length2,id2);
    }
    Halt();
}
