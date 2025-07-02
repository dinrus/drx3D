// Разработка 2018-2025 DinrusPro / Dinrus Group. РНЦП Динрус.

/*************************************************************************
-------------------------------------------------------------------------
История:
- 12:12:2011		Created by Jonathan Bunner

*************************************************************************/

#ifndef __RANDOMDECK_H__
#define __RANDOMDECK_H__

// Functor object for the stl shuffle algorithm currently used by CRandomDeck
class CRandomIntGenerator
{
public:
	CRandomIntGenerator(){}; 

	void Seed(i32 seed);

	template <typename Distance>
	Distance operator () (const Distance& n)
	{
		return static_cast<Distance>(m_twisterNumberGen.GenerateUint32() % n);
	}

	// Random number gen based on Mersenne twister 
	CMTRand_int32 m_twisterNumberGen; 
};

// Helper class
// - shuffles a contiguous series of integers in range minIndex->maxIndex
// and hands out a new index when DealNext() is called. Automatically 
// reshuffles when deck emptied
class CRandomNumberDeck
{
public:

	// TODO - could support weightings here too (e.g. adding more of specified indecies to the deck)
	CRandomNumberDeck(); 
	~CRandomNumberDeck(); 

	u8 DealNext();
	void Init(i32 seed, u8 maxIndexValue, u8 minIndexValue = 0, const bool autoReshuffleOnEmpty = true); 

	void Shuffle(); 
	ILINE bool Empty() {return m_deck.empty();} 
private:

	std::vector<i32> m_deck; 
	i32 m_seed; 
	u8 m_minIndex; 
	u8 m_maxIndex;
	bool m_bAutoReshuffleOnEmpty; 

	CRandomIntGenerator m_randomIntGenerator;
};


#endif //#ifndef __RANDOMDECK_H__
