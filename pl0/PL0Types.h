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
};

enum Types
{
	Types_integer = 201,
	Types_real,
	Types_boolean,
	Types_char,
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