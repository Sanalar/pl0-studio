#pragma once

enum Structure
{
	Structure_error = 0,
	Structure_id,
	Structure_number,
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
	inline int detailType() const { return m_detailType; }

protected:
	wstring m_text;
	int m_startIndex;
	int m_endIndex;
	int m_lineNum;
	int m_colNum;
	Structure m_tokenType;
	int m_detailType;
};