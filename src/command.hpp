#ifndef COMMAND_HPP
#define COMMAND_HPP
#include <unordered_set>
#include <vector>
#include <memory>
#include <functional>

/**
* Abstract. Not available for non-polymorphic use.
* Represents a hard-coded functor with string arguments.
* Inherit from this to create custom commands (Essential for adding functionality to Engine).
* For simpler functions with no parameters, use a TrivialCommand instead (see lower down this source file).
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
protected:
	bool is_trivial;
};

/**
* Exactly the same as Command. However, does not support 'usage' nor command arguments.
* This is used as a wrapper for an invokable to be used in Engine.
* This is an abstract class. To utilise your own TrivialCommands, create classes which inherit and override virtual void operator()() to provide your desired functionality.
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

template<typename Functor>
class TrivialFunctor : public TrivialCommand
{
public:
    TrivialFunctor(Functor&& functor);
    virtual void operator()() override;
protected:
    Functor functor;
};

template<typename Functor, typename... FunctorParameters>
class StaticFunctor : public TrivialCommand
{
public:
    StaticFunctor(Functor&&, FunctorParameters&&... parameters);
    virtual void operator()() override;
protected:
    Functor functor;
    std::tuple<FunctorParameters...> parameters;
};

/**
* System used to hold (but not typically own) Commands.
* Engine uses these to handle command input.
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

namespace tz::util::scheduler
{
	/**
	* Run a functor in the same thread after a specified delay.
	* Note that this does indeed make the thread sleep, so use this on an empty thread or use tz::util::scheduler::async_delayed_functor.
	*/
	template<typename Functor>
	inline void sync_delayed_functor(unsigned int millis_delay, TrivialFunctor<Functor> command);
	/**
	* Sleep in a new thread for a specified delay, and then execute a functor.
	*/
	template<typename Functor>
	inline void async_delayed_functor(unsigned int millis_delay, TrivialFunctor<Functor> command);
}

#include "command.inl"

#endif