
#include "GameObjectBullet.h"

GameObjectBullet::GameObjectBullet()
{
	radius = 0;
	speed = 0;
	active = true;
	Identity(&world);
}

GameObjectBullet::~GameObjectBullet()
{
	bullet = NULL;
}

void GameObjectBullet::draw()
{
	if (!active)
		return;
    D3D10_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        mTech->GetPassByIndex( p )->Apply(0);
        bullet->draw();
    }
		/*box->draw();*/
}

void GameObjectBullet::init(Bullet *b, float r, Vector3 pos, Vector3 vel, float sp, float s)
{
	bullet = b;
	radius = r;
	radius *= 1.01; //fudge factor
	position = pos;
	velocity = vel;
	speed = sp;
	scale = s;
	radiusSquared = radius * radius;
}

void GameObjectBullet::update(float dt)
{
	position += velocity*dt;
	Identity(&world);
	Translate(&world, position.x, position.y, position.z);

}

bool GameObjectBullet::collided(GameObjectBullet *gameObject)
{
	Vector3 diff = position - gameObject->getPosition();
	float length = D3DXVec3LengthSq(&diff);
	float radii = radiusSquared + gameObject->getRadiusSquare();
	if (length <= radii)
		return true;
	return false;
}