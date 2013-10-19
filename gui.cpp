//#include <memory>
//#include <windows.h>

#include "hpsdr.h"
#include <windowsx.h>
#include <commctrl.h>			// Include header
#pragma comment(lib, "comctl32.lib")	// Include library
#pragma warning( disable : 4995 )

#include <strsafe.h>

#include "resource.h"
#include "log.h"
#include "gui.h" 
#include "dllmain.h" // for GetMyHandle()
//#include "hermes.h"  // for HermesSetSamplerate()

//HWND g_hToolbar = NULL;


class GuiImpl {
public:
	HWND g_hToolbar;
	static BOOL HermesCtrlBoxDlgProcOnInit (HWND hDlg, HWND hFocus, LPARAM lParam);
	static BOOL CALLBACK GuiImpl :: HermesCtrlBoxDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static BOOL GuiImpl :: HermesCtrlBoxDlgProcOnHScroll ( HWND hWnd,         // hwnd
												HWND hwndCtl,      // (int)(LOWORD(wParam))
												UINT codeNotify,   // (UINT)HIWORD(wParam))
												int  pos
											  );
	Hermes *pr;
};

void ErrorLog(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
	LOG(("ERROR: %s\n", (char *)lpDisplayBuf ));

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    //ExitProcess(dw); 
}


void AppendText (HWND hDlg, int id, const char *pTxt)
{
	HWND hCtrl = GetDlgItem(hDlg, id);

	int length = GetWindowTextLength(hCtrl);
	if (length > 0) {
		char* tempChar;
		tempChar= (char*)GlobalAlloc(GPTR, length + 1 + strlen(pTxt));
		GetDlgItemText (hDlg, id, (LPSTR)tempChar, length + 1);
		strcpy_s  (tempChar+strlen(tempChar), length + 1 + strlen(pTxt), pTxt);
		strcpy_s (tempChar, length + 1 + strlen(pTxt), pTxt);
		SetWindowText (hCtrl, (LPCSTR)tempChar);
	}
}
void AppendTextToEditCtrl(HWND hWndEdit, LPCTSTR pszText)
{
	int nLength = GetWindowTextLength(hWndEdit); 
	SendMessage (hWndEdit, EM_SETSEL, (WPARAM)nLength, (LPARAM)nLength);
	SendMessage (hWndEdit,EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)pszText);
}

BOOL GetCheckBoxState (HWND hDlg, int id)
{
	HWND hCtrl = GetDlgItem(hDlg, id);
	if (hCtrl != NULL)
		return (SendMessage(hCtrl, BM_GETSTATE, 0, 0) == BST_CHECKED);
	else 
		return FALSE;
}


BOOL GuiImpl :: HermesCtrlBoxDlgProcOnInit (HWND hDlg, HWND hFocus, LPARAM lParam)
{
	LOGX("WM_INITDIALOG: %d\r\n", WM_INITDIALOG);
	LOGX ("%p\r\n", GetWindowLongPtr (hDlg, GWLP_USERDATA) );

	//Gui *pGui = (Gui *) GetWindowLongPtr (hDlg, GWLP_USERDATA);

	// 
	// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760238%28v=vs.85%29.aspx
	//
	HWND sliderAtt = GetDlgItem ( hDlg, IDC_SLIDER_ATT );
	SendMessage ( sliderAtt, TBM_SETRANGE,    (WPARAM)0,  (LPARAM)MAKELONG(0,31));
	SendMessage ( sliderAtt, TBM_SETPOS,      (WPARAM)0,  0                     );
	SendMessage ( sliderAtt, TBM_SETTICFREQ,  (WPARAM)10, 0                     ); 
	SendMessage ( sliderAtt, TBM_SETPAGESIZE, (WPARAM)0,  (LPARAM)MAKELONG(0,10));

	// fillin the rx number combo box
	SendMessage (GetDlgItem ( hDlg, IDC_COMBO_N_RX ), CB_ADDSTRING, 0, (LPARAM)"1");
	SendMessage (GetDlgItem ( hDlg, IDC_COMBO_N_RX ), CB_ADDSTRING, 0, (LPARAM)"2");
	SendMessage (GetDlgItem ( hDlg, IDC_COMBO_N_RX ), CB_ADDSTRING, 0, (LPARAM)"3");
	SendMessage (GetDlgItem ( hDlg, IDC_COMBO_N_RX ), CB_ADDSTRING, 0, (LPARAM)"4");
	SendMessage (GetDlgItem ( hDlg, IDC_COMBO_N_RX ), CB_ADDSTRING, 0, (LPARAM)"5");
	// default at second item, two receiver(s)
	SendMessage (GetDlgItem ( hDlg, IDC_COMBO_N_RX ), CB_SETCURSEL, 1, 0 );

	return TRUE;
}

