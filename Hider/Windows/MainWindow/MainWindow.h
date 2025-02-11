#pragma once
#include "../../DirectX/DirectX.h"
#include <thread>
#include "../../libs/DropManager.h"
#include "../../libs/Utils.h"
#include "../../Globals.h"

using namespace std;

class MainWindow : public Window {
	HWND hwnd = 0;
	DropManager dropManager;
public:
	MainWindow() = default;
	MainWindow(string inputFile);

	void DropCallback(std::vector<std::wstring> files);
	virtual bool Render();
};