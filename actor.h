#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "GraphObject.h"
#include "GameConstants.h"
#include <cstdlib>

class StudentWorld;
        
const int LEFT = -1;
const int RIGHT = 1;

const int BASE_CHANCE_OF_FIRING = 10;
 
// Students:  Add code to this file, actor.cpp, StudentWorld.h, and StudentWorld.cpp
class Actor: public GraphObject
{
public:
	//GraphObject's constructor requires parameters!!!!!
	Actor(int imageID, int startX, int startY, StudentWorld* const world)
		:GraphObject(imageID, startX, startY), m_sw(world), m_dead(false)
	{
		setVisible(true);
	}
	virtual ~Actor() {}
	  // Move the actor by -1, 0, or +1 in each coordinate, marking as dead
      // if off screen
    void moveBy(int deltaX, int deltaY)
	{
		//flying off the bootom of the screen = dead
		//need to be removed from the field
		if(getY()+deltaY < 0)
		{
			//CHECK ME!!! if the new position will be off the screen,
			//should the actor still move?
			//////////////////
			moveTo(getX(), getY()+deltaY);
			setDead();
			return;
		}
		//the new x position should still be on the screen
		if(getX()+deltaX < VIEW_WIDTH)
			moveTo(getX()+deltaX, getY()+deltaY);
		else 
		//CHECK ME!!! new x not on screen
		////////// 
		////////////
			moveTo(getX(), getY()+deltaY);
	}

      // Is this actor dead?
    bool isDead() const
	{ return m_dead; }

      // Mark this actor as dead
	void setDead() 
	{ m_dead = true;}

	virtual void doSomething() {}
	
	StudentWorld* getWorld()
	{ return m_sw; }	

private:
	StudentWorld* m_sw;
	bool m_dead;
};

class Ship : public Actor
{
public:
    enum ProjectileType { BULLET, TORPEDO };

    Ship(int imageID, int startX, int startY, StudentWorld* world, int startEnergy)
		: Actor(imageID, startX, startY, world), m_energy(startEnergy), m_startEnergy(startEnergy) {}
	
      // How much life energy does the ship have?
    int getEnergy() const
	{ return m_energy; }

      // What percentage of the starting life energy does the ship have?
	int getEnergyPct() const 
	{ return getEnergy()*2; } 

      // Decrease the life energy by the number of points
    void decreaseEnergy(int points)
	{ m_energy -= points; }

      // Restore the life energy level to the starting level
	void restoreFullEnergy() 
	{ m_energy = m_startEnergy;}

      // Launch a projectile of the indicated type, fired by either the player
      // or an alien
	void launchProjectile(ProjectileType pt, bool playerFired); 

private:
	int m_energy;
	int m_startEnergy;
};

class Player: public Ship
{
public:
	Player(StudentWorld* world)
		: Ship(IID_PLAYER_SHIP, VIEW_WIDTH/2, 1, world, 50),
			m_shootLastTick(false), m_numTorpedoes(0) {}
	virtual void doSomething();

	 // Cause the indicated number of points damage to the player, who was
      // either hit by a projectile or collided with an alien
    void damage(int points, bool hitByProjectile);

	//can only shoot bullets every other tick
	bool shootLastTick();

      // How many torpedoes does this player have?
    int getNumTorpedoes() const
	{ return m_numTorpedoes; }

      // Add n torpedoes to the players arsenal.
    void addTorpedoes(int n)
	{ m_numTorpedoes += n; }

private:
	bool m_shootLastTick;
	int m_numTorpedoes;
};

class Alien : public Ship
{
public:
    Alien(int imageID, StudentWorld* world, int startEnergy, int worth)
		: Ship(imageID, rand() % 30, 39, world, startEnergy), m_worth(worth), m_moveLastTick(false) { }

      // Cause the indicated number of points damage to the alien, who was
      // either hit by a projectile or collided with the player
	virtual void damage(int points, bool hitByProjectile);

      // Possibly drop a goodie
	virtual void maybeDropGoodie() {}

      // fire a projectile of the indicated type
	void fireProjectile(ProjectileType pt) {}

	bool moveLastTick();
private:
	int m_worth;
	bool m_moveLastTick;
};

class NachlingBase : public Alien
{
public:
    NachlingBase(int imageID, StudentWorld* world,  int worth, int round, int startEnergy)
		:Alien(imageID, world, startEnergy, worth), 
			 m_state(0), minDisToBorder(0), xMoveDis(0), xMoveDir(0), xMoveRemain(0) {}
	
	int getState() const
	{ return m_state; }

	void changeState(int num)
	{ m_state = num; }