//
// (hwnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)), 0L
//
BOOL HermesCtrlBoxDlgProcOnCommand (HWND hWnd,         // hwnd
									int id,            // (int)(LOWORD(wParam))
									HWND hCtl,         // (HWND)(lParam)
									UINT codeNotify    // (UINT)HIWORD(wParam))
									)
{
	LOGX ("%p\n", GetWindowLongPtr (hWnd, GWLP_USERDATA) );
	
	Gui *pGui = (Gui *) GetWindowLongPtr(hWnd, GWLP_USERDATA);


	if ( codeNotify == BN_CLICKED ) {
		// some button/checkbox has been clicked
		// the lower word of wParam holds the controls ID
		if ( id >= IDC_RADIO_BW_384K && id <= IDC_RADIO_BW_48K)  {
			CheckRadioButton( hWnd, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, id );
			switch (id) {
				case IDC_RADIO_BW_384K:
					 AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"New Bandwith: 384 kS/s\r\n");
					 LOGX("%s", "New Bandwith: 384 kS/s\r\n");
					 // HermesSetSampleRate (384000);
					 pGui->pi->pr->setSampleRate (384000);
					 break;
				case IDC_RADIO_BW_192K:
					 AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"New Bandwith: 192 kHz\r\n");
					 LOGX("%s", "New Bandwith: 192 kHz\r\n");
					 //HermesSetSampleRate (192000);
					 pGui->pi->pr->setSampleRate (192000);
					 break;
				case IDC_RADIO_BW_96K:
					 AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"New Bandwith: 96 kHz\r\n");
					 LOGX("%s", "New Bandwith: 96 kHz\r\n");
					 //HermesSetSampleRate (96000);
					 pGui->pi->pr->setSampleRate (96000);
					 break;
				case IDC_RADIO_BW_48K:
					 AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"New Bandwith: 48 KHz\r\n");
					 LOGX("%s", "New Bandwith: 48 KHz\r\n");
					 //HermesSetSampleRate (48000);
					 pGui->pi->pr->setSampleRate (48000);
					 break;
			}
		}
		if ( id >= IDC_CB_DITHER && id <= IDC_CB_RANDOMIZER )  {

			HWND hCtrl = GetDlgItem(hWnd, id);
			char *pszState;
			int   newStatus;

			if ( IsDlgButtonChecked ( hWnd, id ) == BST_CHECKED ) {
				pszState = " activated\r\n";
				newStatus = 1;
			} else {
				pszState = " deactivated\r\n";
				newStatus = 0;
			}
			switch (id) {
				case IDC_CB_FPGA:
					AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"FPGA");
					//HermesSetPreamp (newStatus);
					pGui->pi->pr->setPreamp (newStatus);
					break;
				case IDC_CB_RANDOMIZER:
					AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"Randomizer");
					//HermesSetRandomizer (newStatus);
					pGui->pi->pr->setRandomizer (newStatus);
					break;
			     case IDC_CB_DITHER:
					AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"Dither");
					//HermesSetDither (newStatus);
					pGui->pi->pr->setDither (newStatus);
					break;
 			}
			AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)pszState);
		}
	} 
#if 0
	switch(id) {
		case IDC_PRESS:
	     	//MessageBox(hwnd, "Hi!", "This is a message", 
		    //	MB_OK | MB_ICONEXCLAMATION);
			//AppendText (hwnd, IDC_MSG_PANE, "Urca\r\r\n");
            SetDlgItemText( hWnd, IDC_MSG_PANE, "prima linea\r\nseconda linea\r\n");
    		AppendTextToEditCtrl(GetDlgItem(hWnd, IDC_MSG_PANE), (LPCTSTR)"terza linea\r\n");
		break;
		case IDC_OTHER:
			MessageBox(hWnd, "Bye!", "This is also a message", MB_OK | MB_ICONEXCLAMATION);
			break;
	}
#endif
	return TRUE;
}


BOOL GuiImpl :: HermesCtrlBoxDlgProcOnHScroll ( HWND hWnd,         // hwnd
												HWND hwndCtl,      // (int)(LOWORD(wParam))
												UINT codeNotify,   // (UINT)HIWORD(wParam))
												int  pos
											  )
{
	Gui *pGui = (Gui *) GetWindowLongPtr (hWnd, GWLP_USERDATA);

	//lpNMTrbThumbPosChanging = (NMTRBTHUMBPOSCHANGING*) lParam;
	if ( GetDlgItem(hWnd, IDC_SLIDER_ATT) == hwndCtl ) {
		DWORD dwPos = SendMessage(GetDlgItem(hWnd, IDC_SLIDER_ATT),TBM_GETPOS,0,0); 
		LOGX("New attenuator value: %d\r\n", dwPos );
		//HermesSetAttenuator (dwPos);
		pGui->pi->pr->setAttenuator (dwPos);
	}
	return TRUE;
}
		

