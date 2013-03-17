#include <iostream>
#include "StudentWorld.h"
#include "actor.h"
#include <cstdlib>
#include <string>
using namespace std;


GameWorld* createStudentWorld()
{
    return new StudentWorld();
}

void StudentWorld::init()
{
	//if the round restarts
	//all actors should disappear
	//player restarts at middle
	
	m_player = new Player(this);
	m_numDestroyed = 0;
	/*
	Alien* nachPtr = new Smallbot(this, 1);
	m_actors.push_back(nachPtr);
	Alien* nachPtr2 = new Smallbot(this, 1);
	m_actors.push_back(nachPtr2);
	Goodie* goodie = new TorpedoGoodie(15, 15, this);
	m_actors.push_back(goodie);
	*/
}

void StudentWorld::cleanUp()
{
	std::list<Actor*>::const_iterator itr;

	itr = m_actors.begin();
	while(itr != m_actors.end())
	{
		delete (*itr);
		itr = m_actors.erase(itr); //erase returns pointer to next item
		
	}	
	delete m_player;
}

// Get a list of aliens at the same location as the actor
std::list<Alien*> StudentWorld::getCollidingAliens(Actor* a)
{
	std::list<Alien*> aliens;

	//set the iterator to point to the beginning
	//go through each Actor in the list and add to CollideAlien List if applicable
	for(m_itr = m_actors.begin(); m_itr != m_actors.end(); m_itr++)
	{
		//is the iterator pointing to an Alien?
		Alien* alienPtr = dynamic_cast<Alien*>((*m_itr));

		//iterator is pointing to an Alien
		if(alienPtr != NULL)
		{
			if(alienPtr->getX() == a->getX() 
				&& alienPtr->getY() == a->getY())
			{
				aliens.push_back(alienPtr);
			}
		}
	}
	return aliens;
}


// Get a pointer to the player if at the same location as the actor,
      // otherwise NULL
Player* StudentWorld::getCollidingPlayer(Actor* a)
{
	if(a->getX() == m_player->getX() 
		&& a->getY() == m_player->getY())
	{
		return m_player;
	}
	return NULL;
}

int StudentWorld::getNumAlienFiredProjectiles()
{
	int count = 0;
	std::list<Actor*>::const_iterator itr;

	for(itr = m_actors.begin(); itr != m_actors.end(); itr++)
	{
		Projectile *p = dynamic_cast<Projectile*>((*itr));
		//p does point to a projectile
		if (p != NULL)
		{
			//the projectile was fired by an alien
			if(p->playerFired() == false)
				count++;
		}
	}
	return count;
}

int StudentWorld::numOfActiveAliens()
{
	int count = 0;
	std::list<Actor*>::const_iterator itr;

	for(itr = m_actors.begin(); itr != m_actors.end(); itr++)
	{
		Alien *p = dynamic_cast<Alien*>((*itr));
		//p does point to a Alien
		if (p != NULL)
		{
			//the Alien must be alive
			if(p->isDead() == false)
				count++;
		}
	}
	return count;
}

void StudentWorld:: addAliensOrStars()
{
	//TO DO: TEST PARAMETER
	//testProvidedParamter();
	if(testParamsProvided() == true)
	{
		//if there is at least one non-Player object in the field
		// Stars count too I suppose
		if(m_actors.size() > 0)
		{
			//add no new items in this tick
			return;
		}
		else
		{
			int whichActor = getTestParam(TEST_PARAM_ACTOR_INDEX);
			if(whichActor == TEST_PARAM_NACHLING)
			{
				Alien* nachPtr = new Nachling(this, getRound());
				m_actors.push_back(nachPtr);
			}
			else if(whichActor == TEST_PARAM_WEALTHY_NACHLING)
			{
				Alien* wealthynachPtr = new WealthyNachling(this, getRound());
				m_actors.push_back(wealthynachPtr);
			}
			else if(whichActor == TEST_PARAM_SMALLBOT)
			{
				Alien* botPtr = new Smallbot(this, getRound());
				m_actors.push_back(botPtr);
			}
			else if(whichActor == TEST_PARAM_GOODIE_ENERGY)
			{
				Goodie* energy = new EnergyGoodie(VIEW_WIDTH/2, VIEW_HEIGHT-1, this);
				addActor(energy);
			}
			else if(TEST_PARAM_GOODIE_TORPEDO)
			{
				Goodie* torpedo = new TorpedoGoodie(VIEW_WIDTH/2, VIEW_HEIGHT-1, this);
				addActor(torpedo);
			}
			else if(whichActor == TEST_PARAM_GOODIE_FREE_SHIP)
			{
				Goodie* ship = new FreeShipGoodie(VIEW_WIDTH/2, VIEW_HEIGHT-1, this);
				addActor(ship);
			} //end of if statements
			return;
		}
		
	}
	else  //test parameters were not provided
	{
		int numActive = numOfActiveAliens();
		if(numActive < getAlienCap())
		{
			if(numActive < destoryGoal())	
			{
				//add new alien!
				double RoundFactor = 0.9 + 0.1 * getRound();
				int p1 = rand() % 101; //p1 in range of 0 to 100
				if(p1 < 70)
				{
					int p2 = rand() % 101; //p2 in range of 0 to 100
					if(p2 < 20)
					{
						Alien* wealthynachPtr = new WealthyNachling(this, getRound());
						m_actors.push_back(wealthynachPtr);
					}
					else
					{
						Alien* nachPtr = new Nachling(this, getRound());
						m_actors.push_back(nachPtr);	
					}
				}
				else
				{
					Alien* botPtr = new Smallbot(this, getRound());
					m_actors.push_back(botPtr);
				}
			}
		}

		//add new stars
		int p3 = rand() % 101; //p3 in the range of 0 to 100
		if(p3 < 33)
		{
			Star* starPtr = new Star(this, rand() % VIEW_WIDTH, VIEW_HEIGHT-1);
			m_actors.push_back(starPtr);
		}
	}
}

