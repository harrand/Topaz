#include "numeric.hpp"

namespace tz::utility::numeric
{
	float linear_interpolate(float a, float b, float blend_factor)
	{
		return a * (1.0f - blend_factor) + b * blend_factor;
	}

	float cosine_interpolate(float a, float b, float blend_factor)
	{
		float theta = blend_factor * tz::consts::numeric::pi;
		// get a value between 0-1, then pass that into a simple linear-interpolation.
		return tz::utility::numeric::linear_interpolate(a, b, 1.0f - std::cos(theta) * 0.5f);
	}
}

SmoothNoise::SmoothNoise(int seed): seed(seed), random(seed){}

float SmoothNoise::operator()(std::size_t x, std::size_t z)
{
	return this->smooth_noise(x, z);
}

float SmoothNoise::base_noise(std::size_t x, std::size_t z)
{
	this->random = {static_cast<typename decltype(this->random)::seed_type>(this->seed + x + z)};
	return this->random.next_float(-1.0f, 1.0f);
}

float SmoothNoise::smooth_noise(std::size_t x, std::size_t z)
{
	float corners = (this->base_noise(x - 1, z - 1) + this->base_noise(x + 1, z - 1) + this->base_noise(x + 1, z + 1) + this->base_noise(x - 1, z + 1)) / 12.0f;
	float sides = (this->base_noise(x - 1, z) + this->base_noise(x + 1, z) + this->base_noise(x, z + 1) + this->base_noise(x, z - 1)) / 12.0f;
	float centre = this->base_noise(x, z) / 4.0f;
	return corners + sides + centre;
}

CosineNoise::CosineNoise(int seed): SmoothNoise(seed){}

float CosineNoise::operator()(std::size_t x, std::size_t z, float smoothness)
{
	return this->cosine_noise(x, z, smoothness);
}

float CosineNoise::cosine_noise(std::size_t x, std::size_t z, float smoothness)
{
	float dx = x / smoothness, dz = z / smoothness;
	auto integer_x = static_cast<int>(dx);
	auto integer_z = static_cast<int>(dz);
	float fraction_x = dx - integer_x;
	float fraction_z = dz - integer_z;

	float top_left = this->smooth_noise(integer_x, integer_z + 1);
	float top_right = this->smooth_noise(integer_x + 1, integer_z + 1);
	float bottom_left = this->smooth_noise(integer_x, integer_z);
	float bottom_right = this->smooth_noise(integer_x + 1, integer_z);
	using namespace tz::utility;
	float top_interpolation = numeric::cosine_interpolate(top_left, top_right, fraction_x);
	float bottom_interpolation = numeric::cosine_interpolate(bottom_left, bottom_right, fraction_x);
	return numeric::cosine_interpolate(top_interpolation, bottom_interpolation, fraction_z);
}