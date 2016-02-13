//=======================================================================================
// Axes.cpp by Frank Luna (C) 2008 All Rights Reserved.
//=======================================================================================

#include "Axes.h"
#include "Vertex.h"

Axes::Axes()
	: mNumVertices(0), mNumFaces(0), md3dDevice(0), mVB(0), mIB(0)
{
}

Axes::~Axes()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
}

void Axes::init(ID3D10Device* device, float scale)
{
	md3dDevice = device;

	mNumVertices = 6;
	mNumFaces    = 2; // 2 per quad

	// Create vertex buffer
	Vertex vertices[] =
	{
		{D3DXVECTOR3(0.0f, 0.0f, 0.0f), BLACK},
		{D3DXVECTOR3(0.0f, 1.0f, 0.0f), BLACK},

		{D3DXVECTOR3(0.0f, 0.0f, 0.0f), GREEN},
		{D3DXVECTOR3(1.0f, 0.0f, 0.0f), GREEN},
		

		{D3DXVECTOR3(0.0f, 0.0f, 0.0f), RED},
		{D3DXVECTOR3(0.0f, 0.0f, 1.0f), RED},

	};


	// Scale the Axes.
	for(DWORD i = 0; i < mNumVertices; ++i)
		vertices[i].pos *= scale;


	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * mNumVertices;
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));


	// Create the index buffer

	DWORD indices[] = {
		////front face
		0, 0, 0,
		0, 0, 1
	};

	D3D10_BUFFER_DESC ibd;
	ibd.Usage = D3D10_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * mNumFaces*3;
	ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void Axes::draw()
{
	md3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_LINELIST );

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
	md3dDevice->DrawIndexed(mNumFaces*3, 0, 0);


	md3dDevice->DrawIndexed(mNumFaces*3, 0, 2);
	md3dDevice->DrawIndexed(mNumFaces*3, 0, 4);

	md3dDevice->Draw(2, 0);
	md3dDevice->Draw(2, 0);
	md3dDevice->Draw(2, 0);

	md3dDevice->DrawIndexed(6, 0  , 0);
	md3dDevice->DrawIndexed(6, 6  , 0);
	md3dDevice->DrawIndexed(6, 12 , 0);
}