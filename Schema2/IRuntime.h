// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// #SchematycTODO : Separate into multiple headers?
// #SchematycTODO : Do all of these types really need to live in public interface?

#pragma once

#include <drx3D/Schema2/Prerequisites.h>

#include <drx3D/Schema2/Any.h>
#include <drx3D/Schema2/BasicTypes.h>
#include <drx3D/Schema2/RuntimeParams.h>
#include <drx3D/Schema2/ScratchPad.h>

namespace sxema2
{
	class  CRuntimeNodeData;
	class  CRuntimeParams;
	struct IObject;
	
	enum class ERuntimeStatus
	{
		Repeat,
		Continue,
		End,
		Break,
		Return
	};

	struct SRuntimeResult
	{
		inline SRuntimeResult(ERuntimeStatus _status = ERuntimeStatus::End, u32 _outputIdx = s_invalidIdx)
			: status(_status)
			, outputIdx(_outputIdx)
		{}

		ERuntimeStatus status;
		u32         outputIdx;
	};

	enum class ERuntimeActivationFlags
	{
		None   = 0,
		Input  = BIT(0),
		Output = BIT(1),
		Repeat = BIT(2)
	};

	DECLARE_ENUM_CLASS_FLAGS(ERuntimeActivationFlags)

	struct SRuntimeActivationParams
	{
		inline SRuntimeActivationParams(ERuntimeActivationFlags _flags = ERuntimeActivationFlags::None, u32 _portIdx = s_invalidIdx)
			: flags(_flags)
			, portIdx(_portIdx)
		{}

		ERuntimeActivationFlags flags;
		u32                  portIdx;
	};

	typedef SRuntimeResult (*RuntimeNodeCallbackPtr)(IObject* pObject, const SRuntimeActivationParams& activationParams, CRuntimeNodeData& data);

	struct SRuntimeNodeAttribute
	{
		inline SRuntimeNodeAttribute(u32 _id = s_invalidId, u32 _pos = s_invalidIdx)
			: id(_id)
			, pos(_pos)
		{}

		u32 id;
		u32 pos;
	};

	class CRuntimeNode
	{
	public:

		inline CRuntimeNode(const SGUID& guid, RuntimeNodeCallbackPtr pCallback, u32 attributeCount, u32 inputCount, u32 outputCount)
			: m_guid(guid)
			, m_pCallback(pCallback)
			, m_attributeCount(attributeCount)
			, m_inputCount(inputCount)
			, m_outputCount(outputCount)
		{}

		inline SGUID GetGUID() const
		{
			return m_guid;
		}

		inline RuntimeNodeCallbackPtr GetCallback() const
		{
			return m_pCallback;
		}

		inline u32 GetAttributeCount() const
		{
			return m_attributeCount;
		}

		inline SRuntimeNodeAttribute* GetAttributes()
		{
			return reinterpret_cast<SRuntimeNodeAttribute*>(this + 1);
		}

		inline const SRuntimeNodeAttribute* GetAttributes() const
		{
			return reinterpret_cast<const SRuntimeNodeAttribute*>(this + 1);
		}

		inline u32 GetInputCount() const
		{
			return m_inputCount;
		}

		inline u32* GetInputs()
		{
			return reinterpret_cast<u32*>(GetAttributes() + m_attributeCount);
		}

		inline u32k* GetInputs() const
		{
			return reinterpret_cast<u32k*>(GetAttributes() + m_attributeCount);
		}

		inline u32 GetOutputCount() const
		{
			return m_outputCount;
		}

		inline u32* GetOutputs()
		{
			return reinterpret_cast<u32*>(GetInputs() + m_inputCount);
		}

		inline u32k* GetOutputs() const
		{
			return reinterpret_cast<u32k*>(GetInputs() + m_inputCount);
		}

	private:

		SGUID                  m_guid;
		RuntimeNodeCallbackPtr m_pCallback;
		u32                 m_attributeCount;
		u32                 m_inputCount;
		u32                 m_outputCount;
	};

	class CRuntimeSequenceOp
	{
	public:

		inline CRuntimeSequenceOp(u32 nodePos, const SRuntimeActivationParams& activationParams, u32 outputCount)
			: m_nodePos(nodePos)
			, m_activationParams(activationParams)
			, m_outputCount(outputCount)
		{}

		inline u32 GetNodePos() const
		{
			return m_nodePos;
		}

		inline const SRuntimeActivationParams& GetActivationParams() const
		{
			return m_activationParams;
		}

		inline u32 GetOutputCount() const // N.B. Only used during validation!
		{
			return m_outputCount;
		}

		inline u32* GetOutputs()
		{
			return reinterpret_cast<u32*>(this + 1);
		}

		inline u32k* GetOutputs() const
		{
			return reinterpret_cast<u32k*>(this + 1);
		}

