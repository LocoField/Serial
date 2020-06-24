#include "stdafx.h"
#include "Dialog.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	Dialog dialog;
	dialog.loadOption();

	return dialog.exec();
}
