// Разработка 2018-2023 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 09:12:2011		Created by Jonathan Bunner
*************************************************************************/

#include <drx3D/Game/StdAfx.h>
#include <drx3D/Game/RandomDeck.h>

CRandomNumberDeck::CRandomNumberDeck()
	: m_seed(0)
	, m_minIndex(0)
	, m_maxIndex(0)
	, m_bAutoReshuffleOnEmpty(true)
{

}

CRandomNumberDeck::~CRandomNumberDeck()
{

}

void CRandomNumberDeck::Shuffle()
{
	if(!m_deck.empty())
	{
		m_deck.clear(); 
	}

	// Call Resize to create new (we have possibly been popping them off up until now)
	u8k range = (m_maxIndex - m_minIndex) + 1; // inclusive of both min and max
	m_deck.resize(range); 

	for(uint i = 0; i < m_deck.size(); ++i)
	{
		m_deck[i] = m_minIndex + i; 
	}

	if(!m_deck.empty())
	{
		std::random_shuffle(m_deck.begin(), m_deck.end(), m_randomIntGenerator);
	}
}

u8 CRandomNumberDeck::DealNext()
{
	if(!m_deck.empty())
	{
		u8 next = m_deck.back(); 
		m_deck.pop_back(); 
		return next; 
	}
	else
	{
		if(m_bAutoReshuffleOnEmpty)
		{
			Shuffle();
			u8 next = m_deck.back(); 
			m_deck.pop_back(); 
			return next; 
		}
		else
		{
			return 0; 
		}
	}
}

void CRandomNumberDeck::Init( i32 seed, u8 maxIndexValue, u8 minIndexValue /*= 0*/ , const bool autoReshuffleOnEmpty /*= true*/)
{
	if(!m_deck.empty())
	{
		m_deck.clear(); 
	}

	m_bAutoReshuffleOnEmpty = autoReshuffleOnEmpty; 
	m_maxIndex = maxIndexValue; 
	m_minIndex = minIndexValue; 
	// Error checking
	if(m_minIndex > m_maxIndex)
	{
		m_minIndex = m_maxIndex; 
	}

	m_randomIntGenerator.Seed(seed);
	Shuffle(); 
}


void CRandomIntGenerator::Seed( i32 seed )
{
	m_twisterNumberGen.seed(seed);
}
