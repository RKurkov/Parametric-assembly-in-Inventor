
// ClampingJawsDlg.h: файл заголовка
//

#pragma once


// Диалоговое окно CClampingJawsDlg
class CClampingJawsDlg : public CDialogEx
{
// Создание
public:
	CClampingJawsDlg(CWnd* pParent = nullptr);	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLAMPINGJAWS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV


// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit m_diameter;
	CEdit m_height;
	CEdit m_amount;
	float m_Diameter;
	float m_Height;
	float m_Amount;
	afx_msg void OnBnClickedButtonGo();
	bool CheckData();
	afx_msg void OnBnClickedButton2();
};
