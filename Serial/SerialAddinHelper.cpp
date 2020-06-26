#include "stdafx.h"
#include "SerialAddinHelper.h"
#include "SerialAddinBase.h"
#include "SerialPort.h"

#include <thread>

SerialAddinHelper::SerialAddinHelper()
{
}

SerialAddinHelper::~SerialAddinHelper()
{
	stop();
}

bool SerialAddinHelper::start()
{
	if (addin == nullptr)
		return false;

	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(false);
	}

	running = true;
	finished = false;
	std::thread(onThread, this).detach();

	return true;
}

void SerialAddinHelper::stop()
{
	running = false;

	while (finished == false)
		Sleep(100);

	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(true);
	}
}

void SerialAddinHelper::onThreadFunction()
{
	while (running)
	{
		addin->callback();

		while (addin->sendQueue.size() > 0)
		{
			auto& data = addin->sendQueue.front();
			serialPorts[data.index]->write({ data.buffer.data(), (int)data.buffer.size() });
		}
	}

	finished = true;
}
