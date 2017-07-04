#include "stdafx.h"
#include "PL0Parser.h"

static Token errorToken;

#define error(e) m_reporter.raiseError(e, m_curRow, m_curCol, m_curPos)

void InitKeywordsMap(unordered_map<wstring, Keyword>& m_keywords)
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
	M(L"step", Keyword_step);
#undef M
}

PL0Parser::PL0Parser()
{
	InitKeywordsMap(m_keywords);
}

void PL0Parser::parse()
{
	resetContents();
	program();
}

bool PL0Parser::match(const wchar_t* symbol)
{
	skipWhiteSpaceAndComment();

	saveCursor();

	const wchar_t* target = symbol;
	while (*m_p != 0)
	{
		if (*target == 0)
			return true;

		if (*m_p != *target)
			break;

		moveCursor();
		++target;
	}

	restoreCursor();
	return false;
}

bool PL0Parser::matchKeyword(Keyword key)
{
	skipWhiteSpaceAndComment();
	saveCursor();

	wstring keyStr = getIdentity();
	auto it = m_keywords.find(keyStr.c_str());
	if (it == m_keywords.end() || it->second != key)
	{
		restoreCursor();
		return false;
	}
	
	Token token;
	token.setTokenType(Structure_keyword);
	token.setDetailType(key);
	token.setStartIndex(m_savedPos);
	token.setEndIndex(m_curPos);
	token.setLineNum(m_savedRow);
	token.setColNum(m_savedCol);
	m_tokens.push_back(token);

	return true;
}

std::wstring PL0Parser::getIdentity()
{
	wstring res;

	if (!isalpha(*m_p) && *m_p != '_')
	{
		return res;
	}

	res += *m_p;
	moveCursor();

	while (isalnum(*m_p) || *m_p == '_')
	{
		res += ::tolower(*m_p);
		moveCursor();
	}

	return res;
}

Token& PL0Parser::getVariableToken()
{
	Token& res = getIdentityToken();
	return res.tokenType() == Structure_variable ? res : errorToken;
}

Token& PL0Parser::getTypenameToken()
{
	Token& res = getIdentityToken();
	return res.tokenType() == Structure_typename ? res : errorToken;
}

Token& PL0Parser::getConstNameToken()
{
	Token& res = getIdentityToken();
	return res.tokenType() == Structure_constVariable ? res : errorToken;
}

Token& PL0Parser::getNumberToken()
{
	skipWhiteSpaceAndComment();
	wstring numStr;
	int type = Structure_error;

	saveCursor();

	if (isdigit(*m_p))
	{
		numStr += *m_p;
		moveCursor();
		type = Structure_integer;

		while (isdigit(*m_p))
		{
			numStr += *m_p;
			moveCursor();
		}

		if (*m_p == '.')
		{
			numStr += *m_p;
			moveCursor();
			type = Structure_real;
			while (isdigit(*m_p))
			{
				numStr += *m_p;
				moveCursor();
			}
		}
	}

	if (type == Structure_error)
	{
		restoreCursor();
		return errorToken;
	}

	Token token;
	token.setTokenType((Structure)type);
	token.setDetailType(type);
	token.setStartIndex(m_savedPos);
	token.setEndIndex(m_curPos);
	token.setLineNum(m_savedRow);
	token.setColNum(m_savedCol);
	m_tokens.push_back(token);
	return m_tokens.back();
}

Token& PL0Parser::getStringToken()
{
	skipWhiteSpaceAndComment();
	saveCursor();

	if (*m_p != '\"')
	{
		return errorToken;
	}

	moveCursor();

	while (*m_p != '\"')
	{
		if (*m_p == '\\')
			moveCursor();
		if (*m_p == 0)
			break;

		moveCursor();
	}
	moveCursor();

	Token token;
	token.setTokenType(Structure_string);
	token.setDetailType(Structure_string);
	token.setStartIndex(m_savedPos);
	token.setEndIndex(m_curPos);
	token.setLineNum(m_savedRow);
	token.setColNum(m_savedCol);
	m_tokens.push_back(token);
	return m_tokens.back();
}

Token& PL0Parser::getLastToken()
{
	if (m_tokens.size() == 0)
		return errorToken;
	return m_tokens.back();
}

