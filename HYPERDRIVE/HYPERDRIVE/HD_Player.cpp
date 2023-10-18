#include "HD_Player.h"
//-----------------------------HD_Player-------------------------------------------
HD_Player::HD_Player() :HD_Object(){
	chargeGauge = 0;
	bullet = NULL;
	subwpnorder = 0;
	copy_enable = false; //for copy constructor
	spacePressed = false;
	PlayerSFX = NULL;
	SFXChannel = -1;
}
HD_Player::HD_Player(const HD_Player &src) :HD_Object(src){ // may not work
	chargeGauge = src.chargeGauge;
	bullet = src.bullet;
	chargeShot = src.chargeShot;
	subweapon = new HD_Object[4];
	for (int i = 0; i < 4; i++){
		subweapon[i] = src.subweapon[i];
	}
	subwpnorder = 0;
	copy_enable = true;
	PlayerSFX = src.PlayerSFX;
	SFXChannel = src.SFXChannel;
}
HD_Player::HD_Player(SDL_PixelFormat* Screenformat, SDL_Renderer *render, const char * srcname, int width, int height, int exist) : HD_Object(Screenformat, render, srcname, width, height, exist) {
	chargeGauge = 0;
	area = { 0, 0, 64, 64 };
	dstarea = { 0, 0, 64, 64 };
	bullet = new HD_Object[HD_NUMBER_OF_MY_BULLET];
	for (int i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
		bullet[i] = HD_Object(Screenformat, render, 32, 64, 0);
		bullet[i].area = { 0, 0, 32, 32 };
		bullet[i].dstarea = { 0, 0, 32, 32 };
		bullet[i].LoadSprite("resource/HD_NormalShot.png", bullet[i].area, render);
	}
	subweapon = new HD_Object[4];
	for (int i = 0; i < 4; i++){
		subweapon[i] = HD_Object(Screenformat, render, 32, 32, 0);
		subweapon[i].area = { 0, 0, 64, 64 };
		subweapon[i].dstarea = { 0, 0, 64, 64 };
	}
	chargeShot = HD_Object(Screenformat, render, 32, 32, 0);
	Shield = HD_Object(Screenformat, render, 64, 64, 0);
	chargeShot.SetAniType(2, 0);
	subwpnorder = 0;
	copy_enable = false; //for copy constructor
	spacePressed = false;
	PlayerSFX = Mix_LoadWAV("resource/chargeShot_effect.wav");
	SFXChannel = -1;
}
HD_Player::~HD_Player(){
	if (copy_enable){ // only delete memory when HD_Player object is copied version
		delete[] subweapon;
		subweapon = NULL;
	}
}
void HD_Player::correctMoving(){
	if (GetDirection().x == HD_PLAYER_MOVE_SPEED && GetDirection().y == HD_PLAYER_MOVE_SPEED){
		SetDirection(0.7071 * HD_PLAYER_MOVE_SPEED, 0.7071 * HD_PLAYER_MOVE_SPEED);
	}
	else if (GetDirection().x == HD_PLAYER_MOVE_SPEED && GetDirection().y == -HD_PLAYER_MOVE_SPEED){
		SetDirection(0.7071 * HD_PLAYER_MOVE_SPEED, -0.7071 * HD_PLAYER_MOVE_SPEED);
	}
	else if (GetDirection().x == -HD_PLAYER_MOVE_SPEED && GetDirection().y == HD_PLAYER_MOVE_SPEED){
		SetDirection(-0.7071 * HD_PLAYER_MOVE_SPEED, 0.7071 * HD_PLAYER_MOVE_SPEED);
	}
	else if (GetDirection().x == -HD_PLAYER_MOVE_SPEED && GetDirection().y == -HD_PLAYER_MOVE_SPEED){
		SetDirection(-0.7071 * HD_PLAYER_MOVE_SPEED, -0.7071 * HD_PLAYER_MOVE_SPEED);
	}
	return;
}
void HD_Player::manageBullet(){ // manage player's bullet
	for (int i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
		//delete bullet or move it
		if (bullet[i].existence > 0) {
			if (bullet[i].dstarea.y < 0 || bullet[i].dstarea.y > HD_SCREEN_HEIGHT || bullet[i].dstarea.x < 0 || bullet[i].dstarea.x > HD_SCREEN_WIDTH){
				bullet[i].existence = 0; //delete
			}
			HD_Point tempdirection = bullet[i].GetDirection();
			bullet[i].SetPosition(bullet[i].dstarea.x + tempdirection.x, bullet[i].dstarea.y + tempdirection.y);
		}
	}
	if (chargeShot.existence > 0) {
		if (chargeShot.dstarea.y < 0 || chargeShot.dstarea.y > HD_SCREEN_HEIGHT || chargeShot.dstarea.x < 0 || chargeShot.dstarea.x > HD_SCREEN_WIDTH){
			chargeShot.existence = 0; //delete
		}
		HD_Point tempdirection = chargeShot.GetDirection();
		chargeShot.SetPosition(chargeShot.dstarea.x + tempdirection.x, chargeShot.dstarea.y + tempdirection.y);
	}
	return;
}
bool HD_Player::CollisionCheck(HD_Object &target){
	if (existence != 2 || target.existence != 2){
		return false;
	}
	if ((dstarea.x + dstarea.w * 15 / 32 < target.dstarea.x + target.dstarea.w * 7 / 8) && (dstarea.x + dstarea.w * 17 / 32 > target.dstarea.x + target.dstarea.w / 8) && (dstarea.y + dstarea.h * 15 / 32 < target.dstarea.y + target.dstarea.h * 7 / 8) && (dstarea.y + dstarea.h * 17 / 32 > target.dstarea.y + target.dstarea.h / 8)){
		return true;
	}
	else{
		return false;
	}
	return false;
}