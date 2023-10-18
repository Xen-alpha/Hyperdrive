#ifndef SUB_HEADER_DEFINED
#define SUB_HEADER_DEFINED
#include "HD_Framework_Sub.h"
#endif
class HD_Object {
public:
	HD_Object();
	HD_Object(SDL_PixelFormat* Screenformat, SDL_Renderer *render, int width, int height, int exist);
	HD_Object(SDL_PixelFormat* Screenformat, SDL_Renderer *render, const char * srcname, int width, int height, int exist);
	~HD_Object();
	//public member function
	bool LoadSprite(const char * filename, HD_Rect size, SDL_Renderer* renderer);
	bool LoadText(SDL_Renderer *renderer, const char * text);
	HD_Point GetDirection();
	int GetRenderType(void);
	SDL_Color GetTextColor();
	int GetHealth();
	HD_Point GetObjectCenter(bool framed); // framed : fixed in map/scroll point
	void SetDirection(double dx, double dy);
	void SetAniType(unsigned int totalFrame, int renderType);
	void SetPosition(double x1, double y1);
	void SetHealth(int hp);
	bool SetObjectAttribute(SDL_Renderer * renderer, bool IsText, const char * Text, HD_Rect srcRect, HD_Rect dstRect, int state, int ObjectType, int aniType, int totalFrame); //one-line setting
	// Object Preset - use these functions on spawn condition(false) and manage condition(true)
	bool Preset_MoveDown(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveDown_TrackPlayer(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, HD_Object &Player);
	bool Preset_MoveDown_Accelerate(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveRight(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveLeft(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveDown_StopOnce(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveDown_StopOnce_Long(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveDown_StopOnce_Long_Upperbound(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveDown_StopOnce_BacktoUpperside(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveLeftTilted(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveRightTilted(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveDownwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_x);
	bool Preset_MoveUpwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_x);
	bool Preset_MoveLeftwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_y);
	bool Preset_MoveRightwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_y);
	bool Preset_MoveRight_ChargeTowardPlayer(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, HD_Object &Player);
	bool Preset_MoveLeft_ChargeTowardPlayer(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, HD_Object &Player);
	bool Preset_MoveRight_ThenMoveDown(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	bool Preset_MoveLeft_ThenMoveDown(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype);
	// public member variables
	SDL_Texture* spritebase;
	SDL_Surface* spritebase_Surface;
	SDL_Renderer* renderer;
	HD_Rect area; // position, width & height
	HD_Rect dstarea;
	// Animation-related variable
	unsigned int currentFrame;
	unsigned int totalAniFrame;
	int existence; // 0 : not exist, 1 : play creating anitmation, 2 : exist, 3 : play destructing animation
	int attackType; // usually determine enemy's type; 0- no effect of screen distortion,1~10 - ingame object, 11~20 - ground, 21~99 - air, >=1000 - boss; re-used in HD_Player
	int health; // -1 : immortal, 0: dead, >0: health point
	int AttackPhase;
	HD_Timer attackTimer;
	int MovementPhase;
	HD_Timer MoveTimer;
	int DestroyPhase;
	HD_Timer DestroyTimer;
	// SubWeapon, re-used in HD_Player
	HD_Object* subweapon; // Warning! careful allocation of memory
	//HD_Point ActualPosition;
	HD_Point GetCenterPosition();
	void SetCenterPosition(double x, double y);
	double angle;
	SDL_Point anchor;
	TTF_Font * font;
	bool flip;
private:
	SDL_PixelFormat* format;
	HD_Point direction;
	int movespeed;
	// Animation-related variable
	int AniRenderType; // -1: no animation
	SDL_Color textColor;
};
