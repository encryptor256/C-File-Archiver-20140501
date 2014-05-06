#ifndef archiver_h
#define archiver_h

#include "node.h"
#include "directory.h"

#define archive_signature 0xAA2014AA
#define archive_version 0x20140501
#define archive_entry_archive 0xf1
#define archive_entry_data 0xf2
#define archive_entry_container 0xf3

typedef struct tagArchiveBaseEntry_t
{
	Node_t node; 
	u32 type;
	u32 id;
	u32 namelen;
	ref * name;
}ArchiveBaseEntry_t;

#define szArchiveBaseEntry(X) ( sizeof(u32) * 3 + ((ArchiveBaseEntry_t*)X)->namelen )

typedef struct tagArchiveDataEntry_t
{
	ArchiveBaseEntry_t base;
	u32 offset;
	u32 size;
	u32 pathlen;
	ref * path;
}ArchiveDataEntry_t;

#define szArchiveDataEntry(X) ( szArchiveBaseEntry(X) + sizeof(u32) * 2 )

typedef struct tagArchiveContainerEntry_t
{
	ArchiveBaseEntry_t base;
	u32 entrycount;
	
	u32 pathlen;
	ref * path;
}ArchiveContainerEntry_t;

#define szArchiveContainerEntry(X) ( szArchiveBaseEntry(X) + sizeof(u32) * 1 )

typedef struct tagArchive_t
{
	ArchiveContainerEntry_t container;
	u32 signature;
	u32 version;
	u32 szheader;
	u32 szarchive;
	
	
	FILE * handle;
}Archive_t;

#define szArchive(X) ( szArchiveContainerEntry(X) + sizeof(u32) * 4 )

Archive_t * newArchive(void);
ArchiveContainerEntry_t * newArchiveContainerEntry(void);
ArchiveDataEntry_t * newArchiveDataEntry(void);

s32 ArchiveContainerEntry_add(ArchiveContainerEntry_t * const container, ArchiveBaseEntry_t * const base);

ref * reftext(const s8 * const text, u32 * const len);
ref * reftext3(const s8 * const text0, const s8 * const text1, const s8 * const text2, u32 * const len);

//Writer

ArchiveContainerEntry_t * scandirectory(const s8 * const path);

s32 getfilesize(const s8 * const filepath, size_t * const size);

ref * Node_travert(Node_t * const node, ref * const tag, ref * (* const callback) ( Node_t * const node, ref * const tag ) );

ref * getstructuresizes( Node_t * const node, ref * const tag );

ref * updateids( Node_t * const node, ref * const tag );

ref * updatedataoffsets( Node_t * const node, ref * const tag );

size_t writeArchiveBaseEntry(FILE * const handle, ArchiveBaseEntry_t * const entry);
size_t writeArchiveDataEntry(FILE * const handle, ArchiveDataEntry_t * const entry);
size_t writeArchiveContainerEntry(FILE * const handle, ArchiveContainerEntry_t * const entry);
size_t writeArchive(FILE * const handle, Archive_t * const archive);

ref * structurewriter( Node_t * const node, ref * const tag );
ref * datawriter( Node_t * const node, ref * const tag );

s32 ArchivateDirectory(const s8 * const path, const s8 * const archivefilename);

// Reader

s32 parsearchive(FILE * const handle, ArchiveContainerEntry_t * const container, s8 * const filepath);

Archive_t * readarchive(const s8 * const filepath);

ref * findentry( Node_t * const node, ref * const tag );

s32 extractdata(Archive_t * const archive, const u32 id);
s32 extractdatadirect(FILE * const archivehandle, ArchiveDataEntry_t * const data);

ref * containerdataextractor( Node_t * const node, ref * const tag );
s32 extractcontainer(Archive_t * const archive, const u32 id);

ref * printinfo( Node_t * const node, ref * const tag );
s32 archiveinfo(Archive_t * const archive, const s8 * const filepathout);
#endif