#pragma once

enum Structure
{
	Structure_error = 0,
	Structure_id,
	Structure_integer,
	Structure_real,
	Structure_string,
	Structure_operator,
	Structure_comment,
	Structure_keyword,
	Structure_functionName,
	Structure_constVariable,
	Structure_variable,
	Structure_procudureName,
	Structure_typename,
};

enum Keyword
{
	Keyword_and = 101,
	Keyword_array,
	Keyword_begin,
	Keyword_call,
	Keyword_case,
	Keyword_const,
	Keyword_do,
	Keyword_else,
	Keyword_elseif,
	Keyword_end,
	Keyword_for,
	Keyword_function,
	Keyword_if,
	Keyword_mod,
	Keyword_not,
	Keyword_of,
	Keyword_or,
	Keyword_procedure,
	Keyword_program,
	Keyword_record,
	Keyword_repeat,
	Keyword_then,
	Keyword_to,
	Keyword_type,
	Keyword_until,
	Keyword_var,
	Keyword_while,
	Keyword_read,
	Keyword_write,
	Keyword_return,
	Keyword_odd,
	Keyword_step,
	Keyword_integer,
	Keyword_real,
	Keyword_char,
	Keyword_boolean,
};

enum Types
{
	Types_integer = 201,
	Types_real,
	Types_boolean,
	Types_char,
	Types_record,
	Types_enumerated,
	Types_alias,
	Types_void,
	Types_string,
};

enum VarType
{
	VarType_alone = 301,
	VarType_member,
};

class Token
{
public:
	inline Structure tokenType() const { return m_tokenType; }
	inline void setTokenType(Structure type) { m_tokenType = type; }
	inline int detailType() const { return m_detailType; }
	inline void setDetailType(int type) { m_detailType = type; }
	inline int startIndex() const { return m_startIndex; }
	inline void setStartIndex(int index) { m_startIndex = index; }
	inline int endIndex() const { return m_endIndex; }
	inline void setEndIndex(int index) { m_endIndex = index; }
	inline int lineNum() const { return m_lineNum; }
	inline void setLineNum(int num) { m_lineNum = num; }
	inline int colNum() const { return m_colNum; }
	inline void setColNum(int num) { m_colNum = num; }
	inline const wstring& name() const { return m_name; }
	inline void setName(wstring n) { m_name = n; }

	Token() { m_tokenType = Structure_error; m_detailType = 0; }

protected:
	wstring m_name;
	int m_startIndex;
	int m_endIndex;
	int m_lineNum;
	int m_colNum;
	Structure m_tokenType;
	int m_detailType;
};

enum Command
{
	PUSH,			///< 将一个立即数压入栈顶
	PUSH_VAR,		///< 将一个变量的值入栈
	ADD,			///< 将栈顶与次栈顶的数相加，结果放到次栈顶，并弹出栈顶，不加特殊说明，所有算数运算都是这个规则
	SUB,			///< 次栈顶 = 次栈顶 - 栈顶
	MUL,			///< 次栈顶 = 次栈顶 * 栈顶
	DIV,			///< 次栈顶 = 次栈顶 / 栈顶
	MOD,			///< 次栈顶 = 次栈顶 % 栈顶
	EXIT,			///< 停机指令
	ALLOC,
	JZ,
	JMP,
	OR,
	AND,
	NOT,
	GE,
	LE,
	NE,
	GT,
	LT,
	EQ,
	NOOP,
	NEG,
	LOD,
	CMP,
	JNZ,
	READ,
	WRITE,
	SADD,
	JLE,
	POP,
	PUSH_EBP,
	RESET_EBP,
	STO,
};

struct Instruction
{
	Command cmd;
	int arg1;
	int arg2;
	int target;
};