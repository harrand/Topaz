#include "command.hpp"
#include "time.hpp"
#include "utility.hpp"
#include <thread>

Command::Command(std::string name, std::string description, std::string usage): name(std::move(name)), description(std::move(description)), usage(std::move(usage)){}

const std::string& Command::getName() const
{
	return this->name;
}

const std::string& Command::getDescription() const
{
	return this->description;
}

const std::string& Command::getUsage() const
{
	return this->usage;
}

std::size_t Command::getExpectedParameterSize() const
{
	// equal to number of spaces in the arguments + 1 because we want the number of words in the sentence including the first argument as that does not equal the command name in Topaz.
	return tz::util::string::splitString(this->usage, " ").size() + 1;
}

bool Command::operator==(const Command& rhs) const
{
	// is a shallow equality operator, default works the same
	return this->name == rhs.getName() && this->description == rhs.getDescription() && this->usage == rhs.getUsage();
}

void TrivialCommand::operator()()
{
	// just call operator()(args...) where args is empty. optimiser should prevent the unnecessary construction of the vector
	this->operator()(std::vector<std::string>());
}

const std::unordered_set<Command*>& CommandExecutor::getCommands() const
{
	return this->commands;
}

void CommandExecutor::registerCommand(Command* command)
{
	this->commands.insert(command);	
}

void CommandExecutor::deregisterCommand(Command* command)
{
	this->commands.erase(command);
}

void CommandExecutor::deregisterCommand(const std::string& command_name)
{
	for(auto& command : this->commands)
		if(command->getName() == command_name)
			this->deregisterCommand(command);
}

void CommandExecutor::operator()(const std::string& name, const std::vector<std::string>& args)
{
	for(auto command : this->commands)
		if(command->getName() == name)
			(*command)(args);
}