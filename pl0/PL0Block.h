#pragma once

/*
	һ�����������ε� block ��ɡ�
	ÿ�� block �����Լ��ķ��ű��������ɹ���
	block �Ĵ���ǰ���Ǻ���������Ϣ������
	[return value  ]
	[former ebp    ]    <-- ebp
	[return address]
	[arg1          ]
	[arg2          ]
	...
	[arg n         ]
	<block vars>
*/
class PL0Block
{
protected:
	SymbolTable m_symbols;
	PL0Block* m_parent;
	Types m_returnType;
	vector<VariableInfo> m_args;
};