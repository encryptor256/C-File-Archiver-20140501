#include "directory.h"

Dir_t * opendirectory(const char * path)
{
	if(path==0) return 0;
	
	Dir_t * directory = calloc(1,sizeof(Dir_t));
	
	GetCurrentDirectory(MAX_PATH,directory->sysdirectory);
	
	SetCurrentDirectory(path);
	
	GetCurrentDirectory(MAX_PATH,directory->directory);
	
	directory->handle = FindFirstFile("*",&directory->data);
	
	if(directory->handle != INVALID_HANDLE_VALUE)
	{
		directory->cached = 1;
	}
	else
	{
		free(directory);
		directory=0;
	};
	
	return directory;
};

int closedirectory(Dir_t * directory)
{
	if(directory==0) return -1;
	
	SetCurrentDirectory(directory->sysdirectory);
	
	FindClose(directory->handle);
	
	free(directory);
	
	return 0;
};

DirEntry_t * readdirectory(Dir_t * directory)
{
	if(directory==0) return 0;
	
	if(directory->cached==1)
	{
		directory->cached=0;
	}
	else if(directory->handle==INVALID_HANDLE_VALUE
		||FindNextFile(directory->handle,&directory->data)==FALSE
		||directory->handle==INVALID_HANDLE_VALUE)
	{
		return 0;
	};
	
	directory->entry.isDirectory = (directory->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) !=0 ? 1:0;
	
	directory->entry.namelen=strlen(directory->data.cFileName);
	
	memcpy(directory->entry.name,directory->data.cFileName,directory->entry.namelen+1);
	
	memcpy(directory->entry.path,directory->directory,strlen(directory->directory)+1);
	
	return &directory->entry;
};

unsigned long _GetCurrentDirectory(char * const buffer, const unsigned long bufferlen)
{
	if(buffer==0||bufferlen==0) return 0;
	
	return GetCurrentDirectory(bufferlen,buffer);
};

int _CreateDirectory(const char * path)
{
	if(path==0||CreateDirectory(path,0)!=0) return -1;
	
	return CreateDirectory(path,0);	
};