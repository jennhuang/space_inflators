#include <iostream>
#include "actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <list>
using namespace std;

// Students:  Add code to this file (if you wish), actor.h, StudentWorld.h, and StudentWorld.cpp

// Launch a projectile of the indicated type, fired by either the player
// or an alien
void Ship::launchProjectile(ProjectileType pt, bool playerFired)
{
	Projectile* p;

	//if at top of screen, don't fire anything)
	if(getY() == VIEW_HEIGHT-1 && playerFired == true)
	{
		return;
	}
	if(pt == BULLET)
		p = new Bullet(getX(), getY()+1, getWorld(), playerFired);
	else 
		p = new Torpedo(getX(), getY()+1, getWorld(), playerFired);

	//place the pointer to the new Projectile in the list
	getWorld()->addItemToList(p);
	p->getPlayerFiredSound();
}

int Bullet::getPlayerFiredSound()
{
	if(playerFired())
		getWorld()->playSound(SOUND_PLAYER_FIRE);
	else //the alien fired a bullet 
		getWorld()->playSound(SOUND_ENEMY_FIRE);
	return 0;
}

int Torpedo::getPlayerFiredSound()
{
	if(playerFired())
		getWorld()->playSound(SOUND_PLAYER_TORPEDO);
	else //the alien fired a bullet 
		getWorld()->playSound(SOUND_ENEMY_FIRE);
	return 0;
}

void Player::doSomething()
{
	int key;
	//list of aliens that have collided with the player
	list<Alien*> aliens = getWorld()->getCollidingAliens(this);
		
	list<Alien*>::const_iterator aPtr;
	//list is not empty
	if(!aliens.empty())
	{   //for each alien the player collides with			
		for(aPtr = aliens.begin(); aPtr != aliens.end(); aPtr++)
		{
			//damage the player
			damage(15, false);

			//damage the alien 
			(*aPtr)->damage(0, false);
		}
	}

	if(getEnergy() <= 0)
		return;  //player is dead!
	if(getWorld()->getKey(key))
	{
		switch(key)
		{
		case KEY_PRESS_DOWN:
			if(getY() > 0)
				moveTo(getX(), getY()-1);
			break;
		case KEY_PRESS_UP:
			if(getY() < VIEW_HEIGHT-1)
				moveTo(getX(), getY()+1); 
			break;
		case KEY_PRESS_LEFT:
			if(getX() > 0)
				moveTo(getX()-1, getY());
			break;
		case KEY_PRESS_RIGHT:
			if(getX() < VIEW_WIDTH-1)
				moveTo(getX()+1, getY());
			break;
		case KEY_PRESS_SPACE:
			if(!shootLastTick())  //Player can only shoot every other tick
				launchProjectile(BULLET, true);
			break;
		case KEY_PRESS_TAB:
			if(!shootLastTick())  //Player can only shoot every other tick
				if(getNumTorpedoes() > 0)
				{
					addTorpedoes(-1);
					launchProjectile(TORPEDO, true);
				}
			break;
		} 
	} //end of getting user input

	//check again if Player collided with an alien
	aliens = getWorld()->getCollidingAliens(this);
		
	//list is not empty
	if(!aliens.empty())
	{   //for each alien the player collides with			
		for(aPtr = aliens.begin(); aPtr != aliens.end(); aPtr++)
		{
			//damage the player
			damage(15, false);

			//damage the alien 
			(*aPtr)->damage(0, false);
		}
	}
}

// Cause the indicated number of points damage to the player, who was
// either hit by a projectile or collided with an alien
void Player::damage(int points, bool hitByProjectile)
{
	decreaseEnergy(points);
	if(hitByProjectile) 
		getWorld()->playSound(SOUND_PLAYER_HIT);
	else //crashed into an alien
		getWorld()->playSound(SOUND_ENEMY_PLAYER_COLLISION);
	if(getEnergy() <= 0)
	{
		getWorld()->playSound(SOUND_PLAYER_DIE);
		getWorld()->decLives();
		setDead();
	}
}

