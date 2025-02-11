#include "Hider.h"
#include "../libs/Utils.h"

ZydisDisassembledInstruction ASMToZydis(string input, uintptr_t rva = 0, char* write = nullptr) {
	XEDPARSE xed;
	memset(&xed, 0, sizeof(xed));
	xed.cip = rva;
	input.copy(xed.instr, input.size());
	XEDParseAssemble(&xed);
	//if(this->Ptr)
	ZydisDisassembledInstruction instruction;
	if (ZYAN_SUCCESS(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, (uintptr_t)xed.cip, (char*)(xed.dest), xed.dest_size, &instruction))) {
		if (write)
			memcpy(write, xed.dest, xed.dest_size);
	}
	return instruction;
}

PDB_NO_DISCARD static bool IsError(PDB::ErrorCode errorCode)
{
	switch (errorCode)
	{
	case PDB::ErrorCode::Success:
		return false;

	case PDB::ErrorCode::InvalidSuperBlock:
		printf("Invalid Superblock\n");
		return true;

	case PDB::ErrorCode::InvalidFreeBlockMap:
		printf("Invalid free block map\n");
		return true;

	case PDB::ErrorCode::InvalidStream:
		printf("Invalid stream\n");
		return true;

	case PDB::ErrorCode::InvalidSignature:
		printf("Invalid stream signature\n");
		return true;

	case PDB::ErrorCode::InvalidStreamIndex:
		printf("Invalid stream index\n");
		return true;

	case PDB::ErrorCode::UnknownVersion:
		printf("Unknown version\n");
		return true;
	}

	// only ErrorCode::Success means there wasn't an error, so all other paths have to assume there was an error
	return true;
}

String Hider::GetStringByRecord(const PDB::CodeView::DBI::Record* record, char* o_file, const PDB::ImageSectionStream& imageSectionStream, RecordType funcType) {
	const char* name = nullptr;
	uint32_t rva = 0u;
	uint32_t offset = 0;
	uint32_t size = record->header.size;
	string value;

	switch (record->header.kind) {
	case PDB::CodeView::DBI::SymbolRecordKind::S_PUB32: {
		if (record->data.S_PUB32.flags == PDB::CodeView::DBI::PublicSymbolFlags::None) {
			name = record->data.S_PUB32.name;
			rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_PUB32.section, record->data.S_PUB32.offset);
		}
		break;
	}
	}
	if (name) {
		offset = RVA2Offset(rva);
		if (offset)
		{
			//size = record->header.size - string(name).size();
			value = string(o_file + offset);
		}
	}
	if (!name)
	{
		name = "blank";
		rva = 0;
	}
	String str;
	str.Name = string(name);
	if (name != "blank") {
		str.Name = str.Name.substr(0, record->header.size - ((uintptr_t)&record->data.S_PUB32.name[0] - (uintptr_t)record));
	}
	str.Offset = offset;
	str.RecordType = funcType;
	str.RVA = rva;
	str.Value = value;
	str.Size = value.size();
	return str;
}

DataRecord Hider::GetDataByRecord(const PDB::CodeView::DBI::Record* record, char* o_file, const PDB::ImageSectionStream& imageSectionStream, RecordType funcType)
{
	const char* name = nullptr;
	uint32_t rva = 0u;
	uint32_t offset = 0;
	uint32_t size = record->header.size;
	string value;

	switch (record->header.kind) {
	case PDB::CodeView::DBI::SymbolRecordKind::S_PUB32: {
		if (record->data.S_PUB32.flags == PDB::CodeView::DBI::PublicSymbolFlags::None) {
			name = record->data.S_PUB32.name;
			rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_PUB32.section, record->data.S_PUB32.offset);
		}
		break;
	}
	}
	if (name) {
		offset = RVA2Offset(rva);
	}
	DataRecord str;
	if (name != nullptr) {
		str.Name = string(name);
		str.RVA = rva;
	}
	if (name != "blank") {
		str.Name = str.Name.substr(0, record->header.size - ((uintptr_t)&record->data.S_PUB32.name[0] - (uintptr_t)record));
	}
	str.Offset = offset;

	str.Size = value.size();
	return str;
}

String* Hider::GetStringByRVA(ULONG RVA)
{
	for (auto& s : this->Strings) {
		if (s.RVA == RVA)
			return &s;
	}
	return nullptr;
}

DataRecord* Hider::GetDataByRVA(ULONG RVA)
{
	for (auto& d : this->Datas) {
		if (d.RVA == RVA)
			return &d;
	}
	return nullptr;
}

