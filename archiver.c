#include "archiver.h"

Archive_t * newArchive(void)
{
	Archive_t * archive = (Archive_t*)calloc(1,sizeof(Archive_t));
	archive->container.base.type=archive_entry_archive;
	archive->container.base.node.type=archive_entry_archive;
	archive->signature=archive_signature;
	archive->version=archive_version;
	return archive;
};

ArchiveContainerEntry_t * newArchiveContainerEntry(void)
{
	ArchiveContainerEntry_t * container = (ArchiveContainerEntry_t*)calloc(1,sizeof(ArchiveContainerEntry_t));
	container->base.type=archive_entry_container;
	container->base.node.type=archive_entry_container;
	return container;
};

ArchiveDataEntry_t * newArchiveDataEntry(void)
{
	ArchiveDataEntry_t * data = (ArchiveDataEntry_t*)calloc(1,sizeof(ArchiveDataEntry_t));
	data->base.type=archive_entry_data;
	data->base.node.type=archive_entry_data;
	return data;
};

s32 ArchiveContainerEntry_add(ArchiveContainerEntry_t * const container, ArchiveBaseEntry_t * const base)
{
	if(container==0||base==0) 
	{
		return -1;
	};
	
	if(container->base.node.first==0)
		container->base.node.first=&base->node;
	else
	{
		
		base->node.parent=&container->base.node;
		base->node.prev=container->base.node.last;
		container->base.node.last->next=&base->node;
	};
	
	container->base.node.last=&base->node;
	
	return 0;
};

ref * reftext(const s8 * const text, u32 * const len)
{
	if(text==0||strlen(text)==0) return 0;
	
	u32 textlen = strlen(text) + 1;
	
	ref * address = malloc(textlen);
	
	if(address==0) return 0;
	
	memcpy(address,text,textlen);
	
	if(len!=0) (*len) = textlen;
	
	return address;
};

ref * reftext3(const s8 * const text0, const s8 * const text1, const s8 * const text2, u32 * const len)
{
	if(text0==0||text1==0||text2==0||strlen(text0)==0||strlen(text1)==0||strlen(text2)==0) return 0;
	
	u32 textlen0 = strlen(text0);
	u32 textlen1 = strlen(text1);
	u32 textlen2 = strlen(text2) + 1;
	u32 textlen = textlen0 + textlen1 + textlen2;
	
	ref * address = malloc(textlen);
	
	if(address==0) return 0;
	
	size_t numaddr = (size_t)address;
	
	memcpy((ref*)numaddr+0,text0,textlen0);
	memcpy((ref*)numaddr+textlen0,text1,textlen1);
	memcpy((ref*)numaddr+textlen0+textlen1,text2,textlen2);
	
	if(len!=0) (*len) = textlen;
	
	return address;
};

