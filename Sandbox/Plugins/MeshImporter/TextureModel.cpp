// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#include "StdAfx.h"
#include "TextureModel.h"
#include "TextureManager.h"

CTextureModel::CTextureModel(CTextureManager* pTextureManager, QObject* pParent)
	: QAbstractItemModel(pParent), m_pTextureManager(pTextureManager)
{
}

CTextureManager::TextureHandle CTextureModel::GetTexture(const QModelIndex& index)
{
	DRX_ASSERT(index.isValid());
	return (CTextureManager::TextureHandle)index.internalPointer();
}

CTextureManager* CTextureModel::GetTextureManager()
{
	return m_pTextureManager;
}

QModelIndex CTextureModel::index(i32 row, i32 col, const QModelIndex& parent) const
{
	if (!parent.isValid())
	{
		DRX_ASSERT(row >= 0 && row < m_pTextureManager->GetTextureCount());
		DRX_ASSERT(col >= 0 && col < eColumn_COUNT);
		return createIndex(row, col, m_pTextureManager->GetTextureFromIndex(row));
	}
	return QModelIndex();
}

QModelIndex CTextureModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

i32 CTextureModel::rowCount(const QModelIndex& index) const
{
	if (!index.isValid())
	{
		return m_pTextureManager->GetTextureCount();
	}
	return 0;
}

i32 CTextureModel::columnCount(const QModelIndex& index) const
{
	return eColumn_COUNT;
}

tukk GetDisplayNameOfSettings(CTextureModel::ERcSettings settings);

QVariant CTextureModel::data(const QModelIndex& index, i32 role) const
{
	DRX_ASSERT(index.isValid());

	CTextureManager::TextureHandle tex = (CTextureManager::TextureHandle)index.internalPointer();

	if (index.column() == eColumn_RcSettings)
	{
		i32k setting = GetSettings(m_pTextureManager->GetRcSettings(tex));
		if (role == Qt::DisplayRole)
		{
			return QString(GetDisplayNameOfSettings((ERcSettings)setting));
		}
		else if (role == Qt::EditRole)
		{
			return setting;
		}
	}
	else if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
		case eColumn_Filename:
			return QtUtil::ToQString(m_pTextureManager->GetSourcePath(tex));
		default:
			DRX_ASSERT(0 && "invalid index column");
		}
		;
	}
	else if (role == Qt::ToolTipRole && index.column() == eColumn_Filename)
	{
		const QString source = QtUtil::ToQString(m_pTextureManager->GetSourcePath(tex));
		const QString target = QtUtil::ToQString(m_pTextureManager->GetTargetPath(tex));
		return tr("Source file path: %1;\nTarget file path:%2").arg(source).arg(target);
	}

	return QVariant();
}

bool CTextureModel::setData(const QModelIndex& modelIndex, const QVariant& value, i32 role)
{
	if (modelIndex.column() == eColumn_RcSettings && role == Qt::EditRole && value.canConvert<i32>())
	{
		CTextureManager::TextureHandle tex = (CTextureManager::TextureHandle)modelIndex.internalPointer();
		const string rcSetting = GetSettingsRcString((ERcSettings)value.toInt());
		m_pTextureManager->SetRcSettings(tex, rcSetting);
	}
	return false;
}

Qt::ItemFlags CTextureModel::flags(const QModelIndex& modelIndex) const
{
	if (modelIndex.column() == eColumn_RcSettings)
	{
		return Qt::ItemIsEditable | QAbstractItemModel::flags(modelIndex);
	}
	else
	{
		return QAbstractItemModel::flags(modelIndex);
	}
}

string CTextureModel::GetSettingsRcString(ERcSettings rcSettings) const
{
	switch (rcSettings)
	{
	case eRcSettings_Diffuse:
		return "/preset=Albedo";
	case eRcSettings_Bumpmap:
		return "/preset=Normals";
	case eRcSettings_Specular:
		return "/preset=Reflectance";
	default:
		DRX_ASSERT(0 && "Unkown rc settings");
		return "/preset=Albedo";
	}
}

i32 CTextureModel::GetSettings(const string& rcString) const
{
	if (rcString == GetSettingsRcString(eRcSettings_Bumpmap))
	{
		return eRcSettings_Bumpmap;
	}
	else if (rcString == GetSettingsRcString(eRcSettings_Specular))
	{
		return eRcSettings_Specular;
	}
	else
	{
		return eRcSettings_Diffuse;
	}
}

tukk CTextureModel::GetDisplayNameOfSettings(CTextureModel::ERcSettings settings)
{
	switch(settings)
	{
	case CTextureModel::eRcSettings_Diffuse:
		return "Diffuse";
	case CTextureModel::eRcSettings_Bumpmap:
		return "Bumpmap";
	case CTextureModel::eRcSettings_Specular:
		return "Specular";
	default:
		assert(0 && "unkown rc setting");
		return "<unknown rc setting>";
	}
}

QVariant CTextureModel::headerData(i32 column, Qt::Orientation orientation, i32 role) const
{
	if (role == Qt::DisplayRole)
	{
		switch (column)
		{
		case eColumn_Filename:
			return QString("Texture");
		case eColumn_RcSettings:
			return QString("Type");
		default:
			DRX_ASSERT(0 && "unkown column");
			return QVariant();
		}
		;
	}
	return QVariant();
}

void CTextureModel::OnTexturesChanged()
{
	Reset();
}

void CTextureModel::Reset()
{
	// TODO: Delete me.
	beginResetModel();
	endResetModel();
}

