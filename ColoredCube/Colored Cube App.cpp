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
#include "point.h"

#include <sstream>

int currentBullet;

class ColoredCubeApp : public D3DApp
{

public:
	ColoredCubeApp(HINSTANCE hInstance);
	~ColoredCubeApp();

	void initApp();
	void onResize();
	void updateScene(float dt);
	void drawScene();
	void restartGame();
	void runExplosion(Vector3 pos);
	void placeStars();

	//Input* getInput() {   return input;}
	//Audio* getAudio() {   return audio;}
	void shootBullet(GameObject playerBullets[], float dt, GameObject player);
private:

	void buildFX();
	void buildVertexLayouts();

	Input* input;
	Audio* audio;

	TimeBuffer enemyBuffer;
	TimeBuffer shotBuffer;
	TimeBuffer gameTimer;

	int secondsRemaining;

	Axes mAxes;
	Box mEnemy, mPlayer, mBullet;
	Box mBox;
	Box particleBox;
	Box particleBox2;
	Box starBox;

	GameStateManager* gsm;

	Line mLine;
	Triangle mTriangle;
	Quad mQuad;

	GameObject gameObject1;

	GameObject enemyObjects[MAX_NUM_ENEMIES];
	GameObject playerBullets[MAX_NUM_BULLETS];
	GameObject particles[MAX_NUM_EXP_PARTICLES];
	GameObject stars[MAX_NUM_STARS];

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
	std::wstring timeString;
	bool shotRelease;
	float explosionTimer;
	bool explosionRunning;
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
	secondsRemaining = TOTAL_SECONDS;
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
	explosionTimer = 0;
	explosionRunning = false;
	shotRelease = true;

