// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

#include <drx3D/Animation/stdafx.h>
#include <drx3D/Animation/FaceChannelKeyCleanup.h>

#include <drx3D/Animation/FaceAnimSequence.h>

class FaceChannelCleanupKeysKeyEntry
{
public:
	i32   parent;
	i32   children[2];
	i32   links[2];
	i32   currentList;
	i32   neighbours[2];
	i32   count;
	float time;
	float value;
	float error;
	i32   sortChildren[2];
	i32   sortParent;
};

enum
{
	ACTIVE_KEYS_LIST     = -1,
	REEVALUATE_KEYS_LIST = -2,
	NEIGHBOUR_LIST       = -3,
	SORT_TREE            = -1, // OK to be duplicate.
	MAIN_TREE            = -2,
	KEY_LIST_COUNT       = 3
};

enum
{
	PREVIOUS,
	NEXT
};

#ifdef _DEBUG
void VerifyStructure(i32 numKeys, FaceChannelCleanupKeysKeyEntry* keyEntries)
{
	enum CheckIndexOrderStatus
	{
		NoCheckIndexOrder,
		CheckIndexOrder
	};

	class ListDef
	{
	public:
		ListDef(i32 _startIndex, i32 (FaceChannelCleanupKeysKeyEntry::* _links)[2], CheckIndexOrderStatus _checkIndexOrder, float (FaceChannelCleanupKeysKeyEntry::* _sortKey))
			: startIndex(_startIndex), links(_links), checkIndexOrder(_checkIndexOrder), sortKey(_sortKey) {}
		i32                   startIndex;
		i32 (FaceChannelCleanupKeysKeyEntry::* links)[2];
		CheckIndexOrderStatus checkIndexOrder;
		float (FaceChannelCleanupKeysKeyEntry::* sortKey);
	};

	ListDef listDefs[] = {
		ListDef(ACTIVE_KEYS_LIST,     &FaceChannelCleanupKeysKeyEntry::links,      NoCheckIndexOrder, &FaceChannelCleanupKeysKeyEntry::error),
		ListDef(REEVALUATE_KEYS_LIST, &FaceChannelCleanupKeysKeyEntry::links,      NoCheckIndexOrder, 0),
		ListDef(NEIGHBOUR_LIST,       &FaceChannelCleanupKeysKeyEntry::neighbours, CheckIndexOrder,   0)
	};
	enum {LIST_DEF_COUNT = DRX_ARRAY_COUNT(listDefs)};

	for (i32 listIndex = 0; listIndex < LIST_DEF_COUNT; ++listIndex)
	{
		i32 startIndex = listDefs[listIndex].startIndex;
		i32 (FaceChannelCleanupKeysKeyEntry::* links)[2] = listDefs[listIndex].links;
		for (i32 index = (keyEntries[startIndex].*links)[NEXT]; index >= 0; index = (keyEntries[index].*links)[NEXT])
		{
			assert((keyEntries[(keyEntries[index].*links)[NEXT]].*links)[PREVIOUS] == index);
			assert((keyEntries[(keyEntries[index].*links)[PREVIOUS]].*links)[NEXT] == index);
			if (listDefs[listIndex].checkIndexOrder == CheckIndexOrder)
			{
				assert((keyEntries[index].*links)[PREVIOUS] < 0 || index > (keyEntries[index].*links)[PREVIOUS]);
				assert((keyEntries[index].*links)[NEXT] < 0 || index < (keyEntries[index].*links)[NEXT]);
			}
			if (float (FaceChannelCleanupKeysKeyEntry::* sortKey) = listDefs[listIndex].sortKey)
			{
				assert((keyEntries[index].*links)[PREVIOUS] < 0 || keyEntries[index].*sortKey >= keyEntries[(keyEntries[index].*links)[PREVIOUS]].*sortKey);
				assert((keyEntries[index].*links)[NEXT] < 0 || keyEntries[index].*sortKey <= keyEntries[(keyEntries[index].*links)[NEXT]].*sortKey);
			}
		}
	}

	class TreeEntry
	{
	public:
		TreeEntry(i32 _root, i32 (FaceChannelCleanupKeysKeyEntry::* _parent), i32 (FaceChannelCleanupKeysKeyEntry::* _children)[2])
			: root(_root), parent(_parent), children(_children) {}
		i32 root;
		i32 (FaceChannelCleanupKeysKeyEntry::* parent);
		i32 (FaceChannelCleanupKeysKeyEntry::* children)[2];
	};
	const TreeEntry treeEntries[] =
	{
		TreeEntry(keyEntries[SORT_TREE].sortChildren[1], &FaceChannelCleanupKeysKeyEntry::sortParent, &FaceChannelCleanupKeysKeyEntry::sortChildren),
		TreeEntry(MAIN_TREE,                             &FaceChannelCleanupKeysKeyEntry::parent,     &FaceChannelCleanupKeysKeyEntry::children)
	};
	enum {NUM_TREE_REMOVAL_ENTRIES = DRX_ARRAY_COUNT(treeEntries)};
	i32 countChangedItem = -1;
	for (i32 treeIndex = 0; treeIndex < NUM_TREE_REMOVAL_ENTRIES; ++treeIndex)
	{
		i32 root = treeEntries[treeIndex].root;
		i32 (FaceChannelCleanupKeysKeyEntry::* const parentPtr) = treeEntries[treeIndex].parent;
		i32 (FaceChannelCleanupKeysKeyEntry::* const childrenPtr)[2] = treeEntries[treeIndex].children;

		class TreeRecurser
		{
		public:
			TreeRecurser(FaceChannelCleanupKeysKeyEntry* _keyEntries, i32 (FaceChannelCleanupKeysKeyEntry::* _parent), i32 (FaceChannelCleanupKeysKeyEntry::* _children)[2])
				: keyEntries(_keyEntries), parent(_parent), children(_children) {}

			void operator()(i32 index)
			{
				for (i32 i = 0; i < 2; ++i)
				{
					i32 child = (keyEntries[index].*children)[i];
					if (child >= 0)
					{
						assert((keyEntries[child].*parent) == index);
						(*this)(child);
					}
				}
			}

		private:
			FaceChannelCleanupKeysKeyEntry* keyEntries;
			i32 (FaceChannelCleanupKeysKeyEntry::* parent);
			i32 (FaceChannelCleanupKeysKeyEntry::* children)[2];
		};

		TreeRecurser(keyEntries, parentPtr, childrenPtr)(root);
	}

	for (i32 index = 0; index < numKeys; ++index)
	{
		if (keyEntries[index].currentList == ACTIVE_KEYS_LIST)
		{
			for (i32 child = index, ancestor = keyEntries[child].sortParent; ancestor >= 0; child = ancestor, ancestor = keyEntries[ancestor].sortParent)
			{
				assert(keyEntries[ancestor].sortChildren[0] == child || keyEntries[ancestor].sortChildren[1] == child);
			}
		}
	}
}
	#define VERIFY_STRUCTURE(numKeys, entries) VerifyStructure(numKeys, entries)
