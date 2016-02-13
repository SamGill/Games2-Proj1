//=======================================================================================
// Box.h by Frank Luna (C) 2008 All Rights Reserved.
//=======================================================================================

#ifndef LINE_H
#define LINE_H

#include "d3dUtil.h"

class Line
{
public:

	Line();
	~Line();

	void init(ID3D10Device* device, float scale);
	void draw();

private:
	DWORD mNumVertices;
	DWORD mNumFaces;

	ID3D10Device* md3dDevice;
	ID3D10Buffer* mVB;
	ID3D10Buffer* mIB;
};


#endif // BOX_H
