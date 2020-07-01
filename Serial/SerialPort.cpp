#include "stdafx.h"
#include "SerialPort.h"
#include "CommandSetManager.h"

SerialPort::SerialPort(int id)
	: widgetId(id)
{
	makeWidgets();

	QObject::connect(this, &QIODevice::readyRead, [&]()
	{
		if (autoRead == false)
			return;

		QByteArray data = readAll();

		auto widget = serialWidget->findChild<QListWidget*>("listCommands");
		if (widget)
		{
			QString command;

			for (auto it = data.cbegin(); it != data.cend(); ++it)
			{
				unsigned char hex = *it;
				QString hex_format = QString("%1 ").arg(hex, 2, 16, QLatin1Char('0'));

				command.append(hex_format);
			}

			widget->addItem(command);
			widget->setCurrentRow(widget->count() - 1);
		}
	});

	QObject::connect(this, &QSerialPort::errorOccurred, [&](QSerialPort::SerialPortError error)
	{
		if (error == QSerialPort::SerialPortError::ResourceError)
		{
			auto widget = serialWidget->findChild<QPushButton*>("pushButtonConnect");
			if (widget)
			{
				widget->setChecked(false);

				cout << "ERROR: device disconnected." << endl;
			}
		}
	});
}

void SerialPort::availablePorts(std::vector<QString>& ports)
{
	auto p = QSerialPortInfo::availablePorts();

	ports.clear();
	ports.resize(p.size());

	std::transform(p.cbegin(), p.cend(), ports.begin(), [](const QSerialPortInfo& info)
	{
		return info.portName();
	});
}

QWidget* SerialPort::widgetSerial()
{
	return serialWidget;
}

QLayout* SerialPort::layoutCommandsSet()
{
	auto widget = serialWidget->findChild<QWidget*>("widgetCommandsSet");
	return widget->layout();
}

