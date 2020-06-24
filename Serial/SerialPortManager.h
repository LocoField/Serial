#pragma once

#include <QtSerialPort/QSerialPort>

class QWidget;
class QLayout;

class CommandSet;

class SerialPortManager final : protected QSerialPort
{
public:
	SerialPortManager(int id = 0);
	~SerialPortManager() = default;

public:
	static void availablePorts(std::vector<QString>& ports);

public:
	QWidget* widgetSerial();

protected:
	QLayout* layoutCommandsSet();

protected:
	void makeWidgets();

	bool loadOption();
	bool saveOption();

	void addCommandSet(const CommandSet& commandSet);
	bool loadCommandSets();
	bool saveCommandSets();
	void clearCommandSets();

protected:

	bool connect(QString portName, int baudRate, int mode = 0);
	bool isConnected();
	void disconnect();

	qint64 write(const QByteArray& data);

protected:
	int widgetId = 0;
	QWidget* serialWidget = nullptr;

	std::vector<QWidget*> commandsSetWidget;

};

