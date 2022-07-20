#pragma once

class Timestep
{
public:
	Timestep(float time = 0.0f)
		: m_time(time)
	{}

	operator float() const { return m_time; }
	Timestep& operator+=(const Timestep& ts) { m_time += ts.m_time; return *this; }

	float GetSeconds() const { return m_time; }
	float GetMilliseconds() const { return m_time * 1000.0f; }

private:
	float m_time;
};