Token& PL0Parser::getIdentityToken()
{
	skipWhiteSpaceAndComment();
	saveCursor();
	wstring key = getIdentity();
	if (key.length() == 0)
		return errorToken;

	Token token;
	token.setStartIndex(m_savedPos);
	token.setEndIndex(m_curPos);
	token.setLineNum(m_savedRow);
	token.setColNum(m_savedCol);
	token.setName(key);

	// �Ȳ����ǲ��ǹؼ���
	auto it = m_keywords.find(key.c_str());
	if (it != m_keywords.end())
	{
		token.setTokenType(Structure_keyword);
		token.setDetailType(it->second);
		m_tokens.push_back(token);
		return m_tokens.back();
	}

	// �ٲ����ǲ���������

	// �����ǲ����Ѿ�����ķ���
	int tokenType = m_symTable.getSymbolType(key);
	if (tokenType == Structure_error)
	{
		// ���ڹؼ��ֱ�Ҳ�������ͱ�Ҳ���ڷ��ű��������·���
		token.setTokenType(Structure_id);
		token.setDetailType(Structure_id);
		m_tokens.push_back(token);
		return m_tokens.back();
	}

	switch (tokenType)
	{
	case Structure_typename:
	case Structure_variable:
	case Structure_constVariable:
	case Structure_functionName:
	case Structure_procudureName:
		token.setTokenType((Structure)tokenType);
		token.setDetailType((Structure)tokenType);
		break;
	default:
		restoreCursor();
		return errorToken;
	}
		
	m_tokens.push_back(token);
	return m_tokens.back();
}

void PL0Parser::moveCursor(int step /*= 1*/)
{
	if (step > 1)
	{
		moveCursor(step - 1);
	}

	if (*m_p != 0)
	{
		if (*m_p == '\n')
		{
			++m_curRow;
			m_curCol = 0;
		}
		else
		{
			++m_curCol;
		}
		++m_p;
		++m_curPos;
	}
}

void PL0Parser::saveCursor()
{
	m_savedCol = m_curCol;
	m_savedPos = m_curPos;
	m_savedRow = m_curRow;
}

void PL0Parser::restoreCursor()
{
	m_curCol = m_savedCol;
	m_curPos = m_savedPos;
	m_curRow = m_savedRow;
	m_p = &m_buffer[m_curPos];
}	
	
void PL0Parser::skipWhiteSpaceAndComment()
{
	while ((isspace(*m_p) || *m_p == '/') && *m_p != 0)
	{
		if (*m_p == '/')
		{
			if (!comment())
				return;
			continue;
		}

		moveCursor();
	}
}

bool PL0Parser::comment()
{
	// �����и�������Ϊ��match �ɹ���ʱ�򣬱�����α겻�ᱻ�޸�
	if (match(L"//"))
	{
		while (*m_p != '\n' && *m_p != 0)
		{
			moveCursor();
		}

		Token token;
		token.setTokenType(Structure_comment);
		token.setDetailType(Structure_comment);
		token.setStartIndex(m_savedPos);
		token.setEndIndex(m_curPos);
		token.setLineNum(m_savedRow);
		token.setColNum(m_savedCol);
		m_tokens.push_back(token);
		return true;
	}
	else if (match(L"/*"))
	{
		while (*m_p != 0)
		{
			if (*m_p == '*' && *(m_p + 1) == '/')
			{
				moveCursor(2);
				Token token;
				token.setTokenType(Structure_comment);
				token.setDetailType(Structure_comment);
				token.setStartIndex(m_savedPos);
				token.setEndIndex(m_curPos);
				token.setLineNum(m_savedRow);
				token.setColNum(m_savedCol);
				m_tokens.push_back(token);
				return true;
			}

			moveCursor();
		}
	}

	return false;
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
		Token& token = getIdentityToken();
		if (token.tokenType() == Structure_error)
		{
			// ���ǵ�һ�ν���ʱ������ֱ�ӷ���
			if (!firstInside)
				return;

			// ��һ�ν���ʱ������ƥ�����
			error(PL0Error_wantIdentity);
		}

		m_symTable.putSymbol(token.name(), Structure_typename, 0);
		token.setTokenType(Structure_typename);

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
	Token& token = getIdentityToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	m_symTable.putSymbol(token.name(), Structure_variable, 0);
	token.setTokenType(Structure_variable);

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
	Token token = getNumberToken();
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
	Token& token = getIdentityToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	m_symTable.putSymbol(token.name(), Structure_constVariable, 0);
	token.setTokenType(Structure_constVariable);

	while (match(L","))
	{
		Token& token2 = getIdentityToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantIdentity);
		}
		m_symTable.putSymbol(token2.name(), Structure_constVariable, 0);
		token2.setTokenType(Structure_constVariable);
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
	Token& token = getIdentityToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	if (!match(L"="))
	{
		error(PL0Error_wantEqualSymbol);
	}

	m_symTable.putSymbol(token.name(), Structure_constVariable, 0);
	token.setTokenType(Structure_constVariable);

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
	Token& token = getIdentityToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	m_symTable.putSymbol(token.name(), Structure_procudureName, 0);
	token.setTokenType(Structure_procudureName);

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
	Token& token = getIdentityToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	m_symTable.putSymbol(token.name(), Structure_functionName, 0);
	token.setTokenType(Structure_functionName);

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
		getIdentityToken();
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

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
void PL0Parser::expression()
{
	if (match(L"+") || match(L"-"))
	{
		
	}

	factor();

	while (match(L"+") || match(L"-"))
	{
		factor();
	}
}

