
// CntrItem.cpp : Cpl0_studioCntrItem 类的实现
//

#include "stdafx.h"
#include "pl0_studio.h"

#include "PL0StudioDoc.h"
#include "PL0StudioView.h"
#include "CntrItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cpl0_studioCntrItem 的实现

IMPLEMENT_SERIAL(Cpl0_studioCntrItem, CRichEditCntrItem, 0)

Cpl0_studioCntrItem::Cpl0_studioCntrItem(REOBJECT* preo, CPL0StudioDoc* pContainer)
	: CRichEditCntrItem(preo, pContainer)
{
	// TODO: 在此添加一次性构造代码
}

Cpl0_studioCntrItem::~Cpl0_studioCntrItem()
{
	// TODO: 在此处添加清理代码
}


// Cpl0_studioCntrItem 诊断

#ifdef _DEBUG
void Cpl0_studioCntrItem::AssertValid() const
{
	CRichEditCntrItem::AssertValid();
}

void Cpl0_studioCntrItem::Dump(CDumpContext& dc) const
{
	CRichEditCntrItem::Dump(dc);
}
#endif

