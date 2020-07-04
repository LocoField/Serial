#pragma once

class SerialAddinBase;
class SerialPort;

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
	SerialAddinBase* addin;
	std::vector<SerialPort*> serialPorts;

	QProgressDialog* dialog;
	QTimer* timer;

};
