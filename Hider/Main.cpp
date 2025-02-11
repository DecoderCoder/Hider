#define CPPHTTPLIB_OPENSSL_SUPPORT
#define WIN32_LEAN_AND_MEAN
#include "Main.h"
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

MainWindow* mainWindow;

void Main() {

}

void RenderThread() {
	OleInitialize(NULL);
	DirectX::Init();

	while (DirectX::Windows.size() > 0) {
		DirectX::Render();
	}
}

int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++) {
		Global::LaunchArguments.push_back(string(argv[i]));
	}

	string inputFile = "";

	for (int i = 0; i < Global::LaunchArguments.size(); i++) {
		if (Global::LaunchArguments[i] == "-i") {
			mainWindow = new MainWindow(Global::LaunchArguments[i + 1]);
			break;
		}
	}

	if (!mainWindow)
		mainWindow = new MainWindow();

	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RenderThread, NULL, NULL, NULL);

	while (!GetAsyncKeyState(VK_END)) {
		Sleep(1);
		if (DirectX::Windows.size() == 0)
			return 0;
	}
	DirectX::Deinit();
}