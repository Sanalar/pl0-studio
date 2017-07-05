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

	m_types.insert(make_pair(L"integer", Types_integer));
	m_types.insert(make_pair(L"real", Types_real));
	m_types.insert(make_pair(L"char", Types_char));
	m_types.insert(make_pair(L"boolean", Types_boolean));
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
	auto it2 = m_types.find(key.c_str());
	if (it2 != m_types.end())
	{
		token.setTokenType(Structure_typename);
		token.setDetailType(it->second);
		m_tokens.push_back(token);
		return m_tokens.back();
	}

	// �����ǲ����Ѿ�����ķ���
	int tokenType = m_curBlock->getSymbolType(key);
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
/*
	��ĩβ����ͣ��ָ��
*/
void PL0Parser::program()
{
	m_rootBlock = new PL0Block;
	m_curBlock = m_rootBlock;
	m_ins.generate(PUSH, 0);
	block();

	if (!match(L"."))
	{
		error(PL0Error_missingPeriod);
		moveCursor();
	}

	m_ins.generate(EXIT);

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

		SymbolInfo& info = m_curBlock->putTypenameSymbol(token.name());
		token.setTokenType(Structure_typename);

		if (!match(L"="))
		{
			error(PL0Error_wantEqualSymbol);
		}

		if (matchKeyword(Keyword_record))
		{
			info.detailType = Types_record;
			recordDeclare(info);
		}
		else if (match(L"{"))
		{
			info.detailType = Types_enumerated;
			enumeratedDeclare(info);
		}
		else
		{
			token = getTypenameToken();
			if (token.tokenType() == Structure_error)
			{
				error(PL0Error_unexpectedSymbol);
			}
			info.detailType = Types_alias;
			m_curBlock->setRootType(info, token.name());
		}
	}
}

// <�ṹ������> ::= RECORD; <����������>{; <����������>}; END
void PL0Parser::recordDeclare(SymbolInfo& root)
{
	// ����ú���ʱ���ؼ��� record �Ѿ��ɹ�ƥ��
	if (!match(L";"))
	{
		error(PL0Error_wantSemicolon);
	}

	int addr = 0;
	do 
	{
		SymbolInfo& info = variableAndType();
		info.varInfo.belongType = root.id;
		info.varInfo.address = addr;
		addr += info.varInfo.size;
		if (!match(L";"))
		{
			error(PL0Error_wantSemicolon);
		}
	} while (!matchKeyword(Keyword_end));

	root.typeInfo.size = addr;
}

// <����������> ::= <��ʶ��>['['<����˵��>{, <����˵��>}']'] : <����>
/*
	integer : 4
	boolean : 1
	char    : 1
	real    : 8
*/
SymbolInfo& PL0Parser::variableAndType()
{
	Token& token = getIdentityToken();
	int size = 1;
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	SymbolInfo& res = m_curBlock->putVariableSymbol(token.name());
	token.setTokenType(Structure_variable);

	if (match(L"["))
	{
		size = rangeDeclare();
		while (match(L","))
		{
			size *= rangeDeclare();
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
	else if (token.tokenType() == Structure_typename)
	{
		size *= m_curBlock->getTypeSize(token.name());
		res.varInfo.size = size;
		res.varInfo.type = token.detailType();
	}
}

// <����˵��> ::= <��������> : <��������>
int PL0Parser::rangeDeclare()
{
	int lb = integerOrConst();
	
	if (!match(L":"))
	{
		error(PL0Error_wantColon);
	}

	int ub = integerOrConst();

	if (ub > lb)
		return ub - lb + 1;

	return 0;
}

// <��������>
int PL0Parser::integerOrConst()
{
	Token token = getNumberToken();
	if (token.tokenType() == Structure_error)
		token = getConstNameToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIntegerOrConst);
	}

	if (token.tokenType() == Structure_integer)
	{
		return _wtoi(token.name().c_str());
	}
	else if (token.tokenType() == Structure_constVariable)
	{
		SymbolInfo& info = m_curBlock->getSymbol(token.name());
		if (info.constInfo.type == Types_integer)
		{
			return info.constInfo.value.intVal;
		}
	}

	return 0;
}

