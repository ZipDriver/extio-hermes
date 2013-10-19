// dllmain.cpp : Defines the entry point for the DLL application.
#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
//#include <winsock.h>
#include "dllmain.h"

#pragma data_seg (".SS_DLLMAIN")
int Dll::instance_ = 0;
#pragma data_seg()

HMODULE Dll::hMod = 0;
int Dll::local_instance = 0;

Dll::Dll (HMODULE hm)
{
	hMod = hm;
}
void Dll::inc () 
{	
	instance_ = instance_ + 1;
	local_instance = instance_;
}
	
HMODULE Dll::getMyHandle() { return hMod; }
int Dll::getInstanceNumber () { return local_instance; }


static Dll *pObj = 0;

BOOL APIENTRY DllMain ( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved )
{
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		if (pObj == 0) pObj = ::createDll (hModule);

		if (pObj) {
			pObj->inc ();
			pObj->ProcessAttach ();
		}
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
			delete pObj;
			pObj = 0;
		}
		break;
	}
	return TRUE;
}


HMODULE GetMyHandle()
{
	return pObj->getMyHandle();
}

int GetInstanceNumber ()
{
	return pObj->getInstanceNumber(); 
}

#if 0

BOOL APIENTRY DllMain ( HMODULE hModule, DWORD ul_reason_for_call, LPVOID /* lpReserved */ )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hMod = hModule;
		instance_ = instance_ + 1;
		local_instance = instance_;
		// initialize Windows sockets
		{
			WORD wVersionRequested;
			WSADATA wsaData;
			int err;

			wVersionRequested = MAKEWORD(1, 1);
			err = WSAStartup(wVersionRequested, &wsaData);
		}
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

HMODULE GetMyHandle()
{ return hMod; }

int GetInstanceNumber ()
{ return local_instance; }

#endif