#include "MainWindow.h"

char inputFileName[MAX_PATH];

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
		if (Global::hider.LoadFile() == Hider::Status::Success) {

		}
	}
	ImGui::EndChild();

	ImGui::End();
	return false;
}
