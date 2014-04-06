#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#include <windowsx.h>
#include <commctrl.h>			// Include header
#pragma comment(lib, "comctl32.lib")	// Include library
#pragma warning( disable : 4995 )

#if defined _MSC_VER
#include <strsafe.h>
#endif

#include "util.h"
#include "hpsdr.h"
#include "log.h"
#include "guievent.h"
#include "guiutil.h"


const char * GuiError :: show() {
	MessageBox (NULL, (LPCTSTR)p_, TEXT("Error"), MB_OK);
	return p_;
}

void AppendText(const GuiEvent &ge /*HWND hDlg, int id, */, const char *pTxt)
{
	HWND hCtrl = GetDlgItem(ge.hWnd, ge.id);

	int length = GetWindowTextLength(hCtrl);
	if (length > 0) {
		char* tempChar;
		tempChar = (char*)GlobalAlloc(GPTR, length + 1 + strlen(pTxt));
		GetDlgItemText(ge.hWnd, ge.id, (LPSTR)tempChar, length + 1);
		strcpy_s(tempChar + strlen(tempChar), length + 1 + strlen(pTxt), pTxt);
		strcpy_s(tempChar, length + 1 + strlen(pTxt), pTxt);
		SetWindowText(hCtrl, (LPCSTR)tempChar);
	}
}

void AppendTextToEditCtrl(const GuiEvent & ge, const char * pszText)
{
	HWND hCtl = GetDlgItem(ge.hWnd, ge.id);
	int nLength = GetWindowTextLength(hCtl);
	SendMessage(hCtl, EM_SETSEL, (WPARAM)nLength, (LPARAM)nLength);
	SendMessage(hCtl, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)pszText);
}

BOOL GetCheckBoxState(HWND hDlg, int id)
{
	HWND hCtrl = GetDlgItem(hDlg, id);
	if (hCtrl != NULL)
		return (SendMessage(hCtrl, BM_GETSTATE, 0, 0) == BST_CHECKED);
	else
		return FALSE;
}

bool GetCheckBoxState(const GuiEvent& ge)
{
	HWND hCtrl = GetDlgItem(ge.hWnd, ge.id);
	if (hCtrl != NULL)
		return (SendMessage(hCtrl, BM_GETSTATE, 0, 0) == BST_CHECKED);
	else
		return false;
}


void AppendWinTitle(const GuiEvent& ge, const char *p)
{
	char szBuf[1024];
	char szNew[2028];
	if (GetWindowText(ge.hWnd, szBuf, sizeof (szBuf)) > 0) {
		snprintf(szNew, sizeof(szNew), "%s%s", szBuf, p);
		SetWindowText(ge.hWnd, szNew);
	}
}
