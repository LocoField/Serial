#include "stdafx.h"
#include "CommandSetManager.h"

CommandSetManager* CommandSetManager::instance_(nullptr);

CommandSetManager::CommandSetManager()
{
}

CommandSetManager::~CommandSetManager()
{
}

CommandSetManager& CommandSetManager::getInstance()
{
	if (instance_ == nullptr)
	{
		instance_ = new CommandSetManager;
	}

	return *instance_;
}

bool CommandSetManager::loadFromFile(const QString& filepath, std::vector<CommandSet>& commandSets)
{
	QFile loadFile(filepath);
	if (loadFile.open(QIODevice::ReadOnly) == false)
		return false;

	QJsonDocument doc = QJsonDocument::fromJson(loadFile.readAll());
	if (doc.isNull())
		return false;

	if (doc.isArray() == false)
		return false;

	QJsonArray jsonArray(doc.array());

	commandSets.clear();
	commandSets.reserve(jsonArray.size());

	for (const auto& arrayValue : jsonArray)
	{
		QJsonObject object = arrayValue.toObject();

		CommandSet commandSet;
		commandSet.comment = object["comment"].toString();
		commandSet.command = object["command"].toString();
		commandSet.shortcut = object["shortcut"].toString();
		commandSet.timer = object["timer"].toInt();
		commandSet.count = object["count"].toInt();
		commandSet.inputAscii = object["ascii"].toBool();

		commandSets.push_back(commandSet);
	}

	return true;
}

bool CommandSetManager::saveToFile(const QString& filepath, const std::vector<CommandSet>& commandSets)
{
	QFile loadFile(filepath);
	if (loadFile.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
		return false;

	QJsonArray jsonArray;
	for (auto& commandSet : commandSets)
	{
		QJsonObject object;
		object["comment"] = commandSet.comment;
		object["command"] = commandSet.command;
		object["shortcut"] = commandSet.shortcut;
		object["timer"] = commandSet.timer;
		object["count"] = commandSet.count;
		object["ascii"] = commandSet.inputAscii;

		jsonArray.push_back(object);
	}

	QJsonDocument jsonDocument(jsonArray);
	loadFile.write(jsonDocument.toJson(QJsonDocument::JsonFormat::Indented));
	loadFile.close();

	return true;
}
