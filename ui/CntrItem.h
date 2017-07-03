
// CntrItem.h : Cpl0_studioCntrItem 类的接口
//

#pragma once

class CPL0StudioDoc;
class CPL0StudioView;

class Cpl0_studioCntrItem : public CRichEditCntrItem
{
	DECLARE_SERIAL(Cpl0_studioCntrItem)

// 构造函数
public:
	Cpl0_studioCntrItem(REOBJECT* preo = NULL, CPL0StudioDoc* pContainer = NULL);
		// 注意: 允许 pContainer 为 NULL 以启用 IMPLEMENT_SERIALIZE
		//  IMPLEMENT_SERIALIZE 要求类具有带零
		//  参数的构造函数。  OLE 项通常是用
		//  非 NULL 文档指针构造的

// 特性
public:
	CPL0StudioDoc* GetDocument()
		{ return reinterpret_cast<CPL0StudioDoc*>(CRichEditCntrItem::GetDocument()); }
	CPL0StudioView* GetActiveView()
		{ return reinterpret_cast<CPL0StudioView*>(CRichEditCntrItem::GetActiveView()); }

// 实现
public:
	~Cpl0_studioCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

