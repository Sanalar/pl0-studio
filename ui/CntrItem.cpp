
// CntrItem.cpp : Cpl0_studioCntrItem ���ʵ��
//

#include "stdafx.h"
#include "pl0_studio.h"

#include "PL0StudioDoc.h"
#include "PL0StudioView.h"
#include "CntrItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cpl0_studioCntrItem ��ʵ��

IMPLEMENT_SERIAL(Cpl0_studioCntrItem, CRichEditCntrItem, 0)

Cpl0_studioCntrItem::Cpl0_studioCntrItem(REOBJECT* preo, CPL0StudioDoc* pContainer)
	: CRichEditCntrItem(preo, pContainer)
{
	// TODO: �ڴ����һ���Թ������
}

Cpl0_studioCntrItem::~Cpl0_studioCntrItem()
{
	// TODO: �ڴ˴�����������
}


// Cpl0_studioCntrItem ���

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