#else // DEBUG
	#define VERIFY_STRUCTURE(numKeys, entries)
#endif //_DEBUG

void FaceChannel::CleanupKeys(CFacialAnimChannelInterpolator* pSpline, float errorMax)
{
	// Create a copy of the spline - we need this so that error calculations are made against the original
	// spline rather than the partially simplified one.
	CFacialAnimChannelInterpolator backupSpline(*pSpline);

	// Create the key entries array.
	i32 numKeys = (pSpline ? pSpline->num_keys() : 0);
	std::vector<FaceChannelCleanupKeysKeyEntry> keyEntryVector;
	keyEntryVector.resize(numKeys + KEY_LIST_COUNT);
	FaceChannelCleanupKeysKeyEntry* keyEntries = &keyEntryVector[KEY_LIST_COUNT];

	FaceChannelCleanupKeysKeyEntry& reevaluateKeysList = keyEntries[REEVALUATE_KEYS_LIST];
	FaceChannelCleanupKeysKeyEntry& activeKeysList = keyEntries[ACTIVE_KEYS_LIST];
	FaceChannelCleanupKeysKeyEntry& neighbourList = keyEntries[NEIGHBOUR_LIST];
	FaceChannelCleanupKeysKeyEntry& sortTree = keyEntries[SORT_TREE];
	FaceChannelCleanupKeysKeyEntry& mainTree = keyEntries[MAIN_TREE];

	// Initialize the key parameters.
	for (i32 index = 0; index < numKeys; ++index)
	{
		keyEntries[index].time = (pSpline ? pSpline->GetKeyTime(index) : 0);
		if (pSpline)
			pSpline->GetKeyValueFloat(index, keyEntries[index].value);
	}

	// Initialize the tree structure.
	mainTree.children[0] = mainTree.children[1] = -1;
	for (i32 index = 0; index < numKeys; ++index)
		keyEntries[index].children[0] = keyEntries[index].children[1] = -1;
	for (i32 index = 0; index < numKeys; ++index)
	{
		// Check whether this item is the parent of the hierarchy.
		if ((index & (index + 1)) == 0 && ((index << 1) + 1) >= numKeys)
		{
			keyEntries[index].parent = MAIN_TREE;
			keyEntries[MAIN_TREE].children[0] = index;
		}
		else
		{
			// Use bit tricks to figure out the parent index of the current entry.
			i32 parent = index;
			do
			{
				i32 parentTail = (parent ^ (parent + 1));
				parent = (parent & ~(parentTail << 1)) | parentTail;
			}
			while (parent >= numKeys);

			keyEntries[index].parent = parent;
			keyEntries[parent].children[index > parent ? 1 : 0] = index;
		}
	}

	// Initialize the counts.
	keyEntries[-1].count = 0; // This value will be accessed for any null child references - more efficient than special cases.
	for (i32 level = 0; ((1 << level) - 1) < numKeys; ++level)
	{
		for (i32 index = (1 << level) - 1; index < numKeys; index += (1 << (level + 1)))
		{
			keyEntries[index].count = 1;
			for (i32 childIndex = 0; childIndex < 2; ++childIndex)
			{
				i32 child = keyEntries[index].children[childIndex];
				if (child >= 0)
					keyEntries[index].count += keyEntries[child].count;
			}
		}
	}
	assert(pSpline->num_keys() == keyEntries[mainTree.children[0]].count);

	// Add all the keys to the list of keys to reevaluate, and the list of neighbours.
	reevaluateKeysList.links[NEXT] = REEVALUATE_KEYS_LIST;
	reevaluateKeysList.links[PREVIOUS] = REEVALUATE_KEYS_LIST;
	neighbourList.neighbours[NEXT] = NEIGHBOUR_LIST;
	neighbourList.neighbours[PREVIOUS] = NEIGHBOUR_LIST;
	neighbourList.time = 0.0f;
	neighbourList.value = 0.0f;
	for (i32 index = 0; index < numKeys; ++index)
	{
		const bool selected = pSpline->IsKeySelectedAtAnyDimension(index);

		if (selected)
		{
			keyEntries[index].links[NEXT] = REEVALUATE_KEYS_LIST;
			keyEntries[index].links[PREVIOUS] = reevaluateKeysList.links[PREVIOUS];
			keyEntries[reevaluateKeysList.links[PREVIOUS]].links[NEXT] = index;
			reevaluateKeysList.links[PREVIOUS] = index;
			keyEntries[index].currentList = REEVALUATE_KEYS_LIST;

			keyEntries[index].neighbours[NEXT] = NEIGHBOUR_LIST;
			keyEntries[index].neighbours[PREVIOUS] = neighbourList.neighbours[PREVIOUS];
			keyEntries[neighbourList.neighbours[PREVIOUS]].neighbours[NEXT] = index;
			neighbourList.neighbours[PREVIOUS] = index;
		}
	}

	// Initialize the active list and the sort tree.
	activeKeysList.links[NEXT] = ACTIVE_KEYS_LIST;
	activeKeysList.links[PREVIOUS] = ACTIVE_KEYS_LIST;
	sortTree.sortParent = SORT_TREE;
	sortTree.sortChildren[0] = SORT_TREE;
	sortTree.sortChildren[1] = SORT_TREE;
	sortTree.error = FLT_MAX;
	sortTree.currentList = ACTIVE_KEYS_LIST;

	// Loop until we have finished deleting keys.
	VERIFY_STRUCTURE(numKeys, keyEntries);
	for (;; )
	{
		// Calculate the error that would be introduced if we delete this key. Do this for each
		// key in the reevaluate list.
		VERIFY_STRUCTURE(numKeys, keyEntries);
		for (i32 index = reevaluateKeysList.links[NEXT], next = keyEntries[index].links[NEXT]; index >= 0; index = next, next = keyEntries[index].links[NEXT])
		{
			assert(keyEntries[index].currentList == REEVALUATE_KEYS_LIST);

			// Reevaluate the potential error for deleting this key.
			//for (i32 testWithDeletedKey = 0; testWithDeletedKey < 2; ++testWithDeletedKey)
			{
				i32 testWithDeletedKey = 1;

				// We have to calculate the key index by finding the number of keys before us.
				i32 keyIndex = 0;
				CFacialAnimChannelInterpolator::key_type key;

				if (testWithDeletedKey)
				{
					keyIndex = keyEntries[keyEntries[index].children[0]].count;
					for (i32 ancestor = keyEntries[index].parent, child = index; child >= 0; child = ancestor, ancestor = keyEntries[ancestor].parent)
					{
						if (child == keyEntries[ancestor].children[1])
							keyIndex += keyEntries[keyEntries[ancestor].children[0]].count + 1;
					}

					// Temporarily delete the key.
					assert(keyIndex >= 0 && keyIndex < pSpline->num_keys());
					assert(pSpline->time(keyIndex) == keyEntries[index].time);
					assert(pSpline->value(keyIndex) == keyEntries[index].value);
					key = pSpline->key(keyIndex);
					pSpline->erase(keyIndex);
				}

				// Test at critical points.
				enum {NUM_TEST_POINTS = 3};
				float testPoints[NUM_TEST_POINTS] = { keyEntries[index].time };
				{
					// Find the midpoint of the interval to the left of the key.
					i32 lprev = keyEntries[index].neighbours[PREVIOUS];
					i32 prevPrev = keyEntries[lprev].neighbours[PREVIOUS];
					testPoints[1] = (lprev >= 0 && prevPrev >= 0 ? (keyEntries[lprev].time + keyEntries[prevPrev].time) / 2 : testPoints[0]);

					// Find the midpoint of the interval to the right of the key.
					i32 lnext = keyEntries[index].neighbours[NEXT];
					i32 nextNext = keyEntries[lnext].neighbours[NEXT];
					testPoints[2] = (lnext >= 0 && nextNext >= 0 ? (keyEntries[lnext].time + keyEntries[nextNext].time) / 2 : testPoints[0]);
				}

				float maxError = 0.0f;
				for (i32 testPointIndex = 0; testPointIndex < NUM_TEST_POINTS; ++testPointIndex)
				{
					float newValue, oldValue;
					pSpline->InterpolateFloat(testPoints[testPointIndex], newValue);
					backupSpline.InterpolateFloat(testPoints[testPointIndex], oldValue);
					float error = fabsf(newValue - oldValue);
					if (maxError < error)
						maxError = error;
				}

				if (testWithDeletedKey)
					pSpline->insert_key(key);

				keyEntries[index].error = maxError;
			}

			// Find the position in the sort tree at which to add the node.
			i32 position = SORT_TREE;
			i32 relative = 1;
			for (i32 child = sortTree.sortChildren[1]; child >= 0; )
			{
				position = child;
				float error = keyEntries[index].error;
				float nodeError = keyEntries[child].error;
				relative = (error > nodeError ? 1 : 0);
				child = keyEntries[child].sortChildren[relative];
			}

			// Add the node as a child of the sort tree node.
			keyEntries[position].sortChildren[relative] = index;
			keyEntries[index].sortParent = position;
			keyEntries[index].sortChildren[0] = keyEntries[index].sortChildren[1] = SORT_TREE;

			// Add the node to the active list at the given point (either before or after the sort tree node).
			{
				assert(keyEntries[position].currentList == ACTIVE_KEYS_LIST);
				i32k direction = relative;
				i32k otherDirection = 1 - relative;
				keyEntries[index].links[direction] = keyEntries[position].links[direction];
				keyEntries[keyEntries[index].links[direction]].links[otherDirection] = index;
				keyEntries[index].links[otherDirection] = position;
				keyEntries[position].links[direction] = index;
				keyEntries[index].currentList = ACTIVE_KEYS_LIST;
			}
		}

		// Reinitialize the reevaluate list.
		reevaluateKeysList.links[NEXT] = REEVALUATE_KEYS_LIST;
		reevaluateKeysList.links[PREVIOUS] = REEVALUATE_KEYS_LIST;

		VERIFY_STRUCTURE(numKeys, keyEntries);

		// Delete all the keys we can.
		i32 numKeysDeleted = 0;
		for (;; )
		{
			i32 index = activeKeysList.links[NEXT];
			float error = keyEntries[index].error;
			if (error > errorMax)
				break;

			// Make a list of the six nearest neighbours of the key and place them in the re-evaluate
			// list (since these keys will have been affected by deleting this key).
			i32 keysToRemove2[7] = { index };
			i32 numKeysToRemove2 = 1;
			for (i32 direction = 0; direction < 2; ++direction)
			{
				i32 position = index;
				for (i32 distance = 1; distance <= 3; ++distance)
				{
					position = keyEntries[position].neighbours[direction];
					if (position < 0)
						break;
					if (keyEntries[position].currentList == ACTIVE_KEYS_LIST)
						keysToRemove2[numKeysToRemove2++] = position;
				}
			}

			// Remove all the keys from the active list. Unfortunately they are not necessarily
			// contiguous in the active list, so they must be individually removed.
			for (i32 removeIndex = 0, indexToRemove = -1; (indexToRemove = keysToRemove2[removeIndex]), removeIndex < numKeysToRemove2; ++removeIndex)
			{
				keyEntries[keyEntries[indexToRemove].links[PREVIOUS]].links[NEXT] = keyEntries[indexToRemove].links[NEXT];
				keyEntries[keyEntries[indexToRemove].links[NEXT]].links[PREVIOUS] = keyEntries[indexToRemove].links[PREVIOUS];
			}
			VERIFY_STRUCTURE(numKeys, keyEntries);

			// Delete the key from the spline.
			{
				i32 keyIndex = keyEntries[keyEntries[index].children[0]].count;
				for (i32 ancestor = keyEntries[index].parent, child = index; child >= 0; child = ancestor, ancestor = keyEntries[ancestor].parent)
				{
					if (child == keyEntries[ancestor].children[1])
						keyIndex += keyEntries[keyEntries[ancestor].children[0]].count + 1;
				}
				assert(keyIndex >= 0 && keyIndex < pSpline->num_keys());
				pSpline->erase(keyIndex);
			}

			// Remove the key from the neighbour list.
			keyEntries[keyEntries[index].neighbours[NEXT]].neighbours[PREVIOUS] = keyEntries[index].neighbours[PREVIOUS];
			keyEntries[keyEntries[index].neighbours[PREVIOUS]].neighbours[NEXT] = keyEntries[index].neighbours[NEXT];

			// Remove all the keys from the sort tree and the main tree.
			class TreeRemovalEntry
			{
			public:
				TreeRemovalEntry(i32 (FaceChannelCleanupKeysKeyEntry::* _parent), i32 (FaceChannelCleanupKeysKeyEntry::* _children)[2], i32 _numItemsToRemove, i32* _itemsToRemove)
					: parent(_parent), children(_children), numItemsToRemove(_numItemsToRemove), itemsToRemove(_itemsToRemove) {}
				i32 (FaceChannelCleanupKeysKeyEntry::* parent);
				i32 (FaceChannelCleanupKeysKeyEntry::* children)[2];
				i32  numItemsToRemove;
				i32* itemsToRemove;
			};
			const TreeRemovalEntry treeRemovalEntries[] =
			{
				TreeRemovalEntry(&FaceChannelCleanupKeysKeyEntry::sortParent, &FaceChannelCleanupKeysKeyEntry::sortChildren, numKeysToRemove2, keysToRemove2),
				TreeRemovalEntry(&FaceChannelCleanupKeysKeyEntry::parent,     &FaceChannelCleanupKeysKeyEntry::children,     1,                keysToRemove2)
			};
			enum {NUM_TREE_REMOVAL_ENTRIES = DRX_ARRAY_COUNT(treeRemovalEntries)};
			i32 countChangedItem = -1;
			for (i32 treeIndex = 0; treeIndex < NUM_TREE_REMOVAL_ENTRIES; ++treeIndex)
			{
				i32 (FaceChannelCleanupKeysKeyEntry::* const parentPtr) = treeRemovalEntries[treeIndex].parent;
				i32 (FaceChannelCleanupKeysKeyEntry::* const childrenPtr)[2] = treeRemovalEntries[treeIndex].children;
				i32k numKeysToRemove = treeRemovalEntries[treeIndex].numItemsToRemove;
				i32k* const keysToRemove = treeRemovalEntries[treeIndex].itemsToRemove;

				for (i32 removeIndex = 0, indexToRemove = -1; (indexToRemove = keysToRemove[removeIndex]), removeIndex < numKeysToRemove; ++removeIndex)
				{
					keyEntries[indexToRemove].currentList = -5;

					i32 parent = (keyEntries[indexToRemove].*parentPtr);
					i32 relative = (indexToRemove == (keyEntries[parent].*childrenPtr)[1] ? 1 : 0);

					if ((keyEntries[indexToRemove].*childrenPtr)[0] < 0 || (keyEntries[indexToRemove].*childrenPtr)[1] < 0)
					{
						VERIFY_STRUCTURE(numKeys, keyEntries);
						i32 replacementChild = ((keyEntries[indexToRemove].*childrenPtr)[0] < 0 ? 1 : 0);
						(keyEntries[(keyEntries[indexToRemove].*childrenPtr)[replacementChild]].*parentPtr) = parent;
						(keyEntries[parent].*childrenPtr)[relative] = (keyEntries[indexToRemove].*childrenPtr)[replacementChild];
						countChangedItem = parent;
						VERIFY_STRUCTURE(numKeys, keyEntries);
					}
					else
					{
						VERIFY_STRUCTURE(numKeys, keyEntries);
						// Find the largest item that is smaller than the item to delete.
						i32 replacement = indexToRemove;
						for (i32 child = (keyEntries[indexToRemove].*childrenPtr)[0]; child >= 0; replacement = child, child = (keyEntries[child].*childrenPtr)[1])
							;

						// Remove the item - we know it has no right child.
						i32 replacementParent = (keyEntries[replacement].*parentPtr);
						countChangedItem = (replacementParent == indexToRemove ? replacement : replacementParent);
						i32 replacementRelative = (replacement == (keyEntries[replacementParent].*childrenPtr)[1] ? 1 : 0);
						(keyEntries[(keyEntries[replacement].*childrenPtr)[0]].*parentPtr) = replacementParent;
						(keyEntries[replacementParent].*childrenPtr)[replacementRelative] = (keyEntries[replacement].*childrenPtr)[0];

						// Replace the node with the replacement node.
						(keyEntries[replacement].*parentPtr) = parent;
						(keyEntries[parent].*childrenPtr)[relative] = replacement;
						for (i32 i = 0; i < 2; ++i)
						{
							(keyEntries[replacement].*childrenPtr)[i] = (keyEntries[indexToRemove].*childrenPtr)[i];
							(keyEntries[(keyEntries[replacement].*childrenPtr)[i]].*parentPtr) = replacement;
						}
					}
					VERIFY_STRUCTURE(numKeys, keyEntries);
				}

				VERIFY_STRUCTURE(numKeys, keyEntries);
			}

			// Update the counts in the tree.
			for (i32 updateIndex = countChangedItem; updateIndex >= 0; updateIndex = keyEntries[updateIndex].parent)
			{
				keyEntries[updateIndex].count = 1;
				for (i32 childIndex = 0; childIndex < 2; ++childIndex)
				{
					i32 child = keyEntries[updateIndex].children[childIndex];
					if (child >= 0)
						keyEntries[updateIndex].count += keyEntries[child].count;
				}
			}
			assert(pSpline->num_keys() == keyEntries[mainTree.children[0]].count);
			VERIFY_STRUCTURE(numKeys, keyEntries);

			// Add all the neighbours to the reevaluate list (but not the key to be deleted).
			for (i32 addIndex = 1, indexToAdd = -1; (indexToAdd = keysToRemove2[addIndex]), addIndex < numKeysToRemove2; ++addIndex)
			{
				if (keyEntries[indexToAdd].currentList == ACTIVE_KEYS_LIST)
				{
					keyEntries[indexToAdd].links[NEXT] = REEVALUATE_KEYS_LIST;
					keyEntries[indexToAdd].links[PREVIOUS] = reevaluateKeysList.links[PREVIOUS];
					keyEntries[reevaluateKeysList.links[PREVIOUS]].links[NEXT] = indexToAdd;
					reevaluateKeysList.links[PREVIOUS] = indexToAdd;
					keyEntries[indexToAdd].currentList = REEVALUATE_KEYS_LIST;
				}
			}
			VERIFY_STRUCTURE(numKeys, keyEntries);

			++numKeysDeleted;
		}

		// If we couldn't delete any keys, then terminate.
		if (numKeysDeleted == 0)
			break;
	}
}
#undef VERIFY_STRUCTURE