// <��> ::= <����>{<�˷������><����>}
void PL0Parser::factor()
{
	atom();

	while (match(L"*") || match(L"/"))
	{
		atom();
	}
}

// <����> ::= <��ֵ���ʽ>|<������>|<����>|'(' <���ʽ> ')'|<�������>
/*
	variable, const_name -> <��ֵ���ʽ>
	number, string       -> <������>
	(                    -> <���ű��ʽ>
	call, func, proc     -> <�������>
*/
void PL0Parser::atom()
{
	if (match(L"("))
	{
		expression();
		if (!match(L")"))
		{
			error(PL0Error_wantRParen);
		}
	}
	else if (isdigit(*m_p) || *m_p == '\"')
	{
		literalValue();
	}
	else
	{
		Token token = getIdentityToken();
		switch (token.detailType())
		{
		case Keyword_call:
			getIdentityToken();
		case Structure_functionName:
		case Structure_procudureName:
			callStatement();
			break;
		case Structure_constVariable:
			break;
		case Structure_variable:
			leftValueExpression(false);
			break;
		}
	}
}

// <��ֵ���ʽ> ::= <��ʶ��>['['<���ʽ>'{,<���ʽ>}']'][. <��ֵ���ʽ>]
void PL0Parser::leftValueExpression(bool needToken)
{
	Token token;
	if (needToken)
		token = getIdentityToken();
	else
		token = getLastToken();

	if (token.detailType() != Structure_variable)
	{
		error(PL0Error_wantVariable);
	}

	if (match(L"["))
	{
		expression();
		while (match(L","))
		{
			expression();
		}

		if (!match(L"]"))
		{
			error(PL0Error_wantRSquareBracket);
		}
	}

	if (match(L"."))
	{
		leftValueExpression(true);
	}
}

// <CASE�������> ::= CASE <���ʽ> OF <CASE�Ӿ�>{; <CASE�Ӿ�>} [; ELSE <���>]; END
void PL0Parser::caseStatement()
{
	// ����ú������� case �Ѿ�����ȷʶ��
	expression();

	if (!matchKeyword(Keyword_of))
	{
		error(PL0Error_wantOf);
	}

	do {
		caseSubStatement();
		if (!match(L";"))
		{
			error(PL0Error_wantSemicolon);
		}
	} while (!matchKeyword(Keyword_end) && !matchKeyword(Keyword_else));

	Token token = getLastToken();
	if (token.detailType() == Keyword_else)
	{
		statement();
		if (!match(L";"))
		{
			error(PL0Error_wantSemicolon);
		}
		if (!matchKeyword(Keyword_end))
		{
			error(PL0Error_wantEnd);
		}
	}
}

// <CASE�Ӿ�> ::= (<����>|<������>):<���>
void PL0Parser::caseSubStatement()
{
	Token token;
	if (isalpha(*m_p) || *m_p == '_')
	{
		token = getConstNameToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantConstName);
		}
	}
	else if (isdigit(*m_p))
	{
		token = getNumberToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantNumber);
		}
	}
	else
	{
		error(PL0Error_wantNumber);
	}

	if (!match(L":"))
	{
		error(PL0Error_wantColon);
	}

	statement();
}

