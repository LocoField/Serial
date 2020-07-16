#pragma once

class SerialAddinBase;
class SerialPort;
class SerialData;

class SerialAddinHelper final : public QObject
{
public:
	SerialAddinHelper(SerialAddinBase*);
	~SerialAddinHelper();

public:
	void setSerialPorts(const std::vector<SerialPort*>& serialPorts);
	void execute();

protected:
	void perform();
	void cancel();

private:
	void writeAndRead(const SerialData& data);

private:
	SerialAddinBase* addin;
	std::vector<SerialPort*> serialPorts;

	QProgressDialog* dialog;
	QTimer* timer;

};