// <ö������> ::= '{' <��ʶ��> {, <��ʶ��>} '}'
void PL0Parser::enumeratedDeclare(SymbolInfo& root)
{
	// ������������ʱ���������Ѿ�ƥ��ɹ���
	Token& token = getIdentityToken();
	if (token.tokenType() == Structure_error)
	{
		error(PL0Error_wantIdentity);
	}

	int value = 0;
	SymbolInfo& info = m_curBlock->putConstVariable(token.name());
	token.setTokenType(Structure_constVariable);
	info.constInfo.value.intVal = value;
	info.constInfo.type = Types_integer;
	info.constInfo.belongType = root.id;

	while (match(L","))
	{
		Token& token2 = getIdentityToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantIdentity);
		}
		++value;
		SymbolInfo& info2 = m_curBlock->putConstVariable(token.name());
		token.setTokenType(Structure_constVariable);
		info2.constInfo.value.intVal = value;
		info2.constInfo.type = Types_integer;
		info2.constInfo.belongType = root.id;
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

	SymbolInfo& info = m_curBlock->putConstVaiableSymbol(token.name());
	token.setTokenType(Structure_constVariable);
	info.constInfo.belongType = -1;

	BasicTypeValue value = literalValue();
	info.constInfo.value = value;
}

// <������> ::= <�з�������>|<�з���С��>|<�ַ�������>
BasicTypeValue PL0Parser::literalValue()
{
	BasicTypeValue value;
	value.type = -1;

	if (isalnum(*m_p))
	{
		Token token = getNumberToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantNumber);
		}
		else
		{
			value.type = token.detailType();
			if (value.type == Types_integer)
				value.intVal = _wtoi(token.name().c_str());
			else
				value.realVal = _wtof(token.name().c_str());
		}
	}
	else if (*m_p == '\"')
	{
		Token token = getStringToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantString);
		}
		else
		{
			value.type = Types_string;
			value.strVal = putStringToConstArea(token.name());
		}
	}
	else
	{
		error(PL0Error_wantIdentity);
	}

	return value;
}

