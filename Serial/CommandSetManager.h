#pragma once

#define COMMAND_SET_MANAGER CommandSetManager::getInstance()

class CommandSet
{
public:
	QString comment;
	QString command;
	QString shortcut;
	int timer = 1000;
	int count = 0;
	bool inputAscii;
};

class CommandSetManager
{
protected:
	CommandSetManager();
	~CommandSetManager();

public:
	static CommandSetManager& getInstance();

public:
	bool loadFromFile(const QString& filepath, std::vector<CommandSet>& commandSets);
	bool saveToFile(const QString& filepath, const std::vector<CommandSet>& commandSets);

private:
	static CommandSetManager* instance_;

};

