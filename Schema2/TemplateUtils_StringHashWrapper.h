// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

namespace TemplateUtils
{
	// Fast string hashing function (uses x65599 algorithm).
	////////////////////////////////////////////////////////////////////////////////////////////////////
	inline u32 FastStringHash(tukk szString)
	{
		DRX_ASSERT(szString);
		if(szString)
		{
			u32 hash = 0;
			for( ; *szString; ++ szString)
			{
				hash = 65599 * hash + *szString;
			}
			return hash ^ (hash >> 16);
		}
		return 0;
	}

	// Fast string hashing class (uses x65599 algorithm).
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CFastStringHash
	{
	public:

		inline CFastStringHash()
			: m_hash(0)
		{}

		inline CFastStringHash(const CFastStringHash& rhs)
			: m_hash(rhs.m_hash)
		{}

		inline CFastStringHash(tukk szString)
		{
			m_hash = FastStringHash(szString);
		}

		inline u32 GetValue() const
		{
			return m_hash;
		}

		inline CFastStringHash& operator = (const CFastStringHash& rhs)
		{
			m_hash = rhs.m_hash;
			return *this;
		}

		inline CFastStringHash& operator = (tukk szString)
		{
			m_hash = FastStringHash(szString);
			return *this;
		}

		inline bool operator == (const CFastStringHash& rhs) const 
		{ 
			return m_hash == rhs.m_hash;
		}

		inline bool operator != (const CFastStringHash& rhs) const 
		{ 
			return m_hash != rhs.m_hash;
		}

		inline bool operator < (const CFastStringHash& rhs) const 
		{ 
			return m_hash < rhs.m_hash;
		}

		inline bool operator <= (const CFastStringHash& rhs) const 
		{ 
			return m_hash <= rhs.m_hash;
		}

		inline bool operator > (const CFastStringHash& rhs) const 
		{ 
			return m_hash > rhs.m_hash;
		}

		inline bool operator >= (const CFastStringHash& rhs) const 
		{ 
			return m_hash >= rhs.m_hash;
		}

	private:

		u32 m_hash;
	};

	// CRC32 string hashing class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CStringHashCRC32
	{
	public:

		inline CStringHashCRC32()
			: m_crc32(0)
		{}

		inline CStringHashCRC32(const CStringHashCRC32& rhs)
			: m_crc32(rhs.m_crc32)
		{}

		inline CStringHashCRC32(tukk szString)
		{
			FromString(szString);
		}

		inline u32 GetValue() const
		{
			return m_crc32;
		}

		inline CStringHashCRC32& operator = (const CStringHashCRC32& rhs)
		{
			m_crc32 = rhs.m_crc32;
			return *this;
		}

		inline CStringHashCRC32& operator = (tukk szString)
		{
			FromString(szString);
			return *this;
		}

		inline bool operator == (const CStringHashCRC32& rhs) const 
		{ 
			return m_crc32 == rhs.m_crc32;
		}

		inline bool operator != (const CStringHashCRC32& rhs) const 
		{ 
			return m_crc32 != rhs.m_crc32;
		}

		inline bool operator < (const CStringHashCRC32& rhs) const 
		{ 
			return m_crc32 < rhs.m_crc32;
		}

		inline bool operator <= (const CStringHashCRC32& rhs) const 
		{ 
			return m_crc32 <= rhs.m_crc32;
		}

		inline bool operator > (const CStringHashCRC32& rhs) const 
		{ 
			return m_crc32 > rhs.m_crc32;
		}

		inline bool operator >= (const CStringHashCRC32& rhs) const 
		{ 
			return m_crc32 >= rhs.m_crc32;
		}

	private:

		inline void FromString(tukk szString)
		{
			m_crc32 = DrxStringUtils::CalculateHash(szString);
		}

		u32 m_crc32;
	};

