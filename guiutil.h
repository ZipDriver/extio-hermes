/**
* @file guiutil.h
* @brief Header for GUI utilities
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

#if !defined	__GUIUTIL_H__
#define			__GUIUTIL_H__

struct GuiEvent;

class GuiError {
public:
	GuiError(const char *p) : p_(p) {}
	const char * show();
	operator const char *()  { return p_; }
private:
	const char *p_;
};

//void ShowError(const char *msg);
void AppendText(const GuiEvent &ge /*HWND hDlg, int id, */, const char *pTxt);
bool GetCheckBoxState(const GuiEvent& ge);
void AppendWinTitle(const GuiEvent& ge, const char *p);
void AppendTextToEditCtrl(const GuiEvent & ge, /*HWND hWndEdit,*/ const char * pszText);


#endif
