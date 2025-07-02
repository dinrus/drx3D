// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "stdafx.h"
#include <drx3D/Sandbox/Editor/EditorCommon/EditorCommonAPI.h>
#include <drx3D/CoreX/Serialization/Serializer.h>
#include "Serialization/Qt.h"
#include <drx3D/CoreX/Serialization/STL.h>
#include <drx3D/CoreX/Serialization/IArchive.h>
#include <QSplitter>
#include <QString>
#include <QTreeView>
#include <QHeaderView>

class StringQt : public Serialization::IWString
{
public:
	StringQt(QString& str) : str_(str) {}

	void                  set(const wchar_t* value) { str_.setUnicode((const QChar*)value, (i32)wcslen(value)); }
	const wchar_t*        get() const               { return (wchar_t*)str_.data(); }
	ukk           handle() const            { return &str_; }
	Serialization::TypeID type() const              { return Serialization::TypeID::get<QString>(); }
private:
	QString& str_;
};

bool EDITOR_COMMON_API Serialize(Serialization::IArchive& ar, QString& value, tukk name, tukk label)
{
	StringQt str(value);
	return ar(static_cast<Serialization::IWString&>(str), name, label);
}

bool EDITOR_COMMON_API Serialize(Serialization::IArchive& ar, QByteArray& byteArray, tukk name, tukk label)
{
	std::vector<u8> temp(byteArray.begin(), byteArray.end());
	if (!ar(temp, name, label))
		return false;
	if (ar.isInput())
		byteArray = QByteArray(temp.empty() ? (tuk)0 : (tuk)&temp[0], (i32)temp.size());
	return true;
}

struct QColorSerializable
{
	QColor& color;
	QColorSerializable(QColor& color) : color(color) {}

	void Serialize(Serialization::IArchive& ar)
	{
		// this is not comprehensive, as QColor can store color components
		// in diffrent models, depending on the way they were specified
		u8 r = color.red();
		u8 g = color.green();
		u8 b = color.blue();
		u8 a = color.alpha();
		ar(r, "r", "^R");
		ar(g, "g", "^G");
		ar(b, "b", "^B");
		ar(a, "a", "^A");
		if (ar.isInput())
		{
			color.setRed(r);
			color.setGreen(g);
			color.setBlue(b);
			color.setAlpha(a);
		}
	}
};

bool EDITOR_COMMON_API Serialize(Serialization::IArchive& ar, QColor& color, tukk name, tukk label)
{
	QColorSerializable serializer(color);
	return ar(serializer, name, label);
}