	// CRC32 lower case string hashing class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLowerCaseStringHashCRC32
	{
	public:

		inline CLowerCaseStringHashCRC32()
			: m_crc32(0)
		{}

		inline CLowerCaseStringHashCRC32(const CLowerCaseStringHashCRC32& rhs)
			: m_crc32(rhs.m_crc32)
		{}

		inline CLowerCaseStringHashCRC32(tukk szString)
		{
			FromString(szString);
		}

		inline u32 GetValue() const
		{
			return m_crc32;
		}

		inline CLowerCaseStringHashCRC32& operator = (const CLowerCaseStringHashCRC32& rhs)
		{
			m_crc32 = rhs.m_crc32;
			return *this;
		}

		inline CLowerCaseStringHashCRC32& operator = (tukk szString)
		{
			FromString(szString);
			return *this;
		}

		inline bool operator == (const CLowerCaseStringHashCRC32& rhs) const
		{
			return m_crc32 == rhs.m_crc32;
		}

		inline bool operator != (const CLowerCaseStringHashCRC32& rhs) const
		{
			return m_crc32 != rhs.m_crc32;
		}

		inline bool operator < (const CLowerCaseStringHashCRC32& rhs) const
		{
			return m_crc32 < rhs.m_crc32;
		}

		inline bool operator <= (const CLowerCaseStringHashCRC32& rhs) const
		{
			return m_crc32 <= rhs.m_crc32;
		}

		inline bool operator > (const CLowerCaseStringHashCRC32& rhs) const
		{
			return m_crc32 > rhs.m_crc32;
		}

		inline bool operator >= (const CLowerCaseStringHashCRC32& rhs) const
		{
			return m_crc32 >= rhs.m_crc32;
		}

	private:

		inline void FromString(tukk szString)
		{
			m_crc32 = DrxStringUtils::CalculateHashLowerCase(szString);
		}

		u32 m_crc32;
	};

	// Upper case string conversion class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CUpperCaseStringConversion
	{
	public:

		inline CUpperCaseStringConversion(tukk szString)
			: m_string(szString)
		{
			m_string.MakeUpper();
		}

		inline tukk c_str() const
		{
			return m_string.c_str();
		}

	private:

		stack_string m_string;
	};

	// Lower case string conversion class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CLowerCaseStringConversion
	{
	public:

		inline CLowerCaseStringConversion(tukk szString)
			: m_string(szString)
		{
			m_string.MakeLower();
		}

		inline tukk c_str() const
		{
			return m_string.c_str();
		}

	private:

		stack_string m_string;
	};

	// Empty string conversion class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CEmptyStringConversion
	{
	public:

		inline CEmptyStringConversion(tukk szString)
			: m_szString(szString)
		{}

		inline tukk c_str() const
		{
			return m_szString;
		}

	private:

		tukk m_szString;
	};

	// Raw pointer string storage class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CRawPtrStringStorage
	{
	public:

		inline CRawPtrStringStorage()
			: m_szString("")
		{}

		inline CRawPtrStringStorage(tukk szString)
			: m_szString(szString ? szString : "")
		{}

		inline CRawPtrStringStorage(const CRawPtrStringStorage& rhs)
			: m_szString(rhs.m_szString)
		{}

		inline tukk c_str() const
		{
			return m_szString;
		}

		inline CRawPtrStringStorage& operator = (tukk szString)
		{
			m_szString = szString ? szString : "";
			return *this;
		}

		inline CRawPtrStringStorage& operator = (const CRawPtrStringStorage& rhs)
		{
			m_szString = rhs.m_szString;
			return *this;
		}

		inline bool operator == (const CRawPtrStringStorage& rhs) const 
		{ 
			return (m_szString == rhs.m_szString) || (strcmp(m_szString, rhs.m_szString) == 0);
		}

		inline bool operator != (const CRawPtrStringStorage& rhs) const 
		{ 
			return (m_szString != rhs.m_szString) && (strcmp(m_szString, rhs.m_szString) != 0);
		}

		inline bool operator < (const CRawPtrStringStorage& rhs) const 
		{ 
			return strcmp(m_szString, rhs.m_szString) < 0;
		}

		inline bool operator <= (const CRawPtrStringStorage& rhs) const 
		{ 
			return strcmp(m_szString, rhs.m_szString) <= 0;
		}

		inline bool operator > (const CRawPtrStringStorage& rhs) const 
		{ 
			return strcmp(m_szString, rhs.m_szString) > 0;
		}

		inline bool operator >= (const CRawPtrStringStorage& rhs) const 
		{ 
			return strcmp(m_szString, rhs.m_szString) >= 0;
		}

	private:

		tukk m_szString;
	};

