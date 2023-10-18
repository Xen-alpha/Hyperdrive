#ifndef SUB_HEADER_DEFINED
#define SUB_HEADER_DEFINED
#include "HD_Framework_Sub.h"
#endif
#ifndef OBJECT_HEADER_DEFINED
#define OBJECT_HEADER_DEFINED
#include "HD_Object.h"
#endif
#ifndef MENU_HEADER_DEFINED
#define MENU_HEADER_DEFINED
#include "HD_Menu.h"
#endif
#ifndef GAME_HEADER_DEFINED
#define GAME_HEADER_DEFINED
#include "HD_Game.h"
#endif
#ifndef PLAYER_HEADER_DEFINED
#define PLAYER_HEADER_DEFINED
#include "HD_Player.h"
#endif
#include <vector>
#define HD_BOSS1_HEALTH 2500
#define HD_BOSS2_HEALTH 4000
using namespace std;

class HD_SYSTEM {
public:
	HD_SYSTEM();
	~HD_SYSTEM();
	//public member function
	void HD_BlitSprite(SDL_Renderer *renderer, HD_Object &src, SDL_Rect * srcRect, SDL_Rect * dstRect);
	void HD_ManageKeyboard();
	void spawnEnemy();
	void manageObject();
	void SetMapObject();
	bool SetMapObject_sub(int i);
	void manageEnemy();
	void managePlayer();
	void manageBullet(); // creating, moving, deleting Enemy bullet
	bool manageBoss();
	void inGameTextureUpdate();
	void exitGame();
	void GameOver();
	// public member variables
	// program structure 
	HD_Game HD_gameSys;
	SDL_Window* HD_window;
	SDL_Renderer* HD_renderer; // Do not put this in private area
	SDL_Texture* HD_windowTexture;
	SDL_Texture* HD_gameTexture;
	SDL_Surface* HD_screenSurface;
	SDL_PixelFormat* HD_format;
	SDL_Surface* HD_gameSurface;
	SDL_Event HD_event;
	TTF_Font * font;
	SDL_Color textColor;
	HD_Timer KeyTimer;
	// game structure
	bool init_Flag;
	bool quit_Flag;
	HD_Menu TitleMenu;
	HD_Menu PauseMenu;
	HD_Display Display;
	Mix_Music* bgm; // BGM
	Mix_Chunk* EnemySFX;
	int SFXChannel_Enemy;
	char volumeText[10];
	HD_Object* Object;
	HD_Object* Object_ingame;
	vector<HD_Object> Object_map;
	HD_Timer MapTimer;
	HD_Object* Bullet;
	HD_Object* Enemy;
	HD_Player Player;
	unsigned int scene;
	HD_Object Boss;
	int boss_phase; // value-1 == (actual phase)
	HD_Timer bossTimer;
private:
	void HD_ProcessEvent(int input);
	bool spawnEnemy_Process(int trigger, int index);
	//variables for pause menu
	int focusedObject; // only used for selecting Object in game menu
	int scene_backup;
	//HD_Point MapPosition;
};
// Normal function definition
SDL_Rect RectTransform(HD_Rect src);
SDL_Point PointTransform(HD_Point src);
bool CollisionCheck(HD_Object &src, HD_Object &dst);