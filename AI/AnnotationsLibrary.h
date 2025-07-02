// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/AI/Annotation.h>

namespace MNM
{

class CAnnotationsLibrary : public IAnnotationsLibrary
{
public:
	CAnnotationsLibrary()
		: m_defaultColor(Col_Azure, 0.65f)
	{}

	// IAnnotationsLibrary
	virtual NavigationAreaTypeID GetAreaTypeID(tukk szName) const override;
	virtual tukk          GetAreaTypeName(const NavigationAreaTypeID areaTypeID) const override;
	virtual const MNM::SAreaType*     GetAreaType(const NavigationAreaTypeID areaTypeID) const override;
	virtual size_t               GetAreaTypeCount() const override;
	virtual NavigationAreaTypeID GetAreaTypeID(const size_t index) const override;
	virtual const MNM::SAreaType*     GetAreaType(const size_t index) const override;
	virtual const MNM::SAreaType&     GetDefaultAreaType() const override;

	virtual NavigationAreaFlagID GetAreaFlagID(tukk szName) const override;
	virtual tukk          GetAreaFlagName(const NavigationAreaFlagID areaFlagID) const override;
	virtual const SAreaFlag*     GetAreaFlag(const NavigationAreaFlagID areaFlagID) const override;
	virtual size_t               GetAreaFlagCount() const override;
	virtual NavigationAreaFlagID GetAreaFlagID(const size_t index) const override;
	virtual const SAreaFlag*     GetAreaFlag(const size_t index) const override;

	virtual void                 GetAreaColor(const MNM::AreaAnnotation annotation, ColorB& color) const override;
	// ~IAnnotationsLibrary

	void                 Clear();

	NavigationAreaTypeID CreateAreaType(u32k id, tukk szName, u32k defaultFlags, const ColorB* pColor = nullptr);
	NavigationAreaFlagID CreateAreaFlag(u32k id, tukk szName, const ColorB* pColor = nullptr);

	void                 SetDefaultAreaColor(const ColorB& color) { m_defaultColor = color; }
	bool                 GetFirstFlagColor(const MNM::AreaAnnotation::value_type flags, ColorB& color) const;
private:
	std::vector<MNM::SAreaType> m_areaTypes;
	std::vector<SAreaFlag> m_areaFlags;

	std::unordered_map<MNM::AreaAnnotation::value_type, ColorB> m_areasColorMap;
	std::unordered_map<MNM::AreaAnnotation::value_type, ColorB> m_flagsColorMap;
	ColorB m_defaultColor;
};

} //endns MNM