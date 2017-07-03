#pragma once

class PL0Parser
{
public:
	PL0Parser();
private:
	unordered_map<const wchar_t*, Keyword> m_keywords;
};