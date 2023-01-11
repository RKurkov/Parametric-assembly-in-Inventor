
// ClampingJawsDlg.cpp: файл реализации
//


#include "pch.h"
#include "ClampingJaws.h"
#include "ClampingJawsDlg.h"
#include "afxdialogex.h"
#include "math.h"

#define PI 4*atan(1)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma once
//#include <atlbase.h>
#import <C:\\Program Files\\Autodesk\\Inventor 2022\\Bin\\RxInventor.tlb> \
rename_namespace("InventorNative") \
named_guids raw_dispinterfaces \
high_method_prefix("Method") \
rename("DeleteFile", "APIDeleteFile") \
rename("CopyFile", "APICopyFile") \
rename("MoveFile", "APIMoveFile") \
rename("SetEnvironmentVariable", "APISetEnvironmentVariable") \
rename("GetObject", "APIGetObject") \
exclude("_FILETIME", "IStream", "ISequentialStream", \
"_LARGE_INTEGER", "_ULARGE_INTEGER", "tagSTATSTG", \
"IEnumUnknown", "IPersistFile", "IPersist", "IPictureDisp")

using namespace InventorNative;

CComPtr<Application> pInvApp; //приложение

PartDocumentPtr pPartDoc;  //документ
PartComponentDefinition* pPartComDef;//компоненты детали
TransientGeometry* pTransGeom; //геометрия детали
Transaction* pTrans; //операции

TransactionManagerPtr pTransManager; //менеджер операций


PlanarSketches* sketches; // эскизы
PartFeatures* ft; //элементы детали 

WorkPlanes* wp; //рабочие плоскости
WorkAxes* wax;//рабочие оси
WorkPoints* wpt;//рабочие точки


// диалоговое окно CWSCADDlg



CClampingJawsDlg::CClampingJawsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CLAMPINGJAWS_DIALOG, pParent)
	, m_Diameter(25)
	, m_Height(30)
	, m_Amount(4)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClampingJawsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Text(pDX, IDC_EDIT_D, m_Diameter);
	DDV_MinMaxDouble(pDX, m_Diameter, 20, 40);
	DDX_Text(pDX, IDC_EDIT_H, m_Height);
	DDV_MinMaxDouble(pDX, m_Height, 1, 10000);
	DDX_Text(pDX, IDC_EDIT_N, m_Amount);
	DDV_MinMaxDouble(pDX, m_Amount, 1, 8);
}

