#include "syscall.h" 
#define maxlen 255 
int main() 
{ 
int fileid;
int length;
char filename[maxlen];
char buffer[maxlen];
//ReadString(filename,maxlen);
fileid = Open("hello1.txt",1);
if(fileid != -1){
    length = Seek(-1,fileid);
    Seek(0,fileid);
    Read(buffer,length,fileid);
    PrintString(buffer);
    PrintString("\n");
}
} 
