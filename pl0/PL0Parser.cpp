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

	// 先测试是不是关键字
	auto it = m_keywords.find(key.c_str());
	if (it != m_keywords.end())
	{
		token.setTokenType(Structure_keyword);
		token.setDetailType(it->second);
		m_tokens.push_back(token);
		return m_tokens.back();
	}

	// 再测试是不是类型名
	auto it2 = m_types.find(key.c_str());
	if (it2 != m_types.end())
	{
		token.setTokenType(Structure_typename);
		token.setDetailType(it->second);
		m_tokens.push_back(token);
		return m_tokens.back();
	}

	// 测试是不是已经保存的符号
	int tokenType = m_curBlock->getSymbolType(key);
	if (tokenType == Structure_error)
	{
		// 不在关键字表也不在类型表也不在符号表，可能是新符号
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
	// 这里有个隐藏行为，match 成功的时候，保存的游标不会被修改
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

// <程序> ::= <分程序>.
/*
	最末尾生成停机指令
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

// <分程序> ::= [<常量说明部分>][<类型说明部分>][<变量说明部分>][<模块说明部分>]<语句>
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

// <类型说明部分> ::= TYPE (<标识符>=(<重定义类型> | <枚举类型> | <结构体类型>))+;
// <重定义类型>   ::= <标识符>
void PL0Parser::typeDeclare()
{
	bool firstInside = true;

	while (true)
	{
		Token& token = getIdentityToken();
		if (token.tokenType() == Structure_error)
		{
			// 不是第一次进入时，可以直接返回
			if (!firstInside)
				return;

			// 第一次进入时，必须匹配该项
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

// <结构体类型> ::= RECORD; <变量及类型>{; <变量及类型>}; END
void PL0Parser::recordDeclare(SymbolInfo& root)
{
	// 进入该函数时，关键字 record 已经成功匹配
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

// <变量及类型> ::= <标识符>['['<区间说明>{, <区间说明>}']'] : <类型>
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

// <区间说明> ::= <整数或常量> : <整数或常量>
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

// <整数或常量>
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

// <枚举类型> ::= '{' <标识符> {, <标识符>} '}'
void PL0Parser::enumeratedDeclare(SymbolInfo& root)
{
	// 进入这个代码的时候左花括号已经匹配成功了
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

// <字面量> ::= <有符号整数>|<有符号小数>|<字符串常量>
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

// <变量说明部分> ::= VAR <变量及类型>{, <变量及类型>};
/*
	语句：
		var <name_and_type>;
	生成：
		(ALLOC, size, 0, 0)
	附加操作：
		符号表中添加地址和长度
*/
void PL0Parser::variableDeclare()
{
	// 进入该函数则表明已经成功匹配 var
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

// <模块说明部分> ::= (<过程说明部分> | <函数说明部分>){; <模块说明部分>};
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

// <过程说明部分> ::= <过程说明首部><分程序>
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

	// 退出过程，需要平衡占
	m_curBlock->generateRet(m_ins);

	// 返回上一级
	m_curBlock = m_curBlock->m_parent;
}

// <过程说明首部> ::= PROCEDURE<标识符>['('<参数列表>')'];
void PL0Parser::procedureHeader()
{
	// 进入该函数的时候，procedure关键字已经被正确匹配
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

// <参数列表> ::= (<变量及类型> {, <变量及类型>})|<空>
void PL0Parser::paramList()
{
	// 后跟集为 { ')' }，如果直接为这个，说明推出为空
	if (*m_p == L')')
		return;

	m_curBlock->addParam(variableAndType());  // 内部设置地址
	
	while (match(L","))
	{
		m_curBlock->addParam(variableAndType());
	}
}

// <函数说明部分> ::= <函数说明首部><分程序>
void PL0Parser::functionDeclare()
{
	PL0Block* bb = new PL0Block;
	bb->m_parent = m_curBlock;
	m_curBlock = bb;
	functionHeader();
	block();

	// 退出过程，需要平衡占
	m_curBlock->generateRet(m_ins);

	// 返回上一级
	m_curBlock = m_curBlock->m_parent;

}

// <函数说明首部> ::= FUNCTION<标识符>['('<参数列表>')']:<类型>;
void PL0Parser::functionHeader()
{
	// 进入该函数的时候，function关键字已经成功识别
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

// <语句> ::= <赋值语句>|<条件语句>|<循环语句>|<过程调用语句>|<读语句>|<写语句>|<复合语句>|<返回语句>|<空>
/*
	if, case           -> <条件语句>
	read               -> <读语句>
	write              -> <写语句>
	while, repeat, for -> <循环语句>
	begin              -> <复合语句>
	return             -> <返回语句>
	call               -> <过程调用语句>
	variable           -> <赋值语句>|<过程调用语句>
	后跟集合           -> <空>

	除后跟集合外，first集均为<标识符>。
	后跟集合：
	'.', ';', 'END'
*/
void PL0Parser::statement()
{
	Token token = getIdentityToken();
	// 判断后跟集合，推导空语句
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

// <IF条件语句> ::= IF<条件表达式>THEN<语句>{ELSEIF<条件表达式>THEN<语句>}[ELSE<语句>]
void PL0Parser::ifStatement()
{
	// 进入该函数的时候，if关键字已经成功匹配
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

// <条件表达式> ::= <或条件> {OR <或条件>}
// 栈顶是比较结果
void PL0Parser::conditionExpression()
{
	orCondition();

	while (matchKeyword(Keyword_or))
	{
		orCondition();
		m_ins.generate(OR);
	}
}

// <或条件> ::= <条件> {AND <条件>}
void PL0Parser::orCondition()
{
	condition();

	while (matchKeyword(Keyword_and))
	{
		condition();
		m_ins.generate(AND);
	}
}

// <条件> ::= <表达式><关系运算符><表达式>|ODD<表达式>
// <关系运算符> ::= =|#|<|<=|>|>=|!=
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

// <表达式> ::= [+|-]<项>{<加法运算符><项>}
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

// <项> ::= <因子>{<乘法运算符><因子>}
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

// <因子> ::= <左值表达式>|<字面量>|<常量>|'(' <表达式> ')'|<调用语句>
/*
	variable, const_name -> <左值表达式>
	number, string       -> <字面量>
	(                    -> <括号表达式>
	call, func, proc     -> <调用语句>
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

// <左值表达式> ::= <标识符>['['<表达式>'{,<表达式>}']'][. <左值表达式>]
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

// <CASE条件语句> ::= CASE <表达式> OF <CASE从句>{; <CASE从句>} [; ELSE <语句>]; END
void PL0Parser::caseStatement()
{
	vector<int> jmpList;
	// 进入该函数表明 case 已经被正确识别
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

// <CASE从句> ::= (<常量>|<字面量>):<语句>
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

// <读语句> ::= READ'('<左值表达式>{, <左值表达式>}')'
void PL0Parser::readStatement()
{
	// 进入该函数的时候，read 已经成功匹配
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

// <写语句> ::= WRITE'('<表达式>{, <表达式>}')'
void PL0Parser::writeStatement()
{
	// 进入该函数的时候，write 关键字已经被成功匹配
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

// <当型循环语句> ::= WHILE<条件表达式>DO<语句>
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

// <直到型循环语句> ::= REPEAT <语句> UNTIL <条件表达式>
void PL0Parser::repeatStatement()
{
	// 进入这个函数的时候，repeat 关键字已经成功识别
	int addr1 = m_ins.getNextInstructionAddress();
	statement();

	if (!matchKeyword(Keyword_until))
	{
		error(PL0Error_wantUntil);
	}

	conditionExpression();
	m_ins.generate(JZ, addr1);
}

// <FOR循环语句> ::= FOR <左值表达式> := <表达式> [STEP <表达式>] UNTIL <表达式> DO <语句>
void PL0Parser::forStatement()
{
	// 进入这个函数的时候，for 关键字已经成功识别
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

// <复合语句> ::= BEGIN<语句>{; <语句>}END
void PL0Parser::beginStatement()
{
	// 进入该函数的时候，begin 关键字已经成功匹配
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

// <返回语句> ::= RETURN <表达式>
void PL0Parser::returnStatement()
{
	// 进入刚函数的时候，return 关键字已经成功匹配
	expression();
	m_curBlock->generateReturnInstruction();
}

// <调用语句> ::= [CALL]<标识符>['('[<表达式> {,<表达式>}'])']
/*
	调用函数的过程：
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

// <赋值语句> ::= <左值表达式>:=<表达式>
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

