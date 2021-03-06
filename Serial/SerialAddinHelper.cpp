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

	timer->stop();

	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(true);
	}
}

void SerialAddinHelper::perform()
{
	if (addin->finished())
	{
		dialog->cancel();
		return;
	}

	addin->callback();
	int maximum = addin->maximum();

	if (maximum != 0)
	{
		int value = addin->value();

		dialog->setMaximum(maximum);
		dialog->setValue(value);
	}

	while (addin->sendQueue.size() > 0)
	{
		auto& sendData = addin->sendQueue.front();
		writeAndRead(sendData);
		addin->sendQueue.pop_front();
	}
}

void SerialAddinHelper::cancel()
{
	addin->cancel();

	while (addin->finished() == false)
	{
		addin->callback();

		while (addin->sendQueue.size() > 0)
		{
			auto& sendData = addin->sendQueue.front();
			writeAndRead(sendData);
			addin->sendQueue.pop_front();
		}
	}
}

void SerialAddinHelper::writeAndRead(const SerialData& data)
{
	int begin = data.index;
	int rbegin = data.index;

	if (begin < 0)
	{
		begin = 0;
		rbegin = (int)serialPorts.size() - 1;
	}

	for (int i = begin; i <= rbegin; i++)
	{
		if (serialPorts[i]->isConnected() == false)
			return;

		serialPorts[i]->write({ (char*)data.command.data(), (int)data.command.size() });

		if (data.option > 0)
			Sleep(data.option);


		std::vector<unsigned char> received;

		while (1)
		{
			QByteArray bytes = serialPorts[i]->read();
			if (bytes.isEmpty())
				break;

			received.insert(received.end(), bytes.cbegin(), bytes.cend());

			int length = addin->checkCompleteData({ received.begin(), received.end() });
			if (length == -1)
				continue;

			break;
		}

		addin->recvQueue.push_back({ received, i, 0 });
	}
}
