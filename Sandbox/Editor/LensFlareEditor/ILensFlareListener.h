// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

class CLensFlareItem;
class CLensFlareElement;

class ILensFlareChangeItemListener
{
public:
	virtual void OnLensFlareChangeItem(CLensFlareItem* pLensFlareItem) = 0;
	virtual void OnLensFlareDeleteItem(CLensFlareItem* pLensFlareItem) = 0;
};

class ILensFlareChangeElementListener
{
public:
	virtual void OnLensFlareChangeElement(CLensFlareElement* pLensFlareElement) = 0;
};

