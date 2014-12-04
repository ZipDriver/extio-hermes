/** 
* @file util.c
* @brief Utility functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-03-05
*/

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
#include <stdlib.h>
#if !defined NDEBUG && defined _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#if defined _MSC_VER
#include <strsafe.h>
#endif

#include <stdio.h>
#include <string.h>
#include "guiutil.h"
#include "log.h"
#include "util.h"

#define OZY_BUFFER_SIZE 512

/*
 * 
 */

/* --------------------------------------------------------------------------*/
/** 
* @brief Dump Ozy buffer
* 
* @param prefix
* @param buffer
*/
void dump_ozy_buffer(char* prefix,int frame,unsigned char* buffer) {
    int i;
    LOGT("%s(%d)\n",prefix,frame);
    for(i=0;i<OZY_BUFFER_SIZE;i+=16) {
        LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    LOGT("%s","\n");
}

void dump_ozy_header(char* prefix,int frame,unsigned char* buffer) {
    int i;
    LOGT("%s(%d)\n",prefix,frame);
    i=0;
    LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
            i,
            buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
            buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
            );
    LOGT("%s","\n");
}

void dump_iq_buffer(unsigned char* buffer) {
    int i;
    LOGT("%s", "iq ...\n");
    for(i=0;i<8192;i+=16) {
        LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    LOGT("%s","\n");
}

void dump_udp_buffer(unsigned char* buffer) {
    int i;
    LOGT("%s", "udp ...\n");
    for(i=0;i<512;i+=16) {
        LOGT("  [%04X] %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X\n",
                i,
                buffer[i],buffer[i+1],buffer[i+2],buffer[i+3],buffer[i+4],buffer[i+5],buffer[i+6],buffer[i+7],
                buffer[i+8],buffer[i+9],buffer[i+10],buffer[i+11],buffer[i+12],buffer[i+13],buffer[i+14],buffer[i+15]
                );
    }
    LOGT("%s","\n");
}

void dump_metis_buffer(char* prefix,int frame,unsigned char* buffer) {
    LOGT("%s header\n",prefix);
    LOGT("  %02X%02X%02X%02X%02X%02X%02X%02X\n",
                buffer[0],buffer[1],buffer[2],buffer[3],
                buffer[4],buffer[5],buffer[6],buffer[7]);
    dump_ozy_buffer(prefix,frame,&buffer[8]);
    dump_ozy_buffer(prefix,frame,&buffer[520]);
}

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>


struct MsgAllocatorImpl {
	MsgAllocatorImpl():hh(NULL) 
	{
		hh = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED, 4096, 4096 * 16);
		if (hh == NULL) {
			//ErrorLog ("HEAP CREATION ERROR");
#if defined WIN32 && !defined NDEBUG
			ErrorLog("HeapCreate");
#endif
#if !defined NDEBUG || defined FLOG
			Singleton<Log>().log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP CREATION ERROR: %d\n", GetLastError());
#endif
		}
	}
	~MsgAllocatorImpl()
	{
		if (hh != NULL) HeapDestroy(hh);
	}

	HANDLE hh;
};

MsgAllocator::MsgAllocator()
{
	pi = new MsgAllocatorImpl();
}

MsgAllocator::~MsgAllocator()
{
	delete pi;
}
char * MsgAllocator::xstrdup(const char *p)
{
	if (!(p && strlen(p))) return 0;

	if (pi->hh != NULL) {
		char *pd = (char *)HeapAlloc(pi->hh, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED, strlen(p) + 1);
		if (pd) {
			strcpy(pd, p);
			return pd;
		}
		else {
#if !defined NDEBUG || defined FLOG
			Singleton<Log>().log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP ALLOCATION ERROR: %d\n", GetLastError());
#endif
		}
	}
	else {
#if !defined NDEBUG || defined FLOG
		Singleton<Log>().log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP INVALID ERROR\n");
#endif
	}
	return 0;
}

void MsgAllocator::xstrdel(const char *p, int line)
{
	if (p == 0) return;
	if (pi->hh == 0) return;
	if (p == 0) {
#if !defined NDEBUG || defined FLOG
		Singleton<Log>().log_funcname_printf(0, __FUNCTION__, __LINE__, "POINTER INVALID ERROR: %d\n", line);
#endif
		return;
	}
	if (pi->hh == NULL) {
#if !defined NDEBUG || defined FLOG
		Singleton<Log>().log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP INVALID ERROR: %d\n", line);
#endif
		return;
	}
#if !defined NDEBUG || defined FLOG
	Singleton<Log>().log_funcname_printf(0, __FUNCTION__, __LINE__, "Free: pointer: %p line: %d\n", p, line);
#endif
	if (!HeapFree(pi->hh, HEAP_FREE_CHECKING_ENABLED | HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED, (LPVOID)p)) {
#if !defined NDEBUG || defined FLOG
		Singleton<Log>().log_funcname_printf(0, __FUNCTION__, __LINE__, "Free: %d - ", GetLastError());
#endif
#if defined WIN32 && !defined NDEBUG
		ErrorLog(TEXT("HeapFree:"));
#endif
	}
}