int StudentWorld:: move()
{
	std::list<Actor*>::const_iterator itr;
	itr = m_actors.begin();

	if(getNumDestroyed() >= destoryGoal())
	{
		advanceRound();
		m_numDestroyed = 0;
	}

	updateDisplayText();

	//set the iterator to point to the beginning
	//go through each Actor in the list and do something
	itr = m_actors.begin();
		
	if(!m_actors.empty())
	{
		for(itr = m_actors.begin(); itr != m_actors.end(); itr++)
		{
			if((*itr)->isDead() == false)  //dead actors can't do anything
				(*itr)->doSomething();
		}
	}

	m_player->doSomething();
			
	addAliensOrStars();
		
	//delete actors that are off the screen or dead!
	itr = m_actors.begin();
	//go through entire actors list
 	while(itr != m_actors.end())
	{
		if((*itr)->isDead() || (*itr)->getY() < 0 || (*itr)->getY() > 40)
		{
			delete (*itr);
			itr = m_actors.erase(itr); //erase returns pointer to next item
		}
		else
			++itr;
	}

	if(m_player->isDead())
	{
		return GWSTATUS_PLAYER_DIED;
	}
	return GWSTATUS_CONTINUE_GAME;
}


////////////////////////////////////////////////////////////////
//update the Display Text and helper function
//used by IntToString
int numOfDigits(const int n)
{
	//only 1 digit!
	if(n<10)
		return 1;
	return numOfDigits(n/10) + 1;
}

string intToString(const int& n)
{
	int num = numOfDigits(n);
	int digit = 0;
	
	int current = n;
	char c = n + '0';

	string s;
	

	int i = 1;
	for(i=num; i>1; i--) //i starts from the digit in the largest position
	{
		int divideBy = 1;
		//divide by the correct # of 10
		for(int a = 0; a < i-1; a++)
		{
			divideBy *= 10;
		}
		if(i == num)
		{
			digit = n/divideBy;
			current -= digit*divideBy;
		}
		else
		{
			digit = current/divideBy;
			current -= digit*divideBy;
		}
		c = digit + '0';
		s= s + c;	
	}
	
	if(i == 1)
	{
		c = current + '0';
		s = s + c;
	}
	
	return s;
}

string formatField(const int& field, const int& numDigitsTotal)
{

	int digits = numOfDigits(field);
	string f = "";
	int digitDiff = numDigitsTotal - digits;
	for(int i=0; i<digitDiff; i++)
	{
		f = f + '0';
	}
	f = f + intToString(field);
	return f;
}

string formatText(int score, int round, int energyPercent, int torpedoes, int shipsLeft)
{
	string s;
	s = s + "Score: " + formatField(score, 7) + "  ";
	s = s + "Round: " + formatField(round, 2) + "  ";
	s = s + "Energy: " + intToString(energyPercent) + "%" + "  ";
	s = s + "Torpedoes: " + formatField(torpedoes, 3) + "  ";
	s = s + "Ships: " + formatField(shipsLeft, 2) + "  ";
	return s;
}

void StudentWorld::updateDisplayText()
{
	int score = getScore();
	int round = getRound();
	int energyPercent = m_player->getEnergyPct();
	int torpedoes = m_player->getNumTorpedoes();
	int shipsLeft = getLives();
// Next, create a string from your statistics, of the form: 
// “Score: 0000123 Round: 01 Energy: 98% Torpedoes: 003 Ships: 03”
// where all numeric values except the Energy value must have leading 
// zeros (e.g., 003 vs. 3), and each
// statistic should be separated from the last statistic by two 
// spaces. E.g., between the “0000123” of the
// score and the “R” in “Round” there must be two spaces. Each field 
// must be exactly as wide as shown,
// e.g., the score must be exactly 7 digits long, the torpedo field 
// must be 3 digits long, except for the 
// Energy field, which could be between 1 and 3 digits (e.g., 5%, 89% 
// or 100%)
string s = formatText(score, round, energyPercent, torpedoes, shipsLeft);
// Finally, update the display text at the top of the screen with your 
// newly created stats
setGameStatText(s); // calls GameWorld::setGameStatText
}

