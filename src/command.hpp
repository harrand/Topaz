#ifndef COMMAND_HPP
#define COMMAND_HPP
#include <unordered_set>
#include <vector>
#include <memory>

/*
	Abstract. Not available for non-polymorphic use. Inherit from this to create custom commands (Essential for adding functionality to Engine).
*/
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

/*
	Exactly the same as Command. However, does not support 'usage' nor command arguments. This is used as a wrapper for an invokable to be used in Engine.
*/
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

/*
	TrivialCommand subclass. Templated such that it can take a lambda. Essentially is a lambda-wrapper that can be treated like a command (This is how CommandExecutors can execute lambdas)
*/
template<typename Functor>
class TrivialFunctor : public TrivialCommand
{
public:
	TrivialFunctor(Functor functor);
	virtual void operator()() override;
private:
	Functor functor;
};

/*
	System used to hold (but not typically own) Commands. Engine uses these to handle command input.
*/
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

#include "command.inl"

#endif