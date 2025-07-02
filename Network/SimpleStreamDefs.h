// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __SIMPLESTREAMDEFS_H__
#define __SIMPLESTREAMDEFS_H__

#pragma once

struct SStreamRecord
{
	static const size_t KEY_SIZE = 32;
	static const size_t VALUE_SIZE = 96;

	SStreamRecord(tukk key, tukk value)
	{
		memset(this->key, 0, sizeof(this->key));
		memset(this->value, 0, sizeof(this->value));
		drx_strcpy(this->key, key);
		drx_strcpy(this->value, value);
	}

	SStreamRecord(tukk key)
	{
		memset(this->key, 0, sizeof(this->key));
		memset(this->value, 0, sizeof(this->value));
		drx_strcpy(this->key, key);
	}

	SStreamRecord()
	{
		memset(this->key, 0, sizeof(this->key));
		memset(this->value, 0, sizeof(this->value));
	}

	char key[KEY_SIZE];
	char value[VALUE_SIZE];
};

#endif