		inline const CRuntimeSequenceOp* GetNext() const // N.B. Only used during validation!
		{
			return reinterpret_cast<const CRuntimeSequenceOp*>(reinterpret_cast<u32k*>(this + 1) + m_outputCount);
		}

	private:

		u32                   m_nodePos;
		SRuntimeActivationParams m_activationParams;
		u32                   m_outputCount;
	};

	struct SRuntimeFunctionParam
	{
		inline SRuntimeFunctionParam(u32 _id = s_invalidId, const EnvTypeId& _envTypeId = EnvTypeId())
			: id(_id)
			, envTypeId(_envTypeId)
		{}

		u32    id;
		EnvTypeId envTypeId;
	};

	//typedef std::vector<SRuntimeFunctionParam> RuntimeFunctionParams;

	class CRuntimeFunction
	{
	public:

		inline CRuntimeFunction(const SGUID& guid)
			: m_guid(guid)
		{}

		inline SGUID GetGUID() const
		{
			return m_guid;
		}

		inline u32 AddNode(const SGUID& guid, RuntimeNodeCallbackPtr pCallback, u32 attributeCount, u32 inputCount, u32 outputCount)
		{
			u32k pos = m_nodes.size();
			u32k size = sizeof(CRuntimeNode) + (sizeof(SRuntimeNodeAttribute) * attributeCount) + (sizeof(u32) * (inputCount + outputCount));
			m_nodes.resize(pos + size, 0);
			new (&m_nodes[pos]) CRuntimeNode(guid, pCallback, attributeCount, inputCount, outputCount);
			return pos;
		}

		inline CRuntimeNode* GetNode(u32 pos)
		{
			return reinterpret_cast<CRuntimeNode*>(&m_nodes[pos]);
		}

		inline const CRuntimeNode* GetNode(u32 pos) const
		{
			return reinterpret_cast<const CRuntimeNode*>(&m_nodes[pos]);
		}

		inline u32 AddSequenceOp(u32 nodeIdx, const SRuntimeActivationParams& activationParams, u32 outputCount)
		{
			u32k pos = m_sequenceOps.size();
			u32k size = sizeof(CRuntimeSequenceOp) + (sizeof(u32) * outputCount);
			m_sequenceOps.resize(pos + size, 0);
			new (&m_sequenceOps[pos]) CRuntimeSequenceOp(nodeIdx, activationParams, outputCount);
			return pos;
		}

		inline CRuntimeSequenceOp* GetSequenceOp(u32 pos)
		{
			return reinterpret_cast<CRuntimeSequenceOp*>(&m_sequenceOps[pos]);
		}

		inline const CRuntimeSequenceOp* GetSequenceOp(u32 pos) const
		{
			return reinterpret_cast<const CRuntimeSequenceOp*>(&m_sequenceOps[pos]);
		}

		inline CScratchPad& GetScratchPad()
		{
			return m_scratchPad;
		}

		inline const CScratchPad& GetScratchPad() const
		{
			return m_scratchPad;
		}

		inline bool Empty() const
		{
			return m_sequenceOps.empty();
		}

	private:

		SGUID                 m_guid;
		//RuntimeFunctionParams m_inputs;
		//RuntimeFunctionParams m_outputs;
		std::vector<u8>    m_nodes; // What's a good size to reserve?
		std::vector<u8>    m_sequenceOps; // Can we allocate nodes and sequence ops in a single block?
		CScratchPad           m_scratchPad;
	};

	class CRuntimeNodeData
	{
	public:

		inline CRuntimeNodeData(const CRuntimeFunction& function, const CRuntimeNode& node, const CRuntimeParams& inputs, CRuntimeParams& outputs, CScratchPad& scratchPad)
			: m_function(function)
			, m_node(node)
			, m_inputs(inputs)
			, m_outputs(outputs)
			, m_scratchPad(scratchPad)
		{}

		inline IAny* GetAttribute(u32 attributeId)
		{
			for(const SRuntimeNodeAttribute* pAttribute = m_node.GetAttributes(), *pEndAttribute = pAttribute + m_node.GetAttributeCount(); pAttribute != pEndAttribute; ++ pAttribute)
			{
				if(pAttribute->id == attributeId)
				{
					return m_scratchPad[pAttribute->pos];
				}
			}
			return nullptr;
		}

		inline IAny* GetAttribute(u32 attributeId, const EnvTypeId& typeId)
		{
			IAny* pAttribute = GetAttribute(attributeId);
			return pAttribute && (pAttribute->GetTypeInfo().GetTypeId().AsEnvTypeId() == typeId) ? pAttribute : nullptr;
		}

		template <typename TYPE> inline TYPE* GetAttribute(u32 attributeId)
		{
			IAny* pAttribute = GetAttribute(attributeId, GetEnvTypeId<TYPE>());
			return pAttribute ? static_cast<TYPE*>(pAttribute->ToVoidPtr()) : nullptr;
		}

