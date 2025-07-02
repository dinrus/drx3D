// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

// -------------------------------------------------------------------------
//  Имя файла:   ExcelExport.h
//  Created:     19/03/2008 by Timur.
//  Описание: Implementation of the DinrusX Unit Testing framework
// -------------------------------------------------------------------------
//  История:
//
////////////////////////////////////////////////////////////////////////////

#pragma once
#include <drx3D/Sys/IXml.h>

// Base class for custom DinrusX excel exporers
class CExcelExportBase
{
public:
	enum CellFlags
	{
		CELL_BOLD     = 0x0001,
		CELL_CENTERED = 0x0002,
	};

	bool       SaveToFile(tukk filename);

	void       InitExcelWorkbook(XmlNodeRef Workbook);

	XmlNodeRef NewWorksheet(tukk name);
	void       AddCell(float number);
	void       AddCell(i32 number);
	void       AddCell(u32 number);
	void       AddCell(uint64 number) { AddCell((u32)number); };
	void       AddCell(int64 number)  { AddCell((i32)number); };
	void       AddCell(tukk str, i32 flags = 0);
	void       AddCellAtIndex(i32 nIndex, tukk str, i32 flags = 0);
	void       SetCellFlags(XmlNodeRef cell, i32 flags);
	void       AddRow();
	void       AddCell_SumOfRows(i32 nRows);
	string     GetXmlHeader();

protected:
	XmlNodeRef m_Workbook;
	XmlNodeRef m_CurrTable;
	XmlNodeRef m_CurrWorksheet;
	XmlNodeRef m_CurrRow;
	XmlNodeRef m_CurrCell;
};
