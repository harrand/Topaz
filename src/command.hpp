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
	
	const std::string& get_name() const;
	const std::string& get_description() const;
	const std::string& get_usage() const;
	std::size_t get_expected_parameter_size() const;
	virtual bool operator==(const Command& rhs) const;
	virtual void operator()(const std::vector<std::string>& args) = 0;
private:
	std::string name;
	std::string description;
	std::string usage;
};

class TrivialCommand : public Command
{
public:
	TrivialCommand(std::string name = "", std::string description = ""): Command(name, description, ""){}
	TrivialCommand(const TrivialCommand& copy) = default;
	TrivialCommand(TrivialCommand&& move) = default;
	~TrivialCommand() = default;
	TrivialCommand& operator=(const TrivialCommand& rhs) = default;
	
	virtual void operator()() = 0;
private:
	void operator()([[maybe_unused]] const std::vector<std::string>& args) final{operator()();}
	using Command::get_usage;
	using Command::get_expected_parameter_size;
};

class CommandExecutor
{
public:
	CommandExecutor() = default;
	CommandExecutor(const CommandExecutor& copy) = default;
	CommandExecutor(CommandExecutor&& move) = default;
	CommandExecutor& operator=(const CommandExecutor& rhs) = default;
	~CommandExecutor() = default;
	
	const std::unordered_set<Command*>& get_commands() const;
	void register_command(Command* command);
	void deregister_command(Command* command);
	void deregister_command(const std::string& command_name);
	void operator()(const std::string& name, const std::vector<std::string>& args = std::vector<std::string>());
private:
	std::unordered_set<Command*> commands;
};

#endif