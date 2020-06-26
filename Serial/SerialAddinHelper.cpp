#include "stdafx.h"
#include "SerialAddinHelper.h"
#include "SerialAddinBase.h"
#include "SerialPort.h"

#include <thread>

SerialAddinHelper::SerialAddinHelper(SerialAddinBase* addin)
	: addin(addin)
{
}

SerialAddinHelper::~SerialAddinHelper()
{
	stop();
}

bool SerialAddinHelper::start()
{
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
		for (size_t i = 0; i < serialPorts.size(); i++)
		{
			QByteArray buffer = serialPorts[i]->read(1);
			if (buffer.isEmpty())
				continue;

			addin->recvQueue.push_back({ i, {buffer.begin(), buffer.end()} });
		}

		addin->callback();

		while (addin->sendQueue.size() > 0)
		{
			auto& data = addin->sendQueue.front();
			serialPorts[data.index]->write({ data.buffer.data(), (int)data.buffer.size() });
		}
	}

	finished = true;
}
