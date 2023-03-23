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

	void addCommandSetLayout(const CommandSet& commandSet);

	bool loadCommandSet(const QString& filePath);
	bool saveCommandSet();
	void clearCommandSet();

protected:
	bool connect(QString portName, int baudRate, int mode = 0);
	void disconnect();

public:
	bool isConnected();

	void setAutoRead(bool enable);

	QByteArray read(int timeout = 2000);
	qint64 write(const QByteArray& data);

	bool write(char code);
	bool read(char& code, int timeout = 2000);

	void addCommandSeparator(const QString& text);

protected:
	bool autoRead = true;

	int widgetId = 0;
	QWidget* serialWidget = nullptr;

	std::vector<QWidget*> commandsSetWidget;
	std::vector<QString> commandSeparator;

};

