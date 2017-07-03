
// CntrItem.h : Cpl0_studioCntrItem ��Ľӿ�
//

#pragma once

class CPL0StudioDoc;
class CPL0StudioView;

class Cpl0_studioCntrItem : public CRichEditCntrItem
{
	DECLARE_SERIAL(Cpl0_studioCntrItem)

// ���캯��
public:
	Cpl0_studioCntrItem(REOBJECT* preo = NULL, CPL0StudioDoc* pContainer = NULL);
		// ע��: ���� pContainer Ϊ NULL ������ IMPLEMENT_SERIALIZE
		//  IMPLEMENT_SERIALIZE Ҫ������д���
		//  �����Ĺ��캯����  OLE ��ͨ������
		//  �� NULL �ĵ�ָ�빹���

// ����
public:
	CPL0StudioDoc* GetDocument()
		{ return reinterpret_cast<CPL0StudioDoc*>(CRichEditCntrItem::GetDocument()); }
	CPL0StudioView* GetActiveView()
		{ return reinterpret_cast<CPL0StudioView*>(CRichEditCntrItem::GetActiveView()); }

// ʵ��
public:
	~Cpl0_studioCntrItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
};

