/**
* @file gui.cpp
* @brief Header for Extio DLL GUI classes
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#if defined _MSC_VER || defined __MINGW32__
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")
#include <windowsx.h>
#include <commctrl.h>			// Include header
#pragma comment(lib, "comctl32.lib")	// Include library
#pragma warning( disable : 4995 )
#endif

#if defined _MSC_VER
#include <strsafe.h>
#endif


#include "util.h"
#include "hpsdr.h"
#include "log.h"
#include "resource.h"
#include "log.h"
#include "dllmain.h" // for GetMyHandle()
#include "Extio_config.h"
#include "ExtIO_hermes.h"
#include "guievent.h"
#include "guiutil.h"
#include "gui.h" 

static const char *buildString = " - " __DATE__ ", " __TIME__ " - "
#if defined _MSC_VER
"ms";
#elif defined __MINGW32__
"gcc";
#else
"";
#endif


/*
 *  Gui utilities
 *  
 */


struct WSize {

	WSize() = delete;
	WSize(HWND hd)
	{
		RECT rd, rD;
		GetWindowRect(GetDesktopWindow(), &rD);
		GetWindowRect(hd, &rd);
		x1 = rd.left, x2 = rd.right, y1 = rd.top, y2 = rd.bottom;
		w = x2 - x1;
		h = y2 - y1;
		x1_dt = rD.left, x2_dt = rD.right, y1_dt = rD.top, y2_dt = rD.bottom;
		w_dt = rD.right - rD.left;
		h_dt = rD.bottom - rD.top;
	}
	void center(int &x, int &y)
	{
		x = (w_dt / 2) - (w / 2);
		y = (h_dt / 2) - (h / 2);
	}
	void lower_right(int &xx, int &yy)
	{
		xx = x2_dt - w;
		yy = y2_dt - h;
	}

	int x1, x2, y1, y2, w, h;
	int x1_dt, x2_dt, y1_dt, y2_dt, w_dt, h_dt;
};



/*
 *  Gui base class
 *
 */

class GuiImpl {
public:

	GuiImpl(): hDialog(NULL), pr(0), pExr(0) {}
	HWND hDialog;
	Radio *pr;
	ExtioHpsdrRadio < EXTIO_BASE_TYPE > *pExr;

	virtual ~GuiImpl () {}

public:
	static BOOL CALLBACK CtrlBoxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	static BOOL CtrlBoxDlgProcOnInit(HWND hDlg, HWND hFocus, LPARAM lParam);

	static BOOL CtrlBoxDlgProcOnHScroll(
		HWND hWnd,         // hwnd
		HWND hwndCtl,      // (int)(LOWORD(wParam))
		UINT codeNotify,   // (UINT)HIWORD(wParam))
		int  pos
	);

	static BOOL CtrlBoxDlgProcOnCommand(
		HWND hWnd,         // hwnd
		int id,            // (int)(LOWORD(wParam))
		HWND hCtl,         // (HWND)(lParam)
		UINT codeNotify    // (UINT)HIWORD(wParam))
		);

	static BOOL CALLBACK MyEnumWindowsProc(HWND hwnd, LPARAM lParam);
};

