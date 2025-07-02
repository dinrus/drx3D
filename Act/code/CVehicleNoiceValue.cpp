#include <drx3D/Act/VehicleNoiseGenerator.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// CVehicleNoiseValue
////////////////////////////////////////////////////////////////////////////////////////////////////

CVehicleNoiseValue::CVehicleNoiseValue()
{
	m_offset = 0;
	m_position = 0;
	m_amplitude = 0.f;
	m_frequency = 0.f;
}

float CVehicleNoiseValue::PseudoNoise(i32 x)
{
	// Predictable random number generator for input 'x' (output is -1 to +1)
	x = (x << 13) ^ x;
	x = x * (x * x * 19379 + 819233) + 1266122899;
	return ((x & 0x7fffffff) * (1.f / 1073741823.5f)) - 1.f;
}

float CVehicleNoiseValue::Interp(float a, float b, float t)
{
	// Interpolate between a and b where t is the amount (0 to 1)
	t = (1.f - cosf(t * 3.14159265358979323f)) * 0.5f; // Change t to a curve
	return a * (1.f - t) + b * t;
}

float CVehicleNoiseValue::SoftNoise(i32 x)
{
	// Soft Perlin noise
	float a = PseudoNoise(x >> 16);
	float b = PseudoNoise((x >> 16) + 1);
	return Interp(a, b, (x & 0xffff) / 65536.f);
}

void CVehicleNoiseValue::SetAmpFreq(float amplitude, float frequency)
{
	m_amplitude = amplitude;
	m_frequency = frequency;
}

void CVehicleNoiseValue::Setup(float amplitude, float frequency, i32 offset)
{
	SetAmpFreq(amplitude, frequency);
	m_offset = offset;
	m_position = 0;
}

float CVehicleNoiseValue::Update(float tickTime)
{
	float r = SoftNoise(m_position + m_offset) * m_amplitude;
	m_position += (i32)(tickTime * 65536.f * m_frequency);
	return r;
}