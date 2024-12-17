#include "utils.hpp"

#define TESTING 0
#if TESTING
#define INFILE "testInput.txt"
#else
#define INFILE "input.txt"
#endif

struct Computer
{
	Computer(std::vector<std::string> const& input)
	{
		sscanf_s(input[0].c_str(), "Register A: %lld", &A);
		sscanf_s(input[1].c_str(), "Register B: %lld", &B);
		sscanf_s(input[2].c_str(), "Register C: %lld", &C);

		std::string codeString = input[4].substr(input[4].find(':') + 2);
		std::istringstream codeStream(codeString);
		for (int i; codeStream >> i;)
		{
			code.push_back(i);
			if (codeStream.peek() == ',')
			{
				codeStream.ignore();
			}
		}
	}

	enum class Opcode
	{
		adv = 0,
		bxl = 1,
		bst = 2, 
		jnz = 3,
		bxc = 4,
		out = 5,
		bdv = 6,
		cdv = 7
	};

	Opcode IntToOpcode(int asInt)
	{
		return static_cast<Opcode>(asInt);
	}

	int ComboOperand(int operand)
	{
		switch(operand)
		{
		case 0:
		case 1:
		case 2:
		case 3: return operand;
		case 4: return A;
		case 5: return B;
		case 6: return C;
		default: __debugbreak(); __assume(0);
		}
	}

	void adv(int operand)
	{
		A /= (1ll << ComboOperand(operand));
	}

	void bxl(int operand)
	{
		B ^= operand;
	}

	void bst(int operand)
	{
		B = ComboOperand(operand) % 8;
	}

	bool jnz(int operand)
	{
		if (!A) return false;
		instructionPointer = operand;
		return true;
	}

	void bxc(int operand)
	{
		B ^= C;
	}

	void out(int operand)
	{
		output.push_back(ComboOperand(operand) % 8);
		if (output.size() > code.size() || !VectorFuzzyCompare())
		{
			instructionPointer = code.size();
		}
		if (VectorCompare())
		{
			instructionPointer = code.size();
		}
	}

	void bdv(int operand)
	{
		B = A / (1ll << ComboOperand(operand));
	}

	void cdv(int operand)
	{
		C = A / (1ll << ComboOperand(operand));
	}

	void ExecuteSingleInstruction()
	{
		int operand = code[instructionPointer + 1];
		switch (IntToOpcode(code[instructionPointer]))
		{
		case Opcode::adv: adv(operand); break;
		case Opcode::bxl: bxl(operand); break;
		case Opcode::bst: bst(operand); break;
		case Opcode::jnz: if (jnz(operand)) { return; }; break;
		case Opcode::bxc: bxc(operand); break;
		case Opcode::out: out(operand); break;
		case Opcode::bdv: bdv(operand); break;
		case Opcode::cdv: cdv(operand); break;
		default: __debugbreak(); __assume(0);
		}
		instructionPointer += 2;
	}

	bool Execute()
	{
		while (instructionPointer < code.size())
		{
			ExecuteSingleInstruction();
		}
		return VectorCompare();
	}

	bool VectorFuzzyCompare()
	{
		for (int i = 0; i < output.size(); ++i)
		{
			if (output[i] != code[i])
			{
				//if (i > 8) printf("found %d\n", i);
				return false;
			}
		}
		return true;
	}

	bool VectorCompare()
	{
		if (output.size() != code.size()) return false;
		for (int i = 0; i < output.size(); ++i)
		{
			if (output[i] != code[i]) return false;
		}
		return true;
	}

	int instructionPointer = 0;

	int64_t A;
	int64_t B;
	int64_t C;

	std::vector<int> code;
	std::vector<int> output;
};

int main()
{
	auto inputLines = GetInputAsString(INFILE);
	Computer computer(inputLines);

#pragma omp parallel for
	for (int64_t i = INT32_MAX; i < INT64_MAX; ++i)
	{
		Computer cpy = computer;
		cpy.A = i;
		if (!cpy.Execute()) continue;

		printf("%lld\n", i);
	}



}