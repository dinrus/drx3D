// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef _STARS_H_
#define _STARS_H_

#pragma once

class CStars
{
friend class CSceneForwardStage;

public:
	CStars();
	~CStars();

	void Render(bool bUseMoon);

private:
	bool LoadData();

private:
	u32                  m_numStars;
	_smart_ptr<IRenderMesh> m_pStarMesh;
	CShader*                m_pShader;
};

#endif  // #ifndef _STARS_H_
