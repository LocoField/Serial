#pragma once

#include <deque>
#include <vector>

enum class AddinType
{
	ADDIN_TYPE_PROGRESS,
	ADDIN_TYPE_LOOP,
};

struct MultiSerialData
{
	size_t index;
	std::vector<char> buffer;
};

class SerialAddinBase
{
	friend class SerialAddinHelper;

protected:
	SerialAddinBase() = default;

public:
	virtual ~SerialAddinBase() = default;

public:
	virtual AddinType type() abstract;
	virtual int status() abstract;
	virtual void callback() abstract;

protected:
	std::deque<MultiSerialData> sendQueue;
	std::deque<MultiSerialData> recvQueue;

};

typedef SerialAddinBase* (*SerialAddin)();
