#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>
#include "../../libs/Zydis/zydis.h"

using namespace std;
namespace fs = filesystem;

class Hider {
public:
	enum class RecordType {
		Public,
		Global,
		Module
	};

	class String {
		string Name;
		uint32_t Offset;
		uint32_t RVA;
		uint32_t Size;
		string Value;
		RecordType RType;
	};

	class Function {
	public:
		string Name;
		uint32_t Offset;
		uint32_t RVA;
		uint32_t Size;
		RecordType RType;
	protected:
		vector<ZydisDisassembledInstruction> Instructions;
	};

	enum class Status {
		Success,
	};

	Status LoadFile();
};