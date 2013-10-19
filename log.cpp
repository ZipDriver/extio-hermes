/** 
 * @file hpsdr.cpp
 * @brief Generic log functions
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
#include <stdarg.h>
#include <time.h>
#include "log.h"

static FILE *pLog = 0;

#if defined WIN32
#include "windows.h"
#include "logw.h"
#include "dllmain.h" // for GetMyHandle()

static void AppendTextToEditCtrl(HWND hWndEdit, LPCTSTR pszText)
{
	int n = GetWindowTextLength(hWndEdit); 

	if (n >= (LOG_DLG_SIZE/2)) {
		// trim 10KB
		SendMessage (hWndEdit, EM_SETSEL,     (WPARAM)10240, (LPARAM)n  );
		SendMessage (hWndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)"" );
	}
	SendMessage (hWndEdit, EM_SETSEL,     (WPARAM)n,     (LPARAM)n       );
	SendMessage (hWndEdit, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)pszText );

}


BOOL CALLBACK LogDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
	    case WM_CREATE:
			SendMessage (GetDlgItem(hwnd, IDC_LOG_TEXT), EM_SETLIMITTEXT, (WPARAM)LOG_DLG_SIZE, (LPARAM)0);
			break;

	    case WM_INITDIALOG:
		    break;

	    case WM_COMMAND:
        // WM_COMMAND is sent when an event occurs
        // the HIWORD of WPARAM holds the notification event code

			switch(LOWORD(wParam))
			{
				case IDOK:
					MessageBox (hwnd, "Bye!", "This is also a message", 
                                MB_OK | MB_ICONEXCLAMATION);
				break;
			}
		break;
		case WM_USER+1:
			MessageBox (hwnd, "Bye!", "This is also a message", 
                                MB_OK | MB_ICONEXCLAMATION);
            break;
		case WM_USER+2:
			{
				if (lParam) {
					char buff[100];
					time_t now = time (0);
					strftime (buff, 100, "%Y-%m-%d %H:%M:%S ", localtime (&now));
					AppendTextToEditCtrl(GetDlgItem(hwnd, IDC_LOG_TEXT), buff);
				}
 
				char *pszText = (char *)((LPVOID)wParam);
				AppendTextToEditCtrl(GetDlgItem(hwnd, IDC_LOG_TEXT), (LPCTSTR)pszText);
				free (pszText);
			}
            break;
		default:
			return FALSE;
	}
	return TRUE;
}

static HWND hLogDlg = 0;

void log_CreateLogDialog (const char *pszName)
{
	hLogDlg = CreateDialog (GetMyHandle (), 
							MAKEINTRESOURCE (IDD_LOG_DIALOG),
							NULL, 
							LogDlgProc);
	if (hLogDlg != NULL)	{
		char oldTitle [256];
		char newTitle [1024];
		GetWindowText (hLogDlg, oldTitle, sizeof(oldTitle));
		sprintf (newTitle, "%s - %s", oldTitle, pszName);
		SetWindowText (hLogDlg, newTitle);
		ShowWindow(hLogDlg, SW_SHOW);
	} else {
		MessageBox(0, "CreateDialog returned NULL", "Warning!", MB_OK | MB_ICONINFORMATION);
	}
}

void LogPostMsg (const char *pszText, bool timestamp)
{
	//char *pszBuf = (char *) malloc (strlen(pszText)+1);
	//if (pszBuf) {
	//	strcpy (pszBuf, pszText), PostMessage (hLogDlg, WM_USER+2, (WPARAM)pszBuf, timestamp);
	//	free (pszBuf);  FREE IS IN DIALOG BOX !!!!!!!
	if (pszText) {
		PostMessage (hLogDlg, WM_USER+2, (WPARAM)_strdup(pszText), timestamp);
	}
}

#endif


void log_open (const char *pszLogFileName)
{
	if (pLog == 0) {
		char buf[1024];
		sprintf (buf, "%s-%d.log", pszLogFileName, GetInstanceNumber());
		pLog = fopen (buf, "w+");
#if defined WIN32
		log_CreateLogDialog (buf);
#endif
	}
}
void log_close (void)
{
	if (pLog != 0) {
		fflush (pLog);
		fclose (pLog);
	}
#if defined WIN32
	DestroyWindow (hLogDlg);
#endif
}
void log_funcname_printf( const char *pszFunctionName, int nLine, const char *pszFmt, ...) 
{
	va_list ap; 
	char szFmt[BUFSIZ]; 
	char szBuf[BUFSIZ];

	sprintf(szFmt, "%s:%d - %s", pszFunctionName, nLine, pszFmt);

	va_start (ap, pszFmt); 
	vsprintf (szBuf, szFmt, ap); 
	va_end (ap); 

	fprintf (pLog, "%s", szBuf);
	fflush (pLog);
#if defined WIN32
	LogPostMsg (szBuf, true);
#endif
}

void log_printf (const char *format, ...)
{
	va_list ap;
	char szBuf[BUFSIZ];

	va_start(ap, format);
	vsprintf(szBuf, format, ap);
	va_end(ap);
	fprintf (pLog, "%s", szBuf);
	fflush(pLog);
#if defined WIN32
	LogPostMsg (szBuf, false);
#endif
}

void log_printf_mod (const char *pszFile, int nLine)
{
	char szBuf[BUFSIZ];

	sprintf (szBuf, "%s: %04.4d: ", pszFile, nLine);
	fprintf (pLog, "%s", szBuf);
#if defined WIN32
	LogPostMsg (szBuf, true);
#endif

}



