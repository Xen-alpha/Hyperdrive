#include <stdio.h>
#include <string>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
//constants
#define HD_SCREEN_WIDTH 800
#define HD_SCREEN_HEIGHT 600
#define HD_GAME_WIDTH 640
#define HD_GAME_HEIGHT 600
#define HD_GAME_SCROLL_SPEED 1
#define HD_MAP_SCROLL_SPEED 2
#define HD_NUMBER_OF_OBJECT 24
#define HD_NUMBER_OF_MY_BULLET 192
#define HD_NUMBER_OF_ENEMY_BULLET 1024
#define HD_NUMBER_OF_ENEMY 200
#define HD_PLAYER_ATTACK_POWER 4
#define HD_PLAYER_MOVE_SPEED 8
#define HD_MAX_CHARGE_GAUGE 16 // criterion of increasing HD_Player's chargeGauge variable
#define HD_MAX_RANK 32
#define HD_MAX_MAPOBJECT 256
#define HD_INGAME_DISPLAY_CORRECTION_X 80
using namespace std;
struct HD_Point {
	double x;
	double y;
};
struct HD_Rect {
	double x;
	double y;
	double w;
	double h;
};
class HD_Timer {
public:
	HD_Timer();
	HD_Timer(const HD_Timer &src);
	~HD_Timer();
	void startTimer(int targetTick);
	void pauseTimer();
	void resumeTimer();
	bool checkTimer();
	bool checkStart();
private:
	Uint32 startTime;
	Uint32 pausedTime;
	Uint32 targetInteval;
	bool stopped;
	bool started;
};
class HD_Display{ //utmost Display information
public:
	HD_Display();
	~HD_Display();
	SDL_Point GetGameCenter();
	SDL_Point GetScreenCenter();
	HD_Point GetGamePosition();
	SDL_Point GetGameArea();
	bool DisplayReset();
	bool Shake();
	bool Spin();
	bool Spin180();
	bool HorizontalShrink(int shrinklimit, int speed);
	bool CrossShrink(bool PositiveRelation, int shrinklimit, int speed);
	bool HorizontalMove();
	bool SinWave();
	bool FourSwap();
	bool Swing();
	bool Swing8();
	bool GetScreenCenter(double x, double y);
	bool SetCenterPosition(double x, double y);
	//public member
	unsigned int Display_type; // 0: Default, 1: screen Rotation;
	bool flip;
	double angle;
	double AreaDev; // for phase
	SDL_Point anchor; // creterion of spinning HD_gameTexture
	HD_Timer animation_test_counter;
private:
	SDL_Rect HD_screenRect;
	HD_Point gameScreenPos;
	SDL_Point gameScreenArea;
	HD_Point gameScreenDirection;
	HD_Timer Timer;
};