ArchiveContainerEntry_t * scandirectory(const s8 * const path)
{
	const char * functionname = {"scandirectory"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. ",};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(path!=0&&strlen(path)>=3)
	{
		Dir_t * directory = opendirectory(path);
		
		if(directory!=0)
		{
			ArchiveContainerEntry_t * container = newArchiveContainerEntry();
			
			if(container==0)
			{
				printerror("container==0");
				return 0;
			};
					
			DirEntry_t * entry = (DirEntry_t*)-1;
			
			while(entry!=0)
			{
				entry = readdirectory(directory);
				
				if(entry==0)
					break;
				
				if(entry->namelen<3)
					continue;
				
				container->entrycount++;
				
				container->base.name = reftext(path,&container->base.namelen);
				
				if(container->base.name==0)
				{
					printerror("container->base.name==0");
					return 0;
				};
				
				if(entry->isDirectory)
				{
					ArchiveContainerEntry_t * const newcontainer = scandirectory(entry->name);
					
					if(newcontainer==0)
					{
						printerror("newcontainer==0");
						return 0;	
					};
					
					if(ArchiveContainerEntry_add(container,&newcontainer->base)!=0)
					{
						printerror("ArchiveContainerEntry_add - newcontainer");
						return 0;
					};
				}
				else
				{
					ArchiveDataEntry_t * const data = newArchiveDataEntry();
					
					if(data==0)
					{
						printerror("data==0");
						return 0;	
					};
					
					data->base.name = reftext(entry->name,&data->base.namelen);
					
					if(data->base.name==0)
					{
						printerror("data->base.name==0");
						return 0;
					};
					
					data->path = reftext3(entry->path,"\\",entry->name,&data->pathlen);
					
					if(data->path==0)
					{
						printerror("data->path==0");
						return 0;
					};
					
					size_t filesize;
					
					if(getfilesize(data->path,&filesize)!=0)
					{
						printerror("getfilesize");
						return 0;
					};
					
					if(filesize >= 0xFFFFFFFF)
					{
						printerror("filesize >= 0xFFFFFFFF");
						return 0;
					};
					
					data->size=filesize;
					
					if(ArchiveContainerEntry_add(container,&data->base)!=0)
					{
						printerror("ArchiveContainerEntry_add - data");
						return 0;
					};
				};
			};
			
			closedirectory(directory);
			
			return container;
		}
		else
		{
			printerror("opendirectory");
		};
	}
	else
	{
		printerror("Invalid argument");
	};
	
	return 0;
};

s32 getfilesize(const s8 * const filepath, size_t * const size)
{
	if(filepath==0||strlen(filepath)<3||size==0) return -1;
	
	s32 errorindicator = -1;
	
	FILE * handle = fopen(filepath,"rb");
	
	if(handle!=0 && fseek(handle,0,SEEK_END)==0)
	{
		
		size_t szfile = ftell(handle);
		
		if(szfile!=-1L)
		{
			
			(*size)=szfile;
			errorindicator=0;
		};
	};
	
	fclose(handle);
	
	return errorindicator;
};



ref * Node_travert(Node_t * const node, ref * const tag, ref * (* const callback) ( Node_t * const node, ref * const tag ) )
{
	if(node==0||callback==0) return (ref*)-1;
	
	ref * rvalue = callback(node,tag);
	
	if(rvalue!=0) return rvalue;
	
	Node_t * first = node->first;
	
	if(first==0) return 0;
	
	do
	{
		rvalue = (first->type==archive_entry_data) ? callback(first,tag) : Node_travert(first,tag,callback);
		
		if(rvalue!=0) return rvalue;
		
		first=first->next;
		
	}while(first!=0);
	
	return 0;
};

ref * getstructuresizes( Node_t * const node, ref * const tag )
{
	if(tag==0)
	{
		return (ref *)-1;
	};
	
	u32 * number = tag;
	
	if(node->type==archive_entry_archive)
	{
		Archive_t * const archive = (Archive_t*)node;
		
		(*number) += szArchive(archive);
	}
	else if(node->type==archive_entry_data)
	{
		ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)node;
		
		(*number) += szArchiveDataEntry(data);
	}
	else if(node->type==archive_entry_container)
	{
		ArchiveContainerEntry_t * const container = (ArchiveContainerEntry_t*)node;
		
		(*number) += szArchiveContainerEntry(container);
	};
	
	return 0;
};


ref * getcontainercount( Node_t * const node, ref * const tag )
{
	if(tag==0)
	{
		return (ref *)-1;
	};
	
	u32 * number = tag;
	
	if(node->type==archive_entry_container)
		(*number) += 1;
	
	return 0;
};

ref * getdatacount( Node_t * const node, ref * const tag )
{
	if(tag==0)
	{
		return (ref *)-1;
	};
	
	u32 * number = tag;
	
	if(node->type==archive_entry_data)
		(*number) += 1;
	
	return 0;
};

ref * updateids( Node_t * const node, ref * const tag )
{
	if(tag==0)
	{
		return (ref *)-1;
	};
	
	u32 * number = tag;
	
	ArchiveBaseEntry_t * const base = (ArchiveBaseEntry_t*)node;
	
	base->id = (*number);
	
	(*number) += 1;
	
	return 0;
};