BEGIN_MESSAGE_MAP(CClampingJawsDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_GO, &CClampingJawsDlg::OnBnClickedButtonGo)
	//ON_BN_CLICKED(IDC_BUTTON2, &CClampingJawsDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CClampingJawsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDOK, &CClampingJawsDlg::OnBnClickedOk)
	//ON_BN_CLICKED(IDC_BUTTON_GO, &CClampingJawsDlg::OnBnClickedButtonGo)
	ON_BN_CLICKED(IDC_BUTTON2, &CClampingJawsDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// обработчики сообщений CWSCADDlg

BOOL CClampingJawsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CClampingJawsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CClampingJawsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CClampingJawsDlg::CheckData()
{
	if (!UpdateData())
		return false;
	m_Diameter /= 20;
	m_Height /= 10;
	m_Amount;

	return true;
}

void CClampingJawsDlg::OnBnClickedButtonGo()
{
	// TODO: добавьте свой код обработчика уведомлений
	BeginWaitCursor();

	if (!CheckData())
		return;

	if (!pInvApp)
	{
		// Get hold of the program id of Inventor.
		CLSID InvAppClsid;
		HRESULT hRes = CLSIDFromProgID(L"Inventor.Application", &InvAppClsid);
		if (FAILED(hRes)) {
			pInvApp = nullptr;
			return;// ReturnAndShowCOMError(hRes, L"ConnectToInventor,CLSIDFromProgID failed");
		}

		// See if Inventor is already running...
		CComPtr<IUnknown> pInvAppUnk = nullptr;
		hRes = ::GetActiveObject(InvAppClsid, NULL, &pInvAppUnk);
		if (FAILED(hRes)) {
			// Inventor is not already running, so try to start it...
			TRACE(L"Could not get hold of an active Inventor, will start a new session\n");
			hRes = CoCreateInstance(InvAppClsid, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IUnknown), (void**)&pInvAppUnk);
			if (FAILED(hRes)) {
				pInvApp = nullptr;
				return;
			}
		}

		// Get the pointer to the Inventor application...
		hRes = pInvAppUnk->QueryInterface(__uuidof(Application), (void**)&pInvApp);
		if (FAILED(hRes)) {
			return;
		}
	}
	pInvApp->put_Visible(TRUE);

	pPartDoc = pInvApp->Documents->MethodAdd(kPartDocumentObject, pInvApp->FileManager->MethodGetTemplateFile(kPartDocumentObject, kMetricSystemOfMeasure, kGOST_DraftingStandard), true);

	pPartDoc->DisplayName = _T("ClampingJaws");

	pPartDoc->get_ComponentDefinition(&pPartComDef);

	pInvApp->get_TransientGeometry(&pTransGeom);


	pTransManager = pInvApp->GetTransactionManager();

	Document* Doc = CComQIPtr <Document>(pPartDoc);

	pTransManager->raw_StartTransaction(Doc, _T("ClampingJaws"), &pTrans);

	pPartComDef->get_Sketches(&sketches);



	pPartComDef->get_WorkPlanes(&wp);
	pPartComDef->get_Features(&ft);
	pPartComDef->get_WorkAxes(&wax);
	pPartComDef->get_WorkPoints(&wpt);

	//Основа
	PlanarSketch* pSketch;
	sketches->raw_Add(wp->GetItem(2), false, &pSketch);


	SketchPointPtr point[5];
	SketchLinePtr lines[5];


	SketchPoints* skPoints;
	SketchLines* skLines;

	Profiles* skProfiles;


	pSketch->get_SketchPoints(&skPoints);
	pSketch->get_SketchLines(&skLines);
	pSketch->get_Profiles(&skProfiles);


	point[0] = skPoints->MethodAdd(pTransGeom->MethodCreatePoint2d(0, -2.5f), false);
	point[1] = skPoints->MethodAdd(pTransGeom->MethodCreatePoint2d(3, -2.5f), false);
	point[2] = skPoints->MethodAdd(pTransGeom->MethodCreatePoint2d(3, 0.5f), false);
	point[3] = skPoints->MethodAdd(pTransGeom->MethodCreatePoint2d(1.5f, 2.5f), false);
	point[4] = skPoints->MethodAdd(pTransGeom->MethodCreatePoint2d(0, 2.5f), false);
	


	lines[0] = skLines->MethodAddByTwoPoints(point[0], point[1]);
	lines[1] = skLines->MethodAddByTwoPoints(point[1], point[2]);
	lines[2] = skLines->MethodAddByTwoPoints(point[2], point[3]);
	lines[3] = skLines->MethodAddByTwoPoints(point[3], point[4]);
	lines[4] = skLines->MethodAddByTwoPoints(point[4], point[0]);
	
	
	ProfilePtr pProfile;

	try {

		pProfile = skProfiles->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturesPtr ftExtrude;
	ft->get_ExtrudeFeatures(&ftExtrude);

	ExtrudeFeaturePtr extrude = ftExtrude->MethodAddByDistanceExtent(pProfile, 3, kPositiveExtentDirection, kJoinOperation);


	//Окружность
	PlanarSketch* pSketch1;
	sketches->raw_Add(wp->GetItem(3), false, &pSketch1);

	SketchPointPtr point1[1];
	SketchCirclePtr circ1;


	SketchPoints* skPoints1;
	SketchCircles* skCircles1;

	Profiles* skProfiles1;


	pSketch1->get_SketchPoints(&skPoints1);
	pSketch1->get_SketchCircles(&skCircles1);
	pSketch1->get_Profiles(&skProfiles1);

	point1[0] = skPoints1->MethodAdd(pTransGeom->MethodCreatePoint2d(-3, 1.5f), false);
	
	circ1 = skCircles1->MethodAddByCenterRadius(point1[0], 0.5);

	ProfilePtr pProfile1;

	try {

		pProfile1 = skProfiles1->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude1 = ftExtrude->MethodAddByThroughAllExtent(pProfile1, kSymmetricExtentDirection, kCutOperation);


	//Срез1
	PlanarSketch* pSketch2;
	sketches->raw_Add(wp->GetItem(1), false, &pSketch2);


	SketchPointPtr point2[6];
	SketchLinePtr lines2[6];


	SketchPoints* skPoints2;
	SketchLines* skLines2;

	Profiles* skProfiles2;


	pSketch2->get_SketchPoints(&skPoints2);
	pSketch2->get_SketchLines(&skLines2);
	pSketch2->get_Profiles(&skProfiles2);


	point2[0] = skPoints2->MethodAdd(pTransGeom->MethodCreatePoint2d(3, -2.5f), false);
	point2[1] = skPoints2->MethodAdd(pTransGeom->MethodCreatePoint2d(0, -2.5f), false);
	point2[2] = skPoints2->MethodAdd(pTransGeom->MethodCreatePoint2d(0, -2), false);
	point2[3] = skPoints2->MethodAdd(pTransGeom->MethodCreatePoint2d(1, -2.1667f), false);
	point2[4] = skPoints2->MethodAdd(pTransGeom->MethodCreatePoint2d(1.5f, -2.0527f), false);
	point2[5] = skPoints2->MethodAdd(pTransGeom->MethodCreatePoint2d(2, -2.3333f), false);



	lines2[0] = skLines2->MethodAddByTwoPoints(point2[0], point2[1]);
	lines2[1] = skLines2->MethodAddByTwoPoints(point2[1], point2[2]);
	lines2[2] = skLines2->MethodAddByTwoPoints(point2[2], point2[3]);
	lines2[3] = skLines2->MethodAddByTwoPoints(point2[3], point2[4]);
	lines2[4] = skLines2->MethodAddByTwoPoints(point2[4], point2[5]);
	lines2[5] = skLines2->MethodAddByTwoPoints(point2[5], point2[0]);

	ProfilePtr pProfile2;

	try {

		pProfile2 = skProfiles2->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude2 = ftExtrude->MethodAddByThroughAllExtent(pProfile2, kNegativeExtentDirection, kCutOperation);


	//Отражение
	ObjectCollection* pCollection;

	pInvApp->TransientObjects->raw_CreateObjectCollection(vtMissing, &pCollection);

	pCollection->MethodAdd(extrude);
	pCollection->MethodAdd(extrude1);
	pCollection->MethodAdd(extrude2);


	MirrorFeaturesPtr MirFeat;
	MirrorFeatureDefinitionPtr mirFeatDef;

	ft->get_MirrorFeatures(&MirFeat);

	mirFeatDef = MirFeat->MethodCreateDefinition(pCollection, wp->GetItem(1), kAdjustToModelCompute);

	MirFeat->MethodAddByDefinition(mirFeatDef);


	//Оболочка
	SurfaceBodyPtr SurfBody;
	SurfaceBodiesPtr SurfBodies1;
	pPartComDef->get_SurfaceBodies(&SurfBodies1);

	SurfBodies1->get_Item(1, &SurfBody);


	Face* edge;
	Faces* edges;

	SurfBody->get_Faces(&edges);
	edges->get_Item(13, &edge);


	ShellDefinitionPtr shptr;
	ShellFeaturesPtr shells;
	ft->get_ShellFeatures(&shells);

	FaceCollectionPtr facecoll;
	pInvApp->TransientObjects->raw_CreateFaceCollection(vtMissing, &facecoll);
	facecoll->MethodAdd(edge);

	shptr = shells->MethodCreateShellDefinition(vtMissing, 0.5, kInsideShellDirection);

	shptr->put_InputFaces(facecoll);

	shptr->put_Direction(kInsideShellDirection);

	shptr = shells->MethodAdd(shptr);


	//Перегородка
	PlanarSketch* pSketch7;
	sketches->raw_Add(wp->GetItem(1), false, &pSketch7);


	SketchPointPtr point7[7];
	SketchLinePtr lines7[7];


	SketchPoints* skPoints7;
	SketchLines* skLines7;

	Profiles* skProfiles7;


	pSketch7->get_SketchPoints(&skPoints7);
	pSketch7->get_SketchLines(&skLines7);
	pSketch7->get_Profiles(&skProfiles7);


	point7[0] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(0.5f, 2.5f), false);
	point7[1] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(0.5f, -1.576f), false);
	point7[2] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(0.985f, -1.657), false);
	point7[3] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(1.577f, -1.522f), false);
	point7[4] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(2.168f, -1.854f), false);
	point7[5] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(2.5f, -1.91f), false);
	point7[6] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(2.5f, 2.5f), false);



	lines7[0] = skLines7->MethodAddByTwoPoints(point7[0], point7[1]);
	lines7[1] = skLines7->MethodAddByTwoPoints(point7[1], point7[2]);
	lines7[2] = skLines7->MethodAddByTwoPoints(point7[2], point7[3]);
	lines7[3] = skLines7->MethodAddByTwoPoints(point7[3], point7[4]);
	lines7[4] = skLines7->MethodAddByTwoPoints(point7[4], point7[5]);
	lines7[5] = skLines7->MethodAddByTwoPoints(point7[5], point7[6]);
	lines7[6] = skLines7->MethodAddByTwoPoints(point7[6], point7[0]);

	ProfilePtr pProfile7;

	try {

		pProfile7 = skProfiles7->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude7 = ftExtrude->MethodAddByDistanceExtent(pProfile7, 1, kSymmetricExtentDirection, kJoinOperation);


//	//PlanarSketch* pSketch7;
//	//sketches->raw_Add(wp->GetItem(3), false, &pSketch7);
//
//
//	//SketchPointPtr point7[4];
//	//SketchLinePtr lines7[4];
//
//
//	//SketchPoints* skPoints7;
//	//SketchLines* skLines7;
//
//	//Profiles* skProfiles7;
//
//
//	//pSketch7->get_SketchPoints(&skPoints7);
//	//pSketch7->get_SketchLines(&skLines7);
//	//pSketch7->get_Profiles(&skProfiles7);
//
//
//	//point7[0] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(0.5f, 0.5f), false);
//	//point7[1] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(0.5f, 2.5f), false);
//	//point7[2] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(-0.5f, 2.5f), false);
//	//point7[3] = skPoints7->MethodAdd(pTransGeom->MethodCreatePoint2d(-0.5f, 0.5f), false);
//
//
//	//lines7[0] = skLines7->MethodAddByTwoPoints(point7[0], point7[1]);
//	//lines7[1] = skLines7->MethodAddByTwoPoints(point7[1], point7[2]);
//	//lines7[2] = skLines7->MethodAddByTwoPoints(point7[2], point7[3]);
//	//lines7[3] = skLines7->MethodAddByTwoPoints(point7[3], point7[0]);
//
//
//	//ProfilePtr pProfile7;
//
//	//try {
//
//	//	pProfile7 = skProfiles7->MethodAddForSolid(true);
//	//}
//	//catch (...) {
//
//	//	return;
//	//}
//
//	//ExtrudeFeaturePtr extrude7 = ftExtrude->MethodAddByDistanceExtent(pProfile7, 2.5f, kPositiveExtentDirection, kJoinOperation);
//	//ExtrudeFeaturePtr extrude8 = ftExtrude->MethodAddByToExtent(pProfile7, wp->GetItem(3),kJoinOperation,true, vtMissing);
//
//
	//Срез2
	PlanarSketch* pSketch3;
	sketches->raw_Add(wp->GetItem(1), false, &pSketch3);


	SketchPointPtr point3[3];
	SketchLinePtr lines3[3];


	SketchPoints* skPoints3;
	SketchLines* skLines3;

	Profiles* skProfiles3;


	pSketch3->get_SketchPoints(&skPoints3);
	pSketch3->get_SketchLines(&skLines3);
	pSketch3->get_Profiles(&skProfiles3);


	point3[0] = skPoints3->MethodAdd(pTransGeom->MethodCreatePoint2d(0.5f, 2.5f), false);
	point3[1] = skPoints3->MethodAdd(pTransGeom->MethodCreatePoint2d(3, 0.5), false);
	point3[2] = skPoints3->MethodAdd(pTransGeom->MethodCreatePoint2d(3, 2.5f), false);


	lines3[0] = skLines3->MethodAddByTwoPoints(point3[0], point3[1]);
	lines3[1] = skLines3->MethodAddByTwoPoints(point3[1], point3[2]);
	lines3[2] = skLines3->MethodAddByTwoPoints(point3[2], point3[0]);
	

	ProfilePtr pProfile3;

	try {

		pProfile3 = skProfiles3->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude3 = ftExtrude->MethodAddByThroughAllExtent(pProfile3, kSymmetricExtentDirection, kCutOperation);


	//Вырез
	PlanarSketch* pSketch9;
	sketches->raw_Add(wp->GetItem(2), false, &pSketch9);


	SketchPointPtr point9[3];
	SketchLinePtr lines9[3];


	SketchPoints* skPoints9;
	SketchLines* skLines9;

	Profiles* skProfiles9;


	pSketch9->get_SketchPoints(&skPoints9);
	pSketch9->get_SketchLines(&skLines9);
	pSketch9->get_Profiles(&skProfiles9);


	point9[0] = skPoints9->MethodAdd(pTransGeom->MethodCreatePoint2d(0.8f, -2.5f), false);
	point9[1] = skPoints9->MethodAdd(pTransGeom->MethodCreatePoint2d(0, -1.9f), false);
	point9[2] = skPoints9->MethodAdd(pTransGeom->MethodCreatePoint2d(-0.8f, -2.5f), false);


	lines9[0] = skLines9->MethodAddByTwoPoints(point9[0], point9[1]);
	lines9[1] = skLines9->MethodAddByTwoPoints(point9[1], point9[2]);
	lines9[2] = skLines9->MethodAddByTwoPoints(point9[2], point9[0]);


	ProfilePtr pProfile9;

	try {

		pProfile9 = skProfiles9->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude9 = ftExtrude->MethodAddByThroughAllExtent(pProfile9, kSymmetricExtentDirection, kCutOperation);


	//Цилиндр
	PlanarSketch* pSketch4;
	sketches->raw_Add(wp->GetItem(2), false, &pSketch4);


	SketchPointPtr point4[1];
	SketchCirclePtr circ4;


	SketchPoints* skPoints4;
	SketchCircles* skCircles4;

	Profiles* skProfiles4;


	pSketch4->get_SketchPoints(&skPoints4);
	pSketch4->get_SketchCircles(&skCircles4);
	pSketch4->get_Profiles(&skProfiles4);


	point4[0] = skPoints4->MethodAdd(pTransGeom->MethodCreatePoint2d(0, 0), false);
	
	circ4 = skCircles4->MethodAddByCenterRadius(point4[0], m_Diameter);

	ProfilePtr pProfile4;

	try {

		pProfile4 = skProfiles4->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude4 = ftExtrude->MethodAddByDistanceExtent(pProfile4,m_Height , kNegativeExtentDirection, kJoinOperation);


	//Выступ
	PlanarSketch* pSketch6;
	sketches->raw_Add(wp->GetItem(2), false, &pSketch6);


	SketchPointPtr point6[5];
	SketchLinePtr lines6[3];
	SketchArcPtr arc6;


	SketchPoints* skPoints6;
	SketchLines* skLines6;
	SketchArcs* skArcs6;

	Profiles* skProfiles6;


	pSketch6->get_SketchPoints(&skPoints6);
	pSketch6->get_SketchLines(&skLines6);
	pSketch6->get_SketchArcs(&skArcs6);
	pSketch6->get_Profiles(&skProfiles6);


	point6[0] = skPoints6->MethodAdd(pTransGeom->MethodCreatePoint2d(0.3f, 0), false);
	point6[1] = skPoints6->MethodAdd(pTransGeom->MethodCreatePoint2d(0.3f, m_Diameter + 0.05f), false);
	point6[2] = skPoints6->MethodAdd(pTransGeom->MethodCreatePoint2d(-0.3f, m_Diameter + 0.05f), false);
	point6[3] = skPoints6->MethodAdd(pTransGeom->MethodCreatePoint2d(-0.3f, 0), false);
	point6[4] = skPoints6->MethodAdd(pTransGeom->MethodCreatePoint2d(0, 0), false);


	lines6[0] = skLines6->MethodAddByTwoPoints(point6[0], point6[1]);
	lines6[1] = skLines6->MethodAddByTwoPoints(point6[2], point6[3]);
	lines6[2] = skLines6->MethodAddByTwoPoints(point6[3], point6[0]);

	arc6 = skArcs6->MethodAddByCenterStartEndPoint(point6[4], point6[1], point6[2], true);


	ProfilePtr pProfile6;

	try {

		pProfile6 = skProfiles6->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude6 = ftExtrude->MethodAddByDistanceExtent(pProfile6, m_Height - 0.5f, kNegativeExtentDirection, kJoinOperation);


	//Множим выступ
	pCollection->MethodClear();
	pCollection->MethodAdd(extrude6);

	CircularPatternFeatures* pCircPatFeat;

	ft->get_CircularPatternFeatures(&pCircPatFeat);

	CircularPatternFeature* circFeat = pCircPatFeat->MethodAdd(pCollection, wax->GetItem(2), true, m_Amount, "360 град", true, kIdenticalCompute);


	//Вырез в цилиндре
	PlanarSketch* pSketch5;
	sketches->raw_Add(wp->GetItem(2), false, &pSketch5);


	SketchPointPtr point5[1];
	SketchCirclePtr circ5;


	SketchPoints* skPoints5;
	SketchCircles* skCircles5;
	Profiles* skProfiles5;


	pSketch5->get_SketchPoints(&skPoints5);
	pSketch5->get_SketchCircles(&skCircles5);
	pSketch5->get_Profiles(&skProfiles5);


	point5[0] = skPoints5->MethodAdd(pTransGeom->MethodCreatePoint2d(0, 0), false);

	circ5 = skCircles5->MethodAddByCenterRadius(point5[0], m_Diameter-0.25f);

	ProfilePtr pProfile5;

	try {

		pProfile5 = skProfiles5->MethodAddForSolid(true);
	}
	catch (...) {

		return;
	}

	ExtrudeFeaturePtr extrude5 = ftExtrude->MethodAddByThroughAllExtent(pProfile5, m_Height, kNegativeExtentDirection, kCutOperation);
}


void CClampingJawsDlg::OnBnClickedOk()
{
	// TODO: добавьте свой код обработчика уведомлений
	if (pInvApp)
		pInvApp->MethodQuit();

	CDialog::OnOK();
}


void CClampingJawsDlg::OnBnClickedButton2()
{
	{
		SelectSetPtr pSelect;
		pPartDoc->get_SelectSet(&pSelect);

		if (pSelect->GetCount() > 0)
		{
			FacePtr Seekface = pSelect->GetItem(1);

			int seeknumber = -1;
			for (int i = 1; i <= pPartComDef->SurfaceBodies->GetCount(); i++)
			{
				SurfaceBodyPtr SurfBody;
				SurfaceBodiesPtr SurfBodies;

				pPartComDef->get_SurfaceBodies(&SurfBodies);

				SurfBodies->get_Item(i, &SurfBody);

				FacePtr face;
				FacesPtr faces;

				SurfBody->get_Faces(&faces);
				int N = 0;
				N = SurfBody->Faces->GetCount();
				for (int j = 1; j <= SurfBody->Faces->GetCount(); j++) {
					faces->get_Item(j, &face);

					if (Seekface == face)
					{
						seeknumber = j;
						CString str;
						str.Format(L"%i", j);
						MessageBox(str);
					}
				}

			}

		}
	}

}