Function Hider::GetFunctionByRecord(const PDB::CodeView::DBI::Record* record, const PDB::ImageSectionStream& imageSectionStream, RecordType funcType) {
	const char* name = nullptr;
	uint32_t rva = 0u;
	uint32_t address = 0;
	uint32_t size = record->header.size;

	if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_GDATA32)
	{
		name = record->data.S_GDATA32.name;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_GDATA32.section, record->data.S_GDATA32.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_GTHREAD32)
	{
		name = record->data.S_GTHREAD32.name;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_GTHREAD32.section, record->data.S_GTHREAD32.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LDATA32)
	{
		name = record->data.S_LDATA32.name;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_LDATA32.section, record->data.S_LDATA32.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LTHREAD32)
	{
		name = record->data.S_LTHREAD32.name;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_LTHREAD32.section, record->data.S_LTHREAD32.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_UDT)
	{
		name = record->data.S_UDT.name;
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_UDT_ST)
	{
		name = record->data.S_UDT_ST.name;
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_THUNK32)
	{
		if (record->data.S_THUNK32.thunk == PDB::CodeView::DBI::ThunkOrdinal::TrampolineIncremental)
		{
			// we have never seen incremental linking thunks stored inside a S_THUNK32 symbol, but better be safe than sorry
			name = "ILT";
			rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_THUNK32.section, record->data.S_THUNK32.offset);
		}
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_TRAMPOLINE)
	{
		// incremental linking thunks are stored in the linker module
		name = "ILT";
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_TRAMPOLINE.thunkSection, record->data.S_TRAMPOLINE.thunkOffset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_BLOCK32)
	{
		// blocks never store a name and are only stored for indicating whether other symbols are children of this block
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LABEL32)
	{
		// labels don't have a name
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LPROC32)
	{
		name = record->data.S_LPROC32.name;
		size = record->data.S_LPROC32.codeSize;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_LPROC32.section, record->data.S_LPROC32.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_GPROC32)
	{
		name = record->data.S_GPROC32.name;
		size = record->data.S_GPROC32.codeSize;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_GPROC32.section, record->data.S_GPROC32.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LPROC32_ID)
	{
		name = record->data.S_LPROC32_ID.name;
		size = record->data.S_LPROC32_ID.codeSize;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_LPROC32_ID.section, record->data.S_LPROC32_ID.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_GPROC32_ID)
	{
		name = record->data.S_GPROC32_ID.name;
		size = record->data.S_GPROC32_ID.codeSize;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_GPROC32_ID.section, record->data.S_GPROC32_ID.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_REGREL32)
	{
		name = record->data.S_REGREL32.name;
		// You can only get the address while running the program by checking the register value and adding the offset
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LDATA32)
	{
		name = record->data.S_LDATA32.name;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_LDATA32.section, record->data.S_LDATA32.offset);
	}
	else if (record->header.kind == PDB::CodeView::DBI::SymbolRecordKind::S_LTHREAD32)
	{
		name = record->data.S_LTHREAD32.name;
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_LTHREAD32.section, record->data.S_LTHREAD32.offset);
	}
	else {
		rva = imageSectionStream.ConvertSectionOffsetToRVA(record->data.S_PUB32.section, record->data.S_PUB32.offset);
		name = record->data.S_PUB32.name;
	}
	string name2 = "";
	if (name)
	{
		name2 = ReplaceAll(string(name));
		address = RVA2Offset(rva);
	}
	else
		rva = 0;

	Function func;
	func.Name = name2;
	func.RVA = rva;
	func.Offset = rva ? address : 0;
	func.Size = size;
	func.RecordType = funcType;

	return func;
}

