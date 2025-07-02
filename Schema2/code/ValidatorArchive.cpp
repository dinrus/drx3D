// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema2/StdAfx.h>
#include <drx3D/Schema2/ValidatorArchive.h>

#include <drx3D/Schema2/ISerializationContext.h>
#include <drx3D/Schema2/ILog.h>

namespace sxema2
{
	CValidatorArchive::CValidatorArchive(const SValidatorArchiveParams& params)
		: IValidatorArchive(Serialization::IArchive::OUTPUT | Serialization::IArchive::INPLACE | Serialization::IArchive::VALIDATION)
		, m_flags(params.flags)
		, m_warningCount(0)
		, m_errorCount(0)
	{}

	bool CValidatorArchive::operator () (bool& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (int8& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (u8& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (i32& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (u32& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (int64& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (uint64& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (float& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (Serialization::IString& value, tukk szName, tukk szLabel)
	{
		return true;
	}

	bool CValidatorArchive::operator () (const Serialization::SStruct& value, tukk szName, tukk szLabel)
	{
		value(*this);
		return true;
	}

	bool CValidatorArchive::operator() (Serialization::IContainer& value, tukk szName, tukk szLabel)
	{
		if(value.size())
		{
			do
			{
				value(*this, szName, szLabel);
			} while(value.next());
		}
		return true;
	}

	u32 CValidatorArchive::GetWarningCount() const
	{
		return m_warningCount;
	}

	u32 CValidatorArchive::GetErrorCount() const
	{
		return m_errorCount;
	}

	void CValidatorArchive::validatorMessage(bool bError, ukk handle, const Serialization::TypeID& type, tukk szMessage)
	{
		if(bError)
		{
			if((m_flags & EValidatorArchiveFlags::ForwardErrorsToLog) != 0)
			{
				SValidatorLink validatorLink;
				if(SerializationContext::GetValidatorLink(*this, validatorLink))
				{
					const CLogMessageMetaInfo metaInfo(EDrxLinkCommand::Show, SLogMetaItemGUID(validatorLink.itemGUID), SLogMetaChildGUID(validatorLink.detailGUID));
					SXEMA2_SYSTEM_METAINFO_ERROR(metaInfo, szMessage);
				}
				else
				{
					SXEMA2_SYSTEM_ERROR(szMessage);
				}
			}
			++ m_errorCount;
		}
		else
		{
			if((m_flags & EValidatorArchiveFlags::ForwardWarningsToLog) != 0)
			{
				SValidatorLink validatorLink;
				if(SerializationContext::GetValidatorLink(*this, validatorLink))
				{
					const CLogMessageMetaInfo metaInfo(EDrxLinkCommand::Show, SLogMetaItemGUID(validatorLink.itemGUID), SLogMetaChildGUID(validatorLink.detailGUID));
					SXEMA2_SYSTEM_METAINFO_WARNING(metaInfo, szMessage);
				}
				else
				{
					SXEMA2_SYSTEM_WARNING(szMessage);
				}
			}
			++ m_warningCount;
		}
	}
}