//
// (hwnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L
//
BOOL GuiImpl::CtrlBoxDlgProcOnCommand (
	HWND hWnd,         // hwnd
	int id,            // (int)(LOWORD(wParam))
	HWND hCtl,         // (HWND)(lParam)
	UINT codeNotify    // (UINT)HIWORD(wParam))
)
{
	LOGT("%p\n", GetWindowLongPtr(hWnd, GWLP_USERDATA));
	Gui *pGui = (Gui *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (pGui) {
		LOGT("********************************* Gui addr: %p pi: %p\n", pGui, pGui->pi);

		// some button/checkbox has been clicked
		// the lower word of wParam holds the controls ID
		if (codeNotify == BN_CLICKED) pGui->ButtonClick(GuiEvent(hWnd, id));
		if (codeNotify == LBN_DBLCLK) pGui->ListBoxDoubleClick(GuiEvent(hWnd, id));
	}
	return TRUE;
}



BOOL GuiImpl::CtrlBoxDlgProcOnInit(HWND hDlg, HWND hFocus, LPARAM lParam)
{
	LOGT("%08x\n", lParam);
	Gui *pGui = (Gui *)lParam;
	LOGT("********************************* Gui addr: %p\n", pGui);

	return TRUE;
}


BOOL CALLBACK GuiImpl::CtrlBoxDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL fProcessed = TRUE;
	Gui *pGui = (Gui *)GetWindowLongPtr(hDlg, GWLP_USERDATA);
	//LOGT("********************************* %u %ld %d Gui addr: %p\n", uMsg, wParam, lParam, pGui);

	switch (uMsg) {
		HANDLE_MSG(hDlg, WM_INITDIALOG, GuiImpl::CtrlBoxDlgProcOnInit);
		HANDLE_MSG(hDlg, WM_COMMAND, GuiImpl::CtrlBoxDlgProcOnCommand);
		HANDLE_MSG(hDlg, WM_HSCROLL, GuiImpl::CtrlBoxDlgProcOnHScroll);
	default:
		// process WM_USER messages avoiding to be fooled from WM_APP
		if (uMsg > WM_USER && uMsg < WM_APP) {
			//LOGT("********************************* MSG: 0x%08X WM_USER + %d\n", uMsg, uMsg - WM_USER);
			if (pGui) pGui->OnWmUser(uMsg - WM_USER, GuiEvent(hDlg, wParam));
		} else {
			fProcessed = FALSE;
		}
		break;
	}
	return (fProcessed);
}

BOOL GuiImpl::CtrlBoxDlgProcOnHScroll (
	HWND hWnd,		// hwnd
	HWND hwndCtl,	// (int)(LOWORD(wParam))
	UINT codeNotify,// (UINT)HIWORD(wParam))
	int  pos
	)
{
	Gui *pGui = (Gui *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (pGui) {
		LOGT("********************************* Gui addr: %p pi: %p\n", pGui, pGui->pi);
		return pGui->OnHScroll(GuiEventHScroll(hWnd, hwndCtl, codeNotify, pos));
	}
	else {
		return FALSE;
	}
}


Gui::Gui(): pi(0)
{
}

Gui::Gui (int id) : pi(new GuiImpl)
{
	LOGT("********************************* pImpl: %p Gui addr: %p\n", pi, this);

	if (pi) {

		pi->hDialog = CreateDialogParam(GetMyHandle(),
			MAKEINTRESOURCE(id),
			HWND_DESKTOP,
			GuiImpl::CtrlBoxDlgProc,
			(LONG)this
			);
		if (pi->hDialog != NULL) {
			// do not show window here !!!

			// setup class instance pointer into Windows object user data
			SetWindowLongPtr(pi->hDialog, GWLP_USERDATA, (LONG)this);
			return;
		}
	}
	ErrorLog(TEXT("Gui: main dialog failed !"));
}


Gui :: ~Gui()
{
	LOGT("********************************* GUI destructor: Gui addr: %p\n", pi);
	
	if (pi) {
		if (pi->hDialog) DestroyWindow(pi->hDialog);
		delete pi;
	}
}

bool Gui :: OnInit(const GuiEvent&) 
{ 
	LOGT("%s\n", "*********************************");
	return false; 
}


void Gui::Show()
{
	int x, y;

	if (pi && pi->hDialog) {
		WSize(pi->hDialog).lower_right(x, y);
		LOGT("xxxxxxxxxxxxxxxxxx GUI Show: move to x= %d y= %d\n", x, y);
		SetWindowPos(pi->hDialog, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
		ShowWindow(pi->hDialog, SW_SHOW);
	}
}

void Gui::Hide()
{
	if (pi && pi->hDialog) ShowWindow(pi->hDialog, SW_HIDE);
}

void Gui::EnableControls()
{
	if (pi && pi->hDialog) EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, true));
}

