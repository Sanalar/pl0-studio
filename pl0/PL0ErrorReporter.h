#pragma once

enum PL0Error
{
	PL0Error_none,
	PL0Error_missingPeriod,
	PL0Error_expectedFileEnd,
	PL0Error_wantIdentity,
	PL0Error_wantEqualSymbol,
	PL0Error_unexpectedSymbol,
	PL0Error_wantSemicolon,
	PL0Error_wantRSquareBracket,
	PL0Error_wantColon,
	PL0Error_wantTypename,
	PL0Error_wantIntegerOrConst,
	PL0Error_wantRBrace,
	PL0Error_wantNumber,
	PL0Error_wantString,
};

enum PL0Warning
{
	PL0Warning_none,
};

enum PL0Info
{
	PL0Info_none,
};

class PL0ErrorReporter
{
public:
	void raiseError(PL0Error err, int row, int col, int offset, wstring* msg = nullptr);
	void raiseWarning(PL0Warning warning, int row, int col, int offset, wstring* msg = nullptr);
};