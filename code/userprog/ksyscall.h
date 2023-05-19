/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
#include "synchconsole.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <bits/basic_file.h>
#include <unistd.h>
#include <arpa/inet.h>

struct sockaddr_in serv_addr;
void SysHalt()
{
  
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}


char SysReadChar() { return kernel->synchConsoleIn->GetChar(); }

void SysPrintChar(char character) {
    kernel->synchConsoleOut->PutChar(character);
}

char* SysReadString(int length) {
  int i=0;
    char* buffer = new char[length + 1];
    buffer[0] = SysReadChar();
    while(buffer[i]!='\n'){
      ++i;
      buffer[i] = SysReadChar();
    }
    buffer[i] = '\0';
    return buffer;
}

void SysPrintString(char* buffer, int length) {
    for (int i = 0; i < length; i++) {
        kernel->synchConsoleOut->PutChar(buffer[i]);
    }
}

// int FindFreeSlot(OpenFile** file)
// {
// 	for(int i = 0; i < 20; i++)
// 	{
// 		if(file[i] == NULL) return i;		
// 	}
// }
int CheckConnection(int socketfd){
    int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (socketfd, SOL_SOCKET, SO_ERROR, &error, &len);
    if(retval != 0) return -1;
    return 0;
}
int SysSocketTCP(){
    int socketId;
    //int freeSlot = FindFreeSlot(kernel->fileSystem->socketTable);
    int freeSlot;
    freeSlot=kernel->fileSystem->socketFreeSlot();
    if(freeSlot == -1) return -1;

    socketId = socket(AF_INET, SOCK_STREAM, 0);
    if(socketId == -1) {
        return -1;
    }
    kernel->fileSystem->socketTable[freeSlot] = new OpenFile(socketId);

    return freeSlot;
}

int SysConnect(int socketid, char *ip, int port){
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) 
        return -1;  
    int result = connect(socketid, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    return result;
}

int SysSend(int socketid, char *buffer, int len){
    int size;
    if(CheckConnection(socketid) == -1) return 0;
    size = send(socketid, buffer, len, 0);
    if(size == len) return size;
    return -1;
}

int SysReceive(int socketid, char *buffer, int len){
    int size;
    if(CheckConnection(socketid) == -1) return 0;
    if((size = read(socketid, buffer, len)) == -1) return -1;
    return size;
}

int SysSocketClose(int socketid){
    close(socketid);
    shutdown(socketid, SHUT_RDWR);
    if(CheckConnection(socketid) != -1) return -1;
    return 0;
}





#endif /* ! __USERPROG_KSYSCALL_H__ */
