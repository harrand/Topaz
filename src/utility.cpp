#include "utility.hpp"

Force::Force(Vector3F size): size(size){}

const Vector3F& Force::getSize() const
{
	return this->size;
}

void Force::setSize(Vector3F size)
{
	this->size = size;
}

Force Force::operator+(const Force& other) const
{
	return Force(this->size + other.getSize());
}

Force Force::operator-(const Force& other) const
{
	return Force(this->size - other.getSize());
}

Force Force::operator*(float rhs) const
{
	return Force(this->size * rhs);
}

Force Force::operator/(float rhs) const
{
	return Force(this->size / rhs);
}

Force& Force::operator+=(const Force& other)
{
	this->size += other.getSize();
	return *this;
}

Force& Force::operator-=(const Force& other)
{
	this->size -= other.getSize();
	return *this;
}

MersenneTwister::MersenneTwister(std::mt19937::result_type seed): seed(seed)
{
	this->mersenne_twister.seed(this->seed);
}

MersenneTwister::MersenneTwister(const MersenneTwister& copy): MersenneTwister(copy.seed){}

const std::mt19937::result_type& MersenneTwister::getSeed() const
{
	return this->seed;
}
const std::mt19937& MersenneTwister::getEngine() const
{
	return this->mersenne_twister;
}
std::mt19937& MersenneTwister::getEngineR()
{
	return this->mersenne_twister;
}

int MersenneTwister::nextInt(int min, int max)
{
	return std::uniform_int_distribution<>(min,max)(this->mersenne_twister);
}

float MersenneTwister::nextFloat(float min, float max)
{
	return std::uniform_real_distribution<>(min, max)(this->mersenne_twister);
}