Function* Hider::GetFunctionByRVA(ULONG RVA)
{
	for (auto& f : this->Functions) {
		if (f.RVA == RVA)
			return &f;
	}
	return nullptr;
}
Hider::Status Hider::LoadFile(std::wstring fileName)
{
	wstring exeName = fs::path(fileName).filename().wstring();
	wstring fileExtension = fileName.substr(fileName.size() - 3);
	wstring newExeFileName = fileName;
	newExeFileName = newExeFileName.substr(0, newExeFileName.size() - 4) + L"_new." + fileExtension;
	wstring pdbFileName = fileName;
	pdbFileName.replace(pdbFileName.end() - 3, pdbFileName.end(), L"pdb");

	InputEXEFileName = fileName;
	InputPDBFileName = pdbFileName;

	OutputEXEFileName = newExeFileName;
	SavedStringsFileName = fileName.substr(0, fileName.size() - 4) + L"_strings.txt";
	SavedFuncsFileName = fileName.substr(0, fileName.size() - 4) + L"_funcs.txt";

	if (fs::exists(SavedStringsFileName)) {
		size_t size;
		char* file = ReadAllBytes(SavedStringsFileName, &size);
		string strings = string(file, size);
		replaceAll(strings, "\r", "");
		while (strings.find("\n") != string::npos) {
			string str = strings.substr(0, strings.find("\n"));
			if (str.size() > 0)
				this->SavedStrings.push_back(str);
			strings.erase(0, str.size() + 1);
		}
	}

	if (fs::exists(SavedFuncsFileName)) {
		size_t size;
		char* file = ReadAllBytes(SavedFuncsFileName, &size);
		string strings = string(file, size);
		replaceAll(strings, "\r", "");
		while (strings.find("\n") != string::npos) {
			string str = strings.substr(0, strings.find("\n"));
			if (str.size() > 0)
				this->SavedFuncs.push_back(str);
			strings.erase(0, str.size() + 1);
		}
	}

	if (!fs::exists(InputEXEFileName) || !fs::exists(InputPDBFileName))
		return Hider::Status::FileNotFound;


	InputFile = ReadAllBytes(InputEXEFileName, &InputFileSize);
	ModifiedFile = ReadAllBytes(InputEXEFileName, &ModifiedFileSize);
	PDBFile = ReadAllBytes(InputPDBFileName, &PDBFileSize);

	if (IsError(PDB::ValidateFile(PDBFile, PDBFileSize)))
		return Hider::Status::InvalidPDBFile;

	this->PE = DirectPE(ModifiedFile);
	if (this->PE.OptionalHeader->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) // Only X64
		return Hider::Status::InvalidEXEFile;

	this->PDB = new PDBInfo(PDBFile);

	for (auto sec : this->PDB->ImageSectionStream.GetImageSections()) {
		this->Sections.push_back(sec);
	}

	return Hider::Status::Success;
}

string lastAdded;

Data* Hider::AddData(string name, PDB::IMAGE_SECTION_HEADER* section, uint32_t size)
{
	Data* customData = new Data();
	customData->Name = name;
	customData->Size = size;
	if (this->CustomData.size() == 0)
		customData->RVA = section->VirtualAddress;
	else
	{
		auto last = this->CustomData[lastAdded];
		customData->RVA = last->RVA + align(last->Size, 16);
	}

	customData->Ptr = this->ModifiedFile + RVA2Offset(customData->RVA);

	CustomData[name] = customData;
	lastAdded = name;
	return customData;
}

string lastAddedFunction;

Function* Hider::AddFunction(string name, PDB::IMAGE_SECTION_HEADER* section, uint32_t size)
{
	Function* func = new Function();
	func->Name = name;
	func->Size = size;
	if (this->CustomFunctions.size() == 0)
		func->RVA = section->VirtualAddress;
	else
	{
		auto last = this->CustomFunctions[lastAddedFunction];
		func->RVA = last->RVA + align(last->Size, 16);
	}
	func->Ptr = this->ModifiedFile + RVA2Offset(func->RVA);
	CustomFunctions[name] = func;
	lastAddedFunction = name;
	return func;
}

