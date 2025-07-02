// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

struct IObjectLayerManager
{
	//! Find layer by layer GUID.
	virtual IObjectLayer* FindLayer(DrxGUID guid) const = 0;

	//! Search for layer by name.
	virtual IObjectLayer* FindLayerByFullName(const string& layerFullName) const = 0;

	//! Search for layer by name.
	virtual IObjectLayer* FindLayerByName(const string& layerName) const = 0;
	
	//! Get this layer is current.
	virtual IObjectLayer*  GetCurrentLayer() const = 0;
};