void Gui::DisableControls()
{
	if (pi && pi->hDialog) EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, false));
}

void Gui::setHwAddressGUI(const Ethernet::Device *pDev)
{
	char szBuf[1024];

	if (pDev) {
		snprintf(szBuf, sizeof(szBuf), 
				"%s - %s\t\t\t%s %1.1f", 
				pDev->ip_address, 
				pDev->mac_address, 
				pDev->board_id, 
				((float)pDev->code_version) / 10.0f);
	} else {
		snprintf(szBuf, sizeof(szBuf), "%s", "HARDWARE NOT FOUND !!!!!");
	}
	if (pi && pi->hDialog) PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(szBuf)), static_cast<LPARAM>(0));
	//	free (pszBuf);  FREE IS DONE IN DIALOG BOX !!!!!!!
}


void Gui::setHw (const char *pszBuf)
{
	if (pszBuf && strlen(pszBuf) && pi && pi->hDialog) {
		PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
	}
}

void Gui::appendMessage (const char *pszBuf)
{
	if (pszBuf && strlen(pszBuf) && pi && pi->hDialog) PostMessage(pi->hDialog, WM_USER + 3, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
}

int  Gui::getRecNumber (void)
{
	char  buf[256];
	int   n = 1;

	if (pi && pi->hDialog) {
		SendMessage(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf);
		if (sscanf(buf, "%d", &n) == 1) return n;
	}
	LOGT("Internal error !! Returning %d\n", n);
	return n;
}

void Gui::setRadio (ExtioHpsdrRadio < EXTIO_BASE_TYPE > *pR)
{
	if (pR && pi) {
		pi->pExr = pR;
		pi->pr = pi->pExr->getRadio();
	}
}
ExtioHpsdrRadio < EXTIO_BASE_TYPE > * Gui::getRadio()
{
	if (pi && pi->pExr) {
		return pi->pExr;
	} else
		return 0;
}

BOOL CALLBACK GuiImpl::MyEnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	GuiEvent *pev = (GuiEvent *)lParam;

	if (pev) {
		bool fEnable = (pev->id > 0);

// for debug only
//		char szWindowText[256];
//		::GetWindowText(hWnd, szWindowText, sizeof(szWindowText));
//		LOGT("%10.10s: %d\n", szWindowText, fEnable);

		if (pev->hWnd == 0) // if nullhandle is specified, applies to children
			::EnableWindow(hWnd, fEnable);
		else                // if an handle is specified, applies only to that window
			if (pev->hWnd == hWnd) ::EnableWindow(hWnd, fEnable);
	}
	return TRUE;
}

void Gui::EnableAll (const GuiEvent& ev1, const GuiEvent& ev2)
{
	::EnumChildWindows(ev1.hWnd, GuiImpl::MyEnumWindowsProc, (LPARAM)&ev2);
}


/*
 * H E R M E S   Gui
 *
 *
 */ 


HermesGui::HermesGui(int sample_rate) : Gui(IDD_HERMES), sr(sample_rate)
{

	LOGT("********************************* HermesCreateGUI: addr: %p\n", this);
	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}

void HermesGui::EnableControls()
{
	if (pi) {
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, true));
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), false));
	}
	Gui::Show();
}

void HermesGui::DisableControls()
{
	if (pi) {
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(0, false));
		EnableAll(GuiEvent(pi->hDialog, 0), GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), true));
	}
	Gui::Show();
}