Hider::Status Hider::Analyze()
{
	class ToProceed {
	public:
		PDB::CodeView::DBI::Record* record;
		RecordType recordType;
	};
	std::vector<ToProceed> toProceed;
	{
		const PDB::ArrayView<PDB::HashRecord> hashRecords = this->PDB->PublicSymbolStream.GetRecords();
		for (const PDB::HashRecord& hashRecord : hashRecords)
		{
			const PDB::CodeView::DBI::Record* record = this->PDB->PublicSymbolStream.GetRecord(this->PDB->SymbolRecordStream, hashRecord);
			toProceed.push_back(ToProceed{ (PDB::CodeView::DBI::Record*)record, RecordType::Global });
		}
	}
	{
		const PDB::ArrayView<PDB::HashRecord> hashRecords = this->PDB->GlobalSymbolStream.GetRecords();
		for (const PDB::HashRecord& hashRecord : hashRecords)
		{
			const PDB::CodeView::DBI::Record* record = this->PDB->GlobalSymbolStream.GetRecord(this->PDB->SymbolRecordStream, hashRecord);
			toProceed.push_back(ToProceed{ (PDB::CodeView::DBI::Record*)record, RecordType::Public });
		}
	}
	{
		const PDB::ArrayView<PDB::ModuleInfoStream::Module> modules = this->PDB->ModuleInfoStream.GetModules();
		for (const PDB::ModuleInfoStream::Module& module : modules)
		{
			if (!module.HasSymbolStream())
			{
				continue;
			}
			const PDB::ModuleSymbolStream moduleSymbolStream = module.CreateSymbolStream(this->PDB->RawFile);
			moduleSymbolStream.ForEachSymbol([&toProceed](const PDB::CodeView::DBI::Record* record)
				{
					toProceed.push_back(ToProceed{ (PDB::CodeView::DBI::Record*)record, RecordType::Module });
				});
		}
	}

	for (int i = 0; i < toProceed.size(); i++)
	{
		auto rec = toProceed[i];
		if (Function f = GetFunctionByRecord(rec.record, this->PDB->ImageSectionStream, rec.recordType); f.RVA != 0)
			this->Functions.push_back(f);
		if (String s = GetStringByRecord(rec.record, this->InputFile, this->PDB->ImageSectionStream, rec.recordType); (s.RVA != 0 && isASCII(s.Value)))
			this->Strings.push_back(s);
		if (DataRecord d = GetDataByRecord(rec.record, this->InputFile, this->PDB->ImageSectionStream, rec.recordType); (d.RVA != 0))
			this->Datas.push_back(d);
	}

	{
		std::vector<String*> toAddp;
		std::vector<String> tempStrings;
		for (auto& func : Functions) {
			func.Disassemble(this);
			// sort strings
			for (auto& inst : func.Instructions) {
				if (inst.info.mnemonic == ZydisMnemonic_::ZYDIS_MNEMONIC_LEA)
				{
					uint32_t rva = inst.runtime_address + inst.operands[1].mem.disp.value + inst.info.length; // lea size
					for (auto& str : Strings) {
						if (str.RVA == rva) {
							uint32_t offset = RVA2Offset(rva);
							string s = string(ModifiedFile + offset);
							if (s.size() > 3)
							{
								str.Usage.push_back(&inst);
								if (!FINDVECTOR(toAddp, &str))
									toAddp.push_back(&str);
							}
						}
					}
				}
			}
		}

		for (auto& str : toAddp)
			tempStrings.push_back(*str);
		Strings = tempStrings;

		for (auto& savedStr : this->SavedStrings) {
			for (auto& str : this->Strings) {
				if (savedStr == hashString(str.Value, 10)) {
					this->OO.selectedStrings.push_back(&str);
				}
			}
		}

		for (auto& savedStr : this->SavedFuncs) {
			for (auto& f : this->Functions) {
				if (f.Name != "")
					if (savedStr == hashString(f.Name + to_string((int)f.RecordType), 10)) {
						this->OO.selectedFunctions.push_back(&f);
					}
			}
		}
	}

	return Status::Success;
}