	// In place string storage class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <size_t SIZE> class CInPlaceStringStorage
	{
	public:

		inline CInPlaceStringStorage()
		{
			m_string[0] = '\0';
		}

		inline CInPlaceStringStorage(tukk szString)
		{
			drx_strcpy(m_string, szString);
		}

		inline CInPlaceStringStorage(const CInPlaceStringStorage& rhs)
		{
			drx_strcpy(m_string, rhs.m_string);
		}

		inline tukk c_str() const
		{
			return m_string;
		}

		inline CInPlaceStringStorage& operator = (tukk szString)
		{
			drx_strcpy(m_string, szString);
			return *this;
		}

		inline CInPlaceStringStorage& operator = (const CInPlaceStringStorage& rhs)
		{
			drx_strcpy(m_string, rhs.m_string);
			return *this;
		}

		inline bool operator == (const CInPlaceStringStorage& rhs) const 
		{ 
			return (m_string == rhs.m_string) || (strcmp(m_string, rhs.m_string) == 0);
		}

		inline bool operator != (const CInPlaceStringStorage& rhs) const 
		{ 
			return (m_string != rhs.m_string) && (strcmp(m_string, rhs.m_string) != 0);
		}

		inline bool operator < (const CInPlaceStringStorage& rhs) const 
		{ 
			return strcmp(m_string, rhs.m_string) < 0;
		}

		inline bool operator <= (const CInPlaceStringStorage& rhs) const 
		{ 
			return strcmp(m_string, rhs.m_string) <= 0;
		}

		inline bool operator > (const CInPlaceStringStorage& rhs) const 
		{ 
			return strcmp(m_string, rhs.m_string) > 0;
		}

		inline bool operator >= (const CInPlaceStringStorage& rhs) const 
		{ 
			return strcmp(m_string, rhs.m_string) >= 0;
		}

	private:

		char m_string[SIZE];
	};

	// Ignore case string storage class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	class CIgnoreCaseStringStorage
	{
	public:
		CIgnoreCaseStringStorage() {}
		CIgnoreCaseStringStorage(tukk szString) : value(szString) {}

		inline tukk c_str() const { return value.c_str(); }

		inline CIgnoreCaseStringStorage& operator = (const CIgnoreCaseStringStorage& rhs)
		{
			value = rhs.value;
			return *this;
		}

		inline CIgnoreCaseStringStorage& operator = (tukk szString)
		{
			value = szString;
			return *this;
		}

		inline bool operator == (const CIgnoreCaseStringStorage& rhs) const { return value.compareNoCase(rhs.value) == 0; };
		inline bool operator != (const CIgnoreCaseStringStorage& rhs) const { return value.compareNoCase(rhs.value) != 0; };
		inline bool operator <  (const CIgnoreCaseStringStorage& rhs) const { return value.compareNoCase(rhs.value) < 0; };
		inline bool operator <= (const CIgnoreCaseStringStorage& rhs) const { return value.compareNoCase(rhs.value) <= 0; };
		inline bool operator >  (const CIgnoreCaseStringStorage& rhs) const { return value.compareNoCase(rhs.value) > 0; };
		inline bool operator >= (const CIgnoreCaseStringStorage& rhs) const { return value.compareNoCase(rhs.value) >= 0; };

	private:
		string value;
	};

	// Template string hash wrapper class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class HASH, class CONVERSION = CEmptyStringConversion, typename STORAGE = void> class CStringHashWrapper
	{
	public:

		typedef HASH       Hash;
		typedef CONVERSION Conversion;
		typedef STORAGE    Storage;

		inline CStringHashWrapper() {}

		inline CStringHashWrapper(const CStringHashWrapper& rhs)
			: m_hash(rhs.m_hash)
			, m_storage(rhs.m_storage)
		{}

		inline CStringHashWrapper(tukk szString)
		{
			FromString(szString);
		}

		inline const Hash& GetHash() const
		{
			return m_hash;
		}

		inline const Storage& GetStorage() const
		{
			return m_storage;
		}

		inline tukk c_str() const
		{
			return m_storage.c_str();
		}

		inline CStringHashWrapper& operator = (const CStringHashWrapper& rhs)
		{
			m_hash    = rhs.m_hash;
			m_storage = rhs.m_storage;
			return *this;
		}

		inline CStringHashWrapper& operator = (tukk szString)
		{
			FromString(szString);
			return *this;
		}

		inline bool operator == (const CStringHashWrapper& rhs) const 
		{ 
			return (m_hash == rhs.m_hash) && (m_storage == rhs.m_storage);
		}

		inline bool operator != (const CStringHashWrapper& rhs) const 
		{ 
			return (m_hash != rhs.m_hash) && (m_storage != rhs.m_storage);
		}

		inline bool operator < (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash == rhs.m_hash ? m_storage < rhs.m_storage : m_hash < rhs.m_hash;
		}

		inline bool operator <= (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash == rhs.m_hash ? m_storage <= rhs.m_storage : m_hash < rhs.m_hash;
		}

		inline bool operator > (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash == rhs.m_hash ? m_storage > rhs.m_storage : m_hash > rhs.m_hash;
		}

		inline bool operator >= (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash == rhs.m_hash ? m_storage >= rhs.m_storage : m_hash > rhs.m_hash;
		}

	private:

		inline void FromString(tukk szString)
		{
			const Conversion conversion(szString);
			m_hash    = conversion.c_str();
			m_storage = conversion.c_str();
		}

		Hash    m_hash;
		Storage m_storage;
	};

