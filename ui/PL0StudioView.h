
// PL0StudioView.h : CPL0StudioView ��Ľӿ�
//

#pragma once

class Cpl0_studioCntrItem;

class CPL0StudioView : public CRichEditView
{
protected: // �������л�����
	CPL0StudioView();
	DECLARE_DYNCREATE(CPL0StudioView)

// ����
public:
	CPL0StudioDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // ������һ�ε���
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CPL0StudioView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnDestroy();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // PL0StudioView.cpp �еĵ��԰汾
inline CPL0StudioDoc* CPL0StudioView::GetDocument() const
   { return reinterpret_cast<CPL0StudioDoc*>(m_pDocument); }
#endif

