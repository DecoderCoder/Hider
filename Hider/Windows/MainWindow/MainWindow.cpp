#include "MainWindow.h"

char inputFileName[MAX_PATH];
char searchString[MAX_PATH];
char searchFunc[MAX_PATH];
bool searchGlobalFunc, searchPublicFunc, searchModuleFunc;

MainWindow::MainWindow(string inputFile)
{
	if (fs::exists(inputFile)) {
		memset(inputFileName, 0, sizeof(inputFileName));
		inputFile.copy(inputFileName, inputFile.size());
	}
}

void MainWindow::DropCallback(std::vector<std::wstring> files)
{
	if (files.size() > 0 && files[0].size() < MAX_PATH)
	{
		memset(inputFileName, 0, sizeof(inputFileName));
		to_string(files[0]).copy(inputFileName, files[0].size());
	}
}

bool MainWindow::Render()
{
	auto style = &ImGui::GetStyle();
	style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style->AntiAliasedFill = true;
	style->AntiAliasedLines = true;
	style->AntiAliasedLinesUseTex = true;

	ImGui::Begin("Hider", &this->Opened, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking);
	if (!hwnd)
	{
		hwnd = (HWND)ImGui::GetWindowViewport()->PlatformHandle;
		this->dropManager.dropCallback = [&](std::vector<std::wstring> files) {
			DropCallback(files);
			};
		RegisterDragDrop(hwnd, &this->dropManager);
	}

	ImGui::BeginChild("##inputFileChild", ImVec2(0, 75), ImGuiChildFlags_Border);

	ImGui::Text("Input file");
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40);
	ImGui::InputText("##inputFile", inputFileName, sizeof(inputFileName), ImGuiInputTextFlags_ReadOnly);
	ImGui::SameLine();
	ImGui::Button("...##inputFile", ImVec2(30, 0));
	if (ImGui::Button("Load file", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		if (Global::hider.LoadFile(to_wstring(string(inputFileName))) == Hider::Status::Success) {
			Global::hider.Analyze();
		}
	}
	ImGui::EndChild();

	ImGui::BeginDisabled(!Global::hider.InputFileSize);
	ImGui::BeginChild("Info", ImVec2(0, 150), ImGuiChildFlags_Border);
	ImGui::Columns(2);
	ImGui::Checkbox("Hide WinApi", &Global::hider.OO.Arithmetic);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
	{
		ImGui::SetTooltip("/* For internal */\r\n\r\n- GetModuleHandleW\r\n- GetAsyncKeyState\r\n- VirtualProtect");
	}

	ImGui::Checkbox(("Obfuscate strings [" + to_string(Global::hider.OO.selectedStrings.size()) + "]").c_str(), &Global::hider.OO.Strings);
	ImGui::SameLine();
	if (ImGui::Button("...##stringsbutton")) {
		ImGui::OpenPopup("Select strings");
	}
	if (ImGui::BeginPopupModal("Select strings", nullptr)) {
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputText("##searchstrings", searchString, sizeof(searchString));

		ImGui::BeginChild("##strings", ImVec2(0, 400));
		string search = lowercase(string(searchString));
		for (int i = 0; i < Global::hider.Strings.size(); i++) {
			auto& obj = Global::hider.Strings[i];
			if (search != "") {
				if (lowercase(obj.Value).find(search) == string::npos)
					continue;
			}

			bool checked = FINDVECTOR(Global::hider.OO.selectedStrings, &obj);
			if (ImGui::Checkbox((obj.Value + "##" + to_string(i)).c_str(), &checked)) {
				if (!checked)
					ERASEVECTORVAL(Global::hider.OO.selectedStrings, &obj)
				else
					Global::hider.OO.selectedStrings.push_back(&obj);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip(("RVA: 0x" + to_hex(obj.RVA) + "\r\nSize: " + to_string(obj.Size) + "\r\nName: " + obj.Name).c_str());
			}
		}
		ImGui::EndChild();
		if (ImGui::Button("Select all", ImVec2(ImGui::GetContentRegionAvail().x / 2, 0))) {
			for (int i = 0; i < Global::hider.Strings.size(); i++) {
				auto& obj = Global::hider.Strings[i];
				if (!FINDVECTOR(Global::hider.OO.selectedStrings, &obj))
					Global::hider.OO.selectedStrings.push_back(&obj);
			}

		}
		ImGui::SameLine();
		if (ImGui::Button("Deselect all", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			Global::hider.OO.selectedStrings.clear();
		}
		if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			string selectedStrings = "";
			for (auto str : Global::hider.OO.selectedStrings) {
				selectedStrings += hashString(str->Value, 10) + "\n";
			}
			if (fs::exists(Global::hider.SavedStringsFileName))
				fs::remove(Global::hider.SavedStringsFileName);
			WriteToFile(Global::hider.SavedStringsFileName, selectedStrings);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::Checkbox(("Obfuscate functions [" + to_string(Global::hider.OO.selectedFunctions.size()) + "]").c_str(), &Global::hider.OO.Functions);
	ImGui::SameLine();
	if (ImGui::Button("...##functionsbutton")) {
		ImGui::OpenPopup("Select functions");
	}
	if (ImGui::BeginPopupModal("Select functions", nullptr)) {
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		ImGui::InputText("##searchfunc", searchString, sizeof(searchString));
		ImGui::Checkbox("Global", &searchGlobalFunc);
		ImGui::SameLine();
		ImGui::Checkbox("Public", &searchPublicFunc);
		ImGui::SameLine();
		ImGui::Checkbox("Module", &searchModuleFunc);
		ImGui::BeginChild("##functions", ImVec2(0, 400));
		string search = lowercase(string(searchString));
		for (int i = 0; i < Global::hider.Functions.size(); i++) {
			auto& obj = Global::hider.Functions[i];

			if (!(searchGlobalFunc && obj.RecordType == RecordType::Global || searchModuleFunc && obj.RecordType == RecordType::Module || searchPublicFunc && obj.RecordType == RecordType::Public))
				continue;


			if (search != "") {
				if (lowercase(obj.Name).find(search) == string::npos)
					continue;
			}

			bool checked = FINDVECTOR(Global::hider.OO.selectedFunctions, &obj);
			if (ImGui::Checkbox((obj.Name + "##" + to_string(i)).c_str(), &checked)) {
				if (!checked)
					ERASEVECTORVAL(Global::hider.OO.selectedFunctions, &obj)
				else
					Global::hider.OO.selectedFunctions.push_back(&obj);
			}
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			{
				ImGui::SetTooltip(("RVA:  0x" + to_hex(obj.RVA) + "\r\nSize: 0x" + to_hex(obj.Size) + "\r\nName: " + obj.Name + "\r\n\r\nType: " + recordType(obj.RecordType)).c_str());
			}
		}
		ImGui::EndChild();
		if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
			string selectedFuncs = "";
			for (auto f : Global::hider.OO.selectedFunctions) {
				selectedFuncs += hashString(f->Name + to_string((int)f->RecordType), 10) + "\n";
			}
			if (fs::exists(Global::hider.SavedFuncsFileName))
				fs::remove(Global::hider.SavedFuncsFileName);
			WriteToFile(Global::hider.SavedFuncsFileName, selectedFuncs);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::NextColumn();
	ImGui::Text(("EXE Size: " + SizeToString(Global::hider.InputFileSize)).c_str());
	ImGui::EndChild();

	if (ImGui::Button("Obfuscate", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		Global::hider.Obfuscate();
	}
	ImGui::EndDisabled();
	ImGui::End();
	return false;
}
