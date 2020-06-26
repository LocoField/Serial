#pragma once

class SerialAddinBase;
class SerialPort;

class SerialAddinHelper final
{
public:
	SerialAddinHelper(SerialAddinBase*);
	~SerialAddinHelper();

private:
	bool running = false;
	bool finished = true;

	static void onThread(void* arg)
	{
		SerialAddinHelper* pClass = reinterpret_cast<SerialAddinHelper*>(arg);
		pClass->onThreadFunction();
	}

public:
	bool start();
	void stop();

protected:
	void onThreadFunction();

	SerialAddinBase* addin;
	std::vector<SerialPort*> serialPorts;

};
