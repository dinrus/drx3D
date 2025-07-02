// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __I_AI_MANNEQUIN__H__
#define __I_AI_MANNEQUIN__H__

namespace aiMannequin
{
//////////////////////////////////////////////////////////////////////////
enum EAiMannequinCommandType
{
	eMC_SetTag   = 0,
	eMC_ClearTag = 1,
};

//////////////////////////////////////////////////////////////////////////
struct SCommand
{
	u16 m_nextOffset;
	u16 m_type;
};

//////////////////////////////////////////////////////////////////////////
struct STagCommand
	: public SCommand
{
	u32 m_tagCrc;
};

//////////////////////////////////////////////////////////////////////////
struct SSetTagCommand
	: public STagCommand
{
	static u16k Type = eMC_SetTag;
};

//////////////////////////////////////////////////////////////////////////
struct SClearTagCommand
	: public STagCommand
{
	static u16k Type = eMC_ClearTag;
};
}

//////////////////////////////////////////////////////////////////////////
template<typename CommandBaseType, u16 kBufferSize>
class CCommandList
{
public:
	CCommandList()
		: m_usedBufferBytes(0)
	{
	}

	void ClearCommands()
	{
		m_usedBufferBytes = 0;
	}

	const CommandBaseType* GetFirstCommand() const
	{
		return GetCommand(0);
	}

	const CommandBaseType* GetNextCommand(const CommandBaseType* pCommand) const
	{
		assert(pCommand);
		return GetCommand(pCommand->m_nextOffset);
	}

	const CommandBaseType* GetCommand(u16k offset) const
	{
		u8k* pRawCommand = m_buffer + offset;
		const CommandBaseType* pCommand = reinterpret_cast<const CommandBaseType*>(pRawCommand);
		return (offset < m_usedBufferBytes) ? pCommand : NULL;
	}

	template<typename T>
	T* CreateCommand()
	{
		u16k commandSizeBytes = sizeof(T);
		u16k freeBufferBytes = GetFreeBufferBytes();
		if (freeBufferBytes < commandSizeBytes)
		{
			DRX_ASSERT(false);
			return NULL;
		}

		u8* pRawCommand = m_buffer + m_usedBufferBytes;
		CommandBaseType* pCommand = reinterpret_cast<CommandBaseType*>(pRawCommand);
		m_usedBufferBytes += commandSizeBytes;

		pCommand->m_nextOffset = m_usedBufferBytes;
		pCommand->m_type = T::Type;

		return reinterpret_cast<T*>(pCommand);
	}

	u16k GetBufferSizeBytes() const { return kBufferSize; }
	u16k GetUsedBufferBytes() const { return m_usedBufferBytes; }
	u16k GetFreeBufferBytes() const { return kBufferSize - m_usedBufferBytes; }

private:
	u16 m_usedBufferBytes;
	u8  m_buffer[kBufferSize];
};

//////////////////////////////////////////////////////////////////////////
template<u32 kBufferSize>
class CAIMannequinCommandList
{
public:
	const aiMannequin::SCommand* CreateSetTagCommand(u32k tagCrc)
	{
		aiMannequin::SSetTagCommand* pCommand = m_commandList.template CreateCommand<aiMannequin::SSetTagCommand>();
		if (pCommand)
		{
			pCommand->m_tagCrc = tagCrc;
		}
		return pCommand;
	}

	const aiMannequin::SCommand* CreateClearTagCommand(u32k tagCrc)
	{
		aiMannequin::SClearTagCommand* pCommand = m_commandList.template CreateCommand<aiMannequin::SClearTagCommand>();
		if (pCommand)
		{
			pCommand->m_tagCrc = tagCrc;
		}
		return pCommand;
	}

	void ClearCommands()
	{
		m_commandList.ClearCommands();
	}

	const aiMannequin::SCommand* GetFirstCommand() const
	{
		return m_commandList.GetFirstCommand();
	}

	const aiMannequin::SCommand* GetNextCommand(const aiMannequin::SCommand* pCommand) const
	{
		return m_commandList.GetNextCommand(pCommand);
	}

private:
	CCommandList<aiMannequin::SCommand, kBufferSize> m_commandList;
};

#endif