bool Player::shootLastTick()
{
		if(m_shootLastTick) // Player shot during the previous tick, do nothing
		{
			m_shootLastTick = false;
			return true;
		}
		else //Player did not shoot during previous tick, continue on!
		{
			m_shootLastTick = true;
			return false;
		}
}

bool Alien::moveLastTick()
{
		if(m_moveLastTick) // Nachling moved in the previous tick, do nothing
		{
			m_moveLastTick = false;
			return true;
		}
		else //Nachling did not move in previous tick, continue on!
		{
			m_moveLastTick = true;
			return false;
		}
}

// Cause the indicated number of points damage to the alien, who was
// either hit by a projectile or collided with the player
void Alien::damage(int points, bool hitByProjectile) 
{
	if(hitByProjectile)
	{
		decreaseEnergy(points);
		if(getEnergy() <= 0)  //alien was killed by the projectile!
		{
			getWorld()->increaseScore(m_worth);  //increase score by the Alien's worth
			getWorld()->playSound(SOUND_ENEMY_DIE);
			//increase number of Aliens destroyed 
			getWorld()->addDestroyed();

			WealthyNachling* nach = dynamic_cast<WealthyNachling*>(this);
			
			//current alien is a WealthyNachling
			if(nach != NULL)
			{
				if(rand() % 3 == 0) //1 in 3 chance
				{
					if(rand()% 1 == 0) // 50% chance
					{
						Goodie* energy = new EnergyGoodie(getX(), getY(), getWorld());
						getWorld()->addActor(energy);
						
					}
					else
					{
						Goodie* torpedo = new TorpedoGoodie(getX(), getY(), getWorld());
						getWorld()->addActor(torpedo);
					}
				}
			}

			Smallbot* bot = dynamic_cast<Smallbot*>(this);
			//current alien is a Smallbot
			if(bot != NULL)
			{
				if(rand() % 3 == 0) //1 in 3 chance
				{
					Goodie* freeShip = new FreeShipGoodie(getX(), getY(), getWorld());
						getWorld()->addActor(freeShip);
				}
			}
		}
		else //alien is still alive!
		{
			getWorld()->playSound(SOUND_ENEMY_HIT);
			return;
		}
	}
	else //PLAYER CRASHED INTO ALIEN. XD
	{
		getWorld()->playSound(SOUND_ENEMY_DIE);
	}
	decreaseEnergy(getEnergy()); //the energy of the Alien should be 0!
	setDead(); //alien collided with player and died OR killed by projectile
}

