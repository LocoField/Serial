#include "stdafx.h"
#include "Dialog.h"
#include "SerialPort.h"
#include "SerialAddinHelper.h"
#include "SerialAddinBase.h"

#define DIALOG_TITLE "LocoField Serial v2.2.0"

Dialog::Dialog()
{
	initialize();
}

Dialog::~Dialog()
{
}

void Dialog::initialize()
{
	installEventFilter(this);

	{
		auto groupBox = new QGroupBox;

		helperLayout = new QHBoxLayout;
		helperLayout->addWidget(groupBox);

		{
			auto gridLayout = new QGridLayout;
			gridLayout->setAlignment(Qt::AlignLeft);

			auto labelDec = new QLabel("Decimal: ");
			labelDec->setFixedWidth(100);

			auto labelHex = new QLabel("Hexadecimal: ");
			labelHex->setFixedWidth(100);

			auto lineEditDecToHex = new QLineEdit;
			lineEditDecToHex->setFixedWidth(200);
			lineEditDecToHex->setValidator(new QIntValidator());

			auto lineEditHexToDec = new QLineEdit;
			lineEditHexToDec->setFixedWidth(200);
			lineEditHexToDec->setValidator(new QRegExpValidator(QRegExp("[0-9a-fA-F]+")));

			gridLayout->addWidget(labelDec, 0, 0);
			gridLayout->addWidget(lineEditDecToHex, 0, 1);

			gridLayout->addWidget(labelHex, 1, 0);
			gridLayout->addWidget(lineEditHexToDec, 1, 1);

			groupBox->setLayout(gridLayout);

			//////////////////////////////////////////////////////////////////////////

			connect(lineEditDecToHex, &QLineEdit::textEdited, [lineEditHexToDec](const QString& text)
				{
					auto number = text.toInt();

					QString hexString;
					hexString.setNum(number, 16);
					lineEditHexToDec->setText(hexString);
				}
			);

			connect(lineEditHexToDec, &QLineEdit::textEdited, [lineEditDecToHex](const QString& text)
				{
					auto number = text.toInt(nullptr, 16);

					QString decString;
					decString.setNum(number);
					lineEditDecToHex->setText(decString);
				}
			);
		}
	}

	{
		serialLayout = new QHBoxLayout;
	}

	mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(helperLayout);
	mainLayout->addLayout(serialLayout);

	//////////////////////////////////////////////////////////////////////////

	QMenuBar* menu = new QMenuBar();
	layout()->setMenuBar(menu);

	QMenu* menuView = new QMenu("View");
	{
		QAction* actionAlwaysOnTop = new QAction("Always On Top");
		connect(actionAlwaysOnTop, &QAction::toggled, [&](bool checked)
		{
			if (checked)
			{
				SetWindowPos((HWND)this->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else
			{
				SetWindowPos((HWND)this->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			}
		});

		actionAlwaysOnTop->setCheckable(true);
		actionAlwaysOnTop->setChecked(true);

		menuView->addAction(actionAlwaysOnTop);
		menu->addMenu(menuView);
	}

	QMenu* menuAddin = new QMenu("Addin");
	{
		auto executeAddin = [](const std::string& filePath, std::vector<SerialPort*>& serialPorts)
		{
			HMODULE module = LoadLibraryA(filePath.c_str());

			SerialAddin loadAddin = reinterpret_cast<SerialAddin>(GetProcAddress(module, "loadAddin"));
			if (loadAddin == nullptr)
			{
				cout << "ERROR: invalid addin." << endl;

				FreeLibrary(module);
				return;
			}

			SerialAddinBase* object = loadAddin();

			SerialAddinHelper addinHelper(object);
			addinHelper.setSerialPorts(serialPorts);
			addinHelper.execute();

			delete object;
			FreeLibrary(module);
		};

		QAction* actionYModem = new QAction("YModem");
		connect(actionYModem, &QAction::triggered, [&, this]()
		{
			auto addinPath = QCoreApplication::applicationDirPath() + "/SerialAddinYModem";

#ifdef _DEBUG
			addinPath += "d.dll";
#else
			addinPath += ".dll";
#endif

			executeAddin(addinPath.toStdString(), serialPorts);
		});

		QAction* actionLoadAddin = new QAction("Load");
		connect(actionLoadAddin, &QAction::triggered, [&, this]()
		{
			if (lastFileDialogPath.isEmpty())
				lastFileDialogPath = QCoreApplication::applicationDirPath();

			QString addinPath = QFileDialog::getOpenFileName(this, "Load Addin", lastFileDialogPath, "Addins (*.dll)");

			if (addinPath.isEmpty())
				return;

			QFileInfo fileInfo(addinPath);
			lastFileDialogPath = fileInfo.dir().absolutePath();

			executeAddin(addinPath.toStdString(), serialPorts);
		});

		menuAddin->addAction(actionYModem);
		menuAddin->addAction(actionLoadAddin);
		menu->addMenu(menuAddin);
	}

	setWindowTitle(DIALOG_TITLE);
	setWindowFlag(Qt::WindowMinimizeButtonHint);
}

bool Dialog::loadOption()
{
	bool retval = true;
	int n = 1;
	QVector<QVariant> separator;

	QString filepath = QCoreApplication::applicationDirPath();
	QFile loadFile(filepath + "/option.txt");

	if (loadFile.open(QIODevice::ReadOnly))
	{
		QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
		if (doc.isNull())
		{
			retval = false;
		}
		else
		{
			QJsonObject object = doc.object();
			auto keys = object.keys();

			n = object["device_n"].toInt();

			separator = object["separator"].toArray().toVariantList().toVector();

			if (keys.contains("dialog_x") &&
				keys.contains("dialog_y"))
			{
				int x = object["dialog_x"].toInt();
				int y = object["dialog_y"].toInt();
				__super::move(x, y);
			}

			if (keys.contains("dialog_w") &&
				keys.contains("dialog_h"))
			{
				int w = object["dialog_w"].toInt();
				int h = object["dialog_h"].toInt();
				__super::resize(w, h);
			}

			if (keys.contains("file_path"))
				lastFileDialogPath = object["file_path"].toString();
		}

		loadFile.close();
	}
	else
	{
		retval = false;
	}

	n = std::max(n, 1);
	n = std::min(n, 4);

	for (int i = 0; i < n; i++)
	{
		auto serialPort = new SerialPort(i);
		serialPorts.emplace_back(serialPort);

		for (auto&& c : separator)
			serialPort->addCommandSeparator(c.toString());

		serialLayout->addWidget(serialPort->widgetSerial());
	}

	return retval;
}

bool Dialog::saveOption()
{
	QString filepath = QCoreApplication::applicationDirPath();
	QFile loadFile(filepath + "/option.txt");

	if (loadFile.open(QIODevice::ReadOnly) == false)
		return false;

	QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
	if (doc.isNull())
		return false;

	QPoint xy = __super::pos();
	QSize wh = __super::size();

	QJsonObject object = doc.object();
	object["dialog_x"] = xy.x();
	object["dialog_y"] = xy.y();
	object["dialog_w"] = wh.width();
	object["dialog_h"] = wh.height();
	object["file_path"] = lastFileDialogPath;

	loadFile.close();
	if (loadFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
		return false;

	doc.setObject(object);
	loadFile.write(doc.toJson(QJsonDocument::JsonFormat::Indented));
	loadFile.close();

	return true;
}

bool Dialog::eventFilter(QObject* object, QEvent* event)
{
	QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
	if (keyEvent)
	{
		if (keyEvent->key() == Qt::Key_Escape)
		{
			QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(QApplication::focusWidget());
			if (lineEdit)
			{
				lineEdit->clear();
			}
		}

		return true;
	}

	return __super::eventFilter(object, event);
}

void Dialog::closeEvent(QCloseEvent* event)
{
	saveOption();

	for (auto& serialPort : serialPorts)
	{
		delete serialPort;
	}

	serialPorts.clear();

	__super::closeEvent(event);
}
