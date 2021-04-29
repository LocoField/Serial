#include "stdafx.h"
#include "Dialog.h"

int initializeDialog()
{
	Dialog dialog;
	dialog.loadOption();

	return dialog.exec();
}

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	QApplication app(__argc, __argv);
	return initializeDialog();
}

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	return initializeDialog();
}