std::string fixedWidth(int value, int width)
{
	char buffer[100];
	snprintf(buffer, sizeof(buffer), "%.*d", width, value);
	return buffer;
}
Hider::Status Hider::Obfuscate()
{
	auto ExecuteSection = AddSection("text2", 0x15000, SectionCharacteristics::Executable);
	auto DataSection = AddSection("data2", 0x15000, SectionCharacteristics::Data);

	// Encoding strings
	for (auto& str : this->OO.selectedStrings) {
		int xorVal = 0x8F2BEE9E; Random::get(MININT, MAXINT);

		auto dataString = this->AddData(str->Name, DataSection, str->Size + 2);
		auto decodeFunc = this->AddFunction(str->Name, ExecuteSection, 100); // 64
		dataString->SetData(str->Value, 2);
		//*(uint16_t*)dataString->Ptr = 0;
		for (int i = 0; i < align(str->Size, 16); i += 4) {
			char* e = dataString->Ptr + 2 + i;
			*(int*)e = *(int*)e ^ xorVal;
		}
		memset(ModifiedFile + str->Offset, 0, str->Size);

		// adding function to decode
		decodeFunc->AddInstruction("push rax");
		decodeFunc->AddInstruction("mov ax, [0x" + to_hex(dataString->RVA) + "]");
		decodeFunc->AddInstruction("cmp ax, 1");
		decodeFunc->AddInstruction("je $+0x2F"); // 
		decodeFunc->AddInstruction("push rcx");
		decodeFunc->AddInstruction("lea rax, [0x" + to_hex(dataString->RVA) + "]");
		decodeFunc->AddInstruction("mov word[rax], 1");
		decodeFunc->AddInstruction("push rdx");
		decodeFunc->AddInstruction("xor rcx, rcx");
		decodeFunc->AddInstruction("add rax, 2");
		decodeFunc->AddInstruction("mov edx, dword[rax + rcx]");
		decodeFunc->AddInstruction("xor edx, 0x" + to_hex(xorVal));
		decodeFunc->AddInstruction("mov dword[rax+rcx], edx");
		decodeFunc->AddInstruction("add rcx, 4");
		decodeFunc->AddInstruction("cmp rcx, 0x" + to_hex(align(str->Size, 16) - 1));
		decodeFunc->AddInstruction("jle $-0x14");
		decodeFunc->AddInstruction("pop rdx");
		decodeFunc->AddInstruction("pop rcx");
		decodeFunc->AddInstruction("pop rax");
		decodeFunc->AddInstruction("ret");

		for (auto& usage : str->Usage) {
			auto ptr = ModifiedFile + RVA2Offset(usage->runtime_address);

			auto leaFunc = this->AddFunction(usage->text, ExecuteSection, 16);
			leaFunc->AddInstruction("call 0x" + to_hex(decodeFunc->RVA));
			auto prevInstText = string(usage->text);
			prevInstText = prevInstText.substr(0, prevInstText.find(", "));
			leaFunc->AddInstruction(prevInstText + ", [0x" + to_hex(dataString->RVA + 2) + "]");
			leaFunc->AddInstruction("ret");

			memset(ptr, 0x90, usage->info.length);
			*usage = ASMToZydis("call 0x" + to_hex(leaFunc->RVA), usage->runtime_address, ptr);
			printf("");
		}
	}

	WriteToFile(OutputEXEFileName, ModifiedFile, ModifiedFileSize);
	return Status::Success;
}

PDB::IMAGE_SECTION_HEADER* Hider::AddSection(string name, unsigned int vSize, SectionCharacteristics sectionCharacteristics)
{
	PIMAGE_SECTION_HEADER newSectionHeader = (PIMAGE_SECTION_HEADER)(IMAGE_FIRST_SECTION(this->PE.NtHeader)) + this->PE.FileHeader->NumberOfSections;
	memset(newSectionHeader, 0, sizeof(PIMAGE_SECTION_HEADER));
	PIMAGE_SECTION_HEADER lastSection = newSectionHeader - 1;

	char newSectionName[6];
	memset(newSectionName, 0, sizeof(newSectionName));
	name.copy(&newSectionName[0], min(6, name.size()));
	newSectionHeader->Name[0] = '.';
	memcpy(&newSectionHeader->Name[1], newSectionName, 6);
	auto secAlign = this->PE.OptionalHeader->SectionAlignment;
	newSectionHeader->PointerToRawData = ModifiedFileSize;
	newSectionHeader->Characteristics = (DWORD)sectionCharacteristics;
	newSectionHeader->SizeOfRawData = vSize;
	newSectionHeader->Misc.VirtualSize = newSectionHeader->SizeOfRawData;
	newSectionHeader->VirtualAddress = (uint32_t)(ceil((lastSection->VirtualAddress + lastSection->Misc.VirtualSize) / (double)this->PE.OptionalHeader->SectionAlignment) * this->PE.OptionalHeader->SectionAlignment);
	this->PE.FileHeader->NumberOfSections++;

	auto additionalSize = (uint32_t)(ceil(newSectionHeader->SizeOfRawData / (double)this->PE.OptionalHeader->SectionAlignment) * this->PE.OptionalHeader->SectionAlignment);
	this->PE.OptionalHeader->SizeOfImage = this->PE.OptionalHeader->SizeOfImage + additionalSize;

	auto sectionOffset = (uintptr_t)newSectionHeader - (uintptr_t)ModifiedFile;
	ModifiedFile = (char*)realloc(ModifiedFile, ModifiedFileSize + additionalSize);
	this->PE = DirectPE(ModifiedFile);
	newSectionHeader = (PIMAGE_SECTION_HEADER)((uintptr_t)ModifiedFile + sectionOffset);
	memset(ModifiedFile + ModifiedFileSize, 0, additionalSize);
	ModifiedFileSize = ModifiedFileSize + additionalSize;

	this->Sections.push_back(*(PDB::IMAGE_SECTION_HEADER*)newSectionHeader);

	return &this->Sections.back();
}

