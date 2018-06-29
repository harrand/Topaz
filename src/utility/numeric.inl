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