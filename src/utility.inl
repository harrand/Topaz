namespace tz::util
{
	template<typename Container>
	constexpr std::size_t sizeof_element(Container element_list)
	{
		return sizeof(typename decltype(element_list)::value_type);
	}
	
	namespace cast
	{
		template <typename T>
		inline std::string to_string(T&& obj)
		{
			if constexpr(std::is_same<T, int>::value || std::is_same<T, long>::value || std::is_same<T, long long>::value || std::is_same<T, unsigned>::value || std::is_same<T, unsigned long>::value || std::is_same<T, unsigned long long>::value || std::is_same<T, float>::value || std::is_same<T, double>::value || std::is_same<T, long double>::value)
			{// runs if obj is a valid parameter for std::to_string(...)
				return std::to_string(std::forward<T>(obj));
			}
			std::ostringstream oss;
			oss << std::forward<T>(obj);
			return oss.str();
		}
		
		template <typename T>
		inline T from_string(const std::string& s)
		{
			T ret;
			std::istringstream ss(s);
			ss >> ret;
			return ret;
		}
	}
	
	namespace string
	{
		inline std::string to_lower(std::string data)
		{
			std::transform(data.begin(), data.end(), data.begin(), ::tolower);
			return data;
		}
		
		inline std::string to_upper(std::string data)
		{
			std::transform(data.begin(), data.end(), data.begin(), ::toupper);
			return data;
		}
	
		inline bool begins_with(const std::string& what, const std::string& with_what)
		{
			return mdl::util::begins_with(what, with_what);
		}
		
		inline bool ends_with(const std::string& what, const std::string& with_what)
		{
			return mdl::util::ends_with(what, with_what);
		}
		
		inline bool contains(const std::string& what, char withwhat)
		{
			const char* whatcstr = what.c_str();
			for(std::size_t i = 0; i < what.length(); i++)
				if(whatcstr[i] == withwhat)
					return true;
			return false;
		}
		
		inline std::vector<std::string> split_string(const std::string& s, const std::string& delim)
		{
			return mdl::util::split_string(s, delim);
		}
			
		inline std::vector<std::string> split_string(const std::string& s, char delim)
		{
			return split_string(s, tz::util::cast::to_string(delim));
		}
	
		inline std::string replace_all_char(const std::string& str, char toreplace, const std::string& replacewith)
		{
			std::string res;
			std::vector<std::string> splitdelim = tz::util::string::split_string(str, toreplace);
			for(std::size_t i = 0; i < splitdelim.size(); i++)
			{
				res += splitdelim[i];
				res += replacewith;
			}
			return res;
		}
		
		inline std::string replace_all(std::string str, const std::string& to_replace, const std::string& replace_with)
		{
			std::size_t pos = 0;
			while ((pos = str.find(to_replace, pos)) != std::string::npos)
			{
				str.replace(pos, to_replace.length(), replace_with);
				pos += replace_with.length();
			}
			return str;
		}
		
		inline std::string substring(const std::string& str, unsigned int begin, unsigned int end)
		{
			std::size_t strsize = str.length();
			if(end > strsize)
				return "_";
			return str.substr((begin - 1), (end-begin) + 1);
		}
			
		inline std::string format(const std::vector<std::string>& split)
		{
			std::string ret = "[";
			for(std::size_t i = 0; i < split.size(); i++)
			{
				ret += split[i];
				if(i < (split.size() - 1))
					ret += ",";
				else
					ret += "]";
			}
			return ret;
		}
			
		inline std::vector<std::string> deformat(const std::string& str)
		{
			return tz::util::string::split_string(tz::util::string::replace_all_char(tz::util::string::replace_all_char(str, '[', ""), ']', ""), ',');
		}
			
		template<typename T>
		inline Vector3<T> vectorise_list_3(const std::vector<std::string>& list)
		{
			if(list.size() < 3)
				return {};
			return {tz::util::cast::from_string<T>(list[0]), tz::util::cast::from_string<T>(list[1]), tz::util::cast::from_string<T>(list[2])};
		}
			
		template<typename T>
		inline std::vector<std::string> devectorise_list_3(Vector3<T> v)
		{
			return {tz::util::cast::to_string(v.x), tz::util::cast::to_string(v.y), tz::util::cast::to_string(v.z)};
		}	
	}
	
	namespace log
	{
		inline void silent()
		{
			std::cout << "\n";
		}
		template<typename FirstArg, typename... Args>
		inline void silent(FirstArg arg, Args... args)
		{
			if constexpr(std::is_same<decltype(arg), std::string>::value)
				std::cout << arg;
			else
				std::cout << tz::util::cast::to_string(arg);
			tz::util::log::silent(args...);
		}
			
		template<typename FirstArg = void, typename... Args>
		inline void message(FirstArg arg, Args... args)
		{
			std::cout << "[Message]:\t";
			tz::util::log::silent(arg, args...);
		}
			
		template<typename FirstArg = void, typename... Args>
		inline void warning(FirstArg arg, Args... args)
		{
			std::cout << "[Warning]:\t";
			tz::util::log::silent(arg, args...);
		}
			
		template<typename FirstArg = void, typename... Args>
		inline void error(FirstArg arg, Args... args)
		{
			std::cout << "[Error]:\t";
			tz::util::log::silent(arg, args...);
		}
	}
	
	namespace scheduler
	{
		template<class ReturnType, class... Args>
		inline void sync_delayed_function(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
		{
			std::this_thread::sleep_for(std::chrono::duration<unsigned int, std::milli>(milliseconds_delay));
			f(args...);
		}
		
		template<class ReturnType, class... Args>
		inline void async_delayed_task(unsigned int milliseconds_delay, std::function<ReturnType(Args...)> f, Args... args)
		{
			std::thread(sync_delayed_function<ReturnType, Args...>, milliseconds_delay, f, args...).detach();
		}
	}
	template<typename Number>
	inline Number random()
	{
		static Random rand;
		return rand.operator()<Number>();
	}
}

template<typename Engine, typename EngineResultType>
Random<Engine, EngineResultType>::Random(EngineResultType seed): seed(seed)
{
	this->random_engine.seed(this->seed);
}

template<typename Engine, typename EngineResultType>
Random<Engine, EngineResultType>::Random(const Random<Engine, EngineResultType>& copy): Random<Engine>(copy.seed){}

template<typename Engine, typename EngineResultType>
const EngineResultType& Random<Engine, EngineResultType>::get_seed() const
{
	return this->seed;
}

template<typename Engine, typename EngineResultType>
const Engine& Random<Engine, EngineResultType>::get_engine() const
{
	return this->random_engine;
}

template<typename Engine, typename EngineResultType>
int Random<Engine, EngineResultType>::next_int(int min, int max)
{
	return std::uniform_int_distribution<>(min, max)(this->random_engine);
}

template<typename Engine, typename EngineResultType>
float Random<Engine, EngineResultType>::next_float(float min, float max)
{
	return std::uniform_real_distribution<>(min, max)(this->random_engine);
}

template<typename Engine, typename EngineResultType>
template <typename Number>
inline Number Random<Engine, EngineResultType>::operator()(Number min, Number max)
{
	static_assert(std::is_same<Number, float>::value || std::is_same<Number, int>::value, "Random::operator() must receive template arguments of float or int.");
	if(std::is_same<Number, float>::value)
		return next_float(min, max);
	else if(std::is_same<Number, int>::value)
		return next_int(min, max);
}