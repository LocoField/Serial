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
	QObject::connect(dialog, &QProgressDialog::canceled, this, &SerialAddinHelper::finish);

	if (addin->type() == AddinType::ADDIN_TYPE_LOOP)
	{
		dialog->setCancelButtonText("Stop");
		dialog->setRange(0, 0);
	}

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

	int status = addin->status();
	if (status < 0)
	{
		dialog->setValue(dialog->maximum());
		return;
	}

	dialog->setValue(status);

	while (addin->sendQueue.size() > 0)
	{
		auto& data = addin->sendQueue.front();
		serialPorts[data.index]->write({ data.buffer.data(), (int) data.buffer.size() });
	}

	dialog->setValue(status);
}

void SerialAddinHelper::finish()
{
	timer->stop();

	for (auto& serialPort : serialPorts)
	{
		serialPort->setAutoRead(true);
	}
}
