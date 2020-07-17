#pragma once

#include <deque>
#include <vector>

class SerialData
{
public:
	std::vector<unsigned char> command;

	int index;
	int option;
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
	virtual int checkCompleteData(const std::vector<unsigned char>& data) { return -1; }

protected:
	std::deque<SerialData> sendQueue;
	std::deque<SerialData> recvQueue;

};

typedef SerialAddinBase* (*SerialAddin)();