bool HermesGui::OnInit(const GuiEvent& ev)
{ 
	// 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760238%28v=vs.85%29.aspx
	//
	HWND sliderAtt = GetDlgItem(ev.hWnd, IDC_SLIDER_ATT);
	SendMessage(sliderAtt, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 30));
	SendMessage(sliderAtt, TBM_SETPOS, (WPARAM)0, 0);
	SendMessage(sliderAtt, TBM_SETTICFREQ, (WPARAM)10, 0);
	SendMessage(sliderAtt, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)MAKELONG(0, 10));
	// fillin the rx number combo box
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"2");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"3");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"4");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"5");
	// default at first item, one receiver(s)
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 0, 0);
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 1, 0);

	switch (sr) {
	case 384000:
		// order of parameters in the call: 
		//                                     first             last             check
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_384K);
		break;
	case 192000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_192K);
		break;
	case 96000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_96K);
		break;
	case 48000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_48K);
		break;
	}
	EnableAll(ev, GuiEvent(0, false));
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), true));

	AppendWinTitle(GuiEvent(pi->hDialog, 0), buildString);

	return true; 
}
bool HermesGui::ButtonClick(const GuiEvent &ev)
{
	// some button/checkbox has been clicked
	// the lower word of wParam holds the controls ID
	if (ev.id >= IDC_RADIO_BW_384K && ev.id <= IDC_RADIO_BW_48K)  {
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, ev.id);
		switch (ev.id) {
		case IDC_RADIO_BW_384K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 384 kS/s\r\n");
			LOGT("%s", "New Bandwith: 384 kS/s\r\n");
			(pi->pExr)->setSampleRateHW(384000);
			break;
		case IDC_RADIO_BW_192K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 192 kHz\r\n");
			LOGT("%s", "New Bandwith: 192 kHz\r\n");
			(pi->pExr)->setSampleRateHW(192000);
			break;
		case IDC_RADIO_BW_96K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 96 kHz\r\n");
			LOGT("%s", "New Bandwith: 96 kHz\r\n");
			(pi->pExr)->setSampleRateHW(96000);
			break;
		case IDC_RADIO_BW_48K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwith: 48 KHz\r\n");
			LOGT("%s", "New Bandwith: 48 KHz\r\n");
			(pi->pExr)->setSampleRateHW(48000);
			break;
		}
	}
	if (ev.id >= IDC_CB_DITHER && ev.id <= IDC_CB_RANDOMIZER)  {

		HWND hCtrl = GetDlgItem(ev.hWnd, ev.id);
		const char *pszState;
		int   newStatus;

		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			pszState = " activated\r\n";
			newStatus = 1;
		}
		else {
			pszState = " deactivated\r\n";
			newStatus = 0;
		}
		switch (ev.id) {
		case IDC_CB_PREAMP:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Preamp");
			pi->pr->setPreamp(newStatus==1);
			break;
		case IDC_CB_RANDOMIZER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Randomizer");
			pi->pr->setRandomizer(newStatus==1);
			break;
		case IDC_CB_DITHER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Dither");
			pi->pr->setDither(newStatus==1);
			break;
		}
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), pszState);
	}
	return true;
}

bool HermesGui::OnHScroll(const GuiEventHScroll& ev)
{
	if (GetDlgItem(ev.hWnd, IDC_SLIDER_ATT) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_GETPOS, 0, 0);
		int snap = (newPos % 10);
		LOGT("NEW: %d SNAP: %d\r\n", newPos, snap);
		if (snap) {
			if (snap > 5) newPos += (10 - snap);
			else newPos = newPos - snap;
			if (newPos > 30) newPos = 30;
			LOGT("MOVE: %d\r\n", newPos);
			SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)newPos);
		}
		else {
			LOGT("New attenuator value: %d\r\n", newPos);
			pi->pr->setAttenuator(newPos);
		}
		return true;
	}
	else
		return false;
}

