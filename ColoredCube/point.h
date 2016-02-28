//=======================================================================================
// Box.h by Frank Luna (C) 2008 All Rights Reserved.
//=======================================================================================

#ifndef POINT_H
#define POINT_H

#include "d3dUtil.h"

class Point
{
public:

	Point();
	~Point();

	void init(ID3D10Device* device, float scale, D3DXCOLOR c);
	void draw();

private:
	DWORD mNumVertices;
	DWORD mNumFaces;

	ID3D10Device* md3dDevice;
	ID3D10Buffer* mVB;
	ID3D10Buffer* mIB;
};


#endif // BOX_H