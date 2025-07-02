// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

#pragma once

#ifndef BehaviorTreeSerializationSupport_h
	#define BehaviorTreeSerializationSupport_h

	#include <drx3D/CoreX/Serialization/IArchive.h>
	#include <drx3D/CoreX/Serialization/DrxStrings.h>

namespace BehaviorTree
{

	#ifdef USING_BEHAVIOR_TREE_SERIALIZATION

		#define NODE_COMBOBOX_FIXED_WIDTH          "300"
		#define STATE_TRANSITION_EVENT_FIXED_WIDTH "150"
		#define LINE_NUMBER_FIXED_WIDTH            "58"

//! A simple helper class in which we store hints to the serialization code.
//! These hints can be used to control the layout/contents of the property tree in the editor.
struct NodeSerializationHints
{
	NodeSerializationHints()
		: showXmlLineNumbers(false)
		, showComments(false)
		, allowAutomaticMigration(false)
	{
	}

	bool showXmlLineNumbers;
	bool showComments;
	bool allowAutomaticMigration;
};

static void HandleXmlLineNumberSerialization(Serialization::IArchive& archive, u32k xmlLineNumber)
{
	const NodeSerializationHints* hintsContext = archive.context<NodeSerializationHints>();
	IF_LIKELY (hintsContext != nullptr)
	{
		if (hintsContext->showXmlLineNumbers)
		{
			stack_string xmlLineText;
			const bool isXmlLineKnown = (xmlLineNumber != 0u);     // A bit 'hacky' but XML lines start at 1, so this works when the document has not been saved to XML yet.
			if (isXmlLineKnown)
			{
				xmlLineText.Format("::: %u :::", xmlLineNumber);
			}
			else
			{
				xmlLineText = "::: ? :::";
			}

			archive(xmlLineText, "", "!^^>" LINE_NUMBER_FIXED_WIDTH ">");
		}
	}
}

static void HandleCommentSerialization(Serialization::IArchive& archive, string& comment)
{
	const NodeSerializationHints* pHintsContext = archive.context<NodeSerializationHints>();
	IF_LIKELY (pHintsContext != nullptr)
	{
		if (pHintsContext->showComments)
		{
			archive(comment, "comment", "^Comment");
		}
	}
}

	#endif

}

#endif                                                   // BehaviorTreeSerializationSupport_h
