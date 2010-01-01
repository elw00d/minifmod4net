
#ifndef __FMOD_FACADE_H
#define __FMOD_FACADE_H

#define _CRT_SECURE_NO_WARNINGS

typedef int MODULE_ID ;

#ifdef FMOD_EXPORT

#define FMOD_API __declspec (dllexport)

#else

#define FMOD_API __declspec (dllimport)

#endif

extern "C" {
	FMOD_API bool XM_LoadModuleFromFile(MODULE_ID id, const char* fileName);
	FMOD_API bool XM_LoadModuleFromMemory(MODULE_ID id, const char* data, int bytesCount, bool copyMemory);
	FMOD_API void XM_FreeModule(MODULE_ID module);
	FMOD_API void XM_PlayModule(MODULE_ID module);
	FMOD_API void XM_StopModule(MODULE_ID module);
	FMOD_API int XM_GetCurrentOrder(MODULE_ID module);
	FMOD_API int XM_GetCurrentRow(MODULE_ID module);
	FMOD_API int XM_GetCurrentTime(MODULE_ID module);
}


#endif