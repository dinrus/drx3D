// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

//! \cond INTERNAL

#pragma once

#include <drx3D/CoreX/String/DrxFixedString.h>

//////////////////////////////////////////////////////////////////////////
/** This class keeps file version information.
 */
struct SFileVersion
{
	i32 v[4];

	SFileVersion()
	{
		v[0] = v[1] = v[2] = v[3] = 0;
	}
	SFileVersion(i32k vers[])
	{
		v[0] = vers[0];
		v[1] = vers[1];
		v[2] = vers[2];
		v[3] = 1;
	}

	void Set(tukk s)
	{
		v[0] = v[1] = v[2] = v[3] = 0;

		char t[50];
		const size_t len = (std::min)(strlen(s), sizeof(t) - 1);
		memcpy(t, s, len);
		t[len] = 0;

		tuk p;
		if (!(p = strtok(t, "."))) return;
		v[3] = atoi(p);
		if (!(p = strtok(NULL, "."))) return;
		v[2] = atoi(p);
		if (!(p = strtok(NULL, "."))) return;
		v[1] = atoi(p);
		if (!(p = strtok(NULL, "."))) return;
		v[0] = atoi(p);
	}

	explicit SFileVersion(tukk s)
	{
		Set(s);
	}

	bool operator<(const SFileVersion& v2) const
	{
		if (v[3] < v2.v[3]) return true;
		if (v[3] > v2.v[3]) return false;

		if (v[2] < v2.v[2]) return true;
		if (v[2] > v2.v[2]) return false;

		if (v[1] < v2.v[1]) return true;
		if (v[1] > v2.v[1]) return false;

		if (v[0] < v2.v[0]) return true;
		if (v[0] > v2.v[0]) return false;
		return false;
	}
	bool operator==(const SFileVersion& v1) const
	{
		if (v[0] == v1.v[0] && v[1] == v1.v[1] &&
		    v[2] == v1.v[2] && v[3] == v1.v[3]) return true;
		return false;
	}
	bool operator>(const SFileVersion& v1) const
	{
		return !(*this < v1);
	}
	bool operator>=(const SFileVersion& v1) const
	{
		return (*this == v1) || (*this > v1);
	}
	bool operator<=(const SFileVersion& v1) const
	{
		return (*this == v1) || (*this < v1);
	}

	i32& operator[](i32 i)       { return v[i]; }
	i32  operator[](i32 i) const { return v[i]; }

	void ToShortString(tuk s) const
	{
		sprintf(s, "%d.%d.%d", v[2], v[1], v[0]);
	}

	void ToString(tuk s) const
	{
		sprintf(s, "%d.%d.%d.%d", v[3], v[2], v[1], v[0]);
	}

	DrxFixedStringT<32> ToString() const
	{
		DrxFixedStringT<32> str;
		str.Format("%d.%d.%d.%d", v[3], v[2], v[1], v[0]);

		return str;
	}
};

//! \endcond