
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
END_MESSAGE_MAP()

// CPL0StudioView 构造/析构

CPL0StudioView::CPL0StudioView()
{
	// TODO: 在此处添加构造代码

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
