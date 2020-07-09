#pragma once

#include <deque>
#include <vector>

struct MultiSerialData
{
	int index;
	std::vector<unsigned char> buffer;
};

class SerialAddinBase
{
	friend class SerialAddinHelper;

protected:
	SerialAddinBase() = default;

public:
	virtual ~SerialAddinBase() = default;

public:
	virtual int maximum() abstract;
	virtual int value() abstract;

	virtual void cancel() abstract;
	virtual bool finished() abstract;

	virtual void callback() abstract;

protected:
	std::deque<MultiSerialData> sendQueue;
	std::deque<MultiSerialData> recvQueue;

};

typedef SerialAddinBase* (*SerialAddin)();