	// increments when you run into a cube // just for now
	score = 0;
	mAxes.init(md3dDevice, 1.0f);
	mEnemy.init(md3dDevice, .5f, RED);
	mPlayer.init(md3dDevice, .5f, BLUE);
	mBullet.init(md3dDevice, .25f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 0.0f));
	particleBox.init(md3dDevice, .01f, GREEN);
	particleBox2.init(md3dDevice, .04f, RED);
	starBox.init(md3dDevice, 0.05f, WHITE);
	//mBox.init(md3dDevice, boxScale);
	mLine.init(md3dDevice, 1.0f);
	//mTriangle.init(md3dDevice, 1.0f);
	//mQuad.init(md3dDevice, 10.0f);
	mQuad.init(md3dDevice, 0.0f);

	gameObject1.init(&mPlayer, sqrt(2.0f), Vector3(6,.5,0), Vector3(0,0,0), 5.0f,1.0f);
	//gameObject1.init(&mBox, sqrt(2.0f), Vector3(6,.5,0), Vector3(0,0,0), 5000.0f,1.0f);
	gameObject1.setRadius(gameObject1.getRadius()*boxScale*collisionFixFactor);

	int step = 2;
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		enemyObjects[i].init(&mEnemy, sqrt(2.0), Vector3(-5,.5,step*i - 3.8), Vector3(1,0,0), 8.0f, 1);
		//enemyObjects[i].init(&mBox, sqrt(2.0), Vector3(-5,.5,step*i - 3.8), Vector3(1,0,0), 3000.0f, 1);
		enemyObjects[i].setRadius(enemyObjects[i].getRadius()*boxScale * collisionFixFactor);
		enemyObjects[i].setInActive();
	}

	for (int i = 0; i < MAX_NUM_BULLETS; i++)
	{
		playerBullets[i].init(&mBullet, 0.5f, Vector3(0,0,0), Vector3(-5,0,0), 7000.0f, 1);
		playerBullets[i].setInActive();
	}

	for (int i = 0; i < MAX_NUM_EXP_PARTICLES; i++)
	{

		if(i%5 == 0)particles[i].init(&particleBox2, 0.5f, Vector3(0,0,0), Vector3(0,0,0), 7000.0f, 1);
		else particles[i].init(&particleBox, 0.5f, Vector3(0,0,0), Vector3(0,0,0), 7000.0f, 1);
		particles[i].setInActive();
	}

	for (int i = 0; i < MAX_NUM_STARS; i++)
	{
		stars[i].init(&starBox, 0.5f, Vector3(0,0,0), Vector3(0,0,0), 7000.0f, 1);
		stars[i].setActive();
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

	enemyBuffer.resetClock();
	shotBuffer.resetClock();
	gameTimer.resetClock();


	D3DXVECTOR3 pos(10.0f, 2.0f, 0.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	//Camera Object
	camera.init(pos, Vector3(0,0,0), Vector3(0,0,0));
	camera.setPerspective();
	// camera
	cameraPos = pos;

	audio->playCue(BKG);

	//Places stars in scene
	placeStars();
}

void ColoredCubeApp::onResize()
{
	D3DApp::onResize();
	//Camera Object
	camera.setPerspective();
	float aspect = (float)mClientWidth/mClientHeight;
	D3DXMatrixPerspectiveFovLH(&mProj, 0.25f*PI, aspect, 1.0f, 1000.0f);
}

bool isPreviousPosition(int hValue, int numEnemiesGenerated, Vector3 prevPositions[]) {
	for (int i = 0; i < numEnemiesGenerated; i++)
	{
		if (hValue == prevPositions[i].z)
			return true;
	}

	return false;
}

bool isPreviousDirection(int hValue, int numEnemiesGenerated, Vector3 prevPositions[]) {
	for (int i = 0; i < numEnemiesGenerated; i++)
	{
		if (hValue == prevPositions[i].z || hValue == prevPositions[i].z - 1 || hValue == prevPositions[i].z + 1 )
			return true;
	}

	return false;
}

bool isPreviousLocation(float hValue, int numEnemiesGenerated, Vector3 prevPositions[]) {
	for (int i = 0; i < numEnemiesGenerated; i++)
	{
		if (hValue == prevPositions[i].z || hValue == prevPositions[i].z - 0.5f || hValue == prevPositions[i].z + 0.5f )
			return true;
	}

	return false;
}

void generateEnemy(GameObject enemyObjects[], float dt) {

	int numEnemiesGenerated = 0;

	Vector3 prevPositions[MAX_NUM_ENEMIES];
	Vector3 previousLocations[MAX_NUM_ENEMIES];
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		//This means he's already on the field, so keep going
		if (enemyObjects[i].getActiveState())
			continue;
		else
		{
			float horizontalStartingPoint;
			int leftOrRightSide;
			while (true) {
				horizontalStartingPoint = rand()%5;

				//flip it so that the boxes also appear sometimes on the left part of the screen
				leftOrRightSide = 1;
				if (rand()%2)
					leftOrRightSide = -1;

				horizontalStartingPoint *= leftOrRightSide;

				if (isPreviousPosition(horizontalStartingPoint, numEnemiesGenerated, prevPositions) == false)
					break;
			}




			//put the enemy object somewhere randomly and make them active
			enemyObjects[i].setPosition(Vector3(-5,.5, horizontalStartingPoint));
			enemyObjects[i].setActive();

			prevPositions[numEnemiesGenerated] = enemyObjects[i].getPosition();


			//Now figure out their direction of travel
			int randomZValue = ( rand() % (PLAYER_Z_RANGE + 1) ) * leftOrRightSide;

			//These are the player starting points

			Vector3 location = Vector3(5, .5, (randomZValue)/2.0f);

			while (isPreviousDirection(location.z, numEnemiesGenerated, previousLocations)) {
				randomZValue = ( rand() % ( 2 * (PLAYER_Z_RANGE + 1) ) ) * leftOrRightSide;
				location = Vector3(5, .5, (randomZValue)/2.0f);


			}

			Vector3 direction =  location - enemyObjects[i].getPosition();

			previousLocations[numEnemiesGenerated] = location;


			D3DXVec3Normalize(&direction, &direction);

			enemyObjects[i].setVelocity(direction * enemyObjects[i].getSpeed());


			numEnemiesGenerated++;
			//Most of the time, only one block will be generated. This is the second block
			if (rand()%3 || numEnemiesGenerated == 3)
				return;
		}
	}
}

