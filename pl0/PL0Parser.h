#pragma once

#include "PL0ErrorReporter.h"
#include "PL0SymbolTable.h"
#include "PL0InstructionGernerator.h"
#include "PL0Block.h"

class PL0Parser
{
public:
	PL0Parser();
	PL0Parser& setBuffer(const wchar_t* buffer) { m_buffer = buffer; return *this; }
	void parse();

	inline const vector<Token>& tokens() const { return m_tokens; }

protected:
	// match 方法可以跳过注释
	bool match(const wchar_t* symbol);
	bool matchKeyword(Keyword key);
	wstring getIdentity();
	Token& getVariableToken();
	Token& getTypenameToken();
	Token& getConstNameToken();
	Token& getNumberToken();
	Token& getStringToken();
	Token& getLastToken();
	Token& getIdentityToken();
	inline void moveCursor(int step = 1);
	inline void saveCursor();
	inline void restoreCursor();
	inline void skipWhiteSpaceAndComment();

	bool comment();
	void program();
	void block();
	void typeDeclare();
	void recordDeclare(SymbolInfo& root);
	SymbolInfo& variableAndType();
	int rangeDeclare();
	int integerOrConst();
	void enumeratedDeclare(SymbolInfo& root);
	void constDeclare();
	void constVarDefine();
	BasicTypeValue literalValue();
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
	void expression();
	void factor();
	void atom();
	void leftValueExpression(bool needToken);
	void caseStatement();
	int caseSubStatement();
	void readStatement();
	void writeStatement();
	void whileStatement();
	void repeatStatement();
	void forStatement();
	void beginStatement();
	void returnStatement();
	void callStatement();
	void assignStatement();

	void resetContents();

private:
	unordered_map<wstring, Keyword> m_keywords;
	unordered_map<wstring, Types> m_types;
	vector<Token> m_tokens;
	InstructionGenerator m_ins;
	PL0ErrorReporter m_reporter;
	const wchar_t* m_buffer;
	const wchar_t* m_p;
	int m_curPos;
	int m_curRow;
	int m_curCol;
	int m_savedPos;
	int m_savedRow;
	int m_savedCol;
	PL0Block* m_rootBlock;
	PL0Block* m_curBlock;
};