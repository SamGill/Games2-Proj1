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
#include "Bullet.h"
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

#include "GameStateManager.h"
#include "Camera.h"

#include <sstream>

int currentBullet;

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
	void restartGame();

	//Input* getInput() {   return input;}
	//Audio* getAudio() {   return audio;}

private:
	void buildFX();
	void buildVertexLayouts();

private:
	Axes mAxes;
	Box mEnemy, mPlayer, mBullet;
	Box mBox;

	GameStateManager* gsm;

	Line mLine;
	Triangle mTriangle;
	Quad mQuad;

	GameObject gameObject1;

	GameObject enemyObjects[MAX_NUM_ENEMIES];

	GameObject playerBullets[MAX_NUM_BULLETS];

	ID3D10Effect* mFX;
	ID3D10EffectTechnique* mTech;
	ID3D10InputLayout* mVertexLayout;
	ID3D10EffectMatrixVariable* mfxWVPVar;

	ID3D10EffectVariable* mfxBlack_WhiteVar;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mWVP;

	// Camera stuff
	Vector3 cameraPos;
	Vector3 lookAt;

	//Camera Object stuff
	Camera camera;

	float mTheta;
	float mPhi;

	int score;
	std::wstring finalScore;

	bool shotRelease;
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

ColoredCubeApp::ColoredCubeApp(HINSTANCE hInstance) : D3DApp(hInstance), mFX(0), mTech(0), mVertexLayout(0),
	mfxWVPVar(0), mTheta(0.0f), mPhi(PI*0.25f)
{
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mWVP); 

	input = new Input();
	//audio = new Audio();

	gsm = new GameStateManager();
	camera.setGsm(gsm);

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
	float collisionFixFactor = 1.1f;
	currentBullet = 0;
	shotRelease = true;

	// increments when you run into a cube // just for now
	score = 0;
	mAxes.init(md3dDevice, 1.0f);
	mEnemy.init(md3dDevice, .5f, RED);
	mPlayer.init(md3dDevice, .5f, BLUE);
	mBullet.init(md3dDevice, .25f, BLACK);
	//mBox.init(md3dDevice, boxScale);
	mLine.init(md3dDevice, 1.0f);
	//mTriangle.init(md3dDevice, 1.0f);
	mQuad.init(md3dDevice, 10.0f);

	gameObject1.init(&mPlayer, sqrt(2.0f), Vector3(6,.5,0), Vector3(0,0,0), 5000.0f,1.0f);
	//gameObject1.init(&mBox, sqrt(2.0f), Vector3(6,.5,0), Vector3(0,0,0), 5000.0f,1.0f);
	gameObject1.setRadius(gameObject1.getRadius()*boxScale*collisionFixFactor);

	int step = 2;
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		enemyObjects[i].init(&mEnemy, sqrt(2.0), Vector3(-5,.5,step*i - 3.8), Vector3(1,0,0), 3000.0f, 1);
		//enemyObjects[i].init(&mBox, sqrt(2.0), Vector3(-5,.5,step*i - 3.8), Vector3(1,0,0), 3000.0f, 1);
		enemyObjects[i].setRadius(enemyObjects[i].getRadius()*boxScale * collisionFixFactor);
		enemyObjects[i].setInActive();
	}

	for (int i = 0; i < MAX_NUM_BULLETS; i++)
	{
		playerBullets[i].init(&mBullet, sqrt(2.0), Vector3(0,0,0), Vector3(-5,0,0), 7000.0f, 1);
		playerBullets[i].setInActive();
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

	D3DXVECTOR3 pos(10.0f, 2.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	//Camera Object
	camera.init(pos, Vector3(0,0,0), Vector3(0,0,0));
	camera.setPerspective();
	// camera
	cameraPos = pos;

}

void ColoredCubeApp::onResize()
{
	D3DApp::onResize();
	//Camera Object
	camera.setPerspective();
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

void shootBullet(GameObject playerBullets[], float dt, GameObject player)
{
	/*for (int i = 0; i < MAX_NUM_BULLETS; i++)
	{
		if (playerBullets[i].getActiveState())
			continue;
		else
		{
			D3DXVECTOR3 position = player.getPosition();
			position.y = 0.5;
			playerBullets[i].setActive();
			playerBullets[i].setPosition(position);
			return;
		}
	}*/

	D3DXVECTOR3 position = player.getPosition();
	position.y = 0.5;
	playerBullets[currentBullet].setActive();
	playerBullets[currentBullet].setPosition(position);
	currentBullet++;
	if(currentBullet >= MAX_NUM_BULLETS) currentBullet = 0;
	return;

}


void ColoredCubeApp::updateScene(float dt)
{
	switch (gsm->getGameState()) {
	case GameStateManager::START_GAME: {
		D3DApp::updateScene(dt);
		gameObject1.update(dt);
		D3DXMATRIX w;

		D3DXMatrixTranslation(&w, 2, 2, 0);
		mfxWVPVar->SetMatrix(w);

		// Build the view matrix.
		/*D3DXVECTOR3 pos(10.0f, 2.0f, 0.0f);
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&mView, &pos, &target, &up);*/

		if(GetAsyncKeyState(VK_SPACE) & 0x8000 && gsm->getGameState() != GameStateManager::IN_GAME) {
			gsm->setGameState(GameStateManager::IN_GAME);
		}

		Vector3 oldEnemyPositions[MAX_NUM_ENEMIES];
		for (int i = 0; i < MAX_NUM_ENEMIES; i++)
		{
			oldEnemyPositions[i] = enemyObjects[i].getPosition();
		}

		Vector3 oldBulletPositions[MAX_NUM_BULLETS];
		for (int i = 0; i < MAX_NUM_BULLETS; i++)
		{
			oldBulletPositions[i] = playerBullets[i].getPosition();
		}

		//Camera Object
		camera.update(dt);

		break;

		

	}

	case GameStateManager::IN_GAME:
	{
		//Generate a block every three seconds
		if (timeBuffer.elapsedTime() > 2) {
			timeBuffer.resetClock();

			generateEnemy(enemyObjects, dt);
		}

		for (int i = 0; i < MAX_NUM_BULLETS; i++)
		{
			playerBullets[i].update(dt);
		}

		
		if(GetAsyncKeyState(VK_RETURN) & 0x8000){
			if(shotRelease){
				shootBullet(playerBullets, dt, gameObject1);
				shotRelease = false;
			}
		}

		if(!(GetAsyncKeyState(VK_RETURN) & 0x8000)) shotRelease = true;

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

		if((GetAsyncKeyState('A') & 0x8000) && !(GetAsyncKeyState('D') & 0x8000))  direction.z = -1.0f;
		if((GetAsyncKeyState('D') & 0x8000) && !(GetAsyncKeyState('A') & 0x8000))  direction.z = +1.0f;

		D3DXVec3Normalize(&direction, &direction);

		for (int i = 0; i < MAX_NUM_ENEMIES; i++)
		{
			//if they collide and are active
			if(gameObject1.collided(&enemyObjects[i]) && enemyObjects[i].getActiveState())
			{
				audio->playCue(BEEP1);
				enemyObjects[i].setInActive();
				//score++;
				camera.cameraShake(dt);
				gsm->setGameState(GameStateManager::END_GAME);
			}
		}

		for (int i = 0; i < MAX_NUM_BULLETS; i++)
		{
			for (int j = 0; j < MAX_NUM_ENEMIES; j++)
			{
				if(playerBullets[i].collided(&enemyObjects[j]) && enemyObjects[j].getActiveState())
				{
					enemyObjects[j].setInActive();
					playerBullets[i].setInActive();
					score++;
				}
			}
		}


		gameObject1.setVelocity( direction * gameObject1.getSpeed() * dt);

		if (gameObject1.getPosition().z < -PLAYER_Z_RANGE){
			gameObject1.setPosition(Vector3(oldposition.x, oldposition.y, -PLAYER_Z_RANGE));
			camera.setCameraMoveLeft(false);
			camera.setCameraMoveRight(true);
		}
		else if (gameObject1.getPosition().z > PLAYER_Z_RANGE){
			gameObject1.setPosition(Vector3(oldposition.x, oldposition.y, PLAYER_Z_RANGE));
			camera.setCameraMoveRight(false);
			camera.setCameraMoveLeft(true);
		}
		else {
			camera.setCameraMoveRight(true);
			camera.setCameraMoveLeft(true);
		}

		//Destroys bullet if too far away
		for (int i = 0; i < MAX_NUM_BULLETS; i++)
		{
			if(playerBullets[i].getPosition().x < -10 && playerBullets[i].getActiveState())
				playerBullets[i].setInActive();
		}

		D3DXMATRIX w;

		D3DXMatrixTranslation(&w, 2, 2, 0);
		mfxWVPVar->SetMatrix(w);

		//Camera Object
		camera.update(dt);

		//Get Camera viewMatrix
		mView = camera.getViewMatrix();
		mProj = camera.getProjectionMatrix();
	}


	case GameStateManager::END_GAME:
	{
		//D3DApp::updateScene(dt);
		//gameObject1.update(dt);

		if (camera.isCameraShaking()) {
			camera.cameraShake(dt);
		}

		D3DXMATRIX w;

		D3DXMatrixTranslation(&w, 2, 2, 0);
		mfxWVPVar->SetMatrix(w);

		//// Build the view matrix.
		//D3DXVECTOR3 pos(10.0f, 2.0f, 0.0f);
		//D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		//D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		//D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

		if(GetAsyncKeyState(VK_SPACE) & 0x8000 && gsm->getGameState() != GameStateManager::IN_GAME) {
			restartGame();
			gsm->setGameState(GameStateManager::IN_GAME);
		}

		//Camera Object
		camera.update(dt);

		break;
	}


		default: {
			throw(GameError(gameErrorNS::FATAL_ERROR, "Game State Error"));
			break;
		}
	}
}

void ColoredCubeApp::drawScene()
{
	int foo[1] = {0};
	switch (gsm->getGameState()) {
	case GameStateManager::START_GAME:
		{
			foo[0] = 1;
			break;
		}
	case GameStateManager::IN_GAME:
		{
			foo[0] = 0;
			break;
		}
	case GameStateManager::END_GAME:
		{
			foo[0] = 1;
			break;
		}
	default:
		foo[0] = 0;
		break;
	}

	mfxBlack_WhiteVar->SetRawValue(&foo[0], 0, sizeof(int));

	D3DApp::drawScene();

	// Restore default states, input layout and primitive topology 
	// because mFont->DrawText changes them.  Note that we can 
	// restore the default states by passing null.
	md3dDevice->OMSetDepthStencilState(0, 0);
	float blendFactors[] = {0.0f, 0.0f, 0.0f, 0.0f};
	md3dDevice->OMSetBlendState(0, blendFactors, 0xffffffff);
	md3dDevice->IASetInputLayout(mVertexLayout);
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Get Camera viewMatrix
	 mView = camera.getViewMatrix();
	 mProj = camera.getProjectionMatrix();

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


	for (int i = 0; i < MAX_NUM_BULLETS; i++)
	{
		mWVP = playerBullets[i].getWorldMatrix()  *mView*mProj;
		mfxWVPVar->SetMatrix((float*)&mWVP);
		playerBullets[i].setMTech(mTech);
		playerBullets[i].draw();
	}

	std::wostringstream scoreString;   
	scoreString.precision(6);
	scoreString << score;
	finalScore = scoreString.str();

	// We specify DT_NOCLIP, so we do not care about width/height of the rect.
	RECT R = {5, 5, 0, 0};
	RECT R2 = {GAME_WIDTH/2 + 50, GAME_HEIGHT + 65, 0, 0};
	mFont->DrawText(0, mFrameStats.c_str(), -1, &R, DT_NOCLIP, BLACK);
	scoreFont->DrawText(0, finalScore.c_str(), -1, &R2, DT_NOCLIP, BLACK);


	mSwapChain->Present(0, 0);
}

void ColoredCubeApp::restartGame() {
	gameObject1.setPosition(Vector3(6,.5,0));
	gameObject1.setVelocity(Vector3(0,0,0));

	int step = 2;
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		enemyObjects[i].setInActive();
	}

	//Camera Object
	D3DXVECTOR3 pos(10.0f, 2.0f, 0.0);
	camera.restart();
	// camera
	cameraPos = pos;

	score = 0;
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
	mfxBlack_WhiteVar = mFX->GetVariableByName("black_white");
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