void NachlingBase::doSomething()
{
	if(moveLastTick()) // Nachling moved in the previous tick, do nothing
	{
		return;  
	}

	//at this point, the Nachling has not moved in the previous tick
	if(getState() == 0)
	{		
		//the x coord of the Nachling is not the same as the x coord of player
		if(getX() != getWorld()->getPlayerX())
		{
			int x = rand() % 3;
			 if (x == 0 || x == 1)     // 2/3 probability
			 {
				 moveBy(0, -1); //move down the field
			 }
			 else
			 {
				 //move 1 downward
				 //move 1 square closer to player's x coord
				 if(getWorld()->getPlayerX() < getX())
					moveBy(-1, -1); 
				 else //player has a bigger x coord
					 moveBy(1, -1);
				 return;
			 }
		}
		//still in state 0
		//the x coord of Nachling is the same as the x coord of player
		else if(getX() != 0 && getX() != VIEW_WIDTH - 1)
		{
			//change from state 0 to state 1
			changeState(1);

			int minDisToRight = (VIEW_WIDTH-1) - getX();
			int minDisToLeft = getX();

			//get the smallest distance to a board
			if(minDisToRight > minDisToLeft)
				minDisToBorder = minDisToLeft;
			else //minDisToRight is smaller or equal to minDisToLeft
				minDisToBorder = minDisToRight;

			//If the MDB is larger (but not equal to) than 3, then set the 
			//Nachling’s horizontal movement distance (HMD) to a random 
			//value between 1 and 3. Otherwise set the Nachling’s HMD equal to the computed MDB.
			if(minDisToBorder > 3)
				xMoveDis = (rand() % 3) + 1; //random number b/w 1 and 3
			else
				xMoveDis = minDisToBorder;
			//Choose a random horizontal movement direction (left or right, with equal probability)
			int x = rand() % 2;
			if(x == 0)
				xMoveDir = LEFT; //random number of 0 or 1 (0 is left, 1 is right)
			else
				xMoveDir = RIGHT;
			xMoveRemain = xMoveDis;
		}
		//4.c on spec
			moveBy(0, -1); //move down the field
			if(getY() < 0)
				setDead();
			return;
		}
	
	//the state is 1 (x coord of Nachling is same as player's)
	
	else if(getState() == 1)
	{
		if(getWorld()->getPlayerY() > getY())
		{
			changeState(2);
			return;
		}
		if(xMoveRemain == 0) //HMR value is 0
		{
			//flip the direction 
			if(xMoveDir == LEFT)
				xMoveDir = RIGHT;
			else //originally was RIGHT
				xMoveDir = LEFT;

		//Set the HMR value equal to double the HMD you computed in state 0 earlier.
			xMoveRemain = xMoveDis*2;
		}
		else  //HMR value is not equal to 0
			xMoveRemain--;

		//move one square horizontally in Nachling's current horizontal movement direction
		moveBy(1*xMoveDir, 0);

		//calculate Nachling's chance at firing 
		int chancesOfFiring =int(BASE_CHANCE_OF_FIRING / getWorld()->getRound()) + 1;
		
		int n = getWorld()->getNumAlienFiredProjectiles();
		if(n < getWorld()->getRound()*2)
		{
			launchProjectile(BULLET, false);
		}

		
		if (rand() % 20 == 0) // 1 in 20 chance that Nachling will turn to state 2
		{
			changeState(2);
		}
		return;
	}
	
	else if(getState() == 2)
	{
		if(getY() == VIEW_HEIGHT-1)
		{
			changeState(0);
			return;
		}
		//at the far left
		if(getX() == 0)
		{
			xMoveDir = RIGHT;
		}
		//at the far right
		else if(getX() == VIEW_WIDTH-1)
		{
			xMoveDir = LEFT;
		}
		//in the middle
		else //if(getX() == VIEW_WIDTH/2)
		{
			//Choose a random horizontal movement direction (left or right, with equal probability)
			int x = rand() % 2;
			if(x == 0)
				xMoveDir = LEFT; //random number of 0 or 1 (0 is left, 1 is right)
			else
				xMoveDir = RIGHT;
		}
		//move upward and in its current horizontal move direction 
		moveBy(xMoveDir,1);
		return;
	}
}

void WealthyNachling::doSomething()
{
	//if Wealthy Nachling is malfunctioning, do nothing
	if(m_ticks > 0)
	{
		m_ticks--;
		return;
	}
	if(m_malfun)
		return;
	//not malfunctioning and m_ticks = 0
	if (rand() % 200 == 0) // 1 in 200 chance that Wealthy Nachling will malfunction
	{
		m_malfun = true;
	}
	if(m_malfun) //started to malfunction, now malfunction for the next 30 ticks
	{
		m_ticks = 30;
	}
	NachlingBase::doSomething();
}

void Smallbot::damage(int points, bool hitByProjectile)
{
	//if Smallbot was just hit in the last tick
	changeHitStatus();
	Alien::damage(points, hitByProjectile);
}
	
