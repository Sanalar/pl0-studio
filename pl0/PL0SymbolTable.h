#pragma once

struct SymbolInfo
{
	Structure tokenType;
	int detailType;
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

	void clear()
	{
		m_symbols.clear();
	}

protected:
	unordered_map<wstring, SymbolInfo> m_symbols;
};