void SerialPort::makeWidgets()
{
	auto regexHexValidator = new QRegExpValidator;
	regexHexValidator->setRegExp(QRegExp("([0-9a-fA-F]{2}\\s?)+"));

	auto mainLayout = new QVBoxLayout;
	auto groupBoxSerial = new QGroupBox;
	auto groupBoxCommand = new QGroupBox;

	mainLayout->addWidget(groupBoxSerial);
	mainLayout->addWidget(groupBoxCommand);

	serialWidget = new QWidget;
	serialWidget->setLayout(mainLayout);

	// serial box
	{
		auto verticalLayout = new QVBoxLayout;

		auto horizontalLayout = new QHBoxLayout;
		auto lineEditSerialPortName = new QLineEdit;
		auto comboBoxBaudRate = new QComboBox;
		auto comboBoxProtocol = new QComboBox;
		auto pushButtonConnect = new QPushButton("Connect");
		auto horizontalSpacer = new QSpacerItem(40, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
		auto buttonCommandsClear = new QPushButton("Clear");

		auto listCommands = new QListWidget;

		auto lineEditSendCommand = new QLineEdit;

		lineEditSerialPortName->setPlaceholderText("Press return key");
		lineEditSerialPortName->setObjectName("lineEditSerialPortName");

		comboBoxBaudRate->addItems({ "115200", "57600", "38400", "19200", "9600" });
		comboBoxBaudRate->setObjectName("comboBoxBaudRate");

		comboBoxProtocol->addItems({ "8N1", "8E1", "8O1" });
		comboBoxProtocol->setObjectName("comboBoxProtocol");

		pushButtonConnect->setCheckable(true);
		pushButtonConnect->setObjectName("pushButtonConnect");

		listCommands->setMinimumSize(600, 300);
		listCommands->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		listCommands->setObjectName("listCommands");

		lineEditSendCommand->setValidator(regexHexValidator);
		lineEditSendCommand->setEnabled(false);

		horizontalLayout->addWidget(lineEditSerialPortName);
		horizontalLayout->addWidget(comboBoxBaudRate);
		horizontalLayout->addWidget(comboBoxProtocol);
		horizontalLayout->addWidget(pushButtonConnect);
		horizontalLayout->addItem(horizontalSpacer);
		horizontalLayout->addWidget(buttonCommandsClear);

		verticalLayout->addLayout(horizontalLayout);
		verticalLayout->addWidget(listCommands);
		verticalLayout->addWidget(lineEditSendCommand);

		groupBoxSerial->setLayout(verticalLayout);

		//////////////////////////////////////////////////////////////////////////

		QObject::connect(lineEditSerialPortName, &QLineEdit::returnPressed, [lineEditSerialPortName]()
			{
				std::vector<QString> ports;
				SerialPort::availablePorts(ports);

				QString current = lineEditSerialPortName->text();

				for (const QString& portName : ports)
				{
					if (current != portName)
						lineEditSerialPortName->setText(portName);
				}
			}
		);

		QObject::connect(pushButtonConnect, &QPushButton::toggled,
			[this, pushButtonConnect, lineEditSendCommand, lineEditSerialPortName, comboBoxBaudRate, comboBoxProtocol]()
			{
				if (isConnected())
				{
					disconnect();

					pushButtonConnect->setText("Connect");
					pushButtonConnect->setChecked(false);

					lineEditSendCommand->setEnabled(false);
				}
				else
				{
					bool re = connect(lineEditSerialPortName->text(), comboBoxBaudRate->currentText().toInt(), comboBoxProtocol->currentIndex());
					if (re)
					{
						pushButtonConnect->setText("Disconnect");
						lineEditSendCommand->setEnabled(true);

						saveOption();
					}
					else
					{
						pushButtonConnect->setChecked(false);
					}
				}
			}
		);

		QObject::connect(buttonCommandsClear, &QPushButton::clicked,
			[listCommands]()
			{
				listCommands->clear();
			}
		);

		QObject::connect(lineEditSendCommand, &QLineEdit::returnPressed,
			[this, lineEditSendCommand]()
			{
				QString command = lineEditSendCommand->text();
				command.remove(QRegExp("\\s"));

				QByteArray data;

				for (int index = 0; index < command.length(); index += 2)
				{
					QString parsed = command.mid(index, 2);

					auto value = QByteArray::fromHex(parsed.toLatin1());
					data.append(value);
				}

				if (write(data) == false)
				{
					cout << "send failed: " << data.toStdString() << endl;
				}
			}
		);
	}

	// command box
	{
		auto verticalLayout = new QVBoxLayout;

		auto horizontalLayout = new QHBoxLayout;
		auto pushButtonLoad = new QPushButton("Load");
		auto pushButtonSave = new QPushButton("Save");
		auto pushButtonClear = new QPushButton("Clear");

		auto widget = new QWidget;
		auto layout = new QVBoxLayout;

		widget->setLayout(layout);
		widget->setObjectName("widgetCommandsSet");

		horizontalLayout->addWidget(pushButtonLoad);
		horizontalLayout->addWidget(pushButtonSave);
		horizontalLayout->addWidget(pushButtonClear);

		verticalLayout->addLayout(horizontalLayout);
		verticalLayout->addWidget(widget);

		groupBoxCommand->setLayout(verticalLayout);

		//////////////////////////////////////////////////////////////////////////

		QObject::connect(pushButtonLoad, &QPushButton::clicked,
			[this]()
			{
				loadCommandSets();
			}
		);

		QObject::connect(pushButtonSave, &QPushButton::clicked,
			[this]()
			{
				saveCommandSets();
			}
		);

		QObject::connect(pushButtonClear, &QPushButton::clicked,
			[this]()
			{
				clearCommandSets();
			}
		);

		loadOption();
		loadCommandSets();
	}
}

bool SerialPort::loadOption()
{
	bool retval = true;

	QString filepath = QCoreApplication::applicationDirPath();
	QFile loadFile(filepath + "/s" + QString::number(widgetId));

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

			auto lineEditSerialPortName = serialWidget->findChild<QLineEdit*>("lineEditSerialPortName");
			auto comboBoxBaudRate = serialWidget->findChild<QComboBox*>("comboBoxBaudRate");
			auto comboBoxProtocol = serialWidget->findChild<QComboBox*>("comboBoxProtocol");

			lineEditSerialPortName->setText(object["default_port"].toString());
			comboBoxBaudRate->setCurrentIndex(object["default_baudrate"].toInt());
			comboBoxProtocol->setCurrentIndex(object["default_protocol"].toInt());
		}

		loadFile.close();
	}
	else
	{
		retval = false;
	}

	return retval;
}

