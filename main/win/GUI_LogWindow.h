/***************************************************************************
						  GUI_LogWindow.h  -  description
							 -------------------
	copyright            : (C) 2003 by ShadowPrince
	email                : shadow@emulation64.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _GUI_LOGWINDOW_H
#define _GUI_LOGWINDOW_H

#include <Windows.h>

extern int extLogger;
int  GUI_CreateLogWindow(HWND hwnd);

void ShowWarning(const char* Str, ...);
void ShowError(const char* Str, ...);
void ShowInfo(const char* Str, ...);
void ShowHideLogWindow();
void ShowLogWindow();
void HideLogWindow();
void ClearLogWindow();
void CloseLogWindow();



#endif