void Function::Disassemble(Hider* hider)
{
	uintptr_t FuncAddress = hider->RVA2Offset(this->RVA);
	if (FuncAddress == 0)
		return;

	ZyanUSize offset = 0;
	ZydisDisassembledInstruction instruction;

	ZyanStatus zyanStatus;
	while (ZYAN_SUCCESS(zyanStatus = ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, (uintptr_t)this->RVA + offset, (char*)(hider->ModifiedFile + FuncAddress + offset), this->Size - offset, &instruction)))
	{
		offset += instruction.info.length;
		this->Instructions.push_back(instruction);
	}
}

bool Function::AddInstruction(string asmCode, bool removeInstSize)
{
	XEDPARSE xed;
	memset(&xed, 0, sizeof(xed));
	uint32_t instrBefore = 0;
	for (auto inst : this->Instructions) {
		instrBefore += inst.info.length;
	}
	xed.x64 = true;
	xed.cip = this->RVA + instrBefore;

	{ // 
		if (asmCode.find("$") != string::npos)
		{
			auto spacePos = asmCode.find(" ");
			string expr = asmCode.substr(spacePos + 1);
			string left = expr.find("+") != string::npos ? expr.substr(0, expr.find("+")) : expr.substr(0, expr.find("-"));
			string right = expr.substr(left.size());
			bool add = false;
			if (right[0] == '+')
				add = true;
			right = right.substr(1);

			bool deltaIsLeft = false;
			if (right == "$")
				deltaIsLeft = true;

			replaceAll(left, "$", "0x" + to_hex(xed.cip));
			replaceAll(right, "$", "0x" + to_hex(xed.cip));

			char* p;
			uintptr_t result;

			if (left.find("0x") == string::npos)
				left = "0x" + to_hex(std::stoi(left));
			if (right.find("0x") == string::npos)
				right = "0x" + to_hex(std::stoi(right));

			uintptr_t leftInt = strtol(left.substr(2).c_str(), &p, 16);
			uintptr_t rightInt = strtol(right.substr(2).c_str(), &p, 16);

			int delta = deltaIsLeft ? leftInt : rightInt;
			if (!add)
				delta = -delta;

			bool isShort = (delta <= 127 + 2) && (delta > -127);

			result = leftInt + (delta + ((isShort ? 2 : 5) * (add ? 1 : 0) * removeInstSize));

			asmCode.replace(spacePos + 1, expr.size(), "0x" + to_hex(result));
		}
	}
	asmCode.copy(xed.instr, asmCode.size());

	XEDParseAssemble(&xed);
	//if(this->Ptr)
	ZydisDisassembledInstruction instruction;
	if (ZYAN_SUCCESS(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64, (uintptr_t)xed.cip, (char*)(xed.dest), xed.dest_size, &instruction))) {
		memcpy(this->Ptr + instrBefore, xed.dest, xed.dest_size);
		this->Instructions.push_back(instruction);
		return true;
	}

	return false;
}

PDB::IMAGE_SECTION_HEADER* Hider::GetSectionByName(std::string name) {
	for (auto section : Sections) {
		if (string((char*)section.Name) == name) {
			return &section;
		}
	}
	return nullptr;
}

PDB::IMAGE_SECTION_HEADER* Hider::GetSectionByRVA(uint32_t rva, std::vector<PDB::IMAGE_SECTION_HEADER>* sections) {
	for (auto section : sections ? *sections : Sections) {
		if (rva >= section.VirtualAddress && rva <= section.VirtualAddress + section.Misc.VirtualSize) {
			return &section;
		}
	}
	return nullptr;
}

PDB::IMAGE_SECTION_HEADER* Hider::GetSectionByOffset(uint32_t offset, std::vector<PDB::IMAGE_SECTION_HEADER>* sections) {
	for (auto section : sections ? *sections : Sections) {
		if (offset >= section.PointerToRawData && offset <= section.PointerToRawData + section.SizeOfRawData) {
			return &section;
		}
	}
	return nullptr;
}

uint32_t Hider::RVA2Offset(uint32_t rva, std::vector<PDB::IMAGE_SECTION_HEADER>* sections) {
	auto section = GetSectionByRVA(rva, sections);
	if (!section)
		return 0;
	return rva - section->VirtualAddress + section->PointerToRawData;
}

uintptr_t Hider::Offset2RVA(uint32_t offset, std::vector<PDB::IMAGE_SECTION_HEADER>* sections) {
	auto section = GetSectionByOffset(offset, sections);
	if (!section)
		return 0;
	return offset + section->VirtualAddress - section->PointerToRawData;
}