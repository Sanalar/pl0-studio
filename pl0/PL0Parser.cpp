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
	M(L"read", Keyword_read);
	M(L"write", Keyword_write);
	M(L"return", Keyword_return);
	M(L"odd", Keyword_odd);
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
	while(matchKeyword(Keyword_var))
		variableDeclare();
	while(matchKeyword(Keyword_procedure) || matchKeyword(Keyword_function))
		moduleDeclare();
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

// <����˵������> ::= VAR <����������>{, <����������>};
void PL0Parser::variableDeclare()
{
	// ����ú���������Ѿ��ɹ�ƥ�� var
	variableAndType();

	while (match(L","))
		variableAndType();

	if (!match(L";"))
	{
		error(PL0Error_wantSemicolon);
	}
}

// <ģ��˵������> ::= (<����˵������> | <����˵������>){; <ģ��˵������>};
void PL0Parser::moduleDeclare()
{
	Token token = getLastToken();
	if (token.detailType() == Keyword_procedure)
		procedureDeclare();
	else if (token.detailType() == Keyword_function)
		functionDeclare();

	if (!match(L";"))
	{
		error(PL0Error_wantSemicolon);
	}
}

// <����˵������> ::= <����˵���ײ�><�ֳ���>
void PL0Parser::procedureDeclare()
{
	procedureHeader();
	block();
}

// <����˵���ײ�> ::= PROCEDURE<��ʶ��>['('<�����б�>')'];
void PL0Parser::procedureHeader()
{
	// ����ú�����ʱ��procedure�ؼ����Ѿ�����ȷƥ��
	Token token = getVariableToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	if (match(L"("))
	{
		paramList();
		if (!match(L")"))
		{
			error(PL0Error_wantRParen);
		}
	}

	if (!match(L";"))
	{
		error(PL0Error_wantSemicolon);
	}
}

// <�����б�> ::= (<����������> {, <����������>})|<��>
void PL0Parser::paramList()
{
	// �����Ϊ { ')' }�����ֱ��Ϊ�����˵���Ƴ�Ϊ��
	if (*m_p == L')')
		return;

	variableAndType();
	
	while (match(L","))
	{
		variableAndType();
	}
}

// <����˵������> ::= <����˵���ײ�><�ֳ���>
void PL0Parser::functionDeclare()
{
	functionHeader();
	block();
}

// <����˵���ײ�> ::= FUNCTION<��ʶ��>['('<�����б�>')']:<����>;
void PL0Parser::functionHeader()
{
	// ����ú�����ʱ��function�ؼ����Ѿ��ɹ�ʶ��
	Token token = getVariableToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	if (match(L"("))
	{
		paramList();

		if (!match(L")"))
		{
			error(PL0Error_wantRParen);
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

	if (!match(L";"))
	{
		error(PL0Error_wantColon);
	}
}

// <���> ::= <��ֵ���>|<�������>|<ѭ�����>|<���̵������>|<�����>|<д���>|<�������>|<�������>|<��>
/*
	if, case           -> <�������>
	read               -> <�����>
	write              -> <д���>
	while, repeat, for -> <ѭ�����>
	begin              -> <�������>
	return             -> <�������>
	call               -> <���̵������>
	variable           -> <��ֵ���>|<���̵������>
	�������           -> <��>

	����������⣬first����Ϊ<��ʶ��>��
	������ϣ�
	'.', ';', 'END'
*/
void PL0Parser::statement()
{
	Token token = getIdentityToken();
	// �жϺ�����ϣ��Ƶ������
	if (*m_p == '.' || *m_p == ';' || token.detailType() == Keyword_end)
		return;

	switch (token.detailType())
	{
	case Keyword_if:
		ifStatement();
		break;
	case Keyword_case:
		caseStatement();
		break;
	case Keyword_read:
		readStatement();
		break;
	case Keyword_write:
		writeStatement();
		break;
	case Keyword_while:
		whileStatement();
		break;
	case Keyword_repeat:
		repeatStatement();
		break;
	case Keyword_for:
		forStatement();
		break;
	case Keyword_begin:
		beginStatement();
		break;
	case Keyword_return:
		returnStatement();
		break;
	case Keyword_call:
	case Structure_functionName:
	case Structure_procudureName:
		callStatement();
		break;
	default:
		assignStatement();
		break;
	}
}

// <IF�������> ::= IF<�������ʽ>THEN<���>{ELSEIF<�������ʽ>THEN<���>}[ELSE<���>]
void PL0Parser::ifStatement()
{
	// ����ú�����ʱ��if�ؼ����Ѿ��ɹ�ƥ��
	conditionExpression();

	if (!matchKeyword(Keyword_then))
	{
		error(PL0Error_wantThen);
	}

	statement();

	while (matchKeyword(Keyword_elseif))
	{
		conditionExpression();

		if (!matchKeyword(Keyword_then))
		{
			error(PL0Error_wantThen);
		}

		statement();
	}

	if (matchKeyword(Keyword_else))
	{
		statement();
	}
}

// <�������ʽ> ::= <������> {OR <������>}
void PL0Parser::conditionExpression()
{
	orCondition();

	while (matchKeyword(Keyword_or))
	{
		orCondition();
	}
}

// <������> ::= <����> {AND <����>}
void PL0Parser::orCondition()
{
	condition();

	while (matchKeyword(Keyword_and))
	{
		condition();
	}
}

// <����> ::= <���ʽ><��ϵ�����><���ʽ>|ODD<���ʽ>
// <��ϵ�����> ::= =|#|<|<=|>|>=|!=
void PL0Parser::condition()
{
	if (matchKeyword(Keyword_odd) || match(L"!"))
	{
		expression();
	}
	else
	{
		expression();

		if (match(L">="))
		{

		}
		else if (match(L"<="))
		{

		}
		else if (match(L"!=") || match(L"#"))
		{

		}
		else if (match(L"==") || match(L"="))
		{

		}
		else if (match(L"<"))
		{

		}
		else if (match(L">"))
		{

		}
		else
		{
			error(PL0Error_wantConditionOperator);
		}

		expression();
	}
}
