#pragma once
#include <Windows.h>
#include <string>
#include <filesystem>
#include <vector>
#include <map>
#include "../../libs/Zydis/zydis.h"
#include "../libs/RawPDB/PDB.h"
#include "../libs/RawPDB/PDB_RawFile.h"
#include "../libs/RawPDB/PDB_InfoStream.h"
#include "../libs/RawPDB/PDB_DBIStream.h"
#include "../libs/RawPDB/PDB_TPIStream.h"
#include "../libs/RawPDB/PDB_NamesStream.h"
#include "../libs/RawPDB/PDB_ModuleInfoStream.h"
#include "../libs/random/random.hpp"
#include "../libs/XEDParse/src/XEDParse.h"



using namespace std;
namespace fs = filesystem;
using Random = effolkronium::random_static;

// TODO: mov/lea obfuscation
// TODO: call/jmp obfuscation
// TODO: change control flow

class Hider;

enum class RecordType {
	Public,
	Global,
	Module
};

enum class SectionCharacteristics {
	Executable = 0x60000020,
	Data = 0xC0000040,
};

static string recordType(RecordType type) {
	switch (type) {
	case RecordType::Public:
		return "Public";
	case RecordType::Global:
		return "Global";
	case RecordType::Module:
		return "Module";
	default:
		return "Default";
	}
}

class DataRecord {
public:
	string Name;
	char* Ptr;
	uint32_t Offset;
	uint32_t RVA;
	uint32_t Size;
};

class String : public DataRecord {
public:
	string Value;
	RecordType RecordType;
	vector<ZydisDisassembledInstruction*> Usage;

	bool operator==(const String& rhs) {
		return this->RVA == rhs.RVA;
	}
};

class Function {
public:
	string Name;
	char* Ptr;
	uint32_t Offset;
	uint32_t RVA;
	uint32_t Size;
	RecordType RecordType;
	vector<ZydisDisassembledInstruction> Instructions;
public:
	bool operator==(const Function& rhs) {
		return this->RVA == rhs.RVA;
	}

	void Disassemble(Hider* hider);
	bool AddInstruction(string asmCode, bool removeInstSize = false);
};

class Data {
public:
	string Name;
	char* Ptr;
	uint32_t RVA;
	uint32_t Size;

	void SetData(string s, uint32_t offset = 0) {
		memset(Ptr + offset, 0, Size - offset);
		s.copy(Ptr + offset, min(s.size(), this->Size));
	}
};

class DirectPE {
public:
	PIMAGE_DOS_HEADER DosHeader;
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_FILE_HEADER FileHeader;
	PIMAGE_OPTIONAL_HEADER OptionalHeader;

	DirectPE() = default;

	DirectPE(char* ptr) {
		DosHeader = (PIMAGE_DOS_HEADER)ptr;
		NtHeader = (PIMAGE_NT_HEADERS)(ptr + DosHeader->e_lfanew);
		FileHeader = (PIMAGE_FILE_HEADER)(ptr + DosHeader->e_lfanew + sizeof(NtHeader->Signature));
		OptionalHeader = (PIMAGE_OPTIONAL_HEADER)(ptr + DosHeader->e_lfanew + sizeof(NtHeader->Signature) + sizeof(NtHeader->FileHeader));
	}
};

class ObfuscationOptions {
public:
	bool Arithmetic;
	bool Strings;
	bool Functions;

	std::vector<Function*> selectedFunctions;
	std::vector<String*> selectedStrings;
};

class PDBInfo {
public:
	PDBInfo(char* pdbFile) : RawFile(pdbFile) {
		this->RawDBIStream = PDB::CreateDBIStream(this->RawFile);
		ModuleInfoStream = this->RawDBIStream.CreateModuleInfoStream(this->RawFile);
		SymbolRecordStream = this->RawDBIStream.CreateSymbolRecordStream(this->RawFile);
		PublicSymbolStream = this->RawDBIStream.CreatePublicSymbolStream(this->RawFile);
		ImageSectionStream = this->RawDBIStream.CreateImageSectionStream(this->RawFile);
		GlobalSymbolStream = this->RawDBIStream.CreateGlobalSymbolStream(this->RawFile);

		this->RawTPIStream = PDB::CreateTPIStream(this->RawFile);
	};

	PDB::RawFile RawFile;
	PDB::DBIStream RawDBIStream;
	PDB::TPIStream RawTPIStream;

	PDB::GlobalSymbolStream GlobalSymbolStream;
	PDB::ModuleInfoStream ModuleInfoStream;
	PDB::CoalescedMSFStream SymbolRecordStream;
	PDB::PublicSymbolStream PublicSymbolStream;
	PDB::ImageSectionStream ImageSectionStream;
};

class Hider {
public:

protected:
	wstring InputEXEFileName;
	wstring InputPDBFileName;
public:
	ObfuscationOptions OO;

	wstring OutputEXEFileName;
	wstring SavedStringsFileName;
	vector<string> SavedStrings;

	wstring SavedFuncsFileName;
	vector<string> SavedFuncs;

	char* InputFile;
	size_t InputFileSize;
	char* ModifiedFile;
	size_t ModifiedFileSize;
	char* PDBFile;
	size_t PDBFileSize;

	DirectPE PE;
	PDBInfo* PDB;

	std::vector<PDB::IMAGE_SECTION_HEADER> Sections;
	std::vector<Function> Functions;
	std::vector<String> Strings;
	std::vector<DataRecord> Datas;

	std::map<string, Data*> CustomData;
	std::map<string, Function*> CustomFunctions;

	enum class Status {
		Success,
		FileNotFound,
		InvalidEXEFile,
		InvalidPDBFile,
	};
private:
	Function GetFunctionByRecord(const PDB::CodeView::DBI::Record* record, const PDB::ImageSectionStream& imageSectionStream, RecordType funcType);
	Function* GetFunctionByRVA(ULONG RVA);
	String GetStringByRecord(const PDB::CodeView::DBI::Record* record, char* o_file, const PDB::ImageSectionStream& imageSectionStream, RecordType funcType);
	String* GetStringByRVA(ULONG RVA);
	DataRecord GetDataByRecord(const PDB::CodeView::DBI::Record* record, char* o_file, const PDB::ImageSectionStream& imageSectionStream, RecordType funcType);
	DataRecord* GetDataByRVA(ULONG RVA);
public:

	PDB::IMAGE_SECTION_HEADER* GetSectionByName(std::string name);
	PDB::IMAGE_SECTION_HEADER* GetSectionByRVA(uint32_t rva, std::vector<PDB::IMAGE_SECTION_HEADER>* sections = nullptr);
	PDB::IMAGE_SECTION_HEADER* GetSectionByOffset(uint32_t offset, std::vector<PDB::IMAGE_SECTION_HEADER>* sections = nullptr);
	uint32_t RVA2Offset(uint32_t rva, std::vector<PDB::IMAGE_SECTION_HEADER>* sections = nullptr);
	uintptr_t Offset2RVA(uint32_t offset, std::vector<PDB::IMAGE_SECTION_HEADER>* sections = nullptr);

	Status LoadFile(std::wstring fileName);
	Data* AddData(string name, PDB::IMAGE_SECTION_HEADER* section, uint32_t size);
	Function* AddFunction(string name, PDB::IMAGE_SECTION_HEADER* section, uint32_t size);
	Status Analyze();
	Status Obfuscate();
	PDB::IMAGE_SECTION_HEADER* AddSection(string name, unsigned int vSize, SectionCharacteristics sectionCharacteristics);
};