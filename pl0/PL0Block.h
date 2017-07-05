#pragma once

/*
	一个程序由树形的 block 组成。
	每个 block 都有自己的符号表，代码生成规则。
	block 的代码前面是函数调用信息，即：
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