ref * updatedataoffsets( Node_t * const node, ref * const tag )
{
	if(node->type!=archive_entry_data) return 0;
	
	if(tag==0)
	{
		return (ref *)-1;
	};
	
	u32 * number = tag;
	
	ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)node;
	
	data->offset = (*number);
	
	(*number) += data->size;
	
	return 0;
};

size_t writeArchiveBaseEntry(FILE * const handle, ArchiveBaseEntry_t * const entry)
{
	if(handle==0||entry==0) return 0;
	
	size_t bytesio;
	
	bytesio = fwrite(&entry->type,1,sizeof(u32),handle);
	bytesio += fwrite(&entry->id,1,sizeof(u32),handle);
	bytesio += fwrite(&entry->namelen,1,sizeof(u32),handle);
	
	if(entry->namelen>0)
		bytesio += fwrite(entry->name,1,entry->namelen,handle);
	
	return bytesio;
};

size_t writeArchiveDataEntry(FILE * const handle, ArchiveDataEntry_t * const entry)
{
	if(handle==0||entry==0) return 0;
	
	size_t bytesio;
	
	bytesio = writeArchiveBaseEntry(handle,&entry->base);
	
	bytesio += fwrite(&entry->offset,1,sizeof(u32),handle);
	bytesio += fwrite(&entry->size,1,sizeof(u32),handle);
	
	return bytesio;
};

size_t writeArchiveContainerEntry(FILE * const handle, ArchiveContainerEntry_t * const entry)
{
	if(handle==0||entry==0) return 0;
	
	size_t bytesio;
	
	bytesio = writeArchiveBaseEntry(handle,&entry->base);
	
	bytesio += fwrite(&entry->entrycount,1,sizeof(u32),handle);
	
	return bytesio;
};

size_t writeArchive(FILE * const handle, Archive_t * const archive)
{
	if(handle==0||archive==0) return 0;
	
	size_t bytesio;
	
	bytesio = fwrite(&archive->signature,1,sizeof(u32),handle);
	bytesio += fwrite(&archive->version,1,sizeof(u32),handle);
	bytesio += fwrite(&archive->szheader,1,sizeof(u32),handle);
	bytesio += fwrite(&archive->szarchive,1,sizeof(u32),handle);
	
	bytesio += fwrite(&archive->containercount,1,sizeof(u32),handle);
	bytesio += fwrite(&archive->datacount,1,sizeof(u32),handle);
	
	bytesio += writeArchiveContainerEntry(handle,&archive->container);
	
	return bytesio;
};

ref * structurewriter( Node_t * const node, ref * const tag )
{
	const char * functionname = {"structurewriter"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. ",};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(tag==0)
	{
		printerror("Invalid tag");
		return (ref *)-1;
	};
	
	FILE * handle = tag;
	
	if(node->type==archive_entry_archive)
	{
		Archive_t * const archive = (Archive_t*)node;
	
		if(writeArchive(handle,archive)!=szArchive(archive))
		{
			printerror("writeArchive");
			return (ref*)-1;
		};
	}
	else if(node->type==archive_entry_data)
	{
		ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)node;
	
		if(writeArchiveDataEntry(handle,data)!=szArchiveDataEntry(data))
		{
			printerror("writeArchiveDataEntry");
			return (ref*)-1;
		};
	}
	else if(node->type==archive_entry_container)
	{
		ArchiveContainerEntry_t * const container = (ArchiveContainerEntry_t*)node;
	
		if(writeArchiveContainerEntry(handle,container)!=szArchiveContainerEntry(container))
		{
			printerror("writeArchiveContainerEntry");
			return (ref*)-1;
		};
	};
	
	return 0;
};

