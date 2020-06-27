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

void SerialAddinHelper::start()
{
	running = true;
	std::thread(onThread, this).detach();
}

void SerialAddinHelper::stop()
{
	running = false;
}

bool SerialAddinHelper::isRunning()
{
	return running;
}

void SerialAddinHelper::onThreadFunction()
{
	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(false);
	}

	while (running)
	{
		for( size_t i = 0; i < serialPorts.size();)
		{
			QByteArray buffer = serialPorts[i]->read(1);
			if( buffer.isEmpty() )
			{
				i++;
			}

			addin->recvQueue.push_back({ i, {buffer.begin(), buffer.end()} });
		}

		addin->callback();

		if (addin->status() < 0)
			break;

		while (addin->sendQueue.size() > 0)
		{
			auto& data = addin->sendQueue.front();
			serialPorts[data.index]->write({ data.buffer.data(), (int)data.buffer.size() });
		}
	}

	running = false;

	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(true);
	}
}
