#pragma once

class InstructionGenerator
{
public:
	int generate(Command cmd, int arg1 = 0, int arg2 = 0, int target = 0);
	int getStackTopOffset();
};