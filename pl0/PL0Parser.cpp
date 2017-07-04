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

// <变量说明部分> ::= VAR <变量及类型>{, <变量及类型>};
void PL0Parser::variableDeclare()
{
	// 进入该函数则表明已经成功匹配 var
	variableAndType();

	while (match(L","))
		variableAndType();

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
void PL0Parser::procedureDeclare()
{
	procedureHeader();
	block();
}

// <过程说明首部> ::= PROCEDURE<标识符>['('<参数列表>')'];
void PL0Parser::procedureHeader()
{
	// 进入该函数的时候，procedure关键字已经被正确匹配
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

// <参数列表> ::= (<变量及类型> {, <变量及类型>})|<空>
void PL0Parser::paramList()
{
	// 后跟集为 { ')' }，如果直接为这个，说明推出为空
	if (*m_p == L')')
		return;

	variableAndType();
	
	while (match(L","))
	{
		variableAndType();
	}
}

// <函数说明部分> ::= <函数说明首部><分程序>
void PL0Parser::functionDeclare()
{
	functionHeader();
	block();
}

// <函数说明首部> ::= FUNCTION<标识符>['('<参数列表>')']:<类型>;
void PL0Parser::functionHeader()
{
	// 进入该函数的时候，function关键字已经成功识别
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

// <条件表达式> ::= <或条件> {OR <或条件>}
void PL0Parser::conditionExpression()
{
	orCondition();

	while (matchKeyword(Keyword_or))
	{
		orCondition();
	}
}

// <或条件> ::= <条件> {AND <条件>}
void PL0Parser::orCondition()
{
	condition();

	while (matchKeyword(Keyword_and))
	{
		condition();
	}
}

// <条件> ::= <表达式><关系运算符><表达式>|ODD<表达式>
// <关系运算符> ::= =|#|<|<=|>|>=|!=
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