	// Template string hash wrapper class.
	////////////////////////////////////////////////////////////////////////////////////////////////////
	template <class HASH, class CONVERSION> class CStringHashWrapper<HASH, CONVERSION, void>
	{
	public:

		typedef HASH       Hash;
		typedef CONVERSION Conversion;

		inline CStringHashWrapper() {}

		inline CStringHashWrapper(const CStringHashWrapper& rhs)
			: m_hash(rhs.m_hash)
		{}

		inline CStringHashWrapper(tukk szString)
		{
			FromString(szString);
		}

		inline const Hash& GetHash() const
		{
			return m_hash;
		}

		inline CStringHashWrapper& operator = (const CStringHashWrapper& rhs)
		{
			m_hash = rhs.m_hash;
			return *this;
		}

		inline CStringHashWrapper& operator = (tukk szString)
		{
			FromString(szString);
			return *this;
		}

		inline bool operator == (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash == rhs.m_hash;
		}

		inline bool operator != (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash != rhs.m_hash;
		}

		inline bool operator < (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash < rhs.m_hash;
		}

		inline bool operator <= (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash <= rhs.m_hash;
		}

		inline bool operator > (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash > rhs.m_hash;
		}

		inline bool operator >= (const CStringHashWrapper& rhs) const 
		{ 
			return m_hash >= rhs.m_hash;
		}

	private:

		inline void FromString(tukk szString)
		{
			const Conversion conversion(szString);
			m_hash = conversion.c_str();
		}

		Hash m_hash;
	};

	template <class HASH, class CONVERSION, typename STORAGE>
	class CStringHashWrapperSerializer : public Serialization::IString
	{
		typedef CStringHashWrapper<HASH, CONVERSION, STORAGE> TValue;
	public:
		CStringHashWrapperSerializer(TValue& value) 
			: m_value(value) 
		{}

		virtual void set(tukk value) override 
		{ 
			m_value = value; 
		}

		virtual tukk get() const override
		{ 
			return m_value.c_str(); 
		}

		virtual ukk handle() const override
		{ 
			return &m_value; 
		}

		virtual Serialization::TypeID type() const override
		{ 
			return Serialization::TypeID::get<TValue>(); 
		}
	private:
		TValue& m_value;
	};


	template <class HASH, class CONVERSION, typename STORAGE> bool Serialize(Serialization::IArchive& archive, CStringHashWrapper<HASH, CONVERSION, STORAGE>& value, tukk szName, tukk szLabel)
	{
		CStringHashWrapperSerializer<HASH, CONVERSION, STORAGE> ser(value);
		return archive(static_cast<Serialization::IString&>(ser), szName, szLabel);
	}

	template <class HASH, class CONVERSION> bool Serialize(Serialization::IArchive& archive, CStringHashWrapper<HASH, CONVERSION>& value, tukk szName, tukk szLabel)
	{
		DrxFatalError("Cannot serialize string hash wrapper with no storage type!");
		return false;
	}
}

// Enable use of string hash wrapper class as key in std::unordered containers.
////////////////////////////////////////////////////////////////////////////////////////////////////
namespace std
{
	template <class HASH, class CONVERSION, typename STORAGE> struct hash<TemplateUtils::CStringHashWrapper<HASH, CONVERSION, STORAGE> >
	{
		inline size_t operator () (const TemplateUtils::CStringHashWrapper<HASH, CONVERSION, STORAGE>& rhs) const
		{
			return static_cast<size_t>(rhs.GetHash().GetValue());
		}
	};
}