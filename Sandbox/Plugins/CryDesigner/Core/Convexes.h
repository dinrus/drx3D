// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace Designer
{
typedef std::vector<Vertex> Convex;

class Convexes : public _i_reference_target_t
{
public:

	void    AddConvex(const Convex& convex) { m_Convexes.push_back(convex); }
	i32     GetConvexCount() const          { return m_Convexes.size(); }
	Convex& GetConvex(i32 nIndex)           { return m_Convexes[nIndex]; }

private:

	mutable std::vector<Convex> m_Convexes;
};
}

