#include "HD_Menu.h"
using namespace std;

//-----------------------------HD_Menu----------------------------------------------
HD_Menu::HD_Menu(){ // do not use this
	totalitem = 0;
	currentitem = 0;
	initiated = false;
	for (int i = 0; i < 8; i++)
		state[i] = 0;
}
HD_Menu::HD_Menu(SDL_PixelFormat* Screenformat, SDL_Renderer *render){
	totalitem = 0;
	currentitem = 0;
	initiated = false;
	Display = false;
	for (int i = 0; i < 8; i++){
		Menutext[i] = HD_Object(Screenformat, render, 128, 64, 0);
		state[i] = 0;
	}
	Background = HD_Object(Screenformat, render, 160, 160, 0);
	Cursor = HD_Object(Screenformat, render, 180, 64, 0);
}
HD_Menu::~HD_Menu(){
}
void HD_Menu::InitMenu(SDL_Renderer *renderer){
	totalitem = 0;
	currentitem = 0;
	initiated = true;
	Background.area = { 0, 0, 160, 160 };
	Background.LoadSprite("resource/HD_Menu.png", Background.area, renderer);
	Background.dstarea = { 0, 0, HD_SCREEN_WIDTH / 4, 24 };
	Background.SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
	Background.SetAniType(0, -1);
	Background.existence = 2;
	Cursor.area = { 0, 0, 64, 64 };
	Cursor.LoadSprite("resource/Margin.png", Cursor.area, renderer);
	Cursor.dstarea = { 0, 0, HD_SCREEN_WIDTH / 4 - 8, 24 };
	Cursor.SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
	Cursor.SetAniType(0, -1);
	Cursor.existence = 2;
	Display = false;
	for (int i = 0; i < 8; i++)
		state[i] = 0;
	return;
}
bool HD_Menu::AddMenu(SDL_Renderer *renderer,const char *text, unsigned int referencestate){
		// add item and resize the background
		if (totalitem < 8) {
			Menutext[totalitem].LoadText(renderer, text);
			Menutext[totalitem].SetAniType(0, -1);
			Menutext[totalitem].existence = 2;
			Menutext[totalitem].SetCenterPosition(Background.GetCenterPosition().x, Background.dstarea.y + 9 + totalitem * 18);
			Background.dstarea.h += 18;
			state[totalitem] = referencestate;
			totalitem++;
			return true;
		}
		else return false;
}
unsigned int HD_Menu::GetCursorPosition(){
	return currentitem;
}
unsigned int HD_Menu::GetMenuReference(){
	return state[currentitem];
}
unsigned int HD_Menu::GetTotalItem(){
	return totalitem;
}
void HD_Menu::SetCursorPosition(unsigned int num){
	// set cursor position you want to choose depending on the ID number of item
	if (num < totalitem && num >= 0) {
		currentitem = num;
		Cursor.SetCenterPosition(Background.GetCenterPosition().x, Background.dstarea.y + 12 + num * 24);
	}
	return;
}
void HD_Menu::SetBackgroundSize(int w, int h){
	unsigned int i;
	Background.dstarea.w = w;
	Background.dstarea.h = h;
	Background.SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
	//correction of Cursor position
	Cursor.dstarea.w = w + 4;
	Cursor.dstarea.h = 24;
	SetCursorPosition(GetCursorPosition());
	//correction of item
	for (i = 0; i < GetTotalItem(); i++) {
		Menutext[i].SetCenterPosition(Background.GetCenterPosition().x, Background.dstarea.y + 12 + i * 24);
	}
	return;
}
void HD_Menu::ChangeMenuName(SDL_Renderer *renderer, int itemindex, char * text){
	HD_Point tempPosition = Menutext[itemindex].GetCenterPosition();
	Menutext[itemindex].LoadText(renderer, text);
	Menutext[itemindex].SetCenterPosition(tempPosition.x, tempPosition.y);
	return;
}