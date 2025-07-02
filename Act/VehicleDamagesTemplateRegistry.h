// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
   -------------------------------------------------------------------------
   $Id$
   $DateTime$
   Описание: Implements a registry for vehicle damages templates

   -------------------------------------------------------------------------
   История:
   - 18:07:2006: Created by Mathieu Pinard

*************************************************************************/
#ifndef __VEHICLEDAMAGESTEMPLATEREGISTRY_H__
#define __VEHICLEDAMAGESTEMPLATEREGISTRY_H__

class CVehicleDamagesGroup;

class CVehicleDamagesTemplateRegistry
	: public IVehicleDamagesTemplateRegistry
{
public:

	CVehicleDamagesTemplateRegistry() {}
	virtual ~CVehicleDamagesTemplateRegistry() {}

	virtual bool Init(const string& damagesTemplatesPath);
	virtual void Release() { delete this; }

	virtual bool RegisterTemplates(const string& filename);
	virtual bool UseTemplate(const string& templateName, IVehicleDamagesGroup* pDamagesGroup);

protected:

	typedef std::map<string, XmlNodeRef> TTemplateMap;
	TTemplateMap m_templates;
};

#endif