	virtual void doSomething();
private:
	int m_state;
	int minDisToBorder;	
	//HMD (horizontal movement direction in spec)
	int xMoveDis;
		
	//0 for left, 1 for right
	int xMoveDir;

	//HMR (horizontal move remaining in spec)
	int xMoveRemain;
};

class Nachling : public NachlingBase
{
public:
	//NachlingBase(int imageID, StudentWorld* world,  int worth, int round, int startEnergy)
	//Each new regular Nachling should have a starting energy level of int(5*RoundFactor).
    Nachling(StudentWorld* world, int round)
		: NachlingBase(IID_NACHLING, world, 1000, round, 5*round) {}
};

class WealthyNachling : public NachlingBase
{
public:
	WealthyNachling(StudentWorld* world, int round)
		: NachlingBase(IID_WEALTHY_NACHLING, world, 1200, round, 8*round), m_malfun(false), m_ticks(0) {}
    virtual void doSomething();
	virtual void maybeDropGoodie() {}
private:
	bool m_malfun;
	int m_ticks;
};

class Smallbot : public Alien
{
public:
	//Alien imageID, world, startEnergy, worth (how many points it's worth
	//Each new Smallbot should have a starting energy level of int(12*RoundFactor).
	Smallbot(StudentWorld* world, int round): Alien(IID_SMALLBOT, world, 12*round, 1500), m_justHit(false) {}
    virtual void doSomething();
	virtual void damage(int points, bool hitByProjectile);
	virtual void maybeDropGoodie() {}

	bool justHit()
	{ return m_justHit; }

	void changeHitStatus()
	{
		if(m_justHit)
			m_justHit = false; 
		else //was not hit in previous tick
			m_justHit = true;
	}
private:
	bool m_justHit;	
};

class Projectile : public Actor
{
public:
    Projectile(int imageID, int startX, int startY, StudentWorld* world, bool playerFired, int damagePoints)
		:Actor(imageID, startX, startY, world), m_playerFired(playerFired), m_dmgAmt(damagePoints) {}
    
	virtual void doSomething();

      // Was this projectile fired by the player (as opposed to an alien)?
    bool playerFired() const
	{ return m_playerFired; }

      // What sound should be made when the player fires this projectile?
    virtual int getPlayerFiredSound() = 0;
private:
	bool m_playerFired;
	int m_projectileType;
	int m_dmgAmt;
};

class Bullet : public Projectile
{
public:
    Bullet(int startX, int startY, StudentWorld* world, bool playerFired)
		:Projectile(IID_BULLET, startX, startY, world, playerFired, 2) {}
	virtual int getPlayerFiredSound();
};

class Torpedo : public Projectile
{
public:
    Torpedo(int startX, int startY, StudentWorld* world, bool playerFired)
		 :Projectile(IID_TORPEDO, startX, startY, world, playerFired, 8) {}
	virtual int getPlayerFiredSound();
};

class Goodie : public Actor
{
public:
    Goodie(int imageID, int startX, int startY, StudentWorld* world);
		
	virtual void doSomething();

      // Perform the action specific to the kind of goodie when the
      // player retrieves it
    virtual void doSpecialAction(Player* p) = 0;

	int getLifetime ()
	{ return m_lifetime; }

	void decrTimeLeft()
	{ --m_curTime;}

	double getTimeLeft()
	{ return m_curTime; }

	int getTimeExisted()
	{ return m_timeExisted; }

	void increTimeExisted()
	{ ++m_timeExisted; }

private:
	int m_lifetime;
	double m_curTime;
	int m_timeExisted;
};

class FreeShipGoodie : public Goodie
{
public:
    FreeShipGoodie(int startX, int startY, StudentWorld* world)
		:Goodie(IID_FREE_SHIP_GOODIE, startX, startY, world) {}
	virtual void doSpecialAction(Player* p);
};

class EnergyGoodie : public Goodie
{
public:
	EnergyGoodie(int startX, int startY, StudentWorld* world)
		:Goodie(IID_ENERGY_GOODIE, startX, startY, world) {};
	virtual void doSpecialAction(Player* p);
};

class TorpedoGoodie : public Goodie
{
public:
	TorpedoGoodie(int startX, int startY, StudentWorld* world)
	:Goodie(IID_TORPEDO_GOODIE, startX, startY, world) {}
    virtual void doSpecialAction(Player* p);
};

class Star: public Actor
{
public:
	Star(StudentWorld* world, int x, int y): Actor(IID_STAR, x, y, world) {}
	virtual ~Star() {}
	virtual void doSomething();
private:
};


#endif // _ACTOR_H_