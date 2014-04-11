#if !defined NDEBUG && (defined _MSC_VER || defined __MINGW32__)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

// dllmain.cpp : Defines the entry point for the DLL application.
#pragma once

#include "log.h"
#include "dllmain.h"
#include "util.h"     // for shared macro


#pragma data_seg (".SS_DLLMAIN")
int Dll::instance_ SHARED = 0 ;
#pragma data_seg()

HMODULE Dll::hMod = 0;
int Dll::local_instance = 0;

Dll::Dll (HMODULE hm)
{
	hMod = hm;
}

/**
 * Instance tracker
 */
void Dll::inc () 
{	
	instance_ = instance_ + 1;
	local_instance = instance_;
}
void Dll::dec()
{
	if (instance_) instance_ = instance_ - 1;
	local_instance = 0;
}

HMODULE Dll::getMyHandle() { return hMod; }
int Dll::getInstanceNumber () { return local_instance; }
int Dll::getInstanceQuantity () { return instance_; }

static Dll *pObj = 0;

BOOL APIENTRY DllMain ( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		if (pObj == 0) pObj = ::createDll (hModule); // creates instance of Dll's derived class

		if (pObj) {
			pObj->inc ();
			pObj->ProcessAttach ();
		}
		LOG_OPEN("hermes", GetInstanceNumber());
		#if defined __MINGW32__
		LOGT("%s", "Compiled with MinGW\n");
		#endif
		break;

	case DLL_THREAD_ATTACH:
		if (pObj) pObj->ThreadAttach ();
		break;

	case DLL_THREAD_DETACH:
		if (pObj) pObj->ThreadDetach ();
		break;

	case DLL_PROCESS_DETACH:
		if (pObj) {
			pObj->ProcessDetach ();
			pObj->dec();
			delete pObj;
			pObj = 0;
#if !defined NDEBUG && (defined _MSC_VER || defined __MINGW32__)
			_CrtDumpMemoryLeaks();
#endif
		}
		break;
	}
	return TRUE;
}


class DllX {
	static int APIENTRY DllMain(HMODULE);
};

int APIENTRY DllX::DllMain(HMODULE h)
{
	return 0;
}

HMODULE GetMyHandle()
{
	return pObj->getMyHandle();
}

int GetInstanceNumber ()
{
	return pObj->getInstanceNumber(); 
}

int GetInstanceQuantity ( )
{
	return pObj->getInstanceQuantity ();
}