bool HermesGui::OnWmUser(int n, const GuiEvent& ev)
{
	char * pszText = reinterpret_cast<char *>(ev.id);

	if (n == 2 && pszText) {
		SetWindowText (GetDlgItem(ev.hWnd, IDC_STATIC_HW), pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
	if (n = 3 && pszText) {
		AppendTextToEditCtrl (GuiEvent(ev.hWnd, IDC_MSG_PANE), pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
		return false;
}




bool MercuryGui::OnInit(const GuiEvent& ev)
{
	LOGT("Event ref: %p\n", ev);

	// 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760238%28v=vs.85%29.aspx
	//
	HWND sliderAtt = GetDlgItem(ev.hWnd, IDC_SLIDER_ATT);
	SendMessage(sliderAtt, TBM_SETRANGE, (WPARAM)0, (LPARAM)MAKELONG(0, 30));
	SendMessage(sliderAtt, TBM_SETPOS, (WPARAM)0, 0);
	SendMessage(sliderAtt, TBM_SETTICFREQ, (WPARAM)10, 0);
	SendMessage(sliderAtt, TBM_SETPAGESIZE, (WPARAM)0, (LPARAM)MAKELONG(0, 10));
	// fillin the rx number combo box
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"2");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"3");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"4");
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_ADDSTRING, 0, (LPARAM)"5");
	// default at first item, one receiver(s)
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 0, 0);
	SendMessage(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), CB_SETCURSEL, 1, 0);

	switch (sr) {
	case 384000:
		// first last check
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_384K);
		break;
	case 192000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_192K);
		break;
	case 96000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_96K);
		break;
	case 48000:
		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_48K);
		break;
	}
	CheckRadioButton(ev.hWnd, IDC_ALEX_FILTER_AUTO, IDC_ALEX_FILTER_AUTO, IDC_ALEX_FILTER_AUTO);

	// default to Antenna 1
	CheckRadioButton(ev.hWnd, IDC_ANT_1, IDC_ANT_3, IDC_ANT_1 );
	                       
	// disable all controls
	EnableAll(ev, GuiEvent(0, false));
	
	// enable receivers selection
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_COMBO_N_RX), true));
	// enable bandwidth selection
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_384K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_192K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_96K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_48K), true));	
	
	AppendWinTitle(GuiEvent(pi->hDialog, 0), buildString);

	return true;
}


