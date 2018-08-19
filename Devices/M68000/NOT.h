#include "M68000Instruction.h"
#ifndef __M68000_NOT_H__
#define __M68000_NOT_H__
namespace M68000 {

class NOT :public M68000Instruction
{
public:
	virtual NOT* Clone() const {return new NOT();}
	virtual NOT* ClonePlacement(void* buffer) const {return new(buffer) NOT();}
	virtual size_t GetOpcodeClassByteSize() const {return sizeof(*this);}

	virtual bool RegisterOpcode(OpcodeTable<M68000Instruction>& table) const
	{
		return table.AllocateRegionToOpcode(this, L"01000110CCDDDDDD", L"CC=00-10 DDDDDD=000000-000111,010000-110111,111000,111001");
	}

	virtual std::wstring GetOpcodeName() const
	{
		return L"NOT";
	}

	virtual Disassembly M68000Disassemble(const M68000::LabelSubstitutionSettings& labelSettings) const
	{
		return Disassembly(GetOpcodeName() + L"." + DisassembleSize(_size), _target.Disassemble(labelSettings));
	}

	virtual void M68000Decode(const M68000* cpu, const M68000Long& location, const M68000Word& data, bool transparent)
	{
//	-----------------------------------------------------------------
//	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
//	|---|---|---|---|-----------|---|-------|-----------|-----------|
//	| 1 | 1 | 0 | 0 |  REGISTER | D |  SIZE |    MODE   |  REGISTER |
//	----------------------------=====================================
//	                            |--<opmode>-|----------<ea>---------|
		switch (data.GetDataSegment(6, 2))
		{
		case 0:	//00
			_size = BITCOUNT_BYTE;
			break;
		case 1:	//01
			_size = BITCOUNT_WORD;
			break;
		case 2:	//10
			_size = BITCOUNT_LONG;
			break;
		}

		//NOT	<ea>
		_target.Decode(data.GetDataSegment(0, 3), data.GetDataSegment(3, 3), _size, location + GetInstructionSize(), cpu, transparent, GetInstructionRegister());
		AddInstructionSize(_target.ExtensionSize());

		if ((_target.GetAddressMode() == EffectiveAddress::Mode::DataRegDirect) || (_target.GetAddressMode() == EffectiveAddress::Mode::AddRegDirect))
		{
			if (_size != BITCOUNT_LONG)
			{
				AddExecuteCycleCount(ExecuteTime(4, 1, 0));
			}
			else
			{
				AddExecuteCycleCount(ExecuteTime(6, 1, 0));
			}
		}
		else
		{
			if (_size != BITCOUNT_LONG)
			{
				AddExecuteCycleCount(ExecuteTime(8, 1, 1));
			}
			else
			{
				AddExecuteCycleCount(ExecuteTime(12, 1, 2));
			}
			AddExecuteCycleCount(_target.DecodeTime());
		}
	}

	virtual ExecuteTime M68000Execute(M68000* cpu, const M68000Long& location) const
	{
		double additionalTime = 0;
		Data op1(_size);
		Data result(_size);

		//Perform the operation
		additionalTime += _target.ReadWithoutAdjustingAddress(cpu, op1, GetInstructionRegister());
		result = ~op1;
		additionalTime += _target.Write(cpu, result, GetInstructionRegister());

		//Set the flag results
		cpu->SetN(result.Negative());
		cpu->SetZ(result.Zero());
		cpu->SetV(false);
		cpu->SetC(false);

		//Adjust the PC and return the execution time
		cpu->SetPC(location + GetInstructionSize());
		return GetExecuteCycleCount(additionalTime);
	}

	virtual void GetLabelTargetLocations(std::set<unsigned int>& labelTargetLocations) const
	{
		_target.AddLabelTargetsToSet(labelTargetLocations);
	}

private:
	EffectiveAddress _target;
	Bitcount _size;
};

} //Close namespace M68000
#endif
