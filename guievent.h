/**
* @file guievent.h
* @brief Header for GUI events
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

#if !defined __GUIEVENT_H__
#define __GUIEVENT_H__

struct GuiEvent {
	GuiEvent(HWND h, int i) : hWnd(h), id(i) {}
	HWND hWnd;
	int  id;
};

struct GuiEventHScroll {
	GuiEventHScroll(HWND hW, HWND hC, unsigned cn, int p) :
		hWnd(hW), hwndCtl(hC), codeNotify(cn), pos(p) {}

	HWND hWnd;		// hwnd
	HWND hwndCtl;	// (int)(LOWORD(wParam))
	UINT codeNotify;// (UINT)HIWORD(wParam))
	int  pos;
};
#endif