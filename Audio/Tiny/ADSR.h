#ifndef D3_ADSR_H
#define D3_ADSR_H

#include <drxtypes.h>

class b3ADSR
{
	i32 m_state;
	double m_value;
	double m_target;
	double m_attackRate;
	double m_decayRate;
	double m_releaseRate;
	double m_releaseTime;
	double m_sustainLevel;
	bool m_autoKeyOff;

public:
	b3ADSR();
	virtual ~b3ADSR();

	double tick();
	bool isIdle() const;
	void keyOn(bool autoKeyOff);
	void keyOff();

	void setValues(double attack, double decay, double sustain, double release)
	{
		m_attackRate = attack;
		m_decayRate = decay;
		m_sustainLevel = sustain;
		m_releaseRate = release;
	}
};

#endif  //D3_ADSR_H