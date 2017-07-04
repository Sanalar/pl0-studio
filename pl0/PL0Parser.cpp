#include "stdafx.h"
#include "PL0Parser.h"

#define error(e) m_reporter.raiseError(e, m_curRow, m_curCol, m_curPos)

void InitKeywordsMap(unordered_map<const wchar_t*, Keyword>& m_keywords)
{
#define M(s, k) m_keywords.insert(make_pair(s, k))
	M(L"and", Keyword_and);
	M(L"array", Keyword_array);
	M(L"begin", Keyword_begin);
	M(L"call", Keyword_call);
	M(L"case", Keyword_case);
	M(L"const", Keyword_const);
	M(L"do", Keyword_do);
	M(L"else", Keyword_else);
	M(L"elseif", Keyword_elseif);
	M(L"end", Keyword_end);
	M(L"for", Keyword_for);
	M(L"function", Keyword_function);
	M(L"if", Keyword_if);
	M(L"mod", Keyword_mod);
	M(L"not", Keyword_not);
	M(L"of", Keyword_of);
	M(L"or", Keyword_or);
	M(L"procedure", Keyword_procedure);
	M(L"program", Keyword_program);
	M(L"record", Keyword_record);
	M(L"repeat", Keyword_repeat);
	M(L"then", Keyword_then);
	M(L"to", Keyword_to);
	M(L"type", Keyword_type);
	M(L"until", Keyword_until);
	M(L"var", Keyword_var);
	M(L"while", Keyword_while);
#undef M
}

PL0Parser::PL0Parser()
{

}

// <程序> ::= <分程序>.
void PL0Parser::program()
{
	block();

	if (!match(L"."))
	{
		error(PL0Error_missingPeriod);
		moveCursor();
	}

	if (*m_p != 0)
	{
		error(PL0Error_expectedFileEnd);
	}
}

// <分程序> ::= [<常量说明部分>][<类型说明部分>][<变量说明部分>][<模块说明部分>]<语句>
void PL0Parser::block()
{
	while (matchKeyword(Keyword_const))
		constDeclare();
	while(matchKeyword(Keyword_type))
		typeDeclare();
	variableDeclare();
	functionDeclare();
	statement();
}

// <类型说明部分> ::= TYPE (<标识符>=(<重定义类型> | <枚举类型> | <结构体类型>))+;
// <重定义类型>   ::= <标识符>
void PL0Parser::typeDeclare()
{
	bool firstInside = true;

	while (true)
	{
		Token token = getVariableToken();
		if (token.tokenType() == Structure_error)
		{
			// 不是第一次进入时，可以直接返回
			if (!firstInside)
				return;

			// 第一次进入时，必须匹配该项
			error(PL0Error_wantIdentity);
		}

		if (!match(L"="))
		{
			error(PL0Error_wantEqualSymbol);
		}

		if (matchKeyword(Keyword_record))
		{
			recordDeclare();
		}
		else if (match(L"{"))
		{
			enumeratedDeclare();
		}
		else
		{
			token = getTypenameToken();
			if (token.tokenType() == Structure_error)
			{
				error(PL0Error_unexpectedSymbol);
			}
		}
	}
}

// <结构体类型> ::= RECORD; <变量及类型>{; <变量及类型>}; END
void PL0Parser::recordDeclare()
{
	// 进入该函数时，关键字 record 已经成功匹配
	if (!match(L";"))
	{
		error(PL0Error_wantSemicolon);
	}

	do 
	{
		variableAndType();
		if (!match(L";"))
		{
			error(PL0Error_wantSemicolon);
		}
	} while (!matchKeyword(Keyword_end));
}

// <变量及类型> ::= <标识符>['['<区间说明>{, <区间说明>}']'] : <类型>
void PL0Parser::variableAndType()
{
	Token token = getVariableToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	if (match(L"["))
	{
		rangeDeclare();
		while (match(L","))
		{
			rangeDeclare();
		}

		if (!match(L"]"))
		{
			error(PL0Error_wantRSquareBracket);
		}
	}

	if (!match(L":"))
	{
		error(PL0Error_wantColon);
	}

	token = getTypenameToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantTypename);
	}
}

// <区间说明> ::= <整数或常量> : <整数或常量>
void PL0Parser::rangeDeclare()
{
	integerOrConst();
	
	if (!match(L":"))
	{
		error(PL0Error_wantColon);
	}

	integerOrConst();
}

// <整数或常量>
void PL0Parser::integerOrConst()
{
	Token token = getIntegerToken();
	if (token.tokenType() == Structure_error)
		token = getConstNameToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIntegerOrConst);
	}
}

// <枚举类型> ::= '{' <标识符> {, <标识符>} '}'
void PL0Parser::enumeratedDeclare()
{
	// 进入这个代码的时候左花括号已经匹配成功了
	Token token = getVariableToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	while (match(L","))
	{
		token = getVariableToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantIdentity);
		}
	}

	if (!match(L"}"))
	{
		error(PL0Error_wantRBrace);
	}
}

// <常量说明部分> ::= CONST <常量定义> {, <常量定义>};
void PL0Parser::constDeclare()
{
	// 进入该模块的时候，const 关键字已经成功匹配
	constVarDefine();

	while (match(L","))
	{
		constVarDefine();
	}

	if (!match(L";"))
	{
		error(PL0Error_wantSemicolon);
	}
}

// <常量定义> ::= <标识符>=<字面量>
void PL0Parser::constVarDefine()
{
	Token token = getVariableToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	if (!match(L"="))
	{
		error(PL0Error_wantEqualSymbol);
	}

	literalValue();
}

// <字面量> ::= <有符号整数>|<有符号小数>|<字符串常量>
void PL0Parser::literalValue()
{
	if (isalnum(*m_p))
	{
		Token token = getNumberToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantNumber);
		}
	}
	else if (*m_p == '\"')
	{
		Token token = getStringToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantString);
		}
	}
	else
	{
		error(PL0Error_wantIdentity);
	}
}