ref * datawriter( Node_t * const node, ref * const tag )
{
	const char * functionname = {"datawriter"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. ",};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(tag==0)
	{
		printerror("Invalid tag");
		return (ref *)-1;
	};
	
	if(node->type!=archive_entry_data) return 0;
	
	FILE * handle = tag;
	
	ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)node;
	
	if(data->size==0) return 0;
	
	FILE * filehandle = fopen(data->path,"rb");
	
	if(filehandle==0)
	{
		printerror("Open file error");
		printf("{'%s'}",data->path);
		return (ref *)-1;	
	};
	
	if(fseek(filehandle,0,SEEK_SET)!=0)
	{
		printerror("fseek");
		printf("{'%s'}",data->path);
		return (ref *)-1;
	};
	
	size_t bytesread, byteswritten = 0;
	
	const u32 bufferlen = 4096;
	
	ref * buffer = malloc(bufferlen);
	
	if(buffer==0)
	{
		printerror("Buffer allocation");
		fclose(filehandle);
		return (ref *)-1;
	};
	
	do
	{
		bytesread = fread(buffer,1,bufferlen,filehandle);
		
		if(bytesread>0)
			byteswritten += fwrite(buffer,1,bytesread,handle);
		
	}while(bytesread==bufferlen);
	
	free(buffer);
	fclose(filehandle);
	
	return byteswritten == data->size ? 0 : (ref*)-1;
};

