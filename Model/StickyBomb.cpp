#include "precompiled_header"
#include "../Model/StickyBomb.h"
#include "Enemy.h"
#include "Ship.h"

// used for multiplicative damage in Notification function when bombs are clustered
static std::map<Enemy *, int> numberOfStuckBombsPerEnemy;

StickyBomb::StickyBomb()
{
	SetDamage(0);
	SetDeathAnimation(99);
	SetTexture("Resources/images/missile2.png");
	SetDimensions(16, 16);
	SetColor(1, 1, 1, 1);
	SetSpeed(400);
	stuck = false;
}

StickyBomb::~StickyBomb()
{
	if (target)
		target->RemoveObserver(this); // no more notifications

	Disconnect();
}

void StickyBomb::Disconnect()
{
	if (target)
	{
		if (stuck)
		{
			numberOfStuckBombsPerEnemy[target]--; // not stuck anymore
			stuck = false;
		}
		target = nullptr; // no more tracking
	}
	SetActive(false); // disappear
}

// accessors & mutators
void StickyBomb::SetTarget(Enemy *_target) 
{ 
	target = _target;
}

// interface methods
void StickyBomb::Heartbeat(float _delta)	// seek or stick
{
	if (target)
	{
		if (!stuck){ // seek
			Vec2f forward = target->GetShip()->GetPosition() - GetPosition();
			forward.Normalize();
			SetHeading(SaiphApp::GetHeadingFromUnitVector(forward));

			Bullet::Heartbeat(_delta);
		}
		else // stick
		{
			SetPosition(target->GetShip()->GetPosition() + offset);
		}
	}
}

void StickyBomb::Notification(ObserverEvent _event)	// explode
{
	SetDeathAnimation(2);

	if (_event == SUBJECT_DEATH) // enemy is already gone
	{
		Disconnect();
	}
	else if (!stuck) // detonated mid-flight
	{
		if (target)
			target->RemoveObserver(this); // no more notifications
		Disconnect();
	}
	else // event == ACTIVATE
	{
		float damage = 5;
		int numBombs = numberOfStuckBombsPerEnemy[target];

		// bombs do multiplicative damage when they are grouped together on one subject
		float multiplied = (float)pow(damage, numBombs); // inflate damage
		damage = multiplied / numBombs; // distribute
		SetDamage(damage);


		SetDimensions(GetDimensions()*(float)numBombs); // inflate for explosions
	}
}
void StickyBomb::CollisionResponse() // stop
{
	if (!stuck && // not detonated yet
		target && target->GetShip()->Collide(*this)) // only stick to target
	{
		offset = GetPosition() - target->GetShip()->GetPosition();
		offset *= .8f;
		stuck = true;
		numberOfStuckBombsPerEnemy[target]++;
	}
	else if (stuck && GetDamage() > 0) // explosion already triggered
	{
		if (target)
			target->RemoveObserver(this); // no more notifications
		Disconnect();
	}	

	//otherwise keep sticking and colliding to no effect
}