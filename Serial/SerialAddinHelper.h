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

	static void onThread(void* arg)
	{
		SerialAddinHelper* pClass = reinterpret_cast<SerialAddinHelper*>(arg);
		pClass->onThreadFunction();
	}

public:
	void start();
	void stop();
	bool isRunning();

protected:
	void onThreadFunction();

	SerialAddinBase* addin;
	std::vector<SerialPort*> serialPorts;

};
