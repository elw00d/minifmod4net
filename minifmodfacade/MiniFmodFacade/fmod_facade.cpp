
#include "StdAfx.h"
#include "fmod_facade.h"
#include "..\minifmod\minifmod.h"
#include <map>

using namespace std;

struct ModuleDescriptor
{
public:
	int length;
	int pos;
	void* data;
	bool dataDuplicated;
	bool isFromFile;
	FMUSIC_MODULE* fmodModule;

	ModuleDescriptor() :
		length(0),
		pos(0),
		data(NULL),
		dataDuplicated(false),
		isFromFile(false),
		fmodModule(NULL)
	{
	}
};

map<const char*, MODULE_ID> idAssociation;
map<MODULE_ID, ModuleDescriptor> moduleDescriptors;

static unsigned int openCallback(char* name);
static void closeCallback(unsigned int handle);
static int readCallback(void *buffer, int size, unsigned int handle);
static void seekCallback(unsigned int handle, int pos, signed char mode);
static int tellCallback(unsigned int handle);

static bool fmodSubsystemInitialized = false;
static void initializeFmodIfNeed() {
	if (!fmodSubsystemInitialized) {
		FSOUND_File_SetCallbacks(openCallback, closeCallback, readCallback, seekCallback, tellCallback);
		fmodSubsystemInitialized = true;
	}
}

unsigned int openCallback(char* name)
{
	map<const char*, MODULE_ID>::const_iterator it = idAssociation.find(name);
	_ASSERT(idAssociation.end() != it);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(it->second);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	if (descriptor.isFromFile) {
		FILE* fp;
		fp = fopen(name, "rb");
		if (fp) {
			fseek(fp, 0, SEEK_END);
			descriptor.length = ftell(fp);
			descriptor.data = calloc(descriptor.length, 1);
			descriptor.pos = 0;

			fseek(fp, 0, SEEK_SET);
			fread(descriptor.data, 1, descriptor.length, fp);
			fclose(fp);
		}
	} else {
		_ASSERT(descriptor.data == name);

		if (descriptor.dataDuplicated) {
			descriptor.data = calloc(descriptor.length, 1);
			descriptor.data = memcpy(descriptor.data, name, descriptor.length);
		}
	}

	return it->second;
}

void closeCallback(unsigned int handle)
{
	MODULE_ID id = static_cast<MODULE_ID> (handle);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;
	if (descriptor.isFromFile || (!descriptor.isFromFile && descriptor.dataDuplicated)) {
		if (descriptor.data) {
			free(descriptor.data);
		}
	}
}

int readCallback(void *buffer, int size, unsigned int handle)
{
	MODULE_ID id = static_cast<MODULE_ID> (handle);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	if (descriptor.pos + size >= descriptor.length)
		size = descriptor.length - descriptor.pos;

	memcpy(buffer, static_cast<const char*> (descriptor.data) + descriptor.pos, size);
	descriptor.pos += size;
	
	return size;
}

void seekCallback(unsigned int handle, int pos, signed char mode)
{
	MODULE_ID id = static_cast<MODULE_ID> (handle);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	if (mode == SEEK_SET) 
		descriptor.pos = pos;
	else if (mode == SEEK_CUR) 
		descriptor.pos += pos;
	else if (mode == SEEK_END)
		descriptor.pos = descriptor.length + pos;

	if (descriptor.pos > descriptor.length)
		descriptor.pos = descriptor.length;
}

int tellCallback(unsigned int handle)
{
	MODULE_ID id = static_cast<MODULE_ID> (handle);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	return descriptor.pos;
}

bool XM_LoadModuleFromFile(MODULE_ID id, const char* fileName)
{
	initializeFmodIfNeed();

	_ASSERT(fileName);
	ModuleDescriptor descriptor;
	descriptor.isFromFile = true;

	map<MODULE_ID, ModuleDescriptor>::const_iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() == t);
	moduleDescriptors[id] = descriptor;

	idAssociation[fileName] = id;

	FMUSIC_MODULE* fmodModule = FMUSIC_LoadSong(const_cast<char*> (fileName), NULL);
	if (fmodModule) {
		moduleDescriptors[id].fmodModule = fmodModule;		
		return true;
	} else {
		moduleDescriptors.erase(id);
		return false;
	}
}

bool XM_LoadModuleFromMemory(MODULE_ID id, const char* data, int bytesCount, bool copyMemory)
{
	initializeFmodIfNeed();

	_ASSERT(data);
	ModuleDescriptor descriptor;
	descriptor.data = const_cast<char*> (data);
	descriptor.isFromFile = false;
	descriptor.dataDuplicated = copyMemory;
	descriptor.length = bytesCount;

	map<MODULE_ID, ModuleDescriptor>::const_iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() == t);
	moduleDescriptors[id] = descriptor;

	idAssociation[data] = id;

	FMUSIC_MODULE* fmodModule = FMUSIC_LoadSong(const_cast<char*> (data), NULL);
	if (fmodModule) {
		moduleDescriptors[id].fmodModule = fmodModule;
		return true;
	} else {
		moduleDescriptors.erase(id);
		return false;
	}

}


void XM_FreeModule(MODULE_ID module)
{
	MODULE_ID id = static_cast<MODULE_ID> (module);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	FMUSIC_FreeSong(descriptor.fmodModule);
	moduleDescriptors.erase(t);
}

void XM_PlayModule(MODULE_ID module)
{
	MODULE_ID id = static_cast<MODULE_ID> (module);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	FMUSIC_PlaySong(descriptor.fmodModule);
}

void XM_StopModule(MODULE_ID module)
{
	MODULE_ID id = static_cast<MODULE_ID> (module);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	FMUSIC_StopSong(descriptor.fmodModule);
}

int XM_GetCurrentOrder(MODULE_ID module)
{
	MODULE_ID id = static_cast<MODULE_ID> (module);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	return FMUSIC_GetOrder(descriptor.fmodModule);
}

int XM_GetCurrentRow(MODULE_ID module)
{
	MODULE_ID id = static_cast<MODULE_ID> (module);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	return FMUSIC_GetRow(descriptor.fmodModule);
}

int XM_GetCurrentTime(MODULE_ID module)
{
	MODULE_ID id = static_cast<MODULE_ID> (module);
	map<MODULE_ID, ModuleDescriptor>::iterator t = moduleDescriptors.find(id);
	_ASSERT(moduleDescriptors.end() != t);
	ModuleDescriptor& descriptor = t->second;

	return FMUSIC_GetTime(descriptor.fmodModule);
}
