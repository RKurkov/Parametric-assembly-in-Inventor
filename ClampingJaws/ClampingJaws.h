
// ClampingJaws.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "включить pch.h до включения этого файла в PCH"
#endif

#include "resource.h"		// основные символы


// CClampingJawsApp:
// Сведения о реализации этого класса: ClampingJaws.cpp
//

class CClampingJawsApp : public CWinApp
{
public:
	CClampingJawsApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CClampingJawsApp theApp;
