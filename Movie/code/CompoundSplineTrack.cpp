// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Movie/StdAfx.h>
#include <drx3D/Movie/CompoundSplineTrack.h>
#include <drx3D/Movie/AnimSplineTrack.h>

CCompoundSplineTrack::CCompoundSplineTrack(const CAnimParamType& paramType, i32 nDims, EAnimValue inValueType, CAnimParamType subTrackParamTypes[MAX_SUBTRACKS])
	: m_paramType(paramType), m_guid(DrxGUID::Create())
{
	assert(nDims > 0 && nDims <= MAX_SUBTRACKS);
	m_nDimensions = nDims;
	m_valueType = inValueType;
	m_flags = 0;

	ZeroStruct(m_subTracks);

	for (i32 i = 0; i < m_nDimensions; i++)
	{
		m_subTracks[i] = new CAnimSplineTrack(subTrackParamTypes[i]);

		if (inValueType == eAnimValue_RGB)
		{
			m_subTracks[i]->SetKeyValueRange(0.0f, 255.f);
		}
	}

	m_subTrackNames[0] = "X";
	m_subTrackNames[1] = "Y";
	m_subTrackNames[2] = "Z";
	m_subTrackNames[3] = "W";
}

void CCompoundSplineTrack::SetTimeRange(TRange<SAnimTime> timeRange)
{
	for (i32 i = 0; i < m_nDimensions; i++)
	{
		m_subTracks[i]->SetTimeRange(timeRange);
	}
}

void CCompoundSplineTrack::PrepareNodeForSubTrackSerialization(XmlNodeRef& subTrackNode, XmlNodeRef& xmlNode, i32 i, bool bLoading)
{
	assert(!bLoading || xmlNode->getChildCount() == m_nDimensions);

	if (bLoading)
	{
		subTrackNode = xmlNode->getChild(i);
	}
	else
	{
		if (strcmp(m_subTracks[i]->GetKeyType(), S2DBezierKey::GetType()) == 0)
		{
			subTrackNode = xmlNode->newChild("NewSubTrack");
		}
	}
}

bool CCompoundSplineTrack::Serialize(XmlNodeRef& xmlNode, bool bLoading, bool bLoadEmptyTracks /*=true */)
{
	if (bLoading)
	{
		xmlNode->getAttr("GUIDLo", m_guid.lopart);
		xmlNode->getAttr("GUIDHi", m_guid.hipart);
	}
	else
	{
		xmlNode->setAttr("GUIDLo", m_guid.lopart);
		xmlNode->setAttr("GUIDHi", m_guid.hipart);
	}

	for (i32 i = 0; i < m_nDimensions; i++)
	{
		XmlNodeRef subTrackNode;
		PrepareNodeForSubTrackSerialization(subTrackNode, xmlNode, i, bLoading);
		m_subTracks[i]->Serialize(subTrackNode, bLoading, bLoadEmptyTracks);
	}

	return true;
}

TMovieSystemValue CCompoundSplineTrack::GetValue(SAnimTime time) const
{
	switch (m_valueType)
	{
	case eAnimValue_Float:
		if (m_nDimensions == 1)
		{
			return m_subTracks[0]->GetValue(time);
		}
		break;
	case eAnimValue_Vector:
	case eAnimValue_RGB:
		if (m_nDimensions == 3)
		{
			Vec3 vector;
			for (i32 i = 0; i < m_nDimensions; i++)
			{
				TMovieSystemValue value = m_subTracks[i]->GetValue(time);
				vector[i] = stl::get<float>(value);
			}
			return TMovieSystemValue(vector);
		}
	case eAnimValue_Vector4:
		if (m_nDimensions == 4)
		{
			Vec4 vector;
			for (i32 i = 0; i < m_nDimensions; ++i)
			{
				TMovieSystemValue value = m_subTracks[i]->GetValue(time);
				vector[i] = stl::get<float>(value);
			}
			return TMovieSystemValue(vector);
		}
		break;
	case eAnimValue_Quat:
		if (m_nDimensions == 3)
		{
			float angles[3] = { 0.0f, 0.0f, 0.0f };
			for (i32 i = 0; i < m_nDimensions; ++i)
			{
				if (m_subTracks[i]->HasKeys())
				{
					TMovieSystemValue value = m_subTracks[i]->GetValue(time);
					angles[i] = stl::get<float>(value);
				}
			}
			return TMovieSystemValue(Quat::CreateRotationXYZ(Ang3(DEG2RAD(angles[0]), DEG2RAD(angles[1]), DEG2RAD(angles[2]))));
		}
	}

	return TMovieSystemValue(SMovieSystemVoid());
}

TMovieSystemValue CCompoundSplineTrack::GetDefaultValue() const
{
	switch (m_valueType)
	{
	case eAnimValue_Float:
		if (m_nDimensions == 1)
		{
			return m_subTracks[0]->GetDefaultValue();
		}
		break;
	case eAnimValue_Vector:
	case eAnimValue_RGB:
		if (m_nDimensions == 3)
		{
			Vec3 vector;
			for (i32 i = 0; i < m_nDimensions; i++)
			{
				TMovieSystemValue value = m_subTracks[i]->GetDefaultValue();
				vector[i] = stl::get<float>(value);
			}
			return TMovieSystemValue(vector);
		}
	case eAnimValue_Vector4:
		if (m_nDimensions == 4)
		{
			Vec4 vector;
			for (i32 i = 0; i < m_nDimensions; ++i)
			{
				TMovieSystemValue value = m_subTracks[i]->GetDefaultValue();
				vector[i] = stl::get<float>(value);
			}
			return TMovieSystemValue(vector);
		}
		break;
	case eAnimValue_Quat:
		if (m_nDimensions == 3)
		{
			float angles[3] = { 0.0f, 0.0f, 0.0f };
			for (i32 i = 0; i < m_nDimensions; ++i)
			{
				TMovieSystemValue value = m_subTracks[i]->GetDefaultValue();
				angles[i] = stl::get<float>(value);
			}
			return TMovieSystemValue(Quat::CreateRotationXYZ(Ang3(DEG2RAD(angles[0]), DEG2RAD(angles[1]), DEG2RAD(angles[2]))));
		}
	}

	return TMovieSystemValue(SMovieSystemVoid());
}

