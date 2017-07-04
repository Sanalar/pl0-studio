
// PL0StudioView.cpp : CPL0StudioView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
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
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CRichEditView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CPL0StudioView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_NOTIFY_REFLECT(EN_SELCHANGE, &CPL0StudioView::OnEnSelchange)
END_MESSAGE_MAP()

// CPL0StudioView 构造/析构

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
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CRichEditView::PreCreateWindow(cs);
}

void CPL0StudioView::OnInitialUpdate()
{
	CRichEditView::OnInitialUpdate();


	// 设置打印边距(720 缇 = 1/2 英寸)
	SetMargins(CRect(720, 720, 720, 720));
}


// CPL0StudioView 打印


void CPL0StudioView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CPL0StudioView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}


void CPL0StudioView::OnDestroy()
{
	// 析构时停用此项；这在
	// 使用拆分视图时非常重要 
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


// CPL0StudioView 诊断

#ifdef _DEBUG
void CPL0StudioView::AssertValid() const
{
	CRichEditView::AssertValid();
}

void CPL0StudioView::Dump(CDumpContext& dc) const
{
	CRichEditView::Dump(dc);
}

CPL0StudioDoc* CPL0StudioView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPL0StudioDoc)));
	return (CPL0StudioDoc*)m_pDocument;
}
#endif //_DEBUG


// CPL0StudioView 消息处理程序
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
	cf.dwEffects &= ~CFE_BOLD;// 取消粗体
	cf.dwMask |= CFM_ITALIC;
	cf.dwEffects &= ~CFE_ITALIC;// 取消斜体
	cf.dwMask |= CFM_UNDERLINE;
	cf.dwEffects &= CFE_UNDERLINE;// 取消下划线
	cf.dwMask |= CFM_COLOR;
	cf.crTextColor = RGB(0, 0, 0); // 设置颜色
	cf.dwMask |= CFM_SIZE;
	cf.yHeight = 320; // 设置高度
	cf.dwMask |= CFM_FACE;
	wcscpy_s(cf.szFaceName, _T("Consolas")); // 设置字体
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
	// TODO:  控件将不发送此通知，除非您重写
	// CRichEditView::OnInitDialog() 函数，以将 EM_SETEVENTMASK 消息发送
	// 到该控件，同时将 ENM_SELCHANGE 标志“或”运算到 lParam 掩码中。

	// TODO:  在此添加控件通知处理程序代码
	if (!m_stopDetect)
		colorSyntax();

	*pResult = 0;
}