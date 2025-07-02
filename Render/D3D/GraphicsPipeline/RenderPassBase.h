// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#include <drx3D/CoreX/Containers/DrxArray.h>


class CGraphicsPipeline;


namespace ComputeRenderPassIsDirtyDetail
{
	template <i32 Size, i32 N, typename Tuple>
	struct is_tuple_trivial
	{
		// Check that data is a POD and not a pointer
		using T = typename std::tuple_element<N, Tuple>::type;
		using S = typename std::decay<T>::type;
		static constexpr bool value = !std::is_pointer<S>::value && std::is_trivial<S>::value && is_tuple_trivial<Size, N+1, Tuple>::value;
	};
	template <i32 Size, typename Tuple>
	struct is_tuple_trivial<Size, Size, Tuple>
	{
		static constexpr bool value = true;
	};

// 	template <typename Tuple>
// 	static constexpr bool is_tuple_trivial_v = is_tuple_trivial<std::tuple_size<Tuple>::value, 0, Tuple>::value;
}

class CRenderPassBase
{
	static constexpr size_t nInputVarsStaticSize = 16;

public:
	void SetLabel(tukk label)  { m_label = label; }
	tukk GetLabel() const      { return m_label.c_str(); }

	CGraphicsPipeline& GetGraphicsPipeline() const;

	virtual bool IsDirty() const { return false; }
	// Take a variadic list of trivial parameters. Returns true if and only if IsDirty() is set or the serialized data is different from the input. 
	// Note: Comparison is done as with memcmp on serialized blobs. Types are ignored.
	template <typename T, typename... Ts>
	bool IsDirty(T&& arg, Ts&&... args)
	{
		return IsDirty(std::make_tuple(std::forward<T>(arg), std::forward<Ts>(args)...));
	}
	template <typename... Ts>
	bool IsDirty(std::tuple<Ts...>&& tuple)
	{
		using Tuple = std::tuple<Ts...>;
		static constexpr size_t size = sizeof(Tuple);

		// Verify all types are trivial
		static_assert(ComputeRenderPassIsDirtyDetail::is_tuple_trivial<std::tuple_size<Tuple>::value, 0, Tuple>::value, "All input types must be trivial non-pointer types");

		// Check if input tuple equals the serialized one
		if (size == m_inputVars.size() * sizeof(decltype(m_inputVars)::value_type) &&
			*reinterpret_cast<const Tuple*>(m_inputVars.data()) == tuple)
		{
			// Nothing changed
			return IsDirty();
		}

		// Store new data
		m_inputVars.resize(size);
		*reinterpret_cast<Tuple*>(m_inputVars.data()) = std::move(tuple);

		return true;
	}

protected:
	string m_label;

private:
	LocalDynArray<u8, nInputVarsStaticSize> m_inputVars;
};
