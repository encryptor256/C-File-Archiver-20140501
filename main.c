//#include "archiver.h"
//#include "directory.h"



/*
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
*/

#include "archiver.h"

ref * callbacky( Node_t * const node, ref * const tag )
{
	if(node->type==archive_entry_archive)
	{
		Archive_t * const archive = (Archive_t*)node;
	
		printf("\r\n Archive_t: '%s', '%d'. ",archive->container.base.name,archive->container.entrycount);
	}
	else if(node->type==archive_entry_data)
	{
		ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)node;
		
		printf("\r\n ArchiveDataEntry_t: '%s', '%d'. ",data->base.name,data->size);
	}
	else if(node->type==archive_entry_container)
	{
		ArchiveContainerEntry_t * const container = (ArchiveContainerEntry_t*)node;
		
		printf("\r\n ArchiveContainerEntry_t: '%s', '%d'. ",container->base.name,container->entrycount);
	};
	
	return 0;
};

void command_a(const s8 * const name0, const s8 * const name1)
{
	const char * functionname = {"command_a"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	s32 result = ArchivateDirectory(name1,name0);
	
	if(result!=0)
	{
		printerror("ArchivateDirectory");
		return;
	};
};

void command_i(const s8 * const name0, const s8 * const name1)
{
	const char * functionname = {"command_a"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	Archive_t * const archive = readarchive(name0);
	
	if(archive==0)
	{
		printerror("archive==0");
		return;
	};
	
	if(archiveinfo(archive,name1)!=0)
	{
		printerror("archiveinfo!=0");
		return;
	};
};

void command_e(const s8 * const name0, const u32 id)
{
	const char * functionname = {"command_a"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	Archive_t * const archive = readarchive(name0);
	
	if(archive==0)
	{
		printerror("archive==0");
		return;
	};
	
	if(id==0)
	{
		printerror("Invalid id");
		return;
	};
	
	u32 reqid = id;
	
	ref * entry = Node_travert(&archive->container.base.node,&reqid,findentry);
	
	if(entry==0)
	{
		printerror("ID not found");
		return;
	};
	
	ArchiveBaseEntry_t * const base = (ArchiveBaseEntry_t*)entry;
	
	if(base->type==archive_entry_data)
		extractdata(archive,id);
	else if(base->type==archive_entry_container)
		extractcontainer(archive,id);
};

s32 parsecmd()
{
	const char * functionname = {"parsecmd"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	const char * cmdfile = {"cmd.txt"};
	
	FILE * handle = fopen(cmdfile,"rb");
	
	if(handle==0)
	{
		printerror("Cannot open command file");
		return 0;
	};
	
	s8 command, indicator, byte;
	
	size_t bytesread = 0;
	
	while(fread(&command,1,1,handle)==1)
	{
		if(command==10||command==13)
			continue;
		
		if(command=='e'||command=='i'||command=='a')
		{
			indicator=0;
			
			bytesread += fread(&indicator,1,1,handle);
	
			u32 anamelen=0;
			char aname[MAX_PATH+1];
			
			if(indicator!='"')
			{
				printerror("0. Idicator is missing");
				goto quit;
			};
			
			while(fread(&byte,1,1,handle)==1)
			{
				bytesread++;
				
				if(byte=='"')
					break;
				
				if(anamelen==MAX_PATH)
					break;
				
				aname[anamelen]=byte;
				anamelen++;
				aname[anamelen]=0;
			};
			
			if(byte!='"')
			{
				printerror("1. Idicator is missing");
				goto quit;
			};
				
			indicator=0;
			
			bytesread += fread(&indicator,1,1,handle);
			
			if(indicator!=',')
			{
				printerror("1. Idicator is missing");
				goto quit;
			};
			
			if(anamelen==0)
			{
				printerror("Archive name not provided");
				goto quit;
			};
			
			if(command=='e')
			{
				u32 numlen=0;
				char num[MAX_PATH+1];
				
				while(fread(&byte,1,1,handle)==1)
				{
					bytesread++;
					
					if(byte==';')
						break;
					
					if(numlen==MAX_PATH)
						break;
					
					num[numlen]=byte;
					numlen++;
					num[numlen]=0;
				};
				
				printf("\r\nCMD:e\"%s\",%d;",aname,atoi(num));
				
				u32 eid = atoi(num);
				
				command_e(aname,eid);
				
				continue;
			};
			
			// command: i or a
			
			u32 name2len=0;
			char name2[MAX_PATH+1];
			
			bytesread += fread(&indicator,1,1,handle);
			
			if(indicator!='"')
			{
				printerror("2. Idicator is missing");
				goto quit;
			};
			
			while(fread(&byte,1,1,handle)==1)
			{
				bytesread++;
				
				if(byte=='"')
					break;
				
				if(name2len==MAX_PATH)
					break;
				
				name2[name2len]=byte;
				name2len++;
				name2[name2len]=0;
			};
			
			if(byte!='"')
			{
				printerror("3. Idicator is missing");
				goto quit;
			};
			
			bytesread += fread(&indicator,1,1,handle);
			
			if(indicator!=';')
			{
				printerror("4. Idicator is missing");
				goto quit;
			};
			
			if(name2len==0)
			{
				printerror("Second name is empty");
				goto quit;
			};
			
			printf("\r\nCMD:%c\"%s\",\"%s\";",command,aname,name2);
			
			if(command=='a')
				command_a(aname,name2);
			else 
				command_i(aname,name2);
			
			continue;
		}
		else
		{
			printerror("Incorrect command");
			goto quit;	
		};
		
		
	};
	
quit:
	
	printf("\r\nbytesread: %d",bytesread);
	fclose(handle);	
};


s32 main(void)
{
	const char * functionname = {"main"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	parsecmd();
	
	/*const char * cmdfile = {"cmd.txt"};
	
	FILE * handle = fopen(cmdfile,"rb");
	
	if(handle==0)
	{
		return 0;
	};
	
	fclose(handle);*/
	
	/*ArchiveContainerEntry_t * container = scandirectory("bta");
	
	if(container==0)
	{
		printerror("container==0");
		return 0;
	};*/
	
	//ref * rvalue = Node_travert(&container->base.node,0,callbacky);
	
	/*s32 result = ArchivateDirectory("bta","archive.txt");
	
	if(result!=0)
	{
		printerror("ArchivateDirectory");
		return -1;
	};
	
	Archive_t * const archive = readarchive("archive.txt");
	
	if(archive==0)
	{
		printerror("archive==0");
		return -1;
	};
	
	archiveinfo(archive,"archiveinfo.txt");
	
	//ref * rvalue = Node_travert(&archive->container.base.node,0,callbacky);
	
	Node_travert(&archive->container.base.node,0,printinfo);
	
	//extractdata(archive,11);
	
	//extractcontainer(archive,1);*/
	
	return 0;
};




