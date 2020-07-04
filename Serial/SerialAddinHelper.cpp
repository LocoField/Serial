#include "stdafx.h"
#include "SerialAddinHelper.h"
#include "SerialAddinBase.h"
#include "SerialPort.h"

SerialAddinHelper::SerialAddinHelper(SerialAddinBase* addin)
	: addin(addin)
{
	
}

SerialAddinHelper::~SerialAddinHelper()
{
}

void SerialAddinHelper::setSerialPorts(const std::vector<SerialPort*>& serialPorts)
{
	this->serialPorts = serialPorts;
}

void SerialAddinHelper::execute()
{
	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(false);
	}

	dialog = new QProgressDialog("Addin is running ...", "Cancel", 0, 100);
	dialog->setMaximum(addin->maximum());
	QObject::connect(dialog, &QProgressDialog::canceled, this, &SerialAddinHelper::cancel);

	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &SerialAddinHelper::perform);

	timer->start(0);

	dialog->exec();

	while (addin->finished() == false)
	{
		Sleep(1000);
	}

	timer->stop();

	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(true);
	}
}

void SerialAddinHelper::perform()
{
	addin->callback();

	int value = addin->value();
	dialog->setValue(value);

	while (addin->sendQueue.size() > 0)
	{
		auto& sendData = addin->sendQueue.front();

		size_t i = sendData.index;
		if (serialPorts[i]->isConnected())
		{
			serialPorts[i]->write({ (char*)sendData.buffer.data(), (int)sendData.buffer.size() });

			QByteArray recvData = serialPorts[i]->read();
			addin->recvQueue.push_back({ i, { recvData.begin(), recvData.end() } });
		}

		addin->sendQueue.pop_front();
	}
}

void SerialAddinHelper::cancel()
{
	addin->cancel();
	addin->callback();

	while (addin->sendQueue.size() > 0)
	{
		auto& sendData = addin->sendQueue.front();

		size_t i = sendData.index;
		if (serialPorts[i]->isConnected())
		{
			serialPorts[i]->write({ (char*)sendData.buffer.data(), (int)sendData.buffer.size() });

			QByteArray recvData = serialPorts[i]->read();
			addin->recvQueue.push_back({ i, { recvData.begin(), recvData.end() } });
		}

		addin->sendQueue.pop_front();
	}
}