#if _WIN32
void ErrorLog(const char* lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
#if defined _MSC_VER
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, (LPCTSTR)lpMsgBuf);
#else	
	snprintf ((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %d: %s"), lpszFunction, dw, (LPCTSTR)lpMsgBuf );	
#endif
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
	LOGT("ERROR: %s\n", (char *)lpDisplayBuf);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}
#endif

#if 0

static HANDLE hh = NULL;


char *xstrdup(const char *p)
{
	log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP\n");

	if (hh == NULL) {
//		hh = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 4096, 4096*16);
		hh = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED, 4096, 4096 * 16);
		if (hh == NULL) {
			//ErrorLog ("HEAP CREATION ERROR");
			ErrorLog("HeapCreate");
			log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP CREATION ERROR: %d\n", GetLastError());
			return 0;
		}
	}
	if (!(p && strlen(p))) return 0;

	if (hh != NULL) {
		char *pd = (char *)HeapAlloc(hh, HEAP_ZERO_MEMORY | HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED, strlen(p)+1);
		if (pd) {
			strcpy(pd, p);
			return pd;
		} else {
			log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP ALLOCATION ERROR: %d\n", GetLastError());
		}
	} else {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP INVALID ERROR\n");
	}
	return 0;
}

void  xstrdel(const char *p, int line)
{
	if (p == 0) return;
	if (hh == 0) return;
	if (p == 0) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "POINTER INVALID ERROR: %d\n", line);
		return;
	}
	if (hh == NULL) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "HEAP INVALID ERROR: %d\n", line);
		return;
	}
	log_funcname_printf(0, __FUNCTION__, __LINE__, "Free: pointer: %p line: %d\n", p, line);
	if (!HeapFree(hh, HEAP_FREE_CHECKING_ENABLED | HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED, (LPVOID)p)) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "Free: %d - ", GetLastError());
		ErrorLog(TEXT("HeapFree:"));
	}
//	HeapFree(hh, HEAP_GENERATE_EXCEPTIONS | HEAP_TAIL_CHECKING_ENABLED, (LPVOID)p);
}
#endif

#if 0

#include <stdlib.h>

char *xstrdup(const char *p)
{
	if (p == 0 || strlen(p) == 0) return 0;

	char *pd;
	if (p && strlen(p) && (pd = (char *)malloc(strlen(p) + 1))) {
		strcpy((char *)(pd), p);
		return pd;
	} else {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s: %p %d\n", "ALLOCATION ERROR !", p, strlen(p));
		return 0;
	}
}

void  xstrdel(const char *p)
{
#if !defined NDEBUG
	return;
#endif
	if (p == 0) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s: %p\n", "FREE ERROR: NULL POINTER", p);
		return;
	}
	free((void *)p);
}
#endif


#if 0
char *xstrdup(const char *p)
{
	if (p == 0) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s\n", "NULL POINTER ERROR !");
		return 0;
	}
	if (strlen(p) == 0) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s\n", "EMPTY STRING ERROR !");
		return 0;
	}

	unsigned char *pd = 0;
	if (p && strlen(p) && (pd = new unsigned char[(strlen(p) + 3 + 2)])) {
		
		pd[0] = 0x55;
		pd[1] = (strlen(p) & 0xFF);
		pd[2] = ((strlen(p) >> 8) & 0xFF);
		strcpy((char *)(pd + 3), p);

		pd[strlen(p) + 4] = 0xAA;

		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s: [%8.8s...] %d\n", "ALLOCATION", pd + 3, strlen(p));
		return (char *)(pd + 3);
	} else {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s: %p %d\n", "ALLOCATION ERROR !", p, strlen(p));
		return 0;
	}
}

void  xstrdel(const char *p)
{
	if (p == 0) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s\n", "NULL POINTER ERROR !");
		return;
	}
	if (strlen(p) == 0) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s\n", "EMPTY STRING ERROR !");
		return;
	}

	int len = strlen(p); 
	--p; --p; --p; // point back to the origin

	if (*p != 0x55) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s\n", "FRONT DELETE ERROR !");
	}
	if ((unsigned char)(p[len+4]) != 0xAA) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s\n", "TRAILING DELETE ERROR !");
	}
	int xlen = p[1] | (p[2] << 8);

	if (xlen != len) {
		log_funcname_printf(0, __FUNCTION__, __LINE__, "%s: is %d, %d was expected \n", "LENGTH ERROR !", len, xlen);
	}
	delete[] p;
}

#endif