		inline u32 GetInputCount() const
		{
			return m_node.GetInputCount();
		}

		inline const IAny* GetInput(u32 inputIdx) const
		{
			SXEMA2_SYSTEM_ASSERT(inputIdx < m_node.GetInputCount());
			if(inputIdx < m_node.GetInputCount())
			{
				u32k inputPos = m_node.GetInputs()[inputIdx];
				SXEMA2_SYSTEM_ASSERT(inputPos != s_invalidIdx);
				if(inputPos != s_invalidIdx)
				{
					return m_scratchPad[inputPos];
				}
			}
			return nullptr;
		}

		inline const IAny* GetInput(u32 inputIdx, const EnvTypeId& typeId) const
		{
			const IAny* pInput = GetInput(inputIdx);
			return pInput && (pInput->GetTypeInfo().GetTypeId().AsEnvTypeId() == typeId) ? pInput : nullptr;
		}

		template <typename TYPE> inline const TYPE* GetInput(u32 inputIdx) const
		{
			const IAny* pInput = GetInput(inputIdx, GetEnvTypeId<TYPE>());
			return pInput ? static_cast<const TYPE*>(pInput->ToVoidPtr()) : nullptr;
		}

		inline u32 GetOutputCount() const
		{
			return m_node.GetOutputCount();
		}

		inline IAny* GetOutput(u32 outputIdx)
		{
			SXEMA2_SYSTEM_ASSERT(outputIdx < m_node.GetOutputCount());
			if(outputIdx < m_node.GetOutputCount())
			{
				u32k outputPos = m_node.GetOutputs()[outputIdx];
				if(outputPos != s_invalidIdx)
				{
					return m_scratchPad[outputPos];
				}
			}
			return nullptr;
		}

		inline IAny* GetOutput(u32 outputIdx, const EnvTypeId& typeId)
		{
			IAny* pOutput = GetOutput(outputIdx);
			return pOutput && (pOutput->GetTypeInfo().GetTypeId().AsEnvTypeId() == typeId) ? pOutput : nullptr;
		}

		template <typename TYPE> inline TYPE* GetOutput(u32 outputIdx)
		{
			IAny* pOutput = GetOutput(outputIdx, GetEnvTypeId<TYPE>());
			return pOutput ? static_cast<TYPE*>(pOutput->ToVoidPtr()) : nullptr;
		}

	private:

		const CRuntimeFunction& m_function;
		const CRuntimeNode&     m_node;
		const CRuntimeParams&   m_inputs;
		CRuntimeParams&         m_outputs;
		CScratchPad&            m_scratchPad;
	};

	inline void ExecuteRuntimeFunction(const CRuntimeFunction& function, IObject* pObject, const CRuntimeParams& inputs, CRuntimeParams& outputs)
	{
		// #SchematycTODO : Verify inputs.
		if(!function.Empty())
		{
			u32                    pos = 0;
			const CRuntimeSequenceOp* pSequenceOp = function.GetSequenceOp(pos);
			if(pSequenceOp)
			{
				CScratchPad              scratchPad = function.GetScratchPad();
				SRuntimeActivationParams activationParams = pSequenceOp->GetActivationParams();
				std::vector<u32>      repeatSequenceOps;
				repeatSequenceOps.reserve(64);
				while(true)
				{
					const CRuntimeNode*  pNode = function.GetNode(pSequenceOp->GetNodePos());
					CRuntimeNodeData     nodeData(function, *pNode, inputs, outputs, scratchPad);
					const SRuntimeResult result = (*pNode->GetCallback())(pObject, activationParams, nodeData);
					if(result.status == ERuntimeStatus::Break)
					{
						while(!repeatSequenceOps.empty())
						{
							u32k nodePos = function.GetSequenceOp(repeatSequenceOps.back())->GetNodePos();
							repeatSequenceOps.pop_back();
							if(nodePos == pSequenceOp->GetNodePos())
							{
								break;
							}
						}
					}
					if(result.status == ERuntimeStatus::Repeat)
					{
						repeatSequenceOps.push_back(pos);
					}
					if(result.status != ERuntimeStatus::Return)
					{
						if(result.status != ERuntimeStatus::End)
						{
							pos = pSequenceOp->GetOutputs()[result.outputIdx];
							if(pos != s_invalidIdx)
							{
								pSequenceOp      = function.GetSequenceOp(pos);
								activationParams = pSequenceOp->GetActivationParams();
								continue;
							}
						}
						if(!repeatSequenceOps.empty())
						{
							pos = repeatSequenceOps.back();
							repeatSequenceOps.pop_back();
							pSequenceOp      = function.GetSequenceOp(pos);
							activationParams = pSequenceOp->GetActivationParams();
							activationParams.flags |= ERuntimeActivationFlags::Repeat;
						}
					}
					break;
				}
			}
		}
	}
}
