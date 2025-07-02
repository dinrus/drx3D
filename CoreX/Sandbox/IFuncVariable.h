// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Math/Drx_Vector2.h>
#include <drx3D/CoreX/Math/Drx_Vector3.h>
#include <drx3D/CoreX/Math/Drx_Matrix33.h>
#include <drx3D/CoreX/Math/Drx_Color.h>
#include <drx3D/CoreX/smartptr.h>
#include <drx3D/CoreX/String/StringUtils.h>

class ITexture;

enum FuncParamType
{
	e_FLOAT, e_INT, e_BOOL, e_VEC2, e_VEC3, e_VEC4, e_COLOR, e_MATRIX33,
	//! Though all types of textures are using the same class, it's important for editor to differentiate between them.
	e_TEXTURE2D, e_TEXTURE3D, e_TEXTURE_CUBE
};

class IFuncVariable : public _reference_target_t
{
public:
	// <interfuscator:shuffle>
	virtual ~IFuncVariable(){};

	virtual float     GetMin() const = 0;
	virtual float     GetMax() const = 0;

	virtual void      InvokeSetter(uk param) = 0;

	virtual i32       GetInt() const = 0;
	virtual float     GetFloat() const = 0;
	virtual bool      GetBool() const = 0;
	virtual Vec2      GetVec2() const = 0;
	virtual Vec3      GetVec3() const = 0;
	virtual Vec4      GetVec4() const = 0;
	virtual ColorF    GetColorF() const = 0;
	virtual Matrix33  GetMatrix33() const = 0;
	virtual ITexture* GetTexture() const = 0;
	// </interfuscator:shuffle>

	enum FuncParamType paramType; //!< float, string, i32, vec3 etc.
	string             name;
#if defined(FLARES_SUPPORT_EDITING)
	string             humanName;
	string             description;
#endif
};

template<class T>
class MFPVariable : public IFuncVariable
{
public:
	typedef void (T::* OpticsBase_MFPtr)();
	OpticsBase_MFPtr        pSetter;
	OpticsBase_MFPtr        pGetter;
	T*                      pObj;
	std::pair<float, float> range;

private:
	MFPVariable()
	{
		Set(e_INT, "", "", NULL, NULL, NULL);
	}

public:

	float GetMin() const { return range.first; }
	float GetMax() const { return range.second; }

	MFPVariable(FuncParamType type, tuk _humanname, tuk _description, T* obj, OpticsBase_MFPtr setter, OpticsBase_MFPtr getter, float fMin = 0, float fMax = 1.0f)
	{
		Set(type, _humanname, _description, obj, setter, getter, fMin, fMax);
	}

	void Set(FuncParamType type, tuk _humanname, tuk _description, T* obj, OpticsBase_MFPtr setter, OpticsBase_MFPtr getter, float fMin = 0, float fMax = 1.0f)
	{
		paramType = type;

		char _nameNoSpace[50];
		drx_strcpy(_nameNoSpace, _humanname);
		tuk p1 = _nameNoSpace;
		tuk p2 = p1;
		while (*p1 != 0)
		{
			if ((*p1) == ' ')
				++p1;
			else
				*p2++ = *p1++;
		}

		*p2 = 0;
		name = _nameNoSpace;

#if defined(FLARES_SUPPORT_EDITING)
		humanName = _humanname;
		description = _description;
#endif

		pObj = obj;
		pSetter = setter;
		pGetter = getter;
		range.first = fMin;
		range.second = fMax;
	}

#define INVOKE_SETTER(PARAM_TYPE, param)   (pObj->*(reinterpret_cast<void (T::*)(PARAM_TYPE)>(pSetter)))(*(PARAM_TYPE*)param)
#define INVOKE_SETTER_P(PARAM_TYPE, param) (pObj->*(reinterpret_cast<void (T::*)(PARAM_TYPE)>(pSetter)))((PARAM_TYPE)param)

	void InvokeSetter(uk param)
	{
		switch (paramType)
		{
		case e_FLOAT:
			INVOKE_SETTER(float, param);
			break;
		case e_INT:
			INVOKE_SETTER(i32, param);
			break;
		case e_VEC2:
			INVOKE_SETTER(Vec2, param);
			break;
		case e_VEC3:
			INVOKE_SETTER(Vec3, param);
			break;
		case e_VEC4:
			INVOKE_SETTER(Vec4, param);
			break;
		case e_BOOL:
			INVOKE_SETTER(bool, param);
			break;
		case e_COLOR:
			INVOKE_SETTER(ColorF, param);
			break;
		case e_MATRIX33:
			INVOKE_SETTER(Matrix33, param);
			break;
		case e_TEXTURE2D:
			INVOKE_SETTER_P(ITexture*, param);
			break;
		case e_TEXTURE3D:
			INVOKE_SETTER_P(ITexture*, param);
			break;
		case e_TEXTURE_CUBE:
			INVOKE_SETTER_P(ITexture*, param);
			break;
		}
	}

#define INVOKE_GETTER(PARAM_TYPE) ((pObj->*reinterpret_cast<PARAM_TYPE (T::*)()>(pGetter))())

	i32       GetInt() const      { return INVOKE_GETTER(i32); }
	float     GetFloat() const    { return INVOKE_GETTER(float); }
	bool      GetBool() const     { return INVOKE_GETTER(bool); }
	Vec2      GetVec2() const     { return INVOKE_GETTER(Vec2); }
	Vec3      GetVec3() const     { return INVOKE_GETTER(Vec3); }
	Vec4      GetVec4() const     { return INVOKE_GETTER(Vec4); }
	ColorF    GetColorF() const   { return INVOKE_GETTER(ColorF); }
	Matrix33  GetMatrix33() const { return INVOKE_GETTER(Matrix33); }
	ITexture* GetTexture() const  { return INVOKE_GETTER(ITexture*); }
};

class FuncVariableGroup
{
private:
	DynArray<_smart_ptr<IFuncVariable>> variables;
	string                              m_name;
#if defined(FLARES_SUPPORT_EDITING)
	string                              m_humanname;
#endif
	bool                                bCollapse;

public:
	FuncVariableGroup() : bCollapse(false)
	{
		SetName("");
	}

	~FuncVariableGroup()
	{
	}

	void SetName(tukk name, tukk humanname = 0)
	{
		if (!name)
			return;
		m_name = name;
#if defined(FLARES_SUPPORT_EDITING)
		m_humanname = humanname ? humanname : name;
#endif
	}

	tukk GetName()
	{
		return m_name.c_str();
	}

#if defined(FLARES_SUPPORT_EDITING)
	tukk GetHumanName()
	{
		return m_humanname.c_str();
	}
#endif

	void SetCollapse(bool _bCollapse)
	{
		bCollapse = _bCollapse;
	}

	bool IsCollapse()
	{
		return bCollapse;
	}

	IFuncVariable* FindVariable(tukk name)
	{
		for (i32 i = 0, iSize(variables.size()); i < iSize; ++i)
		{
			if (variables[i] == NULL)
				continue;
			if (!strcmp(variables[i]->name.c_str(), name))
				return variables[i];
		}
		return NULL;
	}

	void           SetVariable(i32 nIndex, IFuncVariable* v) { variables[nIndex] = v; }

	i32            GetVariableCount()                        { return variables.size(); }

	IFuncVariable* GetVariable(i32 nIndex)
	{
		return variables[nIndex];
	}

	void AddVariable(IFuncVariable* var)
	{
		variables.push_back(var);
	}
};
