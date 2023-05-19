// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"

#define MaxFileLength 32
//#include "openfile.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void moveProgramCounter(){
		/* Modify return point */
	
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	
}
// Input: - User space address (int) 
// - Limit of buffer (int) 
// Output:- Buffer (char*) 
// Purpose: Copy buffer from User memory space to System memory space
char* User2System(int virtAddr, int limit)
{
	int i; //index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1];
	if (kernelBuf == NULL)
		return kernelBuf;

	memset(kernelBuf, 0, limit + 1);

	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: - User space address (int) 
// - Limit of buffer (int) 
// - Buffer (char[]) 
// Output:- Number of bytes copied (int) 
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do {
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}
void StringSys2User(char* str, int addr, int convert_length = -1) {
    int length = (convert_length == -1 ? strlen(str) : convert_length);
    for (int i = 0; i < length; i++) {
        kernel->machine->WriteMem(addr + i, 1, str[i]); 
    }
    kernel->machine->WriteMem(addr + length, 1, '\0');
}
void handle_SC_ReadString()
{
    int memPtr = kernel->machine->ReadRegister(4);
    int length = kernel->machine->ReadRegister(5);  
    if (length > 255) {
        DEBUG(dbgSys, "String length exceeds " << 255);
        SysHalt();
    }
    char* buffer = SysReadString(length);
	System2User(memPtr,255, buffer);
    delete[] buffer;
    return;
}
void handle_SC_PrintString()
{
	int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    char* buffer = User2System(memPtr,255);
    SysPrintString(buffer, strlen(buffer));
    delete[] buffer;
    return;
}
void handle_SC_SocketTCP()
{
	int socketId = SysSocketTCP();
	kernel->machine->WriteRegister(2, socketId); 
	return;
}
void handle_SC_Connect()
{
	int socketid = kernel->machine->ReadRegister(4);
	int virtAddr = kernel->machine->ReadRegister(5);
	int port = kernel->machine->ReadRegister(6);
	int fd; 
	char* ip = User2System(virtAddr,255);
	if (kernel->fileSystem->socketTable[socketid] != NULL || socketid < 0 || socketid > 19) 
	{	
		fd = kernel->fileSystem->socketTable[socketid]->GetSocketFD();
		kernel->machine->WriteRegister(2, SysConnect(fd, ip, port));
		return;
	}
	kernel->machine->WriteRegister(2, -1);
	delete[] ip; 
	return;
}
void handle_SC_Send()
{
	int socketid = kernel->machine->ReadRegister(4);
	int virtAddr = kernel->machine->ReadRegister(5);
	int len = kernel->machine->ReadRegister(6);
	int fd;
	char* buffer = User2System(virtAddr,255);
	
	if (kernel->fileSystem->socketTable[socketid] != NULL || socketid != -1) 
	{	
		fd = kernel->fileSystem->socketTable[socketid]->GetSocketFD();
		kernel->machine->WriteRegister(2, SysSend(fd, buffer, len));
		delete[] buffer; 
		return;
	} 
	kernel->machine->WriteRegister(2, -1); 
	delete[] buffer; 
	return;
}
void handle_SC_Receive()
{
	int socketid = kernel->machine->ReadRegister(4);
	int virtAddr = kernel->machine->ReadRegister(5);
	int len = kernel->machine->ReadRegister(6);
	int fd;
	char* buffer = User2System(virtAddr,255);
		
	if (kernel->fileSystem->socketTable[socketid] != NULL || socketid < 0 || socketid > 19) 
	{	
		fd = kernel->fileSystem->socketTable[socketid]->GetSocketFD();
		kernel->machine->WriteRegister(2, SysReceive(fd, buffer, len));
		StringSys2User(buffer, virtAddr);
		delete[] buffer;
		return;
	}
	kernel->machine->WriteRegister(2, -1); 
	delete[] buffer;
	return;
}
void handle_SC_SocketClose()
{
	int socketid = kernel->machine->ReadRegister(4);
	int fd;
		
	if (kernel->fileSystem->socketTable[socketid] != NULL || socketid != -1) 
	{	
		fd = kernel->fileSystem->socketTable[socketid]->GetSocketFD();
		kernel->machine->WriteRegister(2, SysSocketClose(fd));
		return;
	}
	kernel->machine->WriteRegister(2, -1); 
	return;
}
void ExceptionHandlerExec()
{
	DEBUG(dbgSys, "Syscall: Exec(filename)");

	int addr = kernel->machine->ReadRegister(4);
	DEBUG(dbgSys, "Register 4: " << addr);

	char *fileName;
	fileName = User2System(addr, 255);
	DEBUG(dbgSys, "Read file name: " << fileName);

	DEBUG(dbgSys, "Scheduling execution...");
	int result = kernel->pTab->ExecUpdate(fileName);

	DEBUG(dbgSys, "Writing result to register 2: " << result);
	kernel->machine->WriteRegister(2, result);
	delete fileName;
}

// Usage: block the current thread until the child thread has exited
// Input: ID of the thread being joined
// Output: exit code of the thread
void ExceptionHandlerJoin()
{
	DEBUG(dbgSys, "Syscall: Join");
	int id = kernel->machine->ReadRegister(4);
	int result = kernel->pTab->JoinUpdate(id);
	kernel->machine->WriteRegister(2, result);
}

// Usage: exit current thread
// Input: exit code to pass to parent
// Output: none
void ExceptionHandlerExit()
{
	DEBUG(dbgSys, "Syscall: Exit");
	int exitCode = kernel->machine->ReadRegister(4);
	int result = kernel->pTab->ExitUpdate(exitCode);
}

// Usage: Create a semaphore
// Input : name of semphore and int for semaphore value
// Output : success: 0, fail: -1
void ExceptionHandlerCreateSemaphore()
{
	// Load name and value of semaphore
	int virtAddr = kernel->machine->ReadRegister(4); // read name address from 4th register
	int semVal = kernel->machine->ReadRegister(5);	 // read type from 5th register
	char *name = User2System(virtAddr, MaxFileLength); // Copy semaphore name charArray form userSpace to systemSpace
	
	// Validate name
	if(name == NULL)
	{
		// DEBUG(dbgSynch, "\nNot enough memory in System");
		printf("\nNot enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}
	
	int res = kernel->semTab->Create(name, semVal);

	// Check error
	if(res == -1)
	{
		// DEBUG('a', "\nCan not create semaphore");
		printf("\nCan not create semaphore");
	}
	
	delete[] name;
	kernel->machine->WriteRegister(2, res);
	return;
}

// Usage: Sleep
// Input : name of semaphore
// Output : success: 0, fail: -1
void ExceptionHandlerWait()
{
	// Load name of semaphore
	int virtAddr = kernel->machine->ReadRegister(4);
	char *name = User2System(virtAddr, MaxFileLength + 1);

	// Validate name
	if(name == NULL)
	{
		// DEBUG(dbgSynch, "\nNot enough memory in System");
		printf("\nNot enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}

	int res = kernel->semTab->Wait(name);
	
	// Check error
	if(res == -1)
	{
		// DEBUG(dbgSynch, "\nNot exists semaphore");
		printf("\nNot exists semaphore");
	}

	delete[] name;
	kernel->machine->WriteRegister(2, res);
	return;
}

// Usage: Wake up
// Input : name of semaphore
// Output : success: 0, fail: -1
void ExceptionHandlerSignal()
{
	// Load name of semphore
	int virtAddr = kernel->machine->ReadRegister(4);
	char *name = User2System(virtAddr, MaxFileLength + 1);

	// Validate name
	if(name == NULL)
	{
		// DEBUG(dbgSynch, "\nNot enough memory in System");
		printf("\n Not enough memory in System");
		kernel->machine->WriteRegister(2, -1);
		delete[] name;
		return;
	}
	
	int res = kernel->semTab->Signal(name);

	// Check error
	if(res == -1)
	{
		// DEBUG(dbgSynch, "\nNot exists semaphore");
		printf("\nNot exists semaphore");
	}
	
	delete[] name;
	kernel->machine->WriteRegister(2, res);
	return;
}
void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
    switch (which) {
    case NoException:
		return;

	case PageFaultException:
		DEBUG('a', "\n No valid translation found");
		printf("\n\n No valid translation found");
		kernel->interrupt->Halt();
		break;

	case ReadOnlyException:
		DEBUG('a', "\n Write attempted to page marked read-only");
		printf("\n\n Write attempted to page marked read-only");
		kernel->interrupt->Halt();
		break;

	case BusErrorException:
		DEBUG('a', "\n Translation resulted invalid physical address");
		printf("\n\n Translation resulted invalid physical address");
		kernel->interrupt->Halt();
		break;

	case AddressErrorException:
		DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
		printf("\n\n Unaligned reference or one that was beyond the end of the address space");
		kernel->interrupt->Halt();
		break;

	case OverflowException:
		DEBUG('a', "\nInteger overflow in add or sub.");
		printf("\n\n Integer overflow in add or sub.");
		kernel->interrupt->Halt();
		break;

	case IllegalInstrException:
		DEBUG('a', "\n Unimplemented or reserved instr.");
		printf("\n\n Unimplemented or reserved instr.");
		kernel->interrupt->Halt();
		break;

	case NumExceptionTypes:
		DEBUG('a', "\n Number exception types");
		printf("\n\n Number exception types");
		kernel->interrupt->Halt();
		break;
    case SyscallException:
      switch(type) {

      	case SC_Halt:{
		
		DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
		printf("\n");
		SysHalt();

		ASSERTNOTREACHED();
		break;
		}

		case SC_Add:{
		DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
		
		/* Process SysAdd Systemcall*/
		int result;
		result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
				/* int op2 */(int)kernel->machine->ReadRegister(5));

		DEBUG(dbgSys, "Add returning with " << result << "\n");
		/* Prepare Result */
		kernel->machine->WriteRegister(2, (int)result);
		moveProgramCounter();

		return;

		break;
	}
		
		case SC_Create: { 
		int virtAddr; 
		char* filename; 
		DEBUG(dbgFile,"\n SC_Create call ..."); 
		DEBUG(dbgFile,"\n Reading virtual address of filename"); 
		
		// check for exception 
		virtAddr = kernel->machine->ReadRegister(4); 
		DEBUG (dbgFile,"\n Reading filename."); 
		filename = User2System(virtAddr,254+1);
		if (filename == NULL) 
		{ 
			printf("\n Not enough memory in system"); 
			DEBUG(dbgFile,"\n Not enough memory in system"); 
			kernel->machine->WriteRegister(2,-1); 
			moveProgramCounter();
			delete[] filename; 
			return; 
		} 
		
		DEBUG(dbgFile,"\n Finish reading filename."); 

		if (!kernel->fileSystem->Create(filename,0)) 
		{ 
			printf("\n Error create file '%s'",filename); 
			kernel->machine->WriteRegister(2,-1); 
			moveProgramCounter();
			delete[] filename; 
			return; 
		} 
		kernel->machine->WriteRegister(2,0); // trả về cho chương trình người dùng thành công 
	    moveProgramCounter();
		delete[] filename; 


	    return;
		break;
	}
		
		case SC_Open:{
		int virtAddr= kernel->machine->ReadRegister(4);
		int type= kernel->machine->ReadRegister(5);
		char* fileName;
		fileName=User2System(virtAddr,254+1);
		int slotValid=kernel->fileSystem->FreeSlot();

		if (strcmp(fileName, "stdin") == 0){
				printf("\nStdin mode");
				kernel->machine->WriteRegister(2, 0);
				delete[] fileName;
				moveProgramCounter();
				return;
			}
		if (strcmp(fileName, "stdout") == 0){
				printf("\nStdout mode");
				kernel->machine->WriteRegister(2, 1);
				delete[] fileName;
				moveProgramCounter();
				return;
			}

		if(slotValid != -1)
		{//con slot
			if(type == 0||type == 1){
				if((kernel->fileSystem->fileTable[slotValid]=kernel->fileSystem->Open(fileName, type))!=NULL){
				
					// cerr << endl << "Open slot: " << slotValid<<endl;
					kernel->fileSystem->fileTableName[slotValid]=fileName;
					kernel->machine->WriteRegister(2,slotValid);
				}
				else{
					printf("\nFilename %s%s",fileName," khong ton tai:\n");
					kernel->machine->WriteRegister(2,-1);
				}
			}
			else{
				kernel->machine->WriteRegister(2,-1);
				printf("\nnot valid type");
				}
			
			delete[] fileName;
			moveProgramCounter();
			return;
		}
		printf("\nHet_slot:");
		kernel->machine->WriteRegister(2,-1);//return -1
		delete[] fileName;
		moveProgramCounter();
		return;
		break;
	}
		
		case SC_Close:{
	
			int fileid = kernel->machine->ReadRegister(4); // Lay id cua file tu thanh ghi so 4
			if (fileid >= 0 && fileid <= 19) 
			{
				if (kernel->fileSystem->fileTable[fileid]) //if success
				{
					printf("\ndongthanhcong %d",fileid);
					delete kernel->fileSystem->fileTable[fileid]; //Xoa vung nho luu tru file
					kernel->fileSystem->fileTable[fileid] = NULL; // NULL
					kernel->machine->WriteRegister(2, 0);
					moveProgramCounter();
					return;	
				}
			}
			//printf("\nFail %d",fileid);
			kernel->machine->WriteRegister(2, -1);
			moveProgramCounter();
			return;	
			break;
		}
		
		case SC_Read:{
			int virtAddr= kernel->machine->ReadRegister(4);
			int charCount=kernel->machine->ReadRegister(5);
			int fileID=kernel->machine->ReadRegister(6);
			char *buffer;
			int temp;
			buffer=User2System(virtAddr,254+1);
			//neu nam ngoai gioi han fileTable
			if(fileID<0 || fileID>19){
				printf("\nOut of fileTable");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
			}
			
			if(kernel->fileSystem->fileTable[fileID] == NULL){
				printf("\nFile ko ton tai");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
			}
			if(fileID==1){
				printf("\nKhong the doc stdout");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
			}
			if (fileID == 0) {
				temp=kernel->synchConsoleIn->GetString(buffer, charCount);
				kernel->machine->WriteRegister(2,temp);

				printf("So ky tu trong stdin %d",temp);

			}
			else{
				temp=kernel->fileSystem->fileTable[fileID]->Read(buffer, charCount);//So ky tu trong file %s%s%d",buffer," ",temp
				kernel->machine->WriteRegister(2,temp);

			}
			System2User(virtAddr,temp,buffer);
			delete[] buffer;
			moveProgramCounter();
			return;
			break;
			}
		
		case SC_Write:{
			int virtAddr= kernel->machine->ReadRegister(4);
			int charCount=kernel->machine->ReadRegister(5);
			int fileID=kernel->machine->ReadRegister(6);
			char *buffer;
			buffer=User2System(virtAddr,254+1);
			int temp;

			if(fileID<0 || fileID>19){
			printf("\nOut of fileTable");
			kernel->machine->WriteRegister(2,-1);
			moveProgramCounter();
			return;
			}

			if(kernel->fileSystem->fileTable[fileID] == NULL){
			printf("\nFile ko ton tai");
			kernel->machine->WriteRegister(2,-1);
			moveProgramCounter();
			return;
			}
			if(kernel->fileSystem->fileTable[fileID]->type==1){//file only read
			printf("\nKhong the ghi file only read");
			kernel->machine->WriteRegister(2,-1);
			moveProgramCounter();
			return;
			}
			if(fileID == 0){//stdin
			printf("\nKhong the  stdin");
			kernel->machine->WriteRegister(2,-1);
			moveProgramCounter();
			return;
			}
			if (fileID == 1) {//stdout
				temp=kernel->synchConsoleOut->PutString(buffer, charCount);
				kernel->machine->WriteRegister(2,temp);//So ky tu trong stdout %d",temp;
			}//>=2
			else{
				temp=kernel->fileSystem->fileTable[fileID]->Write(buffer, charCount);
				kernel->machine->WriteRegister(2,temp);
				
			}
			System2User(virtAddr,temp,buffer);//So ky tu trong file %s%s%d",buffer," ",temp
			moveProgramCounter();
			return;
			break;
		}
		
		case SC_Seek:{
			int seekPos = kernel->machine->ReadRegister(4);
   			int fileID = kernel->machine->ReadRegister(5);
			if(fileID<0 || fileID>19){
				printf("\nOut of fileTable");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
			}

			if(kernel->fileSystem->fileTable[fileID] == NULL){
				printf("\nFile ko ton tai");
				printf("\nKhong the seek tren console");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
				moveProgramCounter();
				return;
			}
			if(fileID==0||fileID==1){
				printf("\nKhong the seek tren console");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
			}
			if(seekPos < -1 || seekPos > kernel->fileSystem->fileTable[fileID]->Length()){
				printf("\nKhong the seek toi");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
			}
			else if(seekPos == -1){
				seekPos=kernel->fileSystem->fileTable[fileID]->Length();
				kernel->fileSystem->fileTable[fileID]->Seek(seekPos);
				kernel->machine->WriteRegister(2,seekPos);
			}
			else{
				kernel->fileSystem->fileTable[fileID]->Seek(seekPos);	
				kernel->machine->WriteRegister(2,seekPos);	
			}
			moveProgramCounter();
			return;
			break;
		}
		
		case SC_Remove:{
			int virtAddr= kernel->machine->ReadRegister(4);
			char* filename=User2System(virtAddr,255);
			for (int i = 2; i < 20; i++)
			{

				if (kernel->fileSystem->fileTable[i] != NULL && strcmp(filename, kernel->fileSystem->fileTableName[i]) == 0)
				{
					//khong delete file dang open
					kernel->machine->WriteRegister(2, -1);
					moveProgramCounter();
					return;
				}
			}
			if (kernel->fileSystem->Remove(filename))//success
			{
				cout<<"\nxoa thanh cong";
				kernel->machine->WriteRegister(2, 0);
			}
			else//fail to find filename
			{
				cout<<"khong ton tai file name";
				kernel->machine->WriteRegister(2, -1);
			}
			moveProgramCounter();
			return;
			break;
		}

		case SC_ReadString:{
			handle_SC_ReadString();
			moveProgramCounter();
			return;
			break;
		}
		
		case SC_PrintString:{
			handle_SC_PrintString();
			moveProgramCounter();
			return;
			break;
		}

		case SC_SocketTCP:{
			handle_SC_SocketTCP();
			moveProgramCounter();
			break;
		}
		case SC_Connect:{
			handle_SC_Connect();
			moveProgramCounter();
			break;
		}
		case SC_Send:{
			handle_SC_Send();
			moveProgramCounter();
			break;
		}
		case SC_Receive:{
			handle_SC_Receive();
			moveProgramCounter();
			break;
		}
		case SC_SocketClose:{
			handle_SC_SocketClose();
			moveProgramCounter();
			break;
		}
        case SC_Exec:
		{
			// Input: vi tri int
			// Output: Fail return -1, Success: return id cua thread dang chay
			// SpaceId Exec(char *name);
			int virtAddr;
			virtAddr = kernel->machine->ReadRegister(4);	// doc dia chi ten chuong trinh tu thanh ghi r4
			char* name;
			name = User2System(virtAddr, MaxFileLength + 1); // Lay ten chuong trinh, nap vao kernel
	
			if(name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				kernel->machine->WriteRegister(2, -1);
				moveProgramCounter();
				return;
			}
			OpenFile *oFile = kernel->fileSystem->Open(name);
			if (oFile == NULL)
			{
				printf("\nExec:: Can't open this file.");
				kernel->machine->WriteRegister(2,-1);
				moveProgramCounter();
				return;
			}

			delete oFile;

			// Return child process id
			int id = kernel->pTab->ExecUpdate(name); 
			kernel->machine->WriteRegister(2,id);

			delete[] name;	
			moveProgramCounter();
			return;
		}
		case SC_Join:
		{       
			// int Join(SpaceId id)
			// Input: id dia chi cua thread
			// Output: 
			int id = kernel->machine->ReadRegister(4);
			
			int res = kernel->pTab->JoinUpdate(id);
			
			kernel->machine->WriteRegister(2, res);
			moveProgramCounter();
			return;
		}
		case SC_Exit:
		{
			//void Exit(int status);
			// Input: status code
			int exitStatus = kernel->machine->ReadRegister(4);

			if(exitStatus != 0)
			{
				moveProgramCounter();
				return;
				
			}			
			
			int res = kernel->pTab->ExitUpdate(exitStatus);
			//machine->WriteRegister(2, res);

			kernel->currentThread->FreeSpace();
			kernel->currentThread->Finish();
			moveProgramCounter();
			return; 
				
		}
		case SC_CreateSemaphore:
		{
			// int CreateSemaphore(char* name, int semval).
			int virtAddr = kernel->machine->ReadRegister(4);
			int semval = kernel->machine->ReadRegister(5);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if(name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				moveProgramCounter();
				return;
			}
			
			int res = kernel->semTab->Create(name, semval);

			if(res == -1)
			{
				DEBUG('a', "\n Khong the khoi tao semaphore");
				printf("\n Khong the khoi tao semaphore");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				moveProgramCounter();
				return;				
			}
			
			delete[] name;
			kernel->machine->WriteRegister(2, res);
			moveProgramCounter();
			return;
		}

		case SC_Wait:			
		{
			// int Wait(char* name)
			int virtAddr = kernel->machine->ReadRegister(4);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if(name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				moveProgramCounter();
				return;
			}
			
			int res = kernel->semTab->Wait(name);

			if(res == -1)
			{
				DEBUG('a', "\n Khong ton tai ten semaphore nay!");
				printf("\n Khong ton tai ten semaphore nay!");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				moveProgramCounter();
				return;				
			}
			
			delete[] name;
			kernel->machine->WriteRegister(2, res);
			moveProgramCounter();
			return;
		}
		case SC_Signal:		
		{
			// int Signal(char* name)
			int virtAddr = kernel->machine->ReadRegister(4);

			char *name = User2System(virtAddr, MaxFileLength + 1);
			if(name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				moveProgramCounter();
				return;
			}
			
			int res = kernel->semTab->Signal(name);

			if(res == -1)
			{
				DEBUG('a', "\n Khong ton tai ten semaphore nay!");
				printf("\n Khong ton tai ten semaphore nay!");
				kernel->machine->WriteRegister(2, -1);
				delete[] name;
				moveProgramCounter();
				return;				
			}
			
			delete[] name;
			kernel->machine->WriteRegister(2, res);
			moveProgramCounter();
			return;
		}
	    default:
	        cerr << "Unexpected system call " << type << "\n";
	        break;
        }
        break;
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
}
