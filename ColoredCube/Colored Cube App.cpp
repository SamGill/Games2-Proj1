//=============================================================================
// Color Cube App.cpp by Frank Luna (C) 2008 All Rights Reserved.
//
// Demonstrates coloring.
//
// Controls:
//		'A'/'D'/'W'/'S' - Rotate 
//
//=============================================================================

#include "d3dApp.h"
#include "Box.h"
#include "Line.h"
#include "Axes.h"
#include "Vertex.h"
#include "Triangle.h"
#include "Quad.h"
#include "Input.h"
#include "GameObject.h"
#include "audio.h"
#include "c:\Program Files (x86)\Windows Kits\8.0\Include\shared\winerror.h"
#include "TimeBuffer.h"

class ColoredCubeApp : public D3DApp
{
private:
	Input* input;
	Audio* audio;
	TimeBuffer timeBuffer;
public:
	ColoredCubeApp(HINSTANCE hInstance);
	~ColoredCubeApp();

	void initApp();
	void onResize();
	void updateScene(float dt);
	void drawScene();

	//Input* getInput() {   return input;}
	//Audio* getAudio() {   return audio;}

private:
	void buildFX();
	void buildVertexLayouts();

private:
	Axes mAxes;
	Box mBox;


	Line mLine;
	Triangle mTriangle;
	Quad mQuad;

	GameObject gameObject1;

	GameObject enemyObjects[MAX_NUM_ENEMIES];

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTech;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mWVP;

	float mTheta;
	float mPhi;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif


	ColoredCubeApp theApp(hInstance);

	theApp.initApp();

	return theApp.run();
}

ColoredCubeApp::ColoredCubeApp(HINSTANCE hInstance)
	: D3DApp(hInstance), mFX(0), mTech(0), mVertexLayout(0),
	mfxWVPVar(0), mTheta(0.0f), mPhi(PI*0.25f)
{
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mWVP); 

	input = new Input();
	//audio = new Audio();

}

ColoredCubeApp::~ColoredCubeApp()
{
	if( md3dDevice )
		md3dDevice->ClearState();

	ReleaseCOM(mFX);
	ReleaseCOM(mVertexLayout);
	//safeDelete(input);
}

void ColoredCubeApp::initApp()
{
	D3DApp::initApp();
	//input->initialize(getMainWnd(), false);
	//audio->initialize();

	float boxScale = 0.5f;
	mAxes.init(md3dDevice, 1.0f);
	mBox.init(md3dDevice, boxScale);
	mLine.init(md3dDevice, 1.0f);
	//mTriangle.init(md3dDevice, 1.0f);
	mQuad.init(md3dDevice, 10.0f);

	gameObject1.init(&mBox, sqrt(2.0f), Vector3(6,.5,0), Vector3(0,0,0), 5000.0f,1.0f);
	gameObject1.setRadius(gameObject1.getRadius()*boxScale);

	int step = 2;
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		enemyObjects[i].init(&mBox, sqrt(2.0), Vector3(-5,.5,step*i - 3.8), Vector3(1,0,0), 3000.0f, 1);
		enemyObjects[i].setRadius(enemyObjects[i].getRadius()*boxScale);
		enemyObjects[i].setInActive();
	}

	buildFX();
	buildVertexLayouts();

	audio = new Audio();

	if (*WAVE_BANK != '\0' && *SOUND_BANK != '\0')  // if sound files defined
	{
		if (!audio->initialize()) {

		}

		/*if( FAILED( hr = audio->initialize() ) )
		{
		if( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
		throw(GameError(gameErrorNS::FATAL_ERROR, "Failed to initialize sound system because media file not found."));
		else
		throw(GameError(gameErrorNS::FATAL_ERROR, "Failed to initialize sound system."));
		}*/
	}

	timeBuffer.resetClock();

}

void ColoredCubeApp::onResize()
{
	D3DApp::onResize();

	float aspect = (float)mClientWidth/mClientHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, 0.25f*PI, aspect, 1.0f, 1000.0f);
}

void generateEnemy(GameObject enemyObjects[], float dt) {
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		//This means he's already on the field, so keep going
		if (enemyObjects[i].getActiveState())
			continue;
		else
		{

			float horizontalStartingPoint = rand()%5;

			//flip it so that the boxes also appear sometimes on the left part of the screen
			int leftOrRightSide = 1;
			if (rand()%2)
				leftOrRightSide = -1;
				


			horizontalStartingPoint *= leftOrRightSide;


			//put the enemy object somewhere randomly and make them active
			enemyObjects[i].setPosition(Vector3(-5,.5, horizontalStartingPoint));
			enemyObjects[i].setActive();

			//Now figure out their direction of travel

			int randomZValue = ( rand() % (PLAYER_Z_RANGE + 1) ) * leftOrRightSide;

			//These are the player starting points
			Vector3 direction = Vector3(5, .5, randomZValue) - enemyObjects[i].getPosition();

			D3DXVec3Normalize(&direction, &direction);

			enemyObjects[i].setVelocity(direction * enemyObjects[i].getSpeed() * dt);

			return;
		}
	}
}

