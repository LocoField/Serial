#pragma once

class SerialAddinBase;
class SerialPort;

class SerialAddinHelper final : public QObject
{
public:
	SerialAddinHelper(SerialAddinBase*);
	~SerialAddinHelper();

public:
	void execute();

protected:
	void perform();
	void finish();

private:
	SerialAddinBase* addin;
	std::vector<SerialPort*> serialPorts;

	QProgressDialog* dialog;
	QTimer* timer;

};
