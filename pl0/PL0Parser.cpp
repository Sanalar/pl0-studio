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

// <����> ::= <�ֳ���>.
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

// <�ֳ���> ::= [<����˵������>][<����˵������>][<����˵������>][<ģ��˵������>]<���>
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

// <����˵������> ::= TYPE (<��ʶ��>=(<�ض�������> | <ö������> | <�ṹ������>))+;
// <�ض�������>   ::= <��ʶ��>
void PL0Parser::typeDeclare()
{
	bool firstInside = true;

	while (true)
	{
		Token token = getVariableToken();
		if (token.tokenType() == Structure_error)
		{
			// ���ǵ�һ�ν���ʱ������ֱ�ӷ���
			if (!firstInside)
				return;

			// ��һ�ν���ʱ������ƥ�����
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

// <�ṹ������> ::= RECORD; <����������>{; <����������>}; END
void PL0Parser::recordDeclare()
{
	// ����ú���ʱ���ؼ��� record �Ѿ��ɹ�ƥ��
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

// <����������> ::= <��ʶ��>['['<����˵��>{, <����˵��>}']'] : <����>
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

// <����˵��> ::= <��������> : <��������>
void PL0Parser::rangeDeclare()
{
	integerOrConst();
	
	if (!match(L":"))
	{
		error(PL0Error_wantColon);
	}

	integerOrConst();
}

// <��������>
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

// <ö������> ::= '{' <��ʶ��> {, <��ʶ��>} '}'
void PL0Parser::enumeratedDeclare()
{
	// ������������ʱ���������Ѿ�ƥ��ɹ���
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

// <����˵������> ::= CONST <��������> {, <��������>};
void PL0Parser::constDeclare()
{
	// �����ģ���ʱ��const �ؼ����Ѿ��ɹ�ƥ��
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

// <��������> ::= <��ʶ��>=<������>
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

// <������> ::= <�з�������>|<�з���С��>|<�ַ�������>
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
