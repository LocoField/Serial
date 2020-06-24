#include "stdafx.h"
#include "Dialog.h"
#include "SerialPortManager.h"

#define DIALOG_TITLE "LocoField Serial v.2.0"

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
		auto labelDec = new QLabel("DEC: ");
		auto labelEqual = new QLabel(" = ");
		auto labelHex = new QLabel("HEX: ");
		auto horizontalSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);

		auto lineEditHexHelper = new QLineEdit;
		lineEditHexHelper->setFixedWidth(200);
		lineEditHexHelper->setValidator(new QIntValidator());

		auto lineEditDecHelper = new QLineEdit;
		lineEditDecHelper->setFixedWidth(200);
		lineEditDecHelper->setValidator(new QRegExpValidator(QRegExp("([0-9a-fA-F]+")));

		helperLayout = new QHBoxLayout;
		helperLayout->setAlignment(Qt::AlignLeft);
		helperLayout->addWidget(labelDec);
		helperLayout->addWidget(lineEditHexHelper);
		helperLayout->addWidget(labelEqual);
		helperLayout->addWidget(labelHex);
		helperLayout->addWidget(lineEditDecHelper);
		helperLayout->addItem(horizontalSpacer);

		//////////////////////////////////////////////////////////////////////////

		connect(lineEditHexHelper, &QLineEdit::textEdited, [lineEditHexHelper, lineEditDecHelper]()
			{
				auto number = lineEditHexHelper->text().toInt();

				QString hexString;
				hexString.setNum(number, 16);
				lineEditDecHelper->setText(hexString);
			}
		);

		connect(lineEditDecHelper, &QLineEdit::textEdited, [lineEditHexHelper, lineEditDecHelper]()
			{
				QString hexString = lineEditDecHelper->text();

				auto value = QByteArray::fromHex(hexString.toLatin1());
				lineEditHexHelper->setText(value);
			}
		);
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

	setWindowTitle(DIALOG_TITLE);
	setWindowFlag(Qt::WindowMinimizeButtonHint);
}

bool Dialog::loadOption()
{
	bool retval = true;
	int n = 1;
	int x = 500;
	int y = 500;
	int w = 600;
	int h = 800;

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

			n = object["device_n"].toInt();

			x = object["dialog_x"].toInt();
			y = object["dialog_y"].toInt();
			w = object["dialog_w"].toInt();
			h = object["dialog_h"].toInt();

			lastFileDialogPath = object["file_path"].toString();

			__super::move(x, y);
			__super::resize(w, h);
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
		auto manager = new SerialPortManager(i);
		serialManagers.emplace_back(manager);
		serialLayout->addWidget(manager->widgetSerial());
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

	object["device_n"] = (int)serialManagers.size();

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

	for (auto& manager : serialManagers)
	{
		delete manager;
	}

	serialManagers.clear();

	__super::closeEvent(event);
}
