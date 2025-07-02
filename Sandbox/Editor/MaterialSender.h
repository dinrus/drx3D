// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#ifndef __MaterialSender_h__
#define __MaterialSender_h__
#pragma once

#define WM_MATEDITSEND (WM_USER + 315)

enum EMaterialSenderMessage
{
	eMSM_Create              = 1,
	eMSM_GetSelectedMaterial = 2,
	eMSM_Init                = 3,
};

struct SMaterialMapFileHeader
{
	// max
	void SetMaxHWND(HWND hWnd)
	{
		hwndMax = (int64)hWnd;
	}
	HWND GetMaxHWND() const
	{
		return (HWND)hwndMax;
	}
	// editor
	void SetEditorHWND(HWND hWnd)
	{
		hwndMatEdit = (int64)hWnd;
	}
	HWND GetEditorHWND() const
	{
		return (HWND)hwndMatEdit;
	}
	int64  msg;// 64bits for both 32 and 64
	int64  Reserved;// 64bits for both 32 and 64
protected:
	uint64 hwndMax;// HWND for 32 and 64 is different
	uint64 hwndMatEdit;// HWND for 32 and 64 is different
};

class CMaterialSender
{
public:

	CMaterialSender(bool bIsMatEditor) : m_bIsMatEditor(bIsMatEditor)
	{
		m_h.SetEditorHWND(0);
		m_h.SetMaxHWND(0);
		m_h.msg = 0;
		hMapFile = 0;
	}

	CMaterialSender::~CMaterialSender()
	{
		if (hMapFile)
			CloseHandle(hMapFile);
		hMapFile = 0;
	}

	bool GetMessage()
	{
		LoadMapFile();
		return true;
	}

	bool CheckWindows()
	{
		if (!m_h.GetMaxHWND() || !m_h.GetEditorHWND() || !::IsWindow(m_h.GetMaxHWND()) || !::IsWindow(m_h.GetEditorHWND()))
			LoadMapFile();
		if (!m_h.GetMaxHWND() || !m_h.GetEditorHWND() || !::IsWindow(m_h.GetMaxHWND()) || !::IsWindow(m_h.GetEditorHWND()))
			return false;
		return true;
	}

	bool Create()
	{
		hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024 * 1024, "EditMatMappingObject");
		if (hMapFile)
			return true;

		DrxLog("Can't create File Map");

		return false;
	}

	bool SendMessage(i32 msg, const XmlNodeRef& node)
	{
		bool bRet = false;

		if (!CheckWindows())
			return false;

		m_h.msg = msg;

		i32 nDataSize = sizeof(SMaterialMapFileHeader) + strlen(node->getXML().c_str());

		//hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, nDataSize, "EditMatMappingObject");

		HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "EditMatMappingObject");
		if (hMapFile)
		{
			uk pMes = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, nDataSize);
			if (pMes)
			{
				memcpy(pMes, &m_h, sizeof(SMaterialMapFileHeader));
				strcpy(((tuk)pMes) + sizeof(SMaterialMapFileHeader), node->getXML().c_str());
				UnmapViewOfFile(pMes);
				if (m_bIsMatEditor)
					::SendMessage(m_h.GetMaxHWND(), WM_MATEDITSEND, msg, 0);
				else
					::SendMessage(m_h.GetEditorHWND(), WM_MATEDITSEND, msg, 0);
				bRet = true;
			}
			CloseHandle(hMapFile);
		}
		else
			DrxLog("No File Map");

		return bRet;
	}

	void SetupWindows(HWND hwndMax, HWND hwndMatEdit)
	{
		m_h.SetMaxHWND(hwndMax);
		m_h.SetEditorHWND(hwndMatEdit);
	}

private:

	bool LoadMapFile()
	{
		bool bRet = false;
		const HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "EditMatMappingObject");
		if (hMapFile)
		{
			uk const pMes = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);

			if (pMes)
			{
				memcpy(&m_h, pMes, sizeof(SMaterialMapFileHeader));
				tukk const pXml = ((tukk)pMes) + sizeof(SMaterialMapFileHeader);
				m_node = XmlHelpers::LoadXmlFromBuffer(pXml, strlen(pXml));
				UnmapViewOfFile(pMes);
				bRet = true;
			}

			CloseHandle(hMapFile);
		}

		return bRet;
	}

public:
	SMaterialMapFileHeader m_h;
	XmlNodeRef             m_node;
private:
	bool                   m_bIsMatEditor;
	HANDLE                 hMapFile;
};

#endif //__MaterialSender_h__