BOOL CALLBACK GuiImpl :: HermesCtrlBoxDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL fProcessed = TRUE;
	Gui *pGui = (Gui *) GetWindowLongPtr (hDlg, GWLP_USERDATA);


	switch (uMsg) {
		HANDLE_MSG(hDlg, WM_INITDIALOG, pGui->pi->HermesCtrlBoxDlgProcOnInit);
		HANDLE_MSG(hDlg, WM_COMMAND, HermesCtrlBoxDlgProcOnCommand);
		HANDLE_MSG(hDlg, WM_HSCROLL, HermesCtrlBoxDlgProcOnHScroll);
		case WM_USER+2:
		{
			char * pszText = (char *)(LPVOID)wParam;
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_HW), (LPCTSTR)(char *)pszText);
			free (pszText);
		}
		break;
		case WM_USER+3:
		{
			char *pszText = (char *)((LPVOID)wParam);
			AppendTextToEditCtrl (GetDlgItem(hDlg, IDC_MSG_PANE), (LPCTSTR)pszText);
			free (pszText);
		}
		break;
		default:
			fProcessed = FALSE;
			break;
	}
	return (fProcessed);
}



Gui :: Gui (int sample_rate)
{
	pi = new GuiImpl;

	LOG(("********************************* HermesCreateGUI: pImpl: %p Gui addr: %p\n", pi, this));

	pi->g_hToolbar = CreateDialog  (GetMyHandle(), 
									MAKEINTRESOURCE(IDD_TOOLBAR),
									HWND_DESKTOP, 
									pi->HermesCtrlBoxDlgProc
									);

	if (pi->g_hToolbar != NULL) {
		ShowWindow (pi->g_hToolbar, SW_SHOW);
		SetWindowLongPtr (pi->g_hToolbar, GWLP_USERDATA, (LONG)this);

		switch (sample_rate) {
		case 384000:
			// first last check
			CheckRadioButton( pi->g_hToolbar, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_384K);
			break;
		case 192000:
			CheckRadioButton( pi->g_hToolbar, IDC_RADIO_BW_384K, IDC_RADIO_BW_48K, IDC_RADIO_BW_192K);
			break;
		case 96000:
			CheckRadioButton( pi->g_hToolbar, IDC_RADIO_BW_384K,  IDC_RADIO_BW_48K, IDC_RADIO_BW_96K);
			break;
		case 48000:
			CheckRadioButton( pi->g_hToolbar, IDC_RADIO_BW_384K,  IDC_RADIO_BW_48K, IDC_RADIO_BW_48K);
			break;
		}

	} else {
		ErrorLog(TEXT("HermesCreateGUI: main dialog failed !"));
	}





}

Gui :: ~Gui ()
{
	LOG(("********************************* GUI destructor: Gui addr: %p\n", pi));
	DestroyWindow (pi->g_hToolbar);
	delete pi;
}

void Gui :: Show ()
{
	SetWindowPos (pi->g_hToolbar, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow (pi->g_hToolbar, SW_SHOW);
}

void Gui :: Hide ()
{
	ShowWindow (pi->g_hToolbar, SW_HIDE);
}

void Gui :: HermesSetHwAddressGUI (const Ethernet::Device *pDev)
{
	char szBuf [1024];

	if (pDev) {
		sprintf (szBuf, "%s - %s\t\t\t%s %1.1f", pDev->ip_address, pDev->mac_address, pDev->board_id, ((float)pDev->code_version)/10.0f);
	} else {			
		sprintf (szBuf, "%s", "HARDWARE NOT FOUND !!!!!");
	}
	PostMessage (pi->g_hToolbar, WM_USER+2, (WPARAM)_strdup(szBuf), (LPARAM)0);
	//	free (pszBuf);  FREE IS IN DIALOG BOX !!!!!!!
}


void Gui :: SetHw (const char *text)
{
	if (text) PostMessage (pi->g_hToolbar, WM_USER+2, (WPARAM)_strdup(text), (LPARAM)0);
}

void Gui :: AppendMessage (const char *pszBuf)
{
	PostMessage (pi->g_hToolbar, WM_USER+3, (WPARAM)pszBuf, (LPARAM)0);
	//	free (pszBuf);  FREE IS IN DIALOG BOX !!!!!!!
}

int  Gui :: getRecNumber (void)
{
	char  buf[256];
	int   n;

	SendMessage (GetDlgItem ( pi->g_hToolbar, IDC_COMBO_N_RX ), WM_GETTEXT, (WPARAM)sizeof(buf), (LPARAM)buf);
	sscanf (buf, "%d", &n);
	return n;
}

void Gui :: setRadio (Hermes *pR)
{
	pi->pr = pR;
}