// <�����> ::= READ'('<��ֵ���ʽ>{, <��ֵ���ʽ>}')'
void PL0Parser::readStatement()
{
	// ����ú�����ʱ��read �Ѿ��ɹ�ƥ��
	if (!match(L"("))
	{
		error(PL0Error_wantLParen);
	}

	do 
	{
		leftValueExpression(true);
	} while (match(L","));

	if (!match(L")"))
	{
		error(PL0Error_wantRParen);
	}
}

// <д���> ::= WRITE'('<���ʽ>{, <���ʽ>}')'
void PL0Parser::writeStatement()
{
	// ����ú�����ʱ��write �ؼ����Ѿ����ɹ�ƥ��
	if (!match(L"("))
	{
		error(PL0Error_wantLParen);
	}

	do
	{
		expression();
	} while (match(L","));

	if (!match(L")"))
	{
		error(PL0Error_wantRParen);
	}
}

// <����ѭ�����> ::= WHILE<�������ʽ>DO<���>
void PL0Parser::whileStatement()
{
	conditionExpression();
	
	if (!matchKeyword(Keyword_do))
	{
		error(PL0Error_wantDo);
	}

	statement();
}

// <ֱ����ѭ�����> ::= REPEAT <���> UNTIL <�������ʽ>
void PL0Parser::repeatStatement()
{
	// �������������ʱ��repeat �ؼ����Ѿ��ɹ�ʶ��
	statement();

	if (!matchKeyword(Keyword_until))
	{
		error(PL0Error_wantUntil);
	}

	conditionExpression();
}

// <FORѭ�����> ::= FOR <��ֵ���ʽ> := <���ʽ> [STEP <���ʽ>] UNTIL <���ʽ> DO <���>
void PL0Parser::forStatement()
{
	// �������������ʱ��for �ؼ����Ѿ��ɹ�ʶ��
	leftValueExpression(true);

	if (!match(L":="))
	{
		error(PL0Error_wantAssign);
	}

	expression();

	if (matchKeyword(Keyword_step))
	{
		expression();
	}

	if (!matchKeyword(Keyword_until))
	{
		error(PL0Error_wantUntil);
	}

	expression();

	if (!matchKeyword(Keyword_do))
	{
		error(PL0Error_wantDo);
	}

	statement();
}

// <�������> ::= BEGIN<���>{; <���>}END
void PL0Parser::beginStatement()
{
	// ����ú�����ʱ��begin �ؼ����Ѿ��ɹ�ƥ��
	statement();

	while (match(L";"))
	{
		if (matchKeyword(Keyword_end))
			return;

		statement();
	}

	if (!matchKeyword(Keyword_end))
	{
		error(PL0Error_wantEnd);
	}
}

// <�������> ::= RETURN <���ʽ>
void PL0Parser::returnStatement()
{
	// ����պ�����ʱ��return �ؼ����Ѿ��ɹ�ƥ��
	expression();
}

// <�������> ::= [CALL]<��ʶ��>['('[<���ʽ> {,<���ʽ>}'])']
void PL0Parser::callStatement()
{
	Token token = getLastToken();

	if (token.detailType() != Structure_functionName
		&& token.detailType() != Structure_procudureName)
	{
		if (match(L"("))
		{
			if (match(L")"))
				return;

			do
			{
				expression();
			} while (match(L","));

			if (!match(L")"))
			{
				error(PL0Error_wantRParen);
			}
		}
	}
	else
	{
		error(PL0Error_wantIdentity);
	}
}

// <��ֵ���> ::= <��ֵ���ʽ>:=<���ʽ>
void PL0Parser::assignStatement()
{
	leftValueExpression(false);

	if (!match(L":="))
	{
		error(PL0Error_wantAssign);
	}

	expression();
}

void PL0Parser::resetContents()
{
	m_tokens.clear();
	m_p = m_buffer;
	m_curPos = 0;
	m_curRow = 0;
	m_curCol = 0;
	m_savedPos = 0;
	m_savedRow = 0;
	m_savedCol = 0;
	m_symTable.clear();
}

