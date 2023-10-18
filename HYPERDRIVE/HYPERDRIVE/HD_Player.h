#ifndef SUB_HEADER_DEFINED
#define SUB_HEADER_DEFINED
#include "HD_Framework_Sub.h"
#endif
#ifndef OBJECT_HEADER_DEFINED
#define OBJECT_HEADER_DEFINED
#include "HD_Object.h"
#endif

class HD_Player : public HD_Object
{
public:
	HD_Player();
	HD_Player(const HD_Player &src);
	HD_Player(SDL_PixelFormat* Screenformat, SDL_Renderer *render, const char * srcname, int width, int height, int exist);
	~HD_Player();
	bool copy_enable; //for copy constructor
	void correctMoving();
	void manageBullet();// player's bullet
	bool CollisionCheck(HD_Object &target);
	// bool addSubWeapon(HD_Object &src);
	//member variable
	int chargeGauge; // Must see HD_MAX_CHARGE_GAUGE definition
	HD_Object* bullet;
	HD_Object chargeShot;
	HD_Object Shield;
	bool spacePressed;
	bool ctrlPressed;
	int subwpnorder;
	HD_Timer CrashTimer;
	char healthbuf[3];
	Mix_Chunk* PlayerSFX;
	int SFXChannel;
};