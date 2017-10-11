#include "command.hpp"
#include "time.hpp"
#include "utility.hpp"
#include <thread>

Command::Command(std::string name, std::string description, std::string usage): name(std::move(name)), description(std::move(description)), usage(std::move(usage)){}

const std::string& Command::get_name() const
{
	return this->name;
}

const std::string& Command::get_description() const
{
	return this->description;
}

const std::string& Command::get_usage() const
{
	return this->usage;
}

std::size_t Command::get_expected_parameter_size() const
{
	// equal to number of spaces in the arguments + 1 because we want the number of words in the sentence including the first argument as that does not equal the command name in Topaz.
	return tz::util::string::split_string(this->usage, " ").size() + 1;
}

bool Command::operator==(const Command& rhs) const
{
	// is a shallow equality operator, default works the same
	return this->name == rhs.get_name() && this->description == rhs.get_description() && this->usage == rhs.get_usage();
}

void TrivialCommand::operator()()
{
	// just call operator()(args...) where args is empty. optimiser should prevent the unnecessary construction of the vector
	this->operator()(std::vector<std::string>());
}

const std::unordered_set<Command*>& CommandExecutor::get_commands() const
{
	return this->commands;
}

void CommandExecutor::register_command(Command* command)
{
	this->commands.insert(command);	
}

void CommandExecutor::deregister_command(Command* command)
{
	this->commands.erase(command);
}

void CommandExecutor::deregister_command(const std::string& command_name)
{
	for(auto& command : this->commands)
		if(command->get_name() == command_name)
			this->deregister_command(command);
}

void CommandExecutor::operator()(const std::string& name, const std::vector<std::string>& args)
{
	for(auto command : this->commands)
		if(command->get_name() == name)
			(*command)(args);
}