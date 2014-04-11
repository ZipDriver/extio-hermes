/** 
 * @file dllmain.h
 * @brief Header for Extio DLL service functions
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

/* Copyright (C) 
 * Andrea Montefusco IW0HDV
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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

#if !defined	__DLLMAIN_H__
#define			__DLLMAIN_H__

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>

class Dll {
public:
	Dll (HMODULE hm);
	virtual void ProcessAttach () = 0;
	virtual void ProcessDetach () = 0;
	virtual void ThreadAttach () = 0;
	virtual void ThreadDetach () = 0;
	virtual ~Dll () {};

	void inc () ;
	void dec () ;

	HMODULE getMyHandle();
	int getInstanceNumber ();
	int getInstanceQuantity () ;

	Dll() = delete;
	Dll(const Dll&) = delete;
private:
	static HMODULE hMod;
	static int instance_;
	static int local_instance;
};


HMODULE GetMyHandle();
int GetInstanceNumber ();
int GetInstanceQuantity () ;

Dll *createDll (HMODULE); // forward declaration, to be defined using DLL_CLASS macro


struct DllSingleton {
	template < class T >
	Dll *CreateDll(HMODULE h) { return new T(h); }
};


#define DLL_CLASS(T,h) \
Dll *createDll (HMODULE h) \
{ \
	return new T(h); \
};


#endif