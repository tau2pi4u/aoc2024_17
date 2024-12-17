#include <map>
#include <set>
#include <unordered_map>
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
		SCANF(input[0].c_str(), "Register A: %lld", &A);
		SCANF(input[1].c_str(), "Register B: %lld", &B);
		SCANF(input[2].c_str(), "Register C: %lld", &C);

		std::string codeString = input[4].substr(input[4].find(':') + 2);
		std::istringstream codeStream(codeString);
		for (int64_t i; codeStream >> i;)
		{
			code.push_back(i);
			if (codeStream.peek() == ',')
			{
				codeStream.ignore();
			}
		}
	}

	void Reset()
	{
		instructionPointer = 0;
		output.clear();
		A = 0;
		B = 0;
		C = 0;
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

	Opcode IntToOpcode(int64_t asInt)
	{
		return static_cast<Opcode>(asInt);
	}

	int64_t ComboOperand(int64_t operand)
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
		default: Unreachable();
		}
	}

	void adv(int64_t operand)
	{
		A >>= ComboOperand(operand);
	}

	void bxl(int64_t operand)
	{
		B ^= operand;
	}

	void bst(int64_t operand)
	{
		B = ComboOperand(operand) % 8;
	}

	bool jnz(int64_t operand)
	{
		if (!A) return false;
		instructionPointer = operand;
		return true;
	}

	void bxc(int64_t operand)
	{
		B ^= C;
	}

	void out(int64_t operand)
	{
		output.push_back(ComboOperand(operand) % 8);
	}

	void bdv(int64_t operand)
	{
		B = A >> ComboOperand(operand);
	}

	void cdv(int64_t operand)
	{
		C = A >> ComboOperand(operand);
	}

	void ExecuteSingleInstruction()
	{
		int64_t operand = code[instructionPointer + 1];
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
		default: Unreachable();
		}
		instructionPointer += 2;
	}

	bool Execute(bool haltOnOutput = false)
	{
		while (instructionPointer < code.size() && output.empty())
		{
			ExecuteSingleInstruction();
		}
		return VectorCompare();
	}

	bool VectorFuzzyCompare()
	{
		for (size_t i = 0; i < output.size(); ++i)
		{
			if (output[i] != code[i])
			{
				return false;
			}
		}
		return true;
	}

	bool VectorCompare()
	{
		if (output.size() != code.size()) return false;
		for (size_t i = 0; i < output.size(); ++i)
		{
			if (output[i] != code[i]) return false;
		}
		return true;
	}

	void PrintOutput()
	{
		if (output.empty()) return;
		printf("%lld", output[0]);
		for (int i = 1; i < output.size(); ++i)
		{
			printf(",%lld", output[i]);
		}
	}

	size_t instructionPointer = 0;

	int64_t A;
	int64_t B;
	int64_t C;

	std::vector<int64_t> code;
	std::vector<int64_t> output;
};

void BuildValidOutput(Computer & computer, std::unordered_map<int64_t, std::set<int64_t>> const& valueToAs, int64_t candidateA, int64_t position, int64_t & solution)
{
	static std::set<std::pair<int64_t, int64_t>> seen;
	if (seen.count({ candidateA, position })) return;
	seen.insert({ candidateA, position });

	if (candidateA > solution) return;

	if (position < 0)
	{
		solution = candidateA;
		return;
	}

	auto const& candidateNextBits = valueToAs.at(computer.code.at(position));

	for (auto& nextBits : candidateNextBits)
	{
		int64_t bitsInPosition = nextBits << (position * 3);
		int64_t badChangeMask = ~(7ll << (position * 3));
		int64_t mightChange = (bitsInPosition & badChangeMask);
		
		// we are setting bits and changing our previous result
		if (~candidateA & mightChange) continue;

		int64_t newA = candidateA | bitsInPosition;
		int64_t bottom10 = (newA >> (position * 3)) & ((1ll << 10) - 1);

		if (newA > solution) return;

		// bits of the new candidate are being changed
		if (bottom10 != nextBits) continue;

		BuildValidOutput(computer, valueToAs, newA, position - 1, solution);
	}
}

int64_t BuildValidOutput(Computer& computer)
{
	std::unordered_map<int64_t, std::set<int64_t>> candidatesByTargetOutput;

	for (int64_t i = 0; i < (1 << 10); ++i)
	{
		computer.Reset();
		computer.A = i;
		computer.Execute(true);
		if (computer.output.empty()) continue;
		candidatesByTargetOutput[computer.output.front()].insert(i);
	}

	int64_t solution = INT64_MAX;
	BuildValidOutput(computer, candidatesByTargetOutput, 0, computer.code.size() - 1, solution);

	return solution;
}

int main()
{
	auto inputLines = GetInputAsString(INFILE);
	Computer computer(inputLines);
	computer.Execute();
	printf("p1: ");
	computer.PrintOutput();
	printf("\n");

	int64_t p2 = BuildValidOutput(computer);
	printf("p2: %lld\n", p2);
}