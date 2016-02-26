// Game State Manager
// By Andrew Cunningham

#pragma once

class GameStateManager {
public:
	enum GameStates {	START_GAME, IN_GAME, END_GAME   };

	GameStateManager() {	state = START_GAME;}

	void setGameState(GameStates gs) {	state = gs;}
	GameStates getGameState() {return state;}

	void nextGameState() {
		switch (state)
		{
		case START_GAME:
			state = IN_GAME;
			break;
		case IN_GAME:
			state = END_GAME;
			break;
		case END_GAME:
			state = START_GAME;
			break;
		default:
			state = START_GAME;
			break;
		}
	}


private:
	GameStates state;
};