bool SerialPort::saveOption()
{
	QString filepath = QCoreApplication::applicationDirPath();
	QFile loadFile(filepath + "/s" + QString::number(widgetId));

	if (loadFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
		return false;

	auto lineEditSerialPortName = serialWidget->findChild<QLineEdit*>("lineEditSerialPortName");
	auto comboBoxBaudRate = serialWidget->findChild<QComboBox*>("comboBoxBaudRate");
	auto comboBoxProtocol = serialWidget->findChild<QComboBox*>("comboBoxProtocol");

	QJsonObject object;
	object["default_port"] = lineEditSerialPortName->text();
	object["default_baudrate"] = comboBoxBaudRate->currentIndex();
	object["default_protocol"] = comboBoxProtocol->currentIndex();

	QJsonDocument jsonDocument(object);
	loadFile.write(jsonDocument.toJson(QJsonDocument::JsonFormat::Indented));
	loadFile.close();

	return true;
}

void SerialPort::addCommandSet(const CommandSet& commandSet)
{
	auto widget = new QWidget;
	{
		auto checkAsciiCommand = new QCheckBox("ASCII");
		{
			checkAsciiCommand->setChecked(commandSet.inputAscii);
			checkAsciiCommand->setFixedWidth(60);
			checkAsciiCommand->setDisabled(true);
		}

		auto labelComment = new QLabel;
		{
			labelComment->setObjectName("labelComment");
			labelComment->setText(commandSet.comment);
			labelComment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		}

		auto lineEditCommand = new QLineEdit;
		{
			lineEditCommand->setObjectName("lineEditCommand");
			lineEditCommand->setText(commandSet.command);
			lineEditCommand->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			lineEditCommand->setDisabled(true);
		}

		auto spinBoxTimer = new QSpinBox;
		{
			spinBoxTimer->setObjectName("spinBoxTimer");
			spinBoxTimer->setRange(100, 10000);
			spinBoxTimer->setSingleStep(100);
			spinBoxTimer->setValue(commandSet.timer);
			spinBoxTimer->setDisabled(true);
		}

		auto spinBoxCount = new QSpinBox;
		{
			spinBoxCount->setObjectName("spinBoxCount");
			spinBoxCount->setRange(0, 1000);
			spinBoxCount->setSingleStep(1);
			spinBoxCount->setValue(commandSet.count);
			spinBoxCount->setDisabled(true);
		}

		auto buttonExecute = new QPushButton;
		{
			buttonExecute->setObjectName("buttonExecute");
			buttonExecute->setText("Send");
			buttonExecute->setShortcut(QKeySequence(commandSet.shortcut));

			auto commandExecute = [&, checkAsciiCommand, lineEditCommand]()
			{
				QString command = lineEditCommand->text();
				QByteArray data;

				if (checkAsciiCommand->isChecked())
				{
					data = command.toLocal8Bit();
				}
				else
				{
					
					command.remove(QRegExp("\\s"));

					for (int index = 0; index < command.length(); index += 2)
					{
						QString parsed = command.mid(index, 2);

						auto value = QByteArray::fromHex(parsed.toLatin1());
						data.append(value);
					}
				}

				if (write(data) == false)
				{
					cout << "send failed: " << data.toStdString() << endl;
				}
			};

			auto timer = new QTimer(buttonExecute);
			QObject::connect(timer, &QTimer::timeout, [spinBoxCount, buttonExecute, commandExecute]()
				{
					// 버튼에서 실행하는 함수를 실행한다.
					commandExecute();

					int count = spinBoxCount->value() - 1;
					spinBoxCount->setValue(count);

					// 버튼을 눌러서 타이머를 정지시킨다.
					if (count == 0)
						buttonExecute->click();
				}
			);

			QObject::connect(buttonExecute, &QPushButton::clicked, [lineEditCommand, spinBoxTimer, spinBoxCount, buttonExecute, commandExecute, timer]()
				{
					// 기존 타이머를 정지한다.
					if (timer->isActive())
					{
						lineEditCommand->setEnabled(true);
						spinBoxTimer->setEnabled(true);
						spinBoxCount->setEnabled(true);
						buttonExecute->setChecked(false);
						timer->stop();

						return;
					}

					if (spinBoxCount->value() > 0)
					{
						lineEditCommand->setEnabled(false);
						spinBoxTimer->setEnabled(false);
						spinBoxCount->setEnabled(false);
						buttonExecute->setCheckable(true);
						buttonExecute->setChecked(true);
						timer->start(spinBoxTimer->value());
					}
					else
					{
						buttonExecute->setCheckable(false);
					}

					commandExecute();
				}
			);
		}

		auto layoutWidget = new QHBoxLayout;
		layoutWidget->addWidget(checkAsciiCommand);
		layoutWidget->addWidget(labelComment);
		layoutWidget->addWidget(lineEditCommand);
		layoutWidget->addWidget(spinBoxTimer);
		layoutWidget->addWidget(spinBoxCount);
		layoutWidget->addWidget(buttonExecute);

		widget->setLayout(layoutWidget);

		commandsSetWidget.emplace_back(widget);
	}

	auto layout = layoutCommandsSet();
	layout->addWidget(widget);
}

bool SerialPort::loadCommandSets()
{
	QString filepath = QCoreApplication::applicationDirPath();

	std::vector<CommandSet> commandSets;
	if (COMMAND_SET_MANAGER.loadFromFile(filepath + "/c" + QString::number(widgetId), commandSets) == false)
		return false;

	for (auto& commandSet : commandSets)
	{
		addCommandSet(commandSet);
	}

	return true;
}

bool SerialPort::saveCommandSets()
{
	QString filepath = QCoreApplication::applicationDirPath();

	std::vector<CommandSet> commandSets;
	commandSets.reserve(commandsSetWidget.size());

	for (auto& widget : commandsSetWidget)
	{
		auto labelComment = widget->findChild<QLabel*>("labelComment");
		auto lineEditCommand = widget->findChild<QLineEdit*>("lineEditCommand");
		auto spinBoxTimer = widget->findChild<QSpinBox*>("spinBoxTimer");
		auto spinBoxCount = widget->findChild<QSpinBox*>("spinBoxCount");
		auto buttonExecute = widget->findChild<QPushButton*>("buttonExecute");

		CommandSet commandSet;
		commandSet.comment = labelComment->text();
		commandSet.command = lineEditCommand->text();
		commandSet.shortcut = buttonExecute->shortcut().toString();
		commandSet.timer = spinBoxTimer->value();
		commandSet.count = spinBoxCount->value();

		commandSets.push_back(commandSet);
	}

	if (COMMAND_SET_MANAGER.saveToFile(filepath + "/c" + QString::number(widgetId), commandSets) == false)
		return false;

	return true;
}

void SerialPort::clearCommandSets()
{
	auto layout = layoutCommandsSet();

	QLayoutItem* item;
	while ((item = layout->takeAt(0)))
	{
		delete item->widget();
		delete item;
	}

	commandsSetWidget.clear();
}

bool SerialPort::connect(QString portName, int baudRate, int mode)
{
	setPortName(portName);
	setBaudRate(baudRate);
	setDataBits(QSerialPort::Data8);
	setFlowControl(QSerialPort::NoFlowControl);
	setParity(QSerialPort::NoParity);
	setStopBits(QSerialPort::OneStop);

	switch (mode)
	{
		case 1:
		{
			setParity(QSerialPort::EvenParity);
			break;
		}
		case 2:
		{
			setParity(QSerialPort::OddParity);
			break;
		}
	}

	return __super::open(QIODevice::ReadWrite);
}

bool SerialPort::isConnected()
{
	return __super::isOpen();
}

void SerialPort::disconnect()
{
	__super::close();
}

void SerialPort::setAutoRead(bool enable)
{
	autoRead = enable;
}

QByteArray SerialPort::read(int timeout)
{
	if (autoRead)
		return QByteArray();

	if (waitForReadyRead(timeout))
	{
		return __super::readAll();
	}

	return QByteArray();
}

qint64 SerialPort::write(const QByteArray& data)
{
	return __super::write(data) == data.length();
}

bool SerialPort::write(char code)
{
	putChar(code);
	return waitForBytesWritten();
}

bool SerialPort::read(char& code, int timeout)
{
	if (waitForReadyRead(timeout))
	{
		getChar(&code);
		return true;
	}

	return false;
}
