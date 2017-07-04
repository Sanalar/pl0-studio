
// PL0StudioView.h : CPL0StudioView 类的接口
//

#pragma once
#include "PL0Parser.h"

class Cpl0_studioCntrItem;

class CPL0StudioView : public CRichEditView
{
protected: // 仅从序列化创建
	CPL0StudioView();
	DECLARE_DYNCREATE(CPL0StudioView)

// 特性
public:
	CPL0StudioDoc* GetDocument() const;

// 操作
public:
	void setTextFormat(long start, long end, COLORREF color);
	void resetDefaultFormat();
	void colorSyntax();

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 构造后第一次调用
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// 实现
public:
	virtual ~CPL0StudioView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	unordered_map<Structure, COLORREF> m_colorPattern;
	PL0Parser m_parser;
	bool m_stopDetect;

// 生成的消息映射函数
protected:
	afx_msg void OnDestroy();
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // PL0StudioView.cpp 中的调试版本
inline CPL0StudioDoc* CPL0StudioView::GetDocument() const
   { return reinterpret_cast<CPL0StudioDoc*>(m_pDocument); }
#endif

