
#include <drx3D/Physics/Collision/Gimpact/ContactProcessing.h>

#define MAX_COINCIDENT 8

struct CONTACT_KEY_TOKEN
{
	u32 m_key;
	i32 m_value;
	CONTACT_KEY_TOKEN()
	{
	}

	CONTACT_KEY_TOKEN(u32 key, i32 token)
	{
		m_key = key;
		m_value = token;
	}

	CONTACT_KEY_TOKEN(const CONTACT_KEY_TOKEN& rtoken)
	{
		m_key = rtoken.m_key;
		m_value = rtoken.m_value;
	}

	inline bool operator<(const CONTACT_KEY_TOKEN& other) const
	{
		return (m_key < other.m_key);
	}

	inline bool operator>(const CONTACT_KEY_TOKEN& other) const
	{
		return (m_key > other.m_key);
	}
};

class CONTACT_KEY_TOKEN_COMP
{
public:
	bool operator()(const CONTACT_KEY_TOKEN& a, const CONTACT_KEY_TOKEN& b) const
	{
		return (a < b);
	}
};

void ContactArray::merge_contacts(
	const ContactArray& contacts, bool normal_contact_average)
{
	clear();

	i32 i;
	if (contacts.size() == 0) return;

	if (contacts.size() == 1)
	{
		push_back(contacts[0]);
		return;
	}

	AlignedObjectArray<CONTACT_KEY_TOKEN> keycontacts;

	keycontacts.reserve(contacts.size());

	//fill key contacts

	for (i = 0; i < contacts.size(); i++)
	{
		keycontacts.push_back(CONTACT_KEY_TOKEN(contacts[i].calc_key_contact(), i));
	}

	//sort keys
	keycontacts.quickSort(CONTACT_KEY_TOKEN_COMP());

	// Merge contacts
	i32 coincident_count = 0;
	Vec3 coincident_normals[MAX_COINCIDENT];

	u32 last_key = keycontacts[0].m_key;
	u32 key = 0;

	push_back(contacts[keycontacts[0].m_value]);

	GIM_CONTACT* pcontact = &(*this)[0];

	for (i = 1; i < keycontacts.size(); i++)
	{
		key = keycontacts[i].m_key;
		const GIM_CONTACT* scontact = &contacts[keycontacts[i].m_value];

		if (last_key == key)  //same points
		{
			//merge contact
			if (pcontact->m_depth - CONTACT_DIFF_EPSILON > scontact->m_depth)  //)
			{
				*pcontact = *scontact;
				coincident_count = 0;
			}
			else if (normal_contact_average)
			{
				if (Fabs(pcontact->m_depth - scontact->m_depth) < CONTACT_DIFF_EPSILON)
				{
					if (coincident_count < MAX_COINCIDENT)
					{
						coincident_normals[coincident_count] = scontact->m_normal;
						coincident_count++;
					}
				}
			}
		}
		else
		{  //add new contact

			if (normal_contact_average && coincident_count > 0)
			{
				pcontact->interpolate_normals(coincident_normals, coincident_count);
				coincident_count = 0;
			}

			push_back(*scontact);
			pcontact = &(*this)[this->size() - 1];
		}
		last_key = key;
	}
}

void ContactArray::merge_contacts_unique(const ContactArray& contacts)
{
	clear();

	if (contacts.size() == 0) return;

	if (contacts.size() == 1)
	{
		push_back(contacts[0]);
		return;
	}

	GIM_CONTACT average_contact = contacts[0];

	for (i32 i = 1; i < contacts.size(); i++)
	{
		average_contact.m_point += contacts[i].m_point;
		average_contact.m_normal += contacts[i].m_normal * contacts[i].m_depth;
	}

	//divide
	Scalar divide_average = 1.0f / ((Scalar)contacts.size());

	average_contact.m_point *= divide_average;

	average_contact.m_normal *= divide_average;

	average_contact.m_depth = average_contact.m_normal.length();

	average_contact.m_normal /= average_contact.m_depth;
}
