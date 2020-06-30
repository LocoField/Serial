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

void SerialAddinHelper::execute()
{
	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(false);
	}

	dialog = new QProgressDialog("Addin is running ...", "Cancel", 0, 100);
	dialog->setAutoReset(false);
	dialog->setMaximum(addin->maximum());
	QObject::connect(dialog, &QProgressDialog::canceled, this, &SerialAddinHelper::finish);

	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &SerialAddinHelper::perform);

	timer->start(0);
	dialog->exec();

	finish();
}

void SerialAddinHelper::perform()
{
	for (size_t i = 0; i < serialPorts.size(); )
	{
		QByteArray buffer = serialPorts[i]->read(1);
		if (buffer.isEmpty())
		{
			i++;
		}

		addin->recvQueue.push_back({ i, { buffer.begin(), buffer.end() } });
	}

	addin->callback();

	if (addin->finished())
	{
		dialog->reset();
		return;
	}

	int value = addin->value();
	dialog->setValue(value);

	while (addin->sendQueue.size() > 0)
	{
		auto& data = addin->sendQueue.front();
		serialPorts[data.index]->write({ data.buffer.data(), (int) data.buffer.size() });
	}
}

void SerialAddinHelper::finish()
{
	timer->stop();

	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(true);
	}
}
