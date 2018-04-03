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
	/**
	 * Construct a command from all specifications.
	 * @param name - Name to represent the Command
	 * @param description - Brief description of what the command does
	 * @param usage - Explanation of parameters of the Command
	 * @param trivial - Query whether the Command has any parameters or not
	 */
	Command(std::string name = "", std::string description = "", std::string usage = "", bool trivial = true);

	/**
	 * Read the name of the Command.
	 * @return - Name that represents the Command.
	 */
	const std::string& get_name() const;
	/**
	 * Read the brief description of what the Command does.
	 * @return - Brief description of the command.
	 */
	const std::string& get_description() const;
	/**
	 * Read how the parameters of the Command should be used.
	 * @return - Explanation of parameters of the Command.
	 */
	const std::string& get_usage() const;
	/**
	 * Query how many parameters this Command expects.
	 * @return - Number of parameters the Command requires.
	 */
	std::size_t get_expected_parameter_size() const;
	/**
	 * Equate this Command with another.
	 * @param rhs - The Command to equate this with
	 * @return - True if the Commands are equal. False otherwise
	 */
	virtual bool operator==(const Command& rhs) const;
	/**
	 * Pure virtual. Invoked when the Command should be executed.
	 * @param args - N/A
	 */
	virtual void operator()(const std::vector<std::string>& args) = 0;
	/**
	 * Query whether the Command is trivial or not.
	 * @return - True if the Command is trivial. False otherwise
	 */
    bool is_trivial() const;
private:
	/// Name of the Command.
	std::string name;
	/// Brief description of the Command.
	std::string description;
	/// Usage of the Command parameters.
	std::string usage;
protected:
	// Stores whether the Command requires parameters or not.
	bool trivial;
};

/**
* Exactly the same as Command. However, does not support 'usage' nor command arguments.
* This is used as a wrapper for an invokable to be used in Engine.
* This is an abstract class. To utilise your own TrivialCommands, create classes which inherit and override virtual void operator()() to provide your desired functionality.
*/
class TrivialCommand : public Command
{
public:
	/**
	 * Construct a TrivialCommand from all specifications.
	 * @param name - Name of the Command.
	 * @param description - Brief description of what the Command does.
	 */
	TrivialCommand(std::string name = "", std::string description = ""): Command(name, description, ""){}

	/**
	 * Pure Virtual. Will be invoked when the Command is executed.
	 */
	virtual void operator()() = 0;
private:
	/**
	 * Overload the superclass functor.
	 * @param args - N/A
	 */
	void operator()([[maybe_unused]] const std::vector<std::string>& args) final{operator()();}
	using Command::get_usage;
	using Command::get_expected_parameter_size;
};

/**
 * Represent a Functor without arguments.
 * @tparam Functor - Type of the underlying functor.
 */
template<typename Functor>
class TrivialFunctor : public TrivialCommand
{
public:
	/**
	 * Construct a TrivialFunctor from an anonymous type (which can be a C++ lambda).
	 * @param functor - Type of the underlying functor.
	 */
    TrivialFunctor(Functor&& functor);
	/**
	 * Impure virtual. Invoked when the Functor is executed.
	 */
    virtual void operator()() override;
protected:
	/// Stores the underlying functor object.
    Functor functor;
};

/**
 * Represent a Functor storing all its arguments upon construction.
 * @tparam Functor - Type of the underlying functor.
 * @tparam FunctorParameters - Template parameter pack representing argument types.
 */
template<typename Functor, typename... FunctorParameters>
class StaticFunctor : public TrivialCommand
{
public:
	/**
	 * Construct a StaticFunctor from an underlying functor and all the parameters it requires.
	 * @param functor - The underlying functor
	 * @param parameters - Variadic parameters representing parameters of the StaticFunctor. These are stored in the StaticFunctor.
	 */
    StaticFunctor(Functor&& functor, FunctorParameters&&... parameters);
	/**
	 * Invoked when the StaticFunctor is executed. Runs the underlying functor with all variadic parameters.
	 */
    virtual void operator()() override;
protected:
	/// The underlying functor.
    Functor functor;
	/// Tuple containing all the parameters to be used when the functor is executed.
    std::tuple<FunctorParameters...> parameters;
};

/**
* System used to hold (but not typically own) Commands.
* Engine uses these to handle command input.
*/
class CommandExecutor
{
public:
	/**
	 * Trivially Constructible.
	 */
	CommandExecutor() = default;

	/**
	 * Get a container of all the commands in the executor.
	 * @return - Container containing pointers to all contained Commands.
	 */
	std::unordered_set<Command*> get_commands() const;
	/**
	 * Register an existing Command to be executed when the CommandExecutor is invoked.
	 * @param command - The Command to register.
	 */
	void register_command(Command* command);
	/**
	 * Emplace a trivial functor as a command.
	 * @tparam Functor - The anonymous type.
	 * @param functor - Value of the anonymous functor type.
	 * @return - The constructed TrivialFunctor pointer.
	 */
    template<typename Functor>
    TrivialFunctor<Functor>* emplace_trivial_command(Functor&& functor);
	/**
	 * Emplace a StaticFunctor as a command. Stores all parameters in the StaticFunctor.
	 * @tparam Functor - The anonymous type of the functor.
	 * @tparam FunctorParameters - The types of all the parameters to the functor, in order.
	 * @param functor - Value of the anonymous functor type.
	 * @param parameters - Variadic parameters to the underlying functor.
	 * @return - The constructed StaticFunctor pointer.
	 */
    template<typename Functor, typename... FunctorParameters>
    StaticFunctor<Functor, FunctorParameters...>* emplace_static_command(Functor&& functor, FunctorParameters&&... parameters);
	/**
	 * Prevent an existing Command in this CommandExecutor from being executed anymore.
	 * @param command - An existing Command to be removed
	 */
	void deregister_command(Command* command);
	/**
	 * Deregister an existing Command with the specified name.
	 * @param command_name - An existing command with a name equal to this string
	 */
	void deregister_command(const std::string& command_name);
	/**
	 * Invoke a specified Command.
	 * @param name - Name of the Command to execute.
	 * @param args - Arguments with which to execute the specified Command.
	 */
	void operator()(const std::string& name, const std::vector<std::string>& args = std::vector<std::string>());
private:
	/// Container of all Command pointers.
	std::unordered_set<Command*> commands;
	/// Container of all Command pointers with memory owned by this CommandExecutor.
	std::vector<std::unique_ptr<Command>> owned_commands;
};

namespace tz::util::scheduler
{
	/**
	* Run a functor in the same thread after a specified delay.
	* Note that this does indeed make the thread sleep, so use this on an empty thread or use tz::util::scheduler::async_delayed_functor.
	*/
	template<typename Functor>
	inline void sync_delayed_functor(unsigned int millis_delay, const TrivialFunctor<Functor>& command);
	/**
	* Sleep in a new thread for a specified delay, and then execute a functor.
	*/
	template<typename Functor>
	inline void async_delayed_functor(unsigned int millis_delay, const TrivialFunctor<Functor>& command);
}

#include "command.inl"

#endif