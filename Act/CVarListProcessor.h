// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание:  console variables list processor
   -------------------------------------------------------------------------
   История:
   - 05/01/2007   00:31 : Created by Stas Spivakov
*************************************************************************/

#ifndef __CVARLISTPROCESSOR_H__
#define __CVARLISTPROCESSOR_H__

struct ICVar;

struct ICVarListProcessorCallback
{
	virtual ~ICVarListProcessorCallback(){}
	virtual void OnCVar(ICVar*) = 0;
};

class CCVarListProcessor
{
public:
	CCVarListProcessor(tukk path);
	~CCVarListProcessor();

	void Process(ICVarListProcessorCallback* cb);
private:
	string m_fileName;
};

#endif
