#pragma once

struct BasicTypeValue
{
	int type;
	union {
		int intVal;
		double realVal;
		char charVal;
		boolean boolVal;
		int strVal;
	};
};

struct VariableInfo
{
	int type;
	int size;
	int address;
	int belongType;
	vector<int> diamonds;
};

struct FunctionInfo
{
	int returnType;
	int address;
	vector<VariableInfo> args;
};

struct TypenameInfo
{
	int type;
	int size;
};

struct ConstVarInfo
{
	BasicTypeValue value;
	int belongType;
};

struct SymbolInfo
{
	Structure tokenType;
	int detailType;
	int id;
	union {
		VariableInfo varInfo;
		ConstVarInfo constInfo;
		FunctionInfo funInfo;
		TypenameInfo typeInfo;
	};
};

class SymbolTable
{
public:
	int getSymbolType(const wstring& symbolName)
	{
		auto it = m_symbols.find(symbolName);
		if (it == m_symbols.end())
		{
			return Structure_error;
		}

		return it->second.tokenType;
	}

	bool putSymbol(const wstring& symbolName, Structure tokenType, int detailType)
	{
		SymbolInfo info;
		info.tokenType = tokenType;
		info.detailType = detailType;
		m_symbols.insert(make_pair(symbolName, info));
		return true;
	}

	SymbolInfo& get(const wstring& name);

	SymbolInfo& putVariable(const wstring& symbolName)
	{
		SymbolInfo info;
		info.tokenType = Structure_variable;
		info.detailType = Structure_variable;
		m_symbols.insert(make_pair(symbolName, info));
		return m_symbols.find(symbolName)->second;
	}

	SymbolInfo& putFunctionName(const wstring& funcName)

	void clear()
	{
		m_symbols.clear();
	}

protected:
	unordered_map<wstring, SymbolInfo> m_symbols;
};