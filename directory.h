#ifndef directory_h
#define directory_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

typedef struct tagDirEntry_t
{
	char		type; // unused for now
	size_t	namelen;
	char		name[MAX_PATH+1];
	char		path[MAX_PATH+1];
	char 	isDirectory;
	
}DirEntry_t;

typedef struct tagDir_t
{
	DirEntry_t	entry;
	HANDLE 			handle;
	WIN32_FIND_DATA 	data;
	unsigned char 	cached;
	char				directory[MAX_PATH+1];
	char				sysdirectory[MAX_PATH+1];
}Dir_t;

Dir_t * opendirectory(const char * path);

int closedirectory(Dir_t * directory);

DirEntry_t * readdirectory(Dir_t * directory);

//
// Get current directory, null terminated string
//

unsigned long _GetCurrentDirectory(char * const buffer, const unsigned long bufferlen);

int _CreateDirectory(const char * path);

/*
#include <string.h>
#include <windows.h>




void test(const char * path)
{	
	//SetCurrentDirectory(path);
	
	char curdir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,curdir);
	
	WIN32_FIND_DATA * data = calloc(1,sizeof(WIN32_FIND_DATA));
	
	HANDLE handle = FindFirstFile("*",data);
	
	do{
		if(strlen(data->cFileName)<3)
			continue;
		
		printf("\r\n %s\\%s ",curdir,data->cFileName);
		
		if(data->dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			SetCurrentDirectory(data->cFileName);
			test(data->cFileName);
			SetCurrentDirectory(curdir);
		};
		
	}while(FindNextFile(handle,data)!=FALSE);
	
	FindClose(handle);
	
	free(data);
};



void testsomething(const char * t)
{

	Directory_t * directory = opendirectory(t);
	
	if(directory==0)
	{
		printf("\r\n~Error: opendirectory");
		return;
	};
	
	DirectoryEntry_t * entry = 0;
	
	entry = (DirectoryEntry_t*)1;
	
	while(entry!=0)
	{
		entry = readdirectory(directory);
		
		if(entry==0)
			break;
		
		if(entry->namelen<3)
			continue;
		
		printf("\r\n %s\\%s",entry->path,entry->name);
		
		if(entry->isDirectory)
			testsomething(entry->name);

	};
	
	closedirectory(directory);
	
};

int main()
{
	char buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,buffer);
	printf("\r\nCD: %d, %s",MAX_PATH,buffer);
	
	//test(".");
	
	testsomething("bta");
	
	GetCurrentDirectory(MAX_PATH,buffer);
	printf("\r\nCD: %d, %s",MAX_PATH,buffer);
	
	return 0;
};
*/

#endif