// <����˵������> ::= VAR <����������>{, <����������>};
/*
	��䣺
		var <name_and_type>;
	���ɣ�
		(ALLOC, size, 0, 0)
	���Ӳ�����
		���ű�����ӵ�ַ�ͳ���
*/
void PL0Parser::variableDeclare()
{
	// ����ú���������Ѿ��ɹ�ƥ�� var
	int offset = m_ins.getStackTopOffset();
	SymbolInfo& info = variableAndType();
	info.varInfo.address = m_ins.generate(ALLOC, info.varInfo.size);

	while (match(L","))
	{
		SymbolInfo& info2 = variableAndType();
		SymbolInfo& info2 = variableAndType();
		info2.varInfo.address = m_ins.generate(ALLOC, info2.varInfo.size);
	}

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
/*
	[return value]
	[former ebp]      <-- ebp
	[return address]
	[arg1]
	[arg2]
	...
	[arg n]
	<block>

	pop arg n
	...
	pop arg2
	pop arg1
	ret
*/
void PL0Parser::procedureDeclare()
{
	PL0Block* bb = new PL0Block;
	bb->m_parent = m_curBlock;
	m_curBlock = bb;
	procedureHeader();
	block();

	// �˳����̣���Ҫƽ��ռ
	m_curBlock->generateRet(m_ins);

	// ������һ��
	m_curBlock = m_curBlock->m_parent;
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

	m_curBlock->putProcedureNameSymbol(token.name());
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

	m_curBlock->m_returnType = Types_void;
}

// <�����б�> ::= (<����������> {, <����������>})|<��>
void PL0Parser::paramList()
{
	// �����Ϊ { ')' }�����ֱ��Ϊ�����˵���Ƴ�Ϊ��
	if (*m_p == L')')
		return;

	m_curBlock->addParam(variableAndType());  // �ڲ����õ�ַ
	
	while (match(L","))
	{
		m_curBlock->addParam(variableAndType());
	}
}

// <����˵������> ::= <����˵���ײ�><�ֳ���>
void PL0Parser::functionDeclare()
{
	PL0Block* bb = new PL0Block;
	bb->m_parent = m_curBlock;
	m_curBlock = bb;
	functionHeader();
	block();

	// �˳����̣���Ҫƽ��ռ
	m_curBlock->generateRet(m_ins);

	// ������һ��
	m_curBlock = m_curBlock->m_parent;

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

	m_curBlock->putFunctionNameSymbol(token.name());
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

	m_curBlock->m_returnType = token->detailType();

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

	vector<int> jmpList;
	int addr = m_ins.generate(JZ, 0);
	statement();
	jmpList.push_back(m_ins.generate(JMP, 0));
	m_ins.get(addr).arg1 = m_ins.getNextInstructionAddress();

	while (matchKeyword(Keyword_elseif))
	{
		conditionExpression();

		if (!matchKeyword(Keyword_then))
		{
			error(PL0Error_wantThen);
		}

		addr = m_ins.generate(JZ, 0);
		statement();
		jmpList.push_back(m_ins.generate(JMP, 0));
		m_ins.get(addr).arg1 = m_ins.getNextInstructionAddress();
	}

	if (matchKeyword(Keyword_else))
	{
		statement();
	}

	for (int jmp : jmpList)
	{
		m_ins.get(addr).arg1 = m_ins.getNextInstructionAddress();
	}
}

// <�������ʽ> ::= <������> {OR <������>}
// ջ���ǱȽϽ��
void PL0Parser::conditionExpression()
{
	orCondition();

	while (matchKeyword(Keyword_or))
	{
		orCondition();
		m_ins.generate(OR);
	}
}

// <������> ::= <����> {AND <����>}
void PL0Parser::orCondition()
{
	condition();

	while (matchKeyword(Keyword_and))
	{
		condition();
		m_ins.generate(AND);
	}
}

// <����> ::= <���ʽ><��ϵ�����><���ʽ>|ODD<���ʽ>
// <��ϵ�����> ::= =|#|<|<=|>|>=|!=
void PL0Parser::condition()
{
	if (matchKeyword(Keyword_odd) || match(L"!"))
	{
		expression();
		m_ins.generate(NOT);
	}
	else
	{
		expression();

		Command cmd;
		if (match(L">="))
		{
			cmd = GE;
		}
		else if (match(L"<="))
		{
			cmd = LE;
		}
		else if (match(L"!=") || match(L"#"))
		{
			cmd = NE;
		}
		else if (match(L"==") || match(L"="))
		{
			cmd = EQ;
		}
		else if (match(L"<"))
		{
			cmd = LT;
		}
		else if (match(L">"))
		{
			cmd = GT;
		}
		else
		{
			error(PL0Error_wantConditionOperator);
			cmd = NOOP;
		}

		expression();
		m_ins.generate(cmd);
	}
}

// <���ʽ> ::= [+|-]<��>{<�ӷ������><��>}
void PL0Parser::expression()
{
	if (match(L"-"))
	{
		m_ins.generate(NEG);
	}
	else if (match(L"+"))
	{
	}

	factor();

	while (true)
	{
		bool add;
		if (match(L"+"))
			add = true;
		else if (match(L"-"))
			add = false;
		else
			break;

		factor();
		m_ins.generate(add ? ADD : SUB);
	}
}

// <��> ::= <����>{<�˷������><����>}
void PL0Parser::factor()
{
	atom();

	while (true)
	{
		int op;
		if (match(L"+"))
			op = 0;
		else if (match(L"-"))
			op = 1;
		else if (match(L"%"))
			op = 2;
		else
			break;

		atom();
		switch (op)
		{
		case 0:
			m_ins.generate(MUL);
			break;
		case 1:
			m_ins.generate(DIV);
			break;
		case 2:
			m_ins.generate(MOD);
			break;
		}
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
		BasicTypeValue value = literalValue();
		m_ins.generate(PUSH, value.type == Types_integer ? value.intVal : value.realVal);
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
			m_ins.generate(LOD);
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

	SymbolInfo& info = m_curBlock->get(token.name());
	int offset = 0;

	if (match(L"["))
	{
		m_ins.generate(PUSH, info.varInfo.address);
		int d = 0;
		expression();
		m_ins.generate(PUSH, info.varInfo.diamonds[d++]);
		m_ins.generate(MUL);
		while (match(L","))
		{
			expression();
			m_ins.generate(PUSH, info.varInfo.diamonds[d++]);
			m_ins.generate(MUL);
			m_ins.generate(ADD);
		}

		m_ins.generate(ADD);

		if (!match(L"]"))
		{
			error(PL0Error_wantRSquareBracket);
		}
	}

	if (match(L"."))
	{
		leftValueExpression(true);
		m_ins.generate(ADD);
	}
}

// <CASE�������> ::= CASE <���ʽ> OF <CASE�Ӿ�>{; <CASE�Ӿ�>} [; ELSE <���>]; END
void PL0Parser::caseStatement()
{
	vector<int> jmpList;
	// ����ú������� case �Ѿ�����ȷʶ��
	expression();

	if (!matchKeyword(Keyword_of))
	{
		error(PL0Error_wantOf);
	}

	do {
		jmpList.push_back(caseSubStatement());
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

	for (int jmp : jmpList)
	{
		m_ins.get(jmp).arg1 = m_ins.getNextInstructionAddress();
	}
}

// <CASE�Ӿ�> ::= (<����>|<������>):<���>
int PL0Parser::caseSubStatement()
{
	Token token;
	int addr = -1;
	if (isalpha(*m_p) || *m_p == '_')
	{
		token = getConstNameToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantConstName);
		}
		SymbolInfo& info = m_curBlock->getSymbol(token.name());
		m_ins.generate(CMP, info.constInfo.value.intVal);
		addr = m_ins.generate(JNZ, 0);
	}
	else if (isdigit(*m_p))
	{
		token = getNumberToken();
		if (token.tokenType() == Structure_error)
		{
			error(PL0Error_wantNumber);
		}
		m_ins.generate(CMP, _wtoi(token.name().c_str()));
		addr = m_ins.generate(JNZ, 0);
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

	int ret = m_ins.generate(JMP, 0);

	if (addr != -1)
	{
		m_ins.get(addr).arg1 = m_ins.getNextInstructionAddress();
	}

	return ret;
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
		m_ins.generate(READ);
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
		m_ins.generate(WRITE);
	} while (match(L","));

	if (!match(L")"))
	{
		error(PL0Error_wantRParen);
	}
}

// <����ѭ�����> ::= WHILE<�������ʽ>DO<���>
void PL0Parser::whileStatement()
{
	int addr1 = m_ins.getNextInstructionAddress();
	conditionExpression();
	int addr2 = m_ins.generate(JZ, 0);
	
	if (!matchKeyword(Keyword_do))
	{
		error(PL0Error_wantDo);
	}

	statement();
	m_ins.generate(JMP, addr1);
	m_ins.get(addr2).arg1 = m_ins.getNextInstructionAddress();
}

// <ֱ����ѭ�����> ::= REPEAT <���> UNTIL <�������ʽ>
void PL0Parser::repeatStatement()
{
	// �������������ʱ��repeat �ؼ����Ѿ��ɹ�ʶ��
	int addr1 = m_ins.getNextInstructionAddress();
	statement();

	if (!matchKeyword(Keyword_until))
	{
		error(PL0Error_wantUntil);
	}

	conditionExpression();
	m_ins.generate(JZ, addr1);
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
	m_ins.generate(STO);

	if (matchKeyword(Keyword_step))
	{
		expression();
	}
	else
	{
		m_ins.generate(PUSH, 1);
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

	int addr = m_ins.getNextInstructionAddress();
	statement();
	m_ins.generate(SADD, 1, 2);
	m_ins.generate(PUSH_VAR, 2);
	m_ins.generate(JLE, addr);
	m_ins.generate(POP);
	m_ins.generate(POP);
	m_ins.generate(POP);
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
	m_curBlock->generateReturnInstruction();
}

// <�������> ::= [CALL]<��ʶ��>['('[<���ʽ> {,<���ʽ>}'])']
/*
	���ú����Ĺ��̣�
	push <return value>
	push ebp
	mov ebp, esp
	push <return address>
	push arg1
	push arg2
	...
	push argn
	jmp <function address>
*/
void PL0Parser::callStatement()
{
	Token token = getLastToken();
	int addr;

	if (token.detailType() != Structure_functionName
		&& token.detailType() != Structure_procudureName)
	{
		if (token.detailType() == Structure_functionName)
		{
			m_ins.generate(PUSH, 0);
		}
		m_ins.generate(PUSH_EBP);
		m_ins.generate(RESET_EBP);

		addr = m_ins.generate(PUSH, 0);

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

		SymbolInfo& info = m_curBlock->getSymbol(token.name());
		m_ins.generate(JMP, info.funInfo.address);
		m_ins.get(addr).arg1 = m_ins.getNextInstructionAddress();
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

	m_ins.generate(STO);
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
}

