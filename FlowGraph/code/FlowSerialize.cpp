// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/FlowGraph/StdAfx.h>
#include <drx3D/FlowGraph/FlowSerialize.h>

class CFlowDataReadVisitor
{
public:
	CFlowDataReadVisitor(tukk data) : m_data(data), m_ok(false) {}

	void Visit(i32& i)
	{
		m_ok = 1 == sscanf(m_data, "%i", &i);
	}

	void Visit(float& i)
	{
		m_ok = 1 == sscanf(m_data, "%f", &i);
	}

	void Visit(EntityId& i)
	{
		m_ok = 1 == sscanf(m_data, "%u", &i);
	}

	void Visit(Vec3& i)
	{
		m_ok = 3 == sscanf(m_data, "%f,%f,%f", &i.x, &i.y, &i.z);
	}

	void Visit(string& i)
	{
		i = m_data;
		m_ok = true;
	}

	void Visit(bool& b)
	{
		i32 i;
		m_ok = 1 == sscanf(m_data, "%i", &i);
		b = i != 0;
	}

	void Visit(SFlowSystemVoid&)
	{
	}

	bool operator!() const
	{
		return !m_ok;
	}

	template<class T>
	void operator()(T& type)
	{
		Visit(type);
	}

	void operator()(TFlowInputDataVariant& var)
	{
		VisitVariant(var);
	}

private:
	template<size_t I = 0>
	void VisitVariant(TFlowInputDataVariant& var)
	{
		if (var.index() == I)
		{
			Visit(stl::get<I>(var));
		}
		else
		{
			VisitVariant<I + 1>(var);
		}
	}

	tukk m_data;
	bool        m_ok;
};
template<>
void CFlowDataReadVisitor::VisitVariant<stl::variant_size<TFlowInputDataVariant>::value>(TFlowInputDataVariant& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

class CFlowDataWriteVisitor
{
public:
	CFlowDataWriteVisitor(string& out) : m_out(out) {}

	void Visit(i32 i)
	{
		m_out.Format("%i", i);
	}

	void Visit(float i)
	{
		m_out.Format("%f", i);
	}

	void Visit(EntityId i)
	{
		m_out.Format("%u", i);
	}

	void Visit(Vec3 i)
	{
		m_out.Format("%f,%f,%f", i.x, i.y, i.z);
	}

	void Visit(const string& i)
	{
		m_out = i;
	}

	void Visit(bool b)
	{
		Visit(i32(b));
	}

	void Visit(SFlowSystemVoid)
	{
		m_out.resize(0);
	}

	template<class T>
	void operator()(T& type)
	{
		Visit(type);
	}

	void operator()(const TFlowInputDataVariant& var)
	{
		VisitVariant(var);
	}

private:
	template<size_t I = 0>
	void VisitVariant(const TFlowInputDataVariant& var)
	{
		if (var.index() == I)
		{
			Visit(stl::get<I>(var));
		}
		else
		{
			VisitVariant<I + 1>(var);
		}
	}

	string& m_out;
};
template<>
void CFlowDataWriteVisitor::VisitVariant<stl::variant_size<TFlowInputDataVariant>::value>(const TFlowInputDataVariant& var)
{
	DRX_ASSERT_MESSAGE(false, "Invalid variant index.");
}

bool SetFromString(TFlowInputData& value, tukk str)
{
	CFlowDataReadVisitor visitor(str);
	value.Visit(visitor);
	return !!visitor;
}

string ConvertToString(const TFlowInputData& value)
{
	string out;
	CFlowDataWriteVisitor visitor(out);
	value.Visit(visitor);
	return out;
}

bool SetAttr(XmlNodeRef node, tukk attr, const TFlowInputData& value)
{
	string out = ConvertToString(value);
	node->setAttr(attr, out.c_str());
	return true;
}
