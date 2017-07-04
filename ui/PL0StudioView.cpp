
// PL0StudioView.cpp : CPL0StudioView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "pl0_studio.h"
#endif

#include "PL0StudioDoc.h"
#include "CntrItem.h"
#include "resource.h"
#include "PL0StudioView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPL0StudioView

IMPLEMENT_DYNCREATE(CPL0StudioView, CRichEditView)

BEGIN_MESSAGE_MAP(CPL0StudioView, CRichEditView)
	ON_WM_DESTROY()
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CPL0StudioView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_NOTIFY_REFLECT(EN_SELCHANGE, &CPL0StudioView::OnEnSelchange)
END_MESSAGE_MAP()

// CPL0StudioView ����/����

CPL0StudioView::CPL0StudioView()
{
	m_stopDetect = false;
	m_colorPattern.insert(make_pair(Structure_integer, RGB(44, 255, 145)));
	m_colorPattern.insert(make_pair(Structure_real, RGB(44, 255, 145)));
	m_colorPattern.insert(make_pair(Structure_string, RGB(211, 67, 213)));
	m_colorPattern.insert(make_pair(Structure_comment, RGB(0, 255, 0)));
	m_colorPattern.insert(make_pair(Structure_keyword, RGB(0, 0, 255)));
	m_colorPattern.insert(make_pair(Structure_functionName, RGB(255, 0, 0)));
	m_colorPattern.insert(make_pair(Structure_constVariable, RGB(255, 0, 255)));
	m_colorPattern.insert(make_pair(Structure_variable, RGB(255, 255, 0)));
	m_colorPattern.insert(make_pair(Structure_procudureName, RGB(255, 0, 0)));
	m_colorPattern.insert(make_pair(Structure_typename, RGB(0, 255, 255)));
}

CPL0StudioView::~CPL0StudioView()
{
}

BOOL CPL0StudioView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CRichEditView::PreCreateWindow(cs);
}

void CPL0StudioView::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();


	// ���ô�ӡ�߾�(720 � = 1/2 Ӣ��)
	SetMargins(CRect(720, 720, 720, 720));
}


// CPL0StudioView ��ӡ


void CPL0StudioView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CPL0StudioView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}


void CPL0StudioView::OnDestroy()
{
	// ����ʱͣ�ô������
	// ʹ�ò����ͼʱ�ǳ���Ҫ 
   COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
   if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
   {
      pActiveItem->Deactivate();
      ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
   }
   CRichEditView::OnDestroy();
}


void CPL0StudioView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CPL0StudioView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CPL0StudioView ���

#ifdef _DEBUG
void CPL0StudioView::AssertValid() const
{
	CRichEditView::AssertValid();
}

void CPL0StudioView::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}

CPL0StudioDoc* CPL0StudioView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPL0StudioDoc)));
	return (CPL0StudioDoc*)m_pDocument;
}
#endif //_DEBUG


// CPL0StudioView ��Ϣ�������
void CPL0StudioView::setTextFormat(long start, long end, COLORREF color)
{
	CHARFORMAT cf;
	ZeroMemory(&cf, sizeof(CHARFORMAT));
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = color;

	GetRichEditCtrl().SetSel(start, end);
	GetRichEditCtrl().SetSelectionCharFormat(cf);
}

void CPL0StudioView::resetDefaultFormat()
{
	CHARFORMAT cf;
	ZeroMemory(&cf, sizeof(CHARFORMAT));
	cf.dwMask |= CFM_BOLD;
	cf.dwEffects &= ~CFE_BOLD;// ȡ������
	cf.dwMask |= CFM_ITALIC;
	cf.dwEffects &= ~CFE_ITALIC;// ȡ��б��
	cf.dwMask |= CFM_UNDERLINE;
	cf.dwEffects &= CFE_UNDERLINE;// ȡ���»���
	cf.dwMask |= CFM_COLOR;
	cf.crTextColor = RGB(0, 0, 0); // ������ɫ
	cf.dwMask |= CFM_SIZE;
	cf.yHeight = 320; // ���ø߶�
	cf.dwMask |= CFM_FACE;
	wcscpy_s(cf.szFaceName, _T("Consolas")); // ��������
	GetRichEditCtrl().SetSelectionCharFormat(cf);
}

void CPL0StudioView::colorSyntax()
{
	CString code;
	GetWindowText(code);
	m_parser.setBuffer(code).parse();
	auto tokenList = m_parser.tokens();

	long start, end;
	GetRichEditCtrl().GetSel(start, end);
	GetRichEditCtrl().SetRedraw(FALSE);

	m_stopDetect = true;
	GetRichEditCtrl().SetSel(0, -1);
	resetDefaultFormat();

	for (auto token : tokenList)
	{
		auto it = m_colorPattern.find(token.tokenType());
		if (it != m_colorPattern.end())
		{
			setTextFormat(token.startIndex() - token.lineNum(), token.endIndex() - token.lineNum(), it->second);
		}
	}

	GetRichEditCtrl().SetSel(start, end);
	resetDefaultFormat();
	GetRichEditCtrl().SetRedraw(TRUE);
	GetRichEditCtrl().RedrawWindow();
	m_stopDetect = false;
}

void CPL0StudioView::OnEnSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);
	// TODO:  �ؼ��������ʹ�֪ͨ����������д
	// CRichEditView::OnInitDialog() �������Խ� EM_SETEVENTMASK ��Ϣ����
	// ���ÿؼ���ͬʱ�� ENM_SELCHANGE ��־�������㵽 lParam �����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (!m_stopDetect)
		colorSyntax();

	*pResult = 0;
}