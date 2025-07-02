// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CWaterRippleUpr final
{
public:
	static i32 OnEventPhysCollision(const EventPhys* pEvent);

public:
	CWaterRippleUpr();
	~CWaterRippleUpr();

	void GetMemoryUsage(IDrxSizer* pSizer) const;

	void Initialize();

	void Finalize();

	void OnFrameStart();

	void Render(const SRenderingPassInfo& passInfo);

	void AddWaterRipple(const Vec3 position, float scale, float strength);

private:
	std::vector<SWaterRippleInfo> m_waterRippleInfos;
};