void CCompoundSplineTrack::SetDefaultValue(const TMovieSystemValue& value)
{
	switch (value.index())
	{
	case eTDT_Float:
		if (m_nDimensions == 1)
		{
			const float floatValue = stl::get<float>(value);
			m_subTracks[0]->SetDefaultValue(TMovieSystemValue(floatValue));
		}
		break;
	case eTDT_Vec3:
		if (m_nDimensions == 3)
		{
			const Vec3 vecValue = stl::get<Vec3>(value);
			for (i32 i = 0; i < m_nDimensions; ++i)
			{
				m_subTracks[i]->SetDefaultValue(TMovieSystemValue(vecValue[i]));
			}
		}
	case eTDT_Vec4:
		if (m_nDimensions == 4)
		{
			const Vec4 vecValue = stl::get<Vec4>(value);
			for (i32 i = 0; i < m_nDimensions; ++i)
			{
				m_subTracks[i]->SetDefaultValue(TMovieSystemValue(vecValue[i]));
			}
		}
		break;
	case eTDT_Quat:
		if (m_nDimensions == 3)
		{
			const Quat quatValue = stl::get<Quat>(value);

			// Assume Euler Angles XYZ
			Ang3 angles = Ang3::GetAnglesXYZ(quatValue);
			for (i32 i = 0; i < 3; i++)
			{
				const float degree = RAD2DEG(angles[i]);
				m_subTracks[i]->SetDefaultValue(TMovieSystemValue(degree));
			}
		}
	}
}

IAnimTrack* CCompoundSplineTrack::GetSubTrack(i32 nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nDimensions);
	return m_subTracks[nIndex];
}

tukk CCompoundSplineTrack::GetSubTrackName(i32 nIndex) const
{
	assert(nIndex >= 0 && nIndex < m_nDimensions);
	return m_subTrackNames[nIndex];
}

void CCompoundSplineTrack::SetSubTrackName(i32 nIndex, tukk name)
{
	assert(nIndex >= 0 && nIndex < m_nDimensions);
	assert(name);
	m_subTrackNames[nIndex] = name;
}

i32 CCompoundSplineTrack::GetNumKeys() const
{
	i32 nKeys = 0;
	for (i32 i = 0; i < m_nDimensions; i++)
	{
		nKeys += m_subTracks[i]->GetNumKeys();
	}
	return nKeys;
}

bool CCompoundSplineTrack::HasKeys() const
{
	for (i32 i = 0; i < m_nDimensions; i++)
	{
		if (m_subTracks[i]->GetNumKeys())
		{
			return true;
		}
	}
	return false;
}

i32 CCompoundSplineTrack::GetSubTrackIndex(i32& key) const
{
	assert(key >= 0 && key < GetNumKeys());
	i32 count = 0;
	for (i32 i = 0; i < m_nDimensions; i++)
	{
		if (key < count + m_subTracks[i]->GetNumKeys())
		{
			key = key - count;
			return i;
		}
		count += m_subTracks[i]->GetNumKeys();
	}
	return -1;
}

void CCompoundSplineTrack::RemoveKey(i32 num)
{
	assert(num >= 0 && num < GetNumKeys());
	i32 i = GetSubTrackIndex(num);
	assert(i >= 0);
	if (i < 0)
	{
		return;
	}
	m_subTracks[i]->RemoveKey(num);
}

void CCompoundSplineTrack::ClearKeys()
{
	for (uint i = 0; i < m_nDimensions; ++i)
	{
		m_subTracks[i]->ClearKeys();
	}
}

SAnimTime CCompoundSplineTrack::GetKeyTime(i32 index) const
{
	assert(index >= 0 && index < GetNumKeys());
	i32 i = GetSubTrackIndex(index);
	assert(i >= 0);
	if (i < 0)
	{
		return SAnimTime::Min();
	}
	return m_subTracks[i]->GetKeyTime(index);
}

i32 CCompoundSplineTrack::NextKeyByTime(i32 key) const
{
	assert(key >= 0 && key < GetNumKeys());
	SAnimTime time = GetKeyTime(key);
	i32 count = 0, result = -1;
	SAnimTime timeNext = SAnimTime::Max();
	for (i32 i = 0; i < GetSubTrackCount(); ++i)
	{
		for (i32 k = 0; k < m_subTracks[i]->GetNumKeys(); ++k)
		{
			SAnimTime t = m_subTracks[i]->GetKeyTime(k);
			if (t > time)
			{
				if (t < timeNext)
				{
					timeNext = t;
					result = count + k;
				}
				break;
			}
		}
		count += m_subTracks[i]->GetNumKeys();
	}
	return result;
}

_smart_ptr<IAnimKeyWrapper> CCompoundSplineTrack::GetWrappedKey(i32 key)
{
	STrackKey trackKey;
	trackKey.m_time = GetKeyTime(key);

	SAnimKeyWrapper<STrackKey>* pWrappedKey = new SAnimKeyWrapper<STrackKey>();
	pWrappedKey->m_key = trackKey;
	return pWrappedKey;
}
