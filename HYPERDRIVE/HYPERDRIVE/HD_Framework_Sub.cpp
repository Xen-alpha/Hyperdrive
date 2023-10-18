#include "HD_Framework_Sub.h"
using namespace std;

//------------------------------HD_Timer definition-----------------------------------
HD_Timer::HD_Timer(){ // also reset timer
	startTime = 0;
	pausedTime = 0;
	targetInteval = 0;
	stopped = true;
	started = false;
}
HD_Timer::HD_Timer(const HD_Timer &src){
	startTime = src.startTime;
	pausedTime = src.pausedTime;
	targetInteval = src.targetInteval;
	stopped = src.stopped;
	started = src.started;
}
HD_Timer::~HD_Timer(){}
void HD_Timer::startTimer(int targetTick){
	startTime = SDL_GetTicks();
	targetInteval = targetTick;
	stopped = false;
	started = true;
}
void HD_Timer::pauseTimer(){
	if (started == true && stopped == false){
		pausedTime = SDL_GetTicks();
		stopped = true;
	}
}
void HD_Timer::resumeTimer(){
	if (stopped == true && started == true){
		Uint32 tempTick = SDL_GetTicks();
		startTime += (tempTick - pausedTime);
		stopped = false;
		pausedTime = 0;
	}
}
bool HD_Timer::checkTimer(){
	if (started == true && stopped == false){
		Uint32 tempTick = SDL_GetTicks();
		if ((tempTick - startTime) >= targetInteval) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}
bool HD_Timer::checkStart(){
	// return true when the timer is active
	return started;
}

// ---------------- HD_Display Definition -----------------------
HD_Display::HD_Display(){
	Display_type = 0;
	angle = 0;
	gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X;
	gameScreenPos.y = 0;
	gameScreenArea.x = HD_GAME_WIDTH;
	gameScreenArea.y = HD_GAME_HEIGHT;
	gameScreenDirection = { 0, 0 };
	anchor = GetScreenCenter();
	flip = false;
	AreaDev = 0; // for gameScreenArea;
	Timer = HD_Timer();
	animation_test_counter = HD_Timer();
}
HD_Display::~HD_Display() {}
SDL_Point HD_Display::GetGameCenter(){
	// return center point of in-game texture: the criterion displayed onto the screen is (HD_INGAME_DISPLAY_CORRECTION_X, 0)
	SDL_Point temp;
	temp.x = gameScreenArea.x / 2 + HD_INGAME_DISPLAY_CORRECTION_X;
	temp.y = gameScreenArea.y / 2;
	return temp;
}
SDL_Point HD_Display::GetScreenCenter(){
	SDL_Point temppoint;
	temppoint.x = HD_SCREEN_WIDTH / 2;
	temppoint.y = HD_SCREEN_HEIGHT / 2;
	return temppoint;
}
HD_Point HD_Display::GetGamePosition(){
	return gameScreenPos;
}
SDL_Point HD_Display::GetGameArea(){
	return gameScreenArea;
}
bool HD_Display::SetCenterPosition(double x, double y){
	gameScreenPos.x = x - gameScreenArea.x / 2 + HD_INGAME_DISPLAY_CORRECTION_X;
	gameScreenPos.y = y - gameScreenArea.y / 2;
	return true;
}
bool HD_Display::DisplayReset() {
	Display_type = 0;
	gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X;
	gameScreenPos.y = 0;
	gameScreenArea.x = HD_GAME_WIDTH;
	gameScreenArea.y = HD_GAME_HEIGHT;
	gameScreenDirection = { 0, 0 };
	AreaDev = 0;
	angle = 0;
	flip = false;
	anchor = GetGameCenter();
	Timer = HD_Timer();
	return true;
}
bool HD_Display::Shake(){
	int temppos = rand() % 9;
	switch (temppos){
	case 0:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X - 2;
		gameScreenPos.y = -2;
		break;
	case 1:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X;
		gameScreenPos.y = -2;
		break;
	case 2:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X + 2;
		gameScreenPos.y = -2;
		break;
	case 3:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X - 2;
		gameScreenPos.y = 0;
		break;
	case 4:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X;
		gameScreenPos.y = 0;
		break;
	case 5:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X + 2;
		gameScreenPos.y = 0;
		break;
	case 6:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X - 2;
		gameScreenPos.y = 2;
		break;
	case 7:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X;
		gameScreenPos.y = 2;
		break;
	case 8:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X + 2;
		gameScreenPos.y = 2;
		break;
	default:
		gameScreenPos.x = HD_INGAME_DISPLAY_CORRECTION_X;
		gameScreenPos.y = 0;
		break;
	}
	return true;
}
bool HD_Display::HorizontalShrink(int shrinklimit, int speed) {
	if (AreaDev == 0){
		if (gameScreenArea.x < HD_GAME_WIDTH - shrinklimit) AreaDev = 1;
		else {
			gameScreenArea.x -= speed;
		}
	}
	else {
		if (gameScreenArea.x > HD_GAME_WIDTH + shrinklimit) AreaDev = 0;
		else gameScreenArea.x += speed;
	}
	SetCenterPosition(HD_GAME_WIDTH /2, HD_GAME_HEIGHT / 2);
	return true;
}
bool HD_Display::CrossShrink(bool PositiveRelation, int shrinklimit, int speed) {
	if (AreaDev == 0){
		if (gameScreenArea.x < HD_GAME_WIDTH - shrinklimit) AreaDev = 1;
		else {
			gameScreenArea.x -= speed;
			if (PositiveRelation) gameScreenArea.y -= speed;
			else gameScreenArea.y += speed;
		}
	}
	else {
		if (gameScreenArea.x > HD_GAME_WIDTH + shrinklimit) AreaDev = 0;
		else { 
			gameScreenArea.x += speed; 
			if (PositiveRelation) gameScreenArea.y += speed;
			else gameScreenArea.y -= speed;
		}
	}
	SetCenterPosition(HD_GAME_WIDTH / 2, HD_GAME_HEIGHT / 2);
	return true;
}
bool HD_Display::HorizontalMove(){
	if (gameScreenPos.x + HD_GAME_WIDTH >= HD_SCREEN_WIDTH){
		gameScreenDirection.x = -4;
	}
	else if (gameScreenPos.x <= 0){
		gameScreenDirection.x = 4;
	}
	else if (gameScreenDirection.x == 0){
		gameScreenDirection.x = 4;
	}
	gameScreenPos.x += gameScreenDirection.x;
	return true;
}
bool HD_Display::Swing8(){
	gameScreenPos.x =32*sin(AreaDev* M_PI / 90) + HD_INGAME_DISPLAY_CORRECTION_X;
	gameScreenPos.y = 32*sin(AreaDev *M_PI / 180);
	if (AreaDev == 359) AreaDev = 0;
	else AreaDev++;
	return true;
}
bool HD_Display::Swing(){
	if (AreaDev == 0){
		AreaDev = -0.015;
		angle = 0;
	}
	else if (angle <= -0.3){
		AreaDev = 0.015;
	}
	else if (angle >= 0.3){
		AreaDev = -0.015;
	}
	gameScreenPos.x = 320 * sin(angle) + HD_INGAME_DISPLAY_CORRECTION_X;
	gameScreenPos.y = 320 * cos(angle) - 320;
	angle += AreaDev;
	Timer = HD_Timer();
	Timer.startTimer(10);
	return true;
}
bool HD_Display::Spin(){
	if (Display_type != 2) Display_type = 2;
	if (angle < 360) angle += 1;
	else angle = 0;
	return true;
}

bool HD_Display::Spin180(){
	if (Display_type != 2) Display_type = 2;
	switch ((int)AreaDev){
	case 0:
		Timer = HD_Timer();
		Timer.startTimer(6000);
		AreaDev = 1;
		break;
	case 1:
		if (Timer.checkTimer()){
			Timer = HD_Timer();
			AreaDev = 2;
		}
		break;
	case 2:
		if (angle < 180) angle += 1;
		else {
			Timer = HD_Timer();
			Timer.startTimer(6000);
			AreaDev = 3;
		}
		break;
	case 3:
		if (Timer.checkTimer()){
			Timer = HD_Timer();
			AreaDev = 4;
		}
		break;
	case 4:
		if (angle < 360) angle += 1;
		else {
			Timer = HD_Timer();
			Timer.startTimer(6000);
			AreaDev = 1;
			angle = 0;
		}
		break;
	default:
		break;
	}
	return true;
}

