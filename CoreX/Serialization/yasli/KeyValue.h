/**
 *  yasli - Serialization Library.
 *  Разработка (C) 2007-2013 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          http://www.opensource.org/licenses/MIT
 */


#pragma once
namespace yasli {

class Archive;

class KeyValueInterface : StringInterface
{
public:
	virtual tukk get() const = 0;
	virtual void set(tukk key) = 0;
	virtual bool serializeValue(Archive& ar, tukk name, tukk label) = 0;
	template<class TArchive> void YASLI_SERIALIZE_METHOD(TArchive& ar)
	{
		ar(*(StringInterface*)this, "", "^");
		serializeValue(ar, "", "^");
	}
};

}

