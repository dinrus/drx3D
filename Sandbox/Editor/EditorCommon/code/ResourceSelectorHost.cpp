// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

//#include "../stdafx.h"
#include "../IResourceSelectorHost.h"

#include "../PreviewToolTip.h"

bool SStaticResourceSelectorEntry::UsesInputField() const
{
	return validate || validateWithContext;
}

void SStaticResourceSelectorEntry::EditResource(const SResourceSelectorContext& context, tukk value) const
{
	if (edit)
		edit(context, value);
}

dll_string SStaticResourceSelectorEntry::ValidateValue(const SResourceSelectorContext& context, tukk newValue, tukk previousValue) const
{
	dll_string result = previousValue;
	if (validate)
		result = validate(context, newValue, previousValue);
	else if (validateWithContext)
		result = validateWithContext(context, newValue, previousValue, context.contextObject);

	return result;
}

dll_string SStaticResourceSelectorEntry::SelectResource(const SResourceSelectorContext& context, tukk previousValue) const
{
	dll_string result = previousValue;
	if (function)
		result = function(context, previousValue);
	else if (functionWithContext)
		result = functionWithContext(context, previousValue, context.contextObject);

	return result;
}

bool SStaticResourceSelectorEntry::ShowTooltip(const SResourceSelectorContext& context, tukk value) const
{
	return CPreviewToolTip::ShowTrackingToolTip(value, context.parentWidget);
}

void SStaticResourceSelectorEntry::HideTooltip(const SResourceSelectorContext& context, tukk value) const
{
	QTrackingTooltip::HideTooltip();
}
