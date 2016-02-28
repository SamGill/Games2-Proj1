
#include "GameObject.h"

GameObject::GameObject()
{
	radius = 0;
	speed = 0;
	active = true;
	Identity(&world);
}

GameObject::~GameObject()
{
	box = NULL;
}

void GameObject::draw()
{
	if (!active)
		return;
    D3D10_TECHNIQUE_DESC techDesc;
    mTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        mTech->GetPassByIndex( p )->Apply(0);
        box->draw();
    }
		/*box->draw();*/
}

void GameObject::init(Box *b, float r, Vector3 pos, Vector3 vel, float sp, float s)
{
	box = b;
	radius = r;
	radius *= 1.01; //fudge factor
	position = pos;
	velocity = vel;
	speed = sp;
	scale = s;
	radiusSquared = radius * radius;
}

void GameObject::update(float dt)
{
	position += velocity*dt;
	Identity(&world);
	Translate(&world, position.x, position.y, position.z);

}

bool GameObject::collided(GameObject *gameObject)
{
	if (getActiveState() && gameObject ->getActiveState()){

		Vector3 diff = position - gameObject->getPosition();
		float length = D3DXVec3LengthSq(&diff);
		float radii = radiusSquared + gameObject->getRadiusSquare();
		if (length <= radii)
			return true;
	}
	return false;
}