void ColoredCubeApp::updateScene(float dt)
{
	//Generate a block every three seconds
	if (timeBuffer.elapsedTime() > 2) {
		timeBuffer.resetClock();

		generateEnemy(enemyObjects, dt);
	}

	Vector3 oldEnemyPositions[MAX_NUM_ENEMIES];
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		oldEnemyPositions[i] = enemyObjects[i].getPosition();
	}


	Vector3 direction(0, 0, 0);
	Vector3 oldposition = gameObject1.getPosition();

	D3DApp::updateScene(dt);
	gameObject1.update(dt);
	for (int i = 0; i < MAX_NUM_ENEMIES; i++) {
		enemyObjects[i].update(dt);
	}

	if(GetAsyncKeyState('A') & 0x8000)  direction.z = -1.0f;
	if(GetAsyncKeyState('D') & 0x8000)	direction.z = +1.0f;
	// commenting below locks the cube in one dimension
	//if(GetAsyncKeyState('W') & 0x8000)	direction.x = -1.0f;
	//if(GetAsyncKeyState('S') & 0x8000)	direction.x = +1.0f;

	D3DXVec3Normalize(&direction, &direction);


	gameObject1.setVelocity( direction * gameObject1.getSpeed() * dt);

	if (gameObject1.getPosition().z < -PLAYER_Z_RANGE){
		gameObject1.setPosition(Vector3(oldposition.x, oldposition.y, -PLAYER_Z_RANGE));
	}
	if (gameObject1.getPosition().z > PLAYER_Z_RANGE){
		gameObject1.setPosition(Vector3(oldposition.x, oldposition.y, PLAYER_Z_RANGE));
	}

	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{

		//if they collide and are active
		if(gameObject1.collided(&enemyObjects[i]) && enemyObjects[i].getActiveState())
		{
			audio->playCue(BEEP1);
			enemyObjects[i].setInActive();
		}

	}





	D3DXMATRIX w;

	D3DXMatrixTranslation(&w, 2, 2, 0);
	mfxWVPVar->SetMatrix(w);



	// Restrict the angle mPhi.
	if( mPhi < 0.1f )	mPhi = 0.1f;
	if( mPhi > PI-0.1f)	mPhi = PI-0.1f;

	// Convert Spherical to Cartesian coordinates: mPhi measured from +y
	// and mTheta measured counterclockwise from -z.
	/*float x =  5.0f*sinf(mPhi)*sinf(mTheta);
	float z =  -5.0f*sinf(mPhi)*cosf(mTheta);
	float y =  5.0f*cosf(mPhi);*/

	// Build the view matrix.
	D3DXVECTOR3 pos(10.0f, 2.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);
}

void ColoredCubeApp::drawScene()
{
	D3DApp::drawScene();

	// Restore default states, input layout and primitive topology 
	// because mFont->DrawText changes them.  Note that we can 
	// restore the default states by passing null.
	md3dDevice->OMSetDepthStencilState(0, 0);
	float blendFactors[] = {0.0f, 0.0f, 0.0f, 0.0f};
	md3dDevice->OMSetBlendState(0, blendFactors, 0xffffffff);
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	// set constants
	mWVP = mView*mProj;
	mfxWVPVar->SetMatrix((float*)&mWVP);

	D3D10_TECHNIQUE_DESC techDesc;
	mTech->GetDesc( &techDesc );
	for(UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTech->GetPassByIndex( p )->Apply(0);

		//mBox.draw();
		mAxes.draw();
		mLine.draw();
		//mTriangle.draw();
		mQuad.draw();
	}

	mWVP = gameObject1.getWorldMatrix()  *mView*mProj;
	mfxWVPVar->SetMatrix((float*)&mWVP);
	gameObject1.setMTech(mTech);
	gameObject1.draw();


	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		mWVP = enemyObjects[i].getWorldMatrix()  *mView*mProj;
		mfxWVPVar->SetMatrix((float*)&mWVP);
		enemyObjects[i].setMTech(mTech);
		enemyObjects[i].draw();
	}


	// We specify DT_NOCLIP, so we do not care about width/height of the rect.
	RECT R = {5, 5, 0, 0};
	mFont->DrawText(0, mFrameStats.c_str(), -1, &R, DT_NOCLIP, BLACK);

	mSwapChain->Present(0, 0);
}

void ColoredCubeApp::buildFX()
{
	DWORD shaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob* compilationErrors = 0;
	HRESULT hr = 0;
	hr = D3DX10CreateEffectFromFile(L"color.fx", 0, 0, 
		"fx_4_0", shaderFlags, 0, md3dDevice, 0, 0, &mFX, &compilationErrors, 0);
	if(FAILED(hr))
	{
		if( compilationErrors )
		{
			MessageBoxA(0, (char*)compilationErrors->GetBufferPointer(), 0, 0);
			ReleaseCOM(compilationErrors);
		}
		DXTrace(__FILE__, (DWORD)__LINE__, hr, L"D3DX10CreateEffectFromFile", true);
	} 

	mTech = mFX->GetTechniqueByName("ColorTech");

	mfxWVPVar = mFX->GetVariableByName("gWVP")->AsMatrix();
}

void ColoredCubeApp::buildVertexLayouts()
{
	// Create the vertex input layout.
	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	mTech->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &mVertexLayout));
}