s32 ArchivateDirectory(const s8 * const path, const s8 * const archivefilename)
{
	
	const char * functionname = {"ArchivateDirectory"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. ",};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(path==0||strlen(path)<3||archivefilename==0||strlen(archivefilename)<3) 
	{
		printerror("Invalid argument");
		return 0;
	};
	
	ArchiveContainerEntry_t * container = scandirectory(path);
	
	if(container!=0)
	{
		Archive_t * archive = newArchive();
		
		if(archive!=0)
		{
			if(ArchiveContainerEntry_add(&archive->container,&container->base)==0)
			{
				archive->container.entrycount+=1;
				
				u32 structuresizes = 0;
				
				if(Node_travert(&archive->container.base.node,&structuresizes,getstructuresizes)==0)
				{
					archive->szheader=structuresizes;
					
					u32 ids = 0;
					
					if(Node_travert(&archive->container.base.node,&ids,updateids)==0)
					{
						u32 dataoffsets = structuresizes;
						
						if(Node_travert(&archive->container.base.node,&dataoffsets,updatedataoffsets)==0)
						{
							archive->szarchive=dataoffsets;
							
							if(Node_travert(&archive->container.base.node,&archive->containercount,getcontainercount)!=0)
							{
								printerror("Node_travert - getcontainercount");
								return -1;
							};
							
							if(Node_travert(&archive->container.base.node,&archive->datacount,getdatacount)!=0)
							{
								printerror("Node_travert - getdatacount");
								return -1;
							};
							
							FILE * handle = fopen(archivefilename,"wb");
							
							if(fseek(handle,archive->szarchive,SEEK_SET)!=0)
							{
								printerror("Unable to allocate space for acrhive");
								printf("{'%d'}",archive->szarchive);
								return -1;
							};
							
							if(fseek(handle,0,SEEK_SET)!=0)
							{
								printerror("Unable to reset archive file cursor");
								return -1;
							};
							
							if(handle!=0)
							{
							
								if(Node_travert(&archive->container.base.node,handle,structurewriter)==0)
								{
									if(Node_travert(&archive->container.base.node,handle,datawriter)==0)
									{
										
										fclose(handle);
										
										// Check size: Archive vs expected
										
										size_t filesize;
										
										if(getfilesize(archivefilename,&filesize)!=0)
										{
											printerror("getfilesize of archive");
											return 0;
										};
										
										if(archive->szarchive!=(u32)filesize)
										{
											printerror("archive->szarchive!=filesize");
											return -1;
										};
										
										return 0;
									}
									else
									{
										printerror("Node_travert - datawriter");
									};
								}
								else
								{
									printerror("Node_travert - structurewriter");
								};
							}
							else
							{
								printerror("fopen");
							};
						}
						else
						{
							printerror("Node_travert - updatedataoffsets");
						};
					}
					else
					{
						printerror("Node_travert - updateids");
					};
				}
				else
				{
					printerror("Node_travert - getstructuresizes");
				};
			}
			else
			{
				printerror("ArchiveContainerEntry_add");
			};
		}
		else
		{
			printerror("archive==0");
		};
	}
	else
	{
		printerror("container==0");	
	};
	
	return -1;
};


s32 parsearchive(FILE * const handle, ArchiveContainerEntry_t * const container, s8 * const filepath)
{
	const char * functionname = {"parsearchive"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(handle==0||container==0)
	{
		printerror("Invalid arguments");
		return -1;
	};
	
	size_t bytesio;
	
	bytesio = sizeof(u32); // fread(&entry->type,1,sizeof(u32),handle);
	bytesio += fread(&container->base.id,1,sizeof(u32),handle);
	bytesio += fread(&container->base.namelen,1,sizeof(u32),handle);
	
	if(container->base.namelen>0)
	{
		container->base.name=malloc(container->base.namelen);
		bytesio += fread(container->base.name,1,container->base.namelen,handle);
	};
	
	bytesio += fread(&container->entrycount,1,sizeof(u32),handle);
	
	if(bytesio!=szArchiveContainerEntry(container))
	{
		printerror("bytesio!=szArchiveContainerEntry(container)");
		return -1;
	};
	
	if(container->entrycount==0) return 0;
	
	u32 filepathbufferlen;
	ref * filepathbuffer = 0;
	
	if(container->base.type!=archive_entry_archive)
	{
		if(filepath==0)
		{
			filepathbuffer = reftext(container->base.name, &filepathbufferlen);
		}
		else
		{
			filepathbuffer = reftext3(filepath,"\\",container->base.name,&filepathbufferlen);
		};
		
		container->path = reftext(filepathbuffer,&container->pathlen);
	};
	
	u32 counter = 0;
	
	while(counter<container->entrycount)
	{
		ref * address;
		
		u32 type;
		
		bytesio = fread(&type,1,sizeof(u32),handle);
		
		if(type==archive_entry_data)
		{
			ArchiveDataEntry_t * const data = newArchiveDataEntry();
			
			bytesio += fread(&data->base.id,1,sizeof(u32),handle);
			bytesio += fread(&data->base.namelen,1,sizeof(u32),handle);
			
			if(data->base.namelen>0)
			{
				data->base.name=malloc(data->base.namelen);
				bytesio += fread(data->base.name,1,data->base.namelen,handle);
			};
			
			bytesio += fread(&data->offset,1,sizeof(u32),handle);
			bytesio += fread(&data->size,1,sizeof(u32),handle);
			
			if(bytesio!=szArchiveDataEntry(data))
			{
				printerror("bytesio!=szArchiveDataEntry(data)");
				return -1;	
			};
			
			address = data;
			
			if(filepathbuffer==0)
			{
				data->pathlen=0;
				data->path=0;
			}
			else
			{
				data->path = reftext3(filepathbuffer,"\\",data->base.name,&data->pathlen);
			};
		}
		else if(type==archive_entry_container)
		{
			ArchiveContainerEntry_t * const cntr = newArchiveContainerEntry();
			
			if(parsearchive(handle,cntr,filepathbuffer)!=0)
			{
				printerror("parsearchive");
				return -1;	
			};
			
			address = cntr;
		}
		else
		{
			printerror("Unmanaged type");
			return -1;
		};
		
		if(ArchiveContainerEntry_add(container,address)!=0)
		{
			printerror("ArchiveContainerEntry_add");
			return -1;	
		};
		
		counter++;
	};
	
	if(filepathbuffer!=0)
		free(filepathbuffer);
	
	return 0;
};

Archive_t * readarchive(const s8 * const filepath)
{
	const char * functionname = {"readarchive"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(filepath==0)
	{
		printerror("Invalid argument");
		return 0;
	};
	
	Archive_t * archive = newArchive();
	
	if(archive==0)
	{
		printerror("archive==0");
		return 0;
	};
	
	FILE * handle = fopen(filepath,"rb");
	
	if(handle==0)
	{
		printerror("fopen");
		return 0;
	};
	
	size_t bytesio;
	
	bytesio = fread(&archive->signature,1,sizeof(u32),handle);
	
	if(archive->signature!=archive_signature)
	{
		printerror("Archive signature");
		fclose(handle);
		return 0;
	};
	
	bytesio += fread(&archive->version,1,sizeof(u32),handle);
	
	if(archive->version!=archive_version)
	{
		printerror("Archive version");
		fclose(handle);
		return 0;
	};
	
	bytesio += fread(&archive->szheader,1,sizeof(u32),handle);
	bytesio += fread(&archive->szarchive,1,sizeof(u32),handle);
	
	bytesio += fread(&archive->containercount,1,sizeof(u32),handle);
	bytesio += fread(&archive->datacount,1,sizeof(u32),handle);
	
	bytesio += fread(&archive->container.base.type,1,sizeof(u32),handle);
	
	if(parsearchive(handle,&archive->container,0)!=0)
	{
		printerror("parsearchive");
		fclose(handle);
	};
	
	archive->handle=handle;
	
	return archive;
};

ref * findentry( Node_t * const node, ref * const tag )
{
	if(tag==0) return (ref *)-1;
	
	u32 * id = (u32*)tag;
	
	ArchiveBaseEntry_t * const base = (ArchiveBaseEntry_t*)node;
	
	if(base->id==(*id)) return (ref*)base;
	
	return 0;
};

s32 extractdata(Archive_t * const archive, const u32 id)
{
	const char * functionname = {"extractfile"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(archive==0)
	{
		printerror("Invalid argument");
		return -1;
	};
	
	u32 reqid = id;
	
	ref * entry = Node_travert(&archive->container.base.node,&reqid,findentry);
	
	if(entry==0)
	{
		printerror("Entry by id, was not found");
		return -1;
		
	}else if(entry==(ref*)-1)
	{
		printerror("Node_travert - findentry");
		return -1;	
	};
	
	ArchiveBaseEntry_t * const base = (ArchiveBaseEntry_t*)entry;
	
	if(base->type!=archive_entry_data)
	{
		printerror("Entry by id is not data entry");
		return -1;		
	};
	
	ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)base;

	return extractdatadirect(archive->handle,data);
};

s32 extractdatadirect(FILE * const archivehandle, ArchiveDataEntry_t * const data)
{
	const char * functionname = {"extractdatadirect"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(archivehandle==0||data==0)
	{
		printerror("Invalid arguments");
		return -1;
	};	
	
	if(data->pathlen>0)
	{
		s8 buffer[MAX_PATH+1];
		
		memcpy(&buffer,data->path,data->pathlen);
	
		u32 counter = 0;
		
		while(counter<data->pathlen)
		{
			if(buffer[counter]=='\\')
			{
				s8 old = buffer[counter];
				buffer[counter]=0;
				
				_CreateDirectory(buffer);
				
				buffer[counter]=old;
			};
			
			counter+=1;
		};
		
		printf("\r\n Extracting file: '%s'. ",data->path);
	
		FILE * datahandle = fopen(data->path,"wb+");
		
		if(datahandle==0)
		{
			printerror("fopen - output file");
			return -1;
		};
		
		if(fseek(archivehandle,data->offset,SEEK_SET)!=0)
		{
			printerror("fseek - archive");
			return -1;
		};
		
		const u32 wrbufferlen = 2048;
		
		ref * wrbuffer = malloc(wrbufferlen);
		
		if(wrbuffer==0)
		{
			printerror("malloc - buffer");
			return -1;
		};
		
		size_t bytesread, byteswritten, bytesreadtotal=0, byteswrittentotal=0, bytestoread, datasize = data->size;
		
		do
		{
			bytestoread = (datasize<wrbufferlen) ? datasize : wrbufferlen;
			
			datasize -= bytestoread;
			
			bytesread = fread(wrbuffer,1,bytestoread,archivehandle);
			
			bytesreadtotal += bytesread;
			
			if(bytesread>0)
			{
				byteswritten = fwrite(wrbuffer,1,bytesread,datahandle);
				
				byteswrittentotal += byteswritten;
			};
			
		}while(bytesread==wrbufferlen);
		
		free(wrbuffer);
		fclose(datahandle);
		
		if(byteswrittentotal!=data->size)
		{
			printerror("byteswrittentotal!=data->size");
			return -1;
		}
		else if(bytesreadtotal!=data->size)
		{
			printerror("bytesreadtotal!=data->size");
			return -1;
		};
		
		printf("{DONE}");
	}
	else
	{
		printerror("Unmanaged error");
		return -1;
	};
	
	return 0;
};

ref * containerdataextractor( Node_t * const node, ref * const tag )
{
	if(tag==0)
	{
		return (ref*)-1;
	};
	
	if(node->type!=archive_entry_data)
	{
		return 0;
	};
	
	ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)node;
	
	return extractdatadirect((FILE*)tag,data) == 0 ? 0 : (ref*) - 1;
};

s32 extractcontainer(Archive_t * const archive, const u32 id)
{
	const char * functionname = {"extractdirectory"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(archive==0)
	{
		printerror("Invalid argument");
		return -1;
	};
	
	u32 reqid = id;
	
	ref * entry = Node_travert(&archive->container.base.node,&reqid,findentry);
	
	if(entry==0)
	{
		printerror("Entry by id, was not found");
		return -1;
		
	}else if(entry==(ref*)-1)
	{
		printerror("Node_travert - findentry");
		return -1;	
	};
	
	ArchiveBaseEntry_t * const base = (ArchiveBaseEntry_t*)entry;
	
	if(base->type!=archive_entry_container)
	{
		printerror("Entry by id is not container entry");
		return -1;		
	};
	
	ArchiveContainerEntry_t * const container = (ArchiveContainerEntry_t*)base;
	
	ref * rvalue = Node_travert(&container->base.node,archive->handle,containerdataextractor);
	
	if(rvalue!=0)
	{
		printerror("Node_travert - containerdataextractor");
		return -1;		
	};
	
	return 0;
};

ref * printinfo( Node_t * const node, ref * const tag )
{
	if(tag==0)
	{
		return (ref*)-1;
	};
	
	FILE * handle = (FILE*)tag;
	
	if(node->type==archive_entry_data)
	{
		ArchiveDataEntry_t * const data = (ArchiveDataEntry_t*)node;
		
		fprintf(handle,"\r\n ID: '%5d', '%s'. [offset:%d, size:%d]. ",data->base.id,data->path,data->offset,data->size);
	}
	else if(node->type==archive_entry_container)
	{
		ArchiveContainerEntry_t * const container = (ArchiveContainerEntry_t*)node;
		
		fprintf(handle,"\r\n ID: '%5d', '%s'. ",container->base.id,container->path);
	};
	
	return 0;
};

s32 archiveinfo(Archive_t * const archive, const s8 * const filepathout)
{
	const char * functionname = {"archiveinfo"};
	const char * errorformat = {"\r\n ~Error: '%s' -> '%s'. "};
	#define printerror(X) printf(errorformat,functionname,X);
	
	if(archive==0||filepathout==0)
	{
		printerror("Invalid arguments");
		return 0;
	};
	
	FILE * handle = fopen(filepathout,"wb");
	
	if(handle==0)
	{
		printerror("fopen");
		return 0;
	};
	
	fprintf(handle,"\r\n Archive size: '%d' bytes. ",archive->szarchive);
	fprintf(handle,"\r\n Header size: '%d' bytes. ",archive->szheader);
	fprintf(handle,"\r\n Container count: '%d'. ",archive->containercount);
	fprintf(handle,"\r\n Data count: '%d'. ",archive->datacount);
	fprintf(handle,"\r\n",0);
	
	if(Node_travert(&archive->container.base.node,handle,printinfo)!=0)
	{
		printerror("Node_travert - printinfo");
	};
	
	fclose(handle);
	
	return 0;
};