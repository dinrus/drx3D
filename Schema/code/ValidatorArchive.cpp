// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Schema/StdAfx.h>
#include <drx3D/Schema/ValidatorArchive.h>

#include <drx3D/Schema/ISerializationContext.h>
#include <drx3D/Schema/Assert.h>
#include <drx3D/Schema/ILog.h>

namespace sxema
{
CValidatorArchive::CValidatorArchive(const SValidatorArchiveParams& params)
	: IValidatorArchive(Serialization::IArchive::OUTPUT | Serialization::IArchive::INPLACE | Serialization::IArchive::VALIDATION)
	, m_flags(params.flags)
{}

bool CValidatorArchive::operator()(bool& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(int8& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(u8& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(i32& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(u32& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(int64& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(uint64& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(float& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(Serialization::IString& value, tukk szName, tukk szLabel)
{
	return true;
}

bool CValidatorArchive::operator()(const Serialization::SStruct& value, tukk szName, tukk szLabel)
{
	value(*this);
	return true;
}

bool CValidatorArchive::operator()(Serialization::IContainer& value, tukk szName, tukk szLabel)
{
	if (value.size())
	{
		do
		{
			value(*this, szName, szLabel);
		}
		while (value.next());
	}
	return true;
}

void CValidatorArchive::Validate(const Validator& validator) const
{
	SXEMA_CORE_ASSERT(validator);
	if (validator)
	{
		for (const string& warning : m_warnings)
		{
			validator(EValidatorMessageType::Warning, warning.c_str());
		}
		for (const string& error : m_errors)
		{
			validator(EValidatorMessageType::Error, error.c_str());
		}
	}
}

u32 CValidatorArchive::GetWarningCount() const
{
	return m_warnings.size();
}

u32 CValidatorArchive::GetErrorCount() const
{
	return m_errors.size();
}

void CValidatorArchive::validatorMessage(bool bError, ukk handle, const Serialization::TypeID& type, tukk szMessage)
{
	if (bError)
	{
		if (m_flags.Check(EValidatorArchiveFlags::ForwardErrorsToLog))
		{
			SValidatorLink validatorLink;
			if (SerializationContext::GetValidatorLink(*this, validatorLink))
			{
				CLogMetaData logMetaData;
				logMetaData.Set(ELogMetaField::LinkCommand, DrxLinkUtils::ECommand::Show);
				logMetaData.Set(ELogMetaField::ElementGUID, validatorLink.elementGUID);
				logMetaData.Set(ELogMetaField::DetailGUID, validatorLink.detailGUID);
				SXEMA_LOG_SCOPE(logMetaData);
				SXEMA_CORE_WARNING(szMessage);
			}
			else
			{
				SXEMA_CORE_ERROR(szMessage);
			}
		}
		m_errors.push_back(szMessage);
	}
	else
	{
		if (m_flags.Check(EValidatorArchiveFlags::ForwardWarningsToLog))
		{
			SValidatorLink validatorLink;
			if (SerializationContext::GetValidatorLink(*this, validatorLink))
			{
				CLogMetaData logMetaData;
				logMetaData.Set(ELogMetaField::LinkCommand, DrxLinkUtils::ECommand::Show);
				logMetaData.Set(ELogMetaField::ElementGUID, validatorLink.elementGUID);
				logMetaData.Set(ELogMetaField::DetailGUID, validatorLink.detailGUID);
				SXEMA_LOG_SCOPE(logMetaData);
				SXEMA_CORE_WARNING(szMessage);
			}
			else
			{
				SXEMA_CORE_WARNING(szMessage);
			}
		}
		m_warnings.push_back(szMessage);
	}
}
} // sxema