void ColoredCubeApp::shootBullet(GameObject playerBullets[], float dt, GameObject player)
{

	if (shotBuffer.elapsedTime() < 1)
		return;
	else
		shotBuffer.resetClock();
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

	audio->playCue(LASER);
	playerBullets[currentBullet].setActive();
	playerBullets[currentBullet].setPosition(position);

	currentBullet++;
	if(currentBullet >= MAX_NUM_BULLETS) currentBullet = 0;

	return;
}


void ColoredCubeApp::updateScene(float dt)
{
	for (int i = 0; i < MAX_NUM_STARS; i++)
	{
		stars[i].update(dt);
	}

	switch (gsm->getGameState()) {
	case GameStateManager::START_GAME: {
		D3DApp::updateScene(dt);
		gameObject1.update(dt);
		D3DXMATRIX w;

		D3DXMatrixTranslation(&w, 2, 2, 0);
		mfxWVPVar->SetMatrix(w);

		score = 0;

		// Build the view matrix.
		/*D3DXVECTOR3 pos(10.0f, 2.0f, 0.0f);
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMatrixLookAtLH(&mView, &pos, &target, &up);*/

		if(GetAsyncKeyState(VK_SPACE) & 0x8000 && gsm->getGameState() != GameStateManager::IN_GAME) {
			gsm->setGameState(GameStateManager::IN_GAME);
			audio->playCue(SELECT);
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
			if (enemyBuffer.elapsedTime() > 2) {
				enemyBuffer.resetClock();

				generateEnemy(enemyObjects, dt);
			}

			if (gameTimer.elapsedTime() >= 1) {
				gameTimer.resetClock();
				secondsRemaining--;
			}


			for (int i = 0; i < MAX_NUM_BULLETS; i++)
			{
				playerBullets[i].update(dt);
			}


			for (int i = 0; i < MAX_NUM_EXP_PARTICLES; i++)
			{
				particles[i].update(dt);
			}

			

			if(explosionRunning) explosionTimer += dt;
			if (explosionTimer > .55) {
				explosionTimer = 0;
				explosionRunning = false;
				for (int i = 0; i < MAX_NUM_EXP_PARTICLES; i++)
				{
					particles[i].setInActive();
				}
			}

			if(GetAsyncKeyState(VK_RETURN) & 0x8000){
				if(shotRelease) {

					shootBullet(playerBullets, dt, gameObject1);

					//shotRelease = false;
				}
			}


			//if(!(GetAsyncKeyState(VK_RETURN) & 0x8000)) shotRelease = true;

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
					audio->playCue(FAIL);
					enemyObjects[i].setInActive();
					//score++;
					camera.cameraShake(dt);
					gsm->setGameState(GameStateManager::END_GAME);
				}


				if (enemyObjects[i].getPosition().x > 7) {
					enemyObjects[i].setInActive();
				}
			}

			for (int i = 0; i < MAX_NUM_BULLETS; i++)
			{
				for (int j = 0; j < MAX_NUM_ENEMIES; j++)
				{
					if(playerBullets[i].collided(&enemyObjects[j]) && enemyObjects[j].getActiveState())
					{
						audio->playCue(BOOM);
						explosionTimer = 0;
						runExplosion(playerBullets[i].getPosition());
						enemyObjects[j].setInActive();
						playerBullets[i].setInActive();
						score++;	
					}
				}
			}


			gameObject1.setVelocity( direction * gameObject1.getSpeed());

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

		if (secondsRemaining <= 0)
		{
			gsm->setGameState(GameStateManager::END_GAME);
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
				audio->playCue(SELECT);
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
		//mAxes.draw();
		//mLine.draw();
		//mTriangle.draw();
		mQuad.draw();
		//particleBox.draw();
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

	for (int i = 0; i < MAX_NUM_EXP_PARTICLES; i++)
	{
		mWVP = particles[i].getWorldMatrix()  *mView*mProj;
		mfxWVPVar->SetMatrix((float*)&mWVP);
		particles[i].setMTech(mTech);
		particles[i].draw();
	}

	for (int i = 0; i < MAX_NUM_STARS; i++)
	{
		mWVP = stars[i].getWorldMatrix()  *mView*mProj;
		mfxWVPVar->SetMatrix((float*)&mWVP);
		stars[i].setMTech(mTech);
		stars[i].draw();
	}

	if(gsm->getGameState() == GameStateManager::END_GAME){
		std::wostringstream gameOverString;   
		gameOverString.precision(6);
		gameOverString << "GAME OVER!\n";
		gameOverString << "Spacebar to\nplay again.";
		finalScore = gameOverString.str();
		RECT R2 = {GAME_WIDTH/2 - 100, GAME_HEIGHT/2 - 100, 0, 0};
		endFont->DrawText(0, finalScore.c_str(), -1, &R2, DT_NOCLIP, GREEN);
	}
	if(gsm->getGameState() == GameStateManager::START_GAME){
		std::wostringstream gameOverString;   
		gameOverString.precision(6);
		gameOverString << "Controls:\n";
		gameOverString << "Move: A and D.\n";
		gameOverString << "Shoot: Enter \n";
		gameOverString << "Hit the spacebar to begin.";
		finalScore = gameOverString.str();
		RECT R2 = {100, GAME_HEIGHT/2 - 100, 0, 0};
		scoreFont->DrawText(0, finalScore.c_str(), -1, &R2, DT_NOCLIP, GREEN);
	}
	else{
		std::wostringstream scoreString;   
		scoreString.precision(6);
		scoreString << score;
		finalScore = scoreString.str();
		RECT R2 = {GAME_WIDTH/2 + 50, GAME_HEIGHT + 65, 0, 0};
		scoreFont->DrawText(0, finalScore.c_str(), -1, &R2, DT_NOCLIP, GREEN);
	}

	
	std::wostringstream ts;
	ts.precision(6);
	ts << secondsRemaining;
	timeString = ts.str();
	RECT R3 = {GAME_WIDTH/2 + 50, 20, 0, 0};
	scoreFont->DrawText(0, timeString.c_str(), -1, &R3, DT_NOCLIP, GREEN);

	// We specify DT_NOCLIP, so we do not care about width/height of the rect.
	RECT R = {5, 5, 0, 0};
	//mFont->DrawText(0, mFrameStats.c_str(), -1, &R, DT_NOCLIP, BLACK);




	mSwapChain->Present(0, 0);
}

void ColoredCubeApp::restartGame() {
	gameObject1.setPosition(Vector3(6,.5,0));
	gameObject1.setVelocity(Vector3(0,0,0));

	//int step = 2;
	for (int i = 0; i < MAX_NUM_ENEMIES; i++)
	{
		enemyObjects[i].setInActive();
	}

	for (int i = 0; i < MAX_NUM_BULLETS; i++)
	{
		playerBullets[i].setInActive();
	}

	//Camera Object
	D3DXVECTOR3 pos(10.0f, 2.0f, 0.0);
	camera.restart();
	// camera
	cameraPos = pos;


	secondsRemaining = TOTAL_SECONDS;
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

void ColoredCubeApp::runExplosion(Vector3 pos){
	for (int i = 0; i < MAX_NUM_EXP_PARTICLES; i++)
	{
		particles[i].setActive();

		float rand1 = (rand() % 3);
		if(rand() % 2 == 0) rand1 *= -1;
		float rand2 = (rand() % 3);
		if(rand() % 2 == 0) rand2 *= -1;
		float rand3 = (rand() % 3);
		if(rand() % 2 == 0) rand3 *= -1;

		particles[i].setPosition(pos);
		particles[i].setVelocity(D3DXVECTOR3(rand1,rand2,rand3));

	}
	explosionRunning = true;
}

void ColoredCubeApp::placeStars()
{
	for (int i = 0; i < MAX_NUM_STARS; i++)
	{
		float randx = (rand() % 10) + (-10);
		float randz = (rand() % 21) + (-10);
		float randy = (rand() % 21) + (-10);
		Vector3 starPosition(randx, randy, randz);
		stars[i].setPosition(starPosition);
	}
}