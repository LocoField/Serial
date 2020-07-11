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
	if (addin->finished())
	{
		dialog->cancel();
		return;
	}

	addin->callback();

	int value = addin->value();
	dialog->setValue(value);

	while (addin->sendQueue.size() > 0)
	{
		auto& sendData = addin->sendQueue.front();

		int index = sendData.index;
		if (index < 0)
		{
			for (int i = 0; i < (int)serialPorts.size(); i++)
			{
				writeAndRead(i, sendData.buffer);
			}
		}
		else
		{
			writeAndRead(index, sendData.buffer);
		}

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

			int index = sendData.index;
			if (index < 0)
			{
				for (int i = 0; i < (int)serialPorts.size(); i++)
				{
					writeAndRead(i, sendData.buffer);
				}
			}
			else
			{
				writeAndRead(index, sendData.buffer);
			}

			addin->sendQueue.pop_front();
		}
	}
}

void SerialAddinHelper::writeAndRead(int i, const std::vector<unsigned char>& sendData)
{
	if (i < 0 || i >= serialPorts.size())
		return;

	if (serialPorts[i]->isConnected() == false)
		return;

	serialPorts[i]->write({ (char*)sendData.data(), (int)sendData.size() });

	std::vector<unsigned char> recvData;

	while (1)
	{
		QByteArray received = serialPorts[i]->read();
		if (received.isEmpty())
			break;

		std::vector<unsigned char> data = { received.begin(), received.end() };

		int length = addin->checkCompleteData(data);
		if (length > 0)
		{
			recvData.insert(recvData.end(), data.cbegin(), data.cbegin() + length);
			break;
		}

		recvData.insert(recvData.end(), data.cbegin(), data.cend());
	}

	addin->recvQueue.push_back({ i, recvData });
}
