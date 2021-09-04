#pragma once

namespace Acorn
{
	class Timestep
	{
	public:
		// friend float operator*(const Timestep& timestep, float value);
		// friend float operator*(float value, const Timestep& timestep);

		Timestep(float time = 0.0f)
			: m_Time(time)
		{
		}

		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }

		operator float() { return m_Time; }

	private:
		float m_Time;
	};

}