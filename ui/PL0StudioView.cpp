
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
END_MESSAGE_MAP()

// CPL0StudioView ����/����

CPL0StudioView::CPL0StudioView()
{
	// TODO: �ڴ˴���ӹ������

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