bool MercuryGui::ButtonClick(const GuiEvent &ev)
{
	// some button/checkbox has been clicked
	// GuiEvent ev object contains the handle of dialog and the id of control that has been clicked

	if (ev.id >= IDC_RADIO_BW_384K && ev.id <= IDC_RADIO_BW_48K)  {
		if ( GetInstanceQuantity () != 1 ) {
			EnableControls ();
			return false;
		}

		CheckRadioButton(ev.hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, ev.id);
		switch (ev.id) {
		case IDC_RADIO_BW_384K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 384 kS/s\r\n");
			LOGT("%s\n", "New Bandwith: 384 kHz");
			(pi->pExr)->setSampleRateHW(384000);
			break;
		case IDC_RADIO_BW_192K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 192 kHz\r\n");
			LOGT("%s\n", "New Bandwith: 192 kHz");
			(pi->pExr)->setSampleRateHW(192000);
			break;
		case IDC_RADIO_BW_96K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 96 kHz\r\n");
			LOGT("%s\n", "New Bandwith: 96 kHz");
			(pi->pExr)->setSampleRateHW(96000);
			break;
		case IDC_RADIO_BW_48K:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "New Bandwidth: 48 KHz\r\n");
			LOGT("%s\n", "New Bandwith: 48 KHz");
			(pi->pExr)->setSampleRateHW(48000);
			break;
		}
	}


	if (ev.id >= IDC_RB_ALEX_LP_3020 && ev.id <= IDC_RB_ALEX_LP_1715) {
		CheckRadioButton(ev.hWnd, IDC_RB_ALEX_LP_3020, IDC_RB_ALEX_LP_1715, ev.id);
		switch (ev.id) {
		case IDC_RB_ALEX_LP_3020:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 30/20 m\r\n");
			(pi->pr)->setLP(AlexFilter::LowPass::_3020m);
			break;
		case IDC_RB_ALEX_LP_6040:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 60/40 m\r\n");
			(pi->pr)->setLP(AlexFilter::LowPass::_6040m);
			break;
		case IDC_RB_ALEX_LP_80:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 80 m\r\n");
			(pi->pr)->setLP(AlexFilter::LowPass::_80m);
			break;
		case IDC_RB_ALEX_LP_160:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 160 m\r\n");
			(pi->pr)->setLP(AlexFilter::LowPass::_160m);
			break;
		case IDC_RB_ALEX_LP_6:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 6 m\r\n");
			(pi->pr)->setLP(AlexFilter::LowPass::_6m);
			break;
		case IDC_RB_ALEX_LP_1210:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 12/10 m\r\n");
			(pi->pr)->setLP(AlexFilter::LowPass::_1210m);
			break;
		case IDC_RB_ALEX_LP_1715:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "LOW PASS: 17/15 m\r\n");
			(pi->pr)->setLP(AlexFilter::LowPass::_1715m);
			break;
		}
	}

	if (ev.id >= IDC_ALEX_HP_BYPASS && ev.id <= IDC_ALEX_HP_6M_LNP) {
		CheckRadioButton(ev.hWnd, IDC_ALEX_HP_BYPASS, IDC_ALEX_HP_6M_LNP, ev.id);
		switch (ev.id) {
		case IDC_ALEX_HP_BYPASS:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: bypass\r\n");
			(pi->pr)->setHP(AlexFilter::HighPass::_bypass);
			break;
		case IDC_ALEX_HP_13MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 13 MHz\r\n");
			(pi->pr)->setHP(AlexFilter::HighPass::_13M);
			break;
		case IDC_ALEX_HP_20MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 20 MHz\r\n");
			(pi->pr)->setHP(AlexFilter::HighPass::_20M);
			break;
		case IDC_ALEX_HP_9_5MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 9.5 MHz\r\n");
			(pi->pr)->setHP(AlexFilter::HighPass::_9_5M);
			break;
		case IDC_ALEX_HP_6_5MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 6.5 MHz\r\n");
			(pi->pr)->setHP(AlexFilter::HighPass::_6_5M);
			break;
		case IDC_ALEX_HP_1_5MHZ:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 1.5 MHz\r\n");
			(pi->pr)->setHP(AlexFilter::HighPass::_1_5M);
			break;
		case IDC_ALEX_HP_6M_LNP:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "HIGH PASS: 6 MHz + LNP\r\n");
			(pi->pr)->setHP(AlexFilter::HighPass::_6M);
			break;
		}
	}

	if (ev.id == IDC_ALEX_FILTER_AUTO) {
		const char *pszState;

		HWND hCtrl = GetDlgItem(ev.hWnd, ev.id);
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			pszState = "ALEX AUTO\n";
			(pi->pr)->setManual(false);
		}
		else {
			pszState = "ALEX MANUAL\r\n";
			(pi->pr)->setManual(true);
		}
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), pszState);
		LOGT("%p %s", (pi->pr), pszState);
	}

	if (ev.id >= IDC_CB_DITHER && ev.id <= IDC_CB_RANDOMIZER)  {
		HWND hCtrl = GetDlgItem(ev.hWnd, ev.id);
		const char *pszState;
		int   newStatus;
		if (IsDlgButtonChecked(ev.hWnd, ev.id) == BST_CHECKED) {
			pszState = " activated\r\n";
			newStatus = 1;
		}
		else {
			pszState = " deactivated\r\n";
			newStatus = 0;
		}
		switch (ev.id) {
		case IDC_CB_PREAMP:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Preamp");
			pi->pr->setPreamp(newStatus == 1);
			break;
		case IDC_CB_RANDOMIZER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Randomizer");
			pi->pr->setRandomizer(newStatus == 1);
			break;
		case IDC_CB_DITHER:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Dither");
			pi->pr->setDither(newStatus == 1);
			break;
		}
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), pszState);
	}
	
	if (ev.id >= IDC_ANT_1 && ev.id <= IDC_ANT_3) {
		CheckRadioButton(ev.hWnd, IDC_ANT_1, IDC_ANT_3, ev.id);
		switch (ev.id) {
		case IDC_ANT_1:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 1\r\n");
			(pi->pr)->setTxAnt(0);
			break;
		case IDC_ANT_2:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 2\r\n");
			(pi->pr)->setTxAnt(1);
			break;
		case IDC_ANT_3:
			AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_MSG_PANE), "Tx Antenna: 3\r\n");
			(pi->pr)->setTxAnt(2);
			break;			
		}
	}
	return true;
}