void Smallbot::doSomething()
{
	if(moveLastTick()) // Smallbot moved in the previous tick, do nothing
	{
		return;  
	}
	//TO DO!
	//if the Smallbot was hit, 1/3 chance of adjusting X coord cas it moves down
	if(justHit() && rand() % 3 == 0)
	{
		if(getX() == 0)
			moveBy(1,0);
		else if(getX() == VIEW_WIDTH-1)
			moveBy(0, -1);
		else  //randomly adjust x coordinate by +1 or -1, with equal probability
		{
			if(rand() % 2 == 0)
				moveBy(1, 0);
			else
				moveBy(-1, 0);
		}
		
	}
	else //Smallbot was not hit or if it was, with 2/3 probability of not dodging left or right
	{
		moveBy(0, -1);
	}

	//Smallbot in same X as Player but above player in field
	//FIRE!
	if(getX() == getWorld()->getPlayerX() && getY() > getWorld()->getPlayerY())
	{
		int q = int(100.0/getWorld()->getRound())+1;
		
		int n = getWorld()->getNumAlienFiredProjectiles();
		
		//decide on whether to fire
		if(n < getWorld()->getRound())
		{
		//1 in q chance that SmallBot will fire at Torpedo
		//FIRE A TORPEDO
			if(rand() % q == 0)
			{
				launchProjectile(TORPEDO, false); 
			}
			else  //FIRE A BULLET
				launchProjectile(BULLET, false);
		} 		
	}
}

void Projectile::doSomething()
{
	int n = 2;
	
	while(n>0 && isDead()== false)
	{
		//the player fired the Projectile
		 
  		if(playerFired())
		{
			//list of aliens that have collided with the player's projectile
			list<Alien*> aliens = getWorld()->getCollidingAliens(this);
		
			list<Alien*>::const_iterator aPtr;
			//list is not empty
			if(!aliens.empty())
			{
				
				for(aPtr = aliens.begin(); aPtr != aliens.end(); aPtr++)
				{
					//damage the alien 
					(*aPtr)->damage(m_dmgAmt, true);
				}
				//set the projectile status to be dead so it will be removed from field at end of tick
				setDead();
				return;
			}
			//move the Bullet up by 1 if fired by player and does not hit anything
			moveBy(0, 1);
		}
		else   //bullet fired by Alien
		{
			//did the bullet collide with the player?
			Player* p = getWorld()->getCollidingPlayer(this);

			//the bullet collides with the player
			if(p != NULL)
			{
				p->damage(m_dmgAmt, false);
				setDead(); //set the bullet to be dead
				return;
			}
			else //bullet did not collide with player
			{
				moveBy(0,-1);
			}
		}
		n--;
	}
}

Goodie::Goodie(int imageID, int startX, int startY, StudentWorld* world)
	:Actor(imageID, startX, startY, world)
{
	m_lifetime = int(100.0/getWorld()->getRound()) +30;
	m_curTime = m_lifetime;
	m_timeExisted = 1;
}

//GOODIES :D
void Goodie::doSomething()
{
	//check if collided with Player
	Player*p = getWorld()->getCollidingPlayer(this);

	//goodie did not collide with player
	if(p == NULL)
	{
		double brightness = double(getTimeLeft() / getLifetime()) + 0.2;
		setBrightness(brightness);
		
		//goodie dies
		if(getTimeLeft() <= 0)
		{
			setBrightness(0);
			setDead();
			return;
		}
		if(getTimeExisted() % 3 == 0) //the goodie moves every 3 ticks
		{
			moveBy(0, -1);
		}
	}
	p = getWorld()->getCollidingPlayer(this);
	if(p != NULL) //goodie collided with player
	{
		getWorld()->increaseScore(5000);
		getWorld()->playSound(SOUND_GOT_GOODIE);

		//the goodie dies
		setDead();
		
		doSpecialAction(p);
	}
	increTimeExisted();
	decrTimeLeft(); //reduce time left by one tick
}

void FreeShipGoodie::doSpecialAction(Player* p)
{
	//increase player's life by 1
	getWorld()->incLives();
}

void EnergyGoodie::doSpecialAction(Player* p)
{
	p->restoreFullEnergy();
}

void TorpedoGoodie::doSpecialAction(Player* p)
{
	p->addTorpedoes(5);
}

void Star::doSomething()
{
	//move stars down one row on the screen
	moveBy(0, -1);
	if(getY() < 0)
		setDead();
}