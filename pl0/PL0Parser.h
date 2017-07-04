#pragma once

#include "PL0ErrorReporter.h"

class PL0Parser
{
public:
	PL0Parser();
	PL0Parser& setBuffer(const wchar_t* buffer);

protected:
	// match 方法可以跳过注释
	bool match(const wchar_t* symbol);
	bool matchKeyword(Keyword key);
	Token& getVariableToken();
	Token& getTypenameToken();
	Token& getIntegerToken();
	Token& getConstNameToken();
	Token& getNumberToken();
	Token& getStringToken();
	Token& getLastToken();
	Token& getIdentityToken();
	void moveCursor(int step = 1);

	void program();
	void block();
	void typeDeclare();
	void recordDeclare();
	void variableAndType();
	void rangeDeclare();
	void integerOrConst();
	void enumeratedDeclare();
	void constDeclare();
	void constVarDefine();
	void literalValue();
	void variableDeclare();
	void moduleDeclare();
	void procedureDeclare();
	void procedureHeader();
	void paramList();
	void functionDeclare();
	void functionHeader();
	void statement();
	void ifStatement();
	void conditionExpression();
	void orCondition();
	void condition();

private:
	unordered_map<const wchar_t*, Keyword> m_keywords;
	vector<Token> m_tokens;
	PL0ErrorReporter m_reporter;
	const wchar_t* m_buffer;
	const wchar_t* m_p;
	int m_curPos;
	int m_curRow;
	int m_curCol;
};