#pragma once

#include <QtSerialPort/QSerialPort>

class QWidget;
class QLayout;

class CommandSet;

class SerialPort final : protected QSerialPort
{
public:
	SerialPort(int id = 0);
	~SerialPort() = default;

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

public:
	void setAutoRead(bool enable);

	QByteArray read(qint64 maxlen);
	qint64 write(const QByteArray& data);

	bool write(char code);
	bool read(char& code, int timeout = 2000);

protected:
	bool autoRead = true;

	int widgetId = 0;
	QWidget* serialWidget = nullptr;

	std::vector<QWidget*> commandsSetWidget;

};

