#ifndef COMMAND_HPP
#define COMMAND_HPP
#include <unordered_set>
#include <vector>

class Command
{
public:
	Command(std::string name = "", std::string description = "", std::string usage = "");
	Command(const Command& copy) = default;
	Command(Command&& move) = default;
	Command& operator=(const Command& rhs) = default;
	~Command() = default;
	
	const std::string& getName() const;
	const std::string& getDescription() const;
	const std::string& getUsage() const;
	std::string& getUsageR();
	std::string& getNameR();
	std::string& getDescriptionR();
	std::size_t getExpectedParameterSize() const;
	virtual bool operator==(const Command& rhs) const;
	virtual void operator()(const std::vector<std::string>& args = std::vector<std::string>()) = 0;
private:
	std::string name;
	std::string description;
	std::string usage;
};

class CommandExecutor
{
public:
	CommandExecutor() = default;
	CommandExecutor(const CommandExecutor& copy) = default;
	CommandExecutor(CommandExecutor&& move) = default;
	CommandExecutor& operator=(const CommandExecutor& rhs) = default;
	~CommandExecutor() = default;
	
	const std::unordered_set<Command*>& getCommands() const;
	std::unordered_set<Command*>& getCommandsR();
	void registerCommand(Command* command);
	void deregisterCommand(Command* command);
	void deregisterCommand(const std::string& command_name);
	void operator()(const std::string& name, const std::vector<std::string>& args = std::vector<std::string>());
private:
	std::unordered_set<Command*> commands;
};

#endif