void MercuryGui::EnableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, true));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), false));
	
	if ( GetInstanceQuantity () != 1 ) {
		// disable bandwidth selection
		EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_384K), false));	
		EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_192K), false));	
		EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_96K), false));	
		EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_48K), false));	
	}
	Gui::Show();
}

void MercuryGui::DisableControls()
{
	GuiEvent ev(pi->hDialog, 0);
	
	EnableAll(ev, GuiEvent(0, false));
	EnableAll(ev, GuiEvent(GetDlgItem(pi->hDialog, IDC_COMBO_N_RX), true));
	// enable bandwidth selection
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_384K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_192K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_96K), true));	
	EnableAll(ev, GuiEvent(GetDlgItem(ev.hWnd, IDC_RADIO_BW_48K), true));	
	Gui::Show();
}


MercuryGui::MercuryGui(int sample_rate) : Gui(IDD_MERCURY), sr(sample_rate)
{
	LOGT("********************************* MercuryCreateGUI: pImpl: %p Gui addr: %p\n", pi, this);

	if (pi && pi->hDialog) OnInit(GuiEvent(pi->hDialog, -1));
}

bool MercuryGui::OnWmUser(int n, const GuiEvent& ev)
{
	char * pszText = reinterpret_cast<char *>(ev.id);

	if (n == 2 && pszText) {
		LOGT("2: %s\n", pszText);
		SetWindowText (GetDlgItem(ev.hWnd, IDC_STATIC_HW), (LPCTSTR)pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
	if (n = 3 && pszText) {
		LOGT("3: %s\n", pszText);
		AppendTextToEditCtrl (GuiEvent(ev.hWnd, IDC_MSG_PANE), pszText);
		xstrdel(pszText, __LINE__);
		return true;
	} else
		return false;
}

bool MercuryGui::OnHScroll(const GuiEventHScroll& ev)
{
	if (GetDlgItem(ev.hWnd, IDC_SLIDER_ATT) == ev.hwndCtl) {
		DWORD newPos = SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_GETPOS, 0, 0);
		int snap = (newPos % 10);
		LOGT("NEW: %d SNAP: %d\r\n", newPos, snap);
		if (snap) {
			if (snap > 5) newPos += (10 - snap);
			else newPos = newPos - snap;
			if (newPos > 30) newPos = 30;
			LOGT("MOVE: %d\r\n", newPos);
			SendMessage(GetDlgItem(ev.hWnd, IDC_SLIDER_ATT), TBM_SETPOS, (WPARAM)TRUE, (LPARAM)newPos);
		}
		else {
			LOGT("New attenuator value: %d\r\n", newPos);
			pi->pr->setAttenuator(newPos);
		}
		return true;
	} else
	return false;
}


/**
 *
 * Splash Screen dialog
 *
 */



bool HpsdrSplash::ListBoxDoubleClick(const GuiEvent &ev)
{
	if (ev.id == IDC_LBOX_RADIO_FOUND) {
		// recover the item of list box that has been clicked
		sel = ListBox_GetCurSel(ev.hWnd);
		LOGT("xxxxxxxxxxxxxxxxxx GUI Splash: item #%d selected\n", sel);
		if (!(*(ppGui_))) {
			LOGT("Gui Splash: BOARD ID creating gui: [%s]\n", pDev->board_id);

			if (!pi) {
				GuiError("Bad environment, unable to start receiver !").show();
				return false;
			} 
			
			if (!(pi->pExr)) {

				// Create Radio
				pi->pExr = CreateExtioHpsdrRadio<EXTIO_BASE_TYPE>(pDev->board_id);

				if (! (pi->pExr)) {
					GuiError("Hardware unsupported, unable to start receiver !").show();
					return 0;
				} else {
					
					// Create Gui
					*(ppGui_) = pi->pExr->CreateGui(EXTIO_DEFAULT_SAMPLE_RATE);

					if (*(ppGui_)) {
						(*(ppGui_))->setRadio(pi->pExr); // assign ExtioRadio to Gui
						Hide();							 // hide ourselves (Splash)
						(*(ppGui_))->Show();			 // show Radio Gui
					} else
						return false;
				}
			}
		}
	}
	return true;
}



bool HpsdrSplash::OnWmUser(int n, const GuiEvent& ev)
{
	char * pszText = reinterpret_cast<char *>(ev.id);
	
	if (n == 2 && pszText)	{
		SetWindowText(GetDlgItem(ev.hWnd, IDC_SPLSH_MSG), (LPCTSTR)(char *)pszText);
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
	} else
	if (n == 3 && pszText) {
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_SPLSH_MSG_PANE), pszText);
		AppendTextToEditCtrl(GuiEvent(ev.hWnd, IDC_SPLSH_MSG_PANE), "\r\n");
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
	} else 
	if (n == 4 && pszText)	{
		SendMessage(GetDlgItem(ev.hWnd, IDC_LBOX_RADIO_FOUND), LB_INSERTSTRING, -1 /* Index */, (LPARAM)pszText);
		BringWindowToTop(ev.hWnd);
		xstrdel(pszText, __LINE__);
	} 
	return true;
}




HpsdrSplash::HpsdrSplash(Gui **p) : Gui(IDD_SPLASH_DIALOG), sel(-1), ppGui_(p), pDev(0)
{
	OnInit(GuiEvent(pi->hDialog, -1));
}

bool HpsdrSplash::OnInit(const GuiEvent& ev)
{
	AppendWinTitle(GuiEvent(pi->hDialog, 0), buildString);
	return true;
}


void HpsdrSplash::AppendMessage(const char *pszBuf)
{
	PostMessage(pi->hDialog, WM_USER + 3, reinterpret_cast<WPARAM>(xstrdup(pszBuf)), static_cast<LPARAM>(0));
}


template <typename... ARGS>
void HpsdrSplash::SetStatus(const char *format, ARGS... args)
{
	char szBuf[1024];

	snprintf(szBuf, sizeof(szBuf), format, args...);
	PostMessage(pi->hDialog, WM_USER + 2, reinterpret_cast<WPARAM>(xstrdup(szBuf)), static_cast<LPARAM>(0));
}

int HpsdrSplash::ScanStarted()
{
	SetStatus("%s", "Searching for hardware..."); 
	return 0; 
}

int HpsdrSplash::ScanStopped(int nh)
{ 
	if (nh == 0) SetStatus("%s", "Search done. No device found"); 
	else SetStatus("Search done. %d device(s) found", nh);
	return 0; 
}

int	HpsdrSplash::InterfaceFound(Ethernet::NetInterface *pni)	
{
	AppendMessage(pni->name);  
	return 0;
}

int	HpsdrSplash::DeviceFound(Ethernet::Device *pd)		
{
	// append a clear text (progress) message
	AppendMessage(pd->board_id); 
	// append to the list box
	char szBuf[1024];
	snprintf(szBuf, sizeof(szBuf), "%s - %s", pd->board_id, pd->ip_address);
	PostMessage(pi->hDialog, WM_USER + 4, reinterpret_cast<WPARAM>(xstrdup(szBuf)), static_cast<LPARAM>(0) );
	pDev = pd;
	return 0; 
}

void HpsdrSplash::Show()
{
	int x, y;

	if (pi && pi->hDialog) {
		WSize(pi->hDialog).center(x, y);
		SetWindowPos(pi->hDialog, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
		ShowWindow(pi->hDialog, SW_SHOW);
	}
}







