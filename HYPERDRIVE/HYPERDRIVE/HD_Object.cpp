#include "HD_Object.h"
using namespace std;

// ---------------- HD_Object Definition -----------------------
HD_Object::HD_Object(){
	area.x = 0;
	area.y = 0;
	direction.x = 0.0;
	direction.y = 0.0;
	//Set default value of Object's area
	area.w = 64.0;
	area.h = 64.0;
	dstarea.x = 0.0;
	dstarea.y = 0.0;
	movespeed = 0;
	currentFrame = 0;
	totalAniFrame = 0;
	AniRenderType = -1; // no animation
	attackType = 0;
	existence = 0;
	angle = 0.0;
	flip = false;
	dstarea.w = area.w;
	dstarea.h = area.h;
	anchor.x = 32.0;
	anchor.y = 32.0;
	textColor = { 0xff, 0xff, 0xff };
	health = 0;
	attackTimer = HD_Timer();
	MoveTimer = HD_Timer();
	DestroyTimer = HD_Timer();
	AttackPhase = 0;
	MovementPhase = 0;
	DestroyPhase = 0;
}
HD_Object::HD_Object(SDL_PixelFormat* Screenformat, SDL_Renderer *render, int width, int height, int exist){
	//set default value of Object's area
	area.x = 0;
	area.y = 0;
	direction.x = 0.0;
	direction.y = 0.0;
	area.w = width;
	area.h = height;
	movespeed = 0;
	format = Screenformat;
	renderer = render;
	spritebase_Surface = SDL_CreateRGBSurface(0, width, height, 32, 0x00FF0000, 0x0000FF00,	0x000000FF,	0xFF000000);
	spritebase = SDL_CreateTexture(render,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STREAMING, width, height);
	currentFrame = 0;
	totalAniFrame = 0;
	AniRenderType = -1;
	subweapon = NULL;
	attackType = 0;
	existence = exist;
	dstarea.x = 0.0;
	dstarea.y = 0.0;
	dstarea.w = area.w;
	dstarea.h = area.h;
	angle = 0.0;
	flip = false;
	anchor.x = width / 2;
	anchor.y = height / 2;
	font = TTF_OpenFontRW(SDL_RWFromFile("resource/PressStart2P.ttf", "r"), 1, 18);
	textColor = { 0xff, 0xff, 0xff };
	health = 1;
	attackTimer = HD_Timer();
	MoveTimer = HD_Timer();
	DestroyTimer = HD_Timer();
	AttackPhase = 0;
	MovementPhase = 0;
	DestroyPhase = 0;
}
HD_Object::HD_Object(SDL_PixelFormat* Screenformat, SDL_Renderer *render, const char * srcname, int width, int height, int exist){
	//set default value of Object's area
	area.x = 0;
	area.y = 0;
	direction.x = 0;
	direction.y = 0;
	area.w = width;
	area.h = height;
	movespeed = 0;
	format = Screenformat;
	renderer = render;
	spritebase_Surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);
	spritebase = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	currentFrame = 0;
	totalAniFrame = 0;
	AniRenderType = -1;
	subweapon = NULL;
	attackType = 0;
	existence = exist;
	dstarea.x = 0;
	dstarea.y = 0;
	dstarea.w = area.w;
	dstarea.h = area.h;
	angle = 0;
	flip = false;
	anchor.x = width / 2;
	anchor.y = height / 2;
	font = TTF_OpenFontRW(SDL_RWFromFile("resource/PressStart2P.ttf","r"), 1, 18);
	textColor = { 0xff, 0xff, 0xff };
	health = 1;
	attackTimer = HD_Timer();
	MoveTimer = HD_Timer();
	DestroyTimer = HD_Timer();
	AttackPhase = 0;
	MovementPhase = 0;
	DestroyPhase = 0;
}
HD_Object::~HD_Object(){
}
bool HD_Object::LoadSprite(const char * filename, HD_Rect size, SDL_Renderer* renderer){
	//debug point
	Uint32 format_debug;
	SDL_Point area_debug;
	SDL_QueryTexture(spritebase, &format_debug, NULL, &area_debug.x, &area_debug.y);
	/*cout << "Texture(Before): " << format_debug << " ( " << SDL_GetPixelFormatName(format_debug) << " ) " << endl;
	cout << "Texture area(Before): " << area_debug.x << " , " << area_debug.y << endl;
	cout << "Surface(Before): " << spritebase_Surface->format->format << "(" << SDL_GetPixelFormatName(spritebase_Surface->format->format) << ")" << endl;
	cout << "Surface area(Before): " << spritebase_Surface->w << " , " << spritebase_Surface->h << endl;*/
	// -------------------------------------------------------------------------------------------
	// ---------------------------------------------main code-------------------------------------
	void *pixels;
	int pitch;
	SDL_PixelFormat* Format_temp = SDL_AllocFormat(format_debug);
	SDL_DestroyTexture(spritebase);
	spritebase = NULL;
	SDL_FreeSurface(spritebase_Surface);
	spritebase_Surface = NULL;
	SDL_Surface *tempSurface = IMG_Load(filename);
	if (tempSurface == NULL){
		SDL_FreeFormat(Format_temp);
		return false;
	}
	else {
		spritebase_Surface = SDL_ConvertSurface(tempSurface, Format_temp, NULL);
		spritebase = SDL_CreateTexture(renderer, format_debug, SDL_TEXTUREACCESS_STREAMING, spritebase_Surface->w, spritebase_Surface->h);
		SDL_LockTexture(spritebase, NULL, &pixels, &pitch);
		memcpy(pixels, spritebase_Surface->pixels, pitch * spritebase_Surface->h);
		SDL_UnlockTexture(spritebase);
		pixels = NULL;
		pitch = 0;
		SDL_SetSurfaceBlendMode(spritebase_Surface, SDL_BLENDMODE_BLEND);
		SDL_SetTextureBlendMode(spritebase, SDL_BLENDMODE_BLEND);
		SDL_FreeFormat(Format_temp);
		SDL_FreeSurface(tempSurface);
		return true;
	}
}
HD_Point HD_Object::GetCenterPosition(){
	HD_Point temp;
	temp.x = dstarea.x + dstarea.w / 2.000;
	temp.y = dstarea.y + dstarea.h / 2.000;
	return temp;
}
void HD_Object::SetCenterPosition(double x, double y){
	dstarea.x = x - (double)dstarea.w / 2.000;
	dstarea.y = y - (double)dstarea.h / 2.000;
	return;
}
bool HD_Object::LoadText(SDL_Renderer *renderer, const char * text){
	Uint32 format_debug;
	SDL_Point area_debug;
	SDL_QueryTexture(spritebase, &format_debug, NULL, &area_debug.x, &area_debug.y);
	void *pixels;
	int pitch;
	SDL_PixelFormat* Format_temp = SDL_AllocFormat(format_debug);
	SDL_DestroyTexture(spritebase);
	spritebase = NULL;
	SDL_FreeSurface(spritebase_Surface);
	spritebase_Surface = NULL;
	SDL_Surface *tempSurface = TTF_RenderText_Solid(font, text, textColor);
	if (tempSurface == NULL){
		SDL_FreeFormat(Format_temp);
		return false;
	}
	else {
		spritebase_Surface = SDL_ConvertSurface(tempSurface, Format_temp, NULL);
		spritebase = SDL_CreateTexture(renderer, format_debug, SDL_TEXTUREACCESS_STREAMING, spritebase_Surface->w, spritebase_Surface->h);
		SDL_LockTexture(spritebase, NULL, &pixels, &pitch);
		memcpy(pixels, spritebase_Surface->pixels, pitch * spritebase_Surface->h);
		SDL_UnlockTexture(spritebase);
		pixels = NULL;
		pitch = 0;
		area.x = 0;
		area.y = 0;
		area.w = spritebase_Surface->w;
		area.h = spritebase_Surface->h;
		SDL_SetTextureBlendMode(spritebase, SDL_BLENDMODE_BLEND);
		dstarea.w = spritebase_Surface->w;
		dstarea.h = spritebase_Surface->h;
		SDL_FreeFormat(Format_temp);
		SDL_FreeSurface(tempSurface);
		return true;
	}
}
HD_Point HD_Object::GetDirection(){
	return direction;
}
void HD_Object::SetDirection(double dx, double dy){
	direction.x = dx;
	direction.y = dy;
	return;
}
int HD_Object::GetRenderType(void){
	return AniRenderType;
}
HD_Point HD_Object::GetObjectCenter(bool framed){
	HD_Point temp;
	temp.x = HD_INGAME_DISPLAY_CORRECTION_X + area.w / 2;
	temp.y = area.h / 2;
	return temp;
}
void HD_Object::SetAniType(unsigned int totalFrame, int renderType){
	currentFrame = 0;
	totalAniFrame = totalFrame;
	AniRenderType = renderType;
	return;
}
void HD_Object::SetPosition(double x1, double y1){
	dstarea.x = (double)x1;
	dstarea.y = (double)y1;
	return;
}
SDL_Color HD_Object::GetTextColor(){
	return textColor;
}
int HD_Object::GetHealth(){
	return health;
}
void HD_Object::SetHealth(int hp){
	health = hp;
	return;
}
bool HD_Object::SetObjectAttribute(SDL_Renderer *renderer, bool IsText, const char * Text, HD_Rect srcRect, HD_Rect dstRect, int state, int ObjectType,int aniType, int totalFrame){ //one-line setting
	area = srcRect;
	dstarea = dstRect;
	if (IsText) LoadText(renderer, Text);
	else LoadSprite(Text, area, renderer);
	existence = state;
	SetAniType(totalFrame, aniType);
	attackType = ObjectType;
	currentFrame = 0;
	AttackPhase = 0;
	MovementPhase = 0;
	DestroyPhase = 0;
	angle = 0;
	anchor.x = dstarea.x + GetObjectCenter(false).x;
	anchor.y = dstarea.y + GetObjectCenter(false).y;
	return true;
}

// Preset implementation
// Do not put attacktype in these function
// 'sizetype' parameter will only work when 'Condition' parameter is false
bool HD_Object::Preset_MoveDown(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetCenterPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(0, 6); 
		angle = 0;
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MovementPhase = 0;
		angle = 0;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
		existence = 2;
	}
	return true;
}
bool HD_Object::Preset_MoveDown_TrackPlayer(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, HD_Object &Player){
	if (Condition == true){
		if (Player.GetCenterPosition().x > GetCenterPosition().x && Player.GetCenterPosition().y > GetCenterPosition().y){
			if (GetDirection().x < 2) SetDirection(GetDirection().x + 0.2, GetDirection().y);
			if (angle > -20) angle -= 1;
		}
		else if (Player.GetCenterPosition().x < GetCenterPosition().x && Player.GetCenterPosition().y > GetCenterPosition().y){
			if (GetDirection().x > -2) SetDirection(GetDirection().x - 0.2, GetDirection().y);
			if (angle < 20)angle += 1;
		}
		else{
			SetDirection(0, GetDirection().y);
			angle = 0;
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetCenterPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(0, 8);
		AttackPhase = 0;
		angle = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MovementPhase = 0;
		angle = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveDown_Accelerate(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y + 0.1);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetCenterPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(0, HD_GAME_SCROLL_SPEED);
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MovementPhase = 0;
		angle = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveRight(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetCenterPosition(PosRect.x + 32, PosRect.y + 32);
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		existence = 2;
		SetDirection(5, 0);
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveLeft(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetCenterPosition(PosRect.x + 32, PosRect.y + 32);
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		existence = 2;
		SetDirection(-5, 0);
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveDown_StopOnce(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		if (!MoveTimer.checkStart() && dstarea.y >= 160 && MovementPhase == 0){
			SetDirection(0, 0);
			MoveTimer.startTimer(5000);
			MovementPhase = 1;
		}
		else if (MoveTimer.checkStart() && MoveTimer.checkTimer() && MovementPhase == 1){
			SetDirection(0, HD_GAME_SCROLL_SPEED + 4);
			MovementPhase = 2;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x+32, PosRect.y+32);
		SetDirection(0, 5); //downward x4 speed
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		existence = 2;
	}
	return true;
}
bool HD_Object::Preset_MoveDown_StopOnce_Long(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		if (!MoveTimer.checkStart() && dstarea.y >= 160 && MovementPhase == 0){
			SetDirection(0, 0);
			MoveTimer.startTimer(20000);
			MovementPhase = 1;
		}
		else if (MoveTimer.checkStart() && MoveTimer.checkTimer() && MovementPhase == 1){
			SetDirection(0, HD_GAME_SCROLL_SPEED + 4);
			MovementPhase = 2;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(0, 5); 
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		existence = 2;
	}
	return true;
}
bool HD_Object::Preset_MoveDown_StopOnce_Long_Upperbound(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		if (!MoveTimer.checkStart() && dstarea.y >= 10 && MovementPhase == 0){
			SetDirection(0, 0);
			MoveTimer.startTimer(20000);
			MovementPhase = 1;
		}
		else if (MoveTimer.checkStart() && MoveTimer.checkTimer() && MovementPhase == 1){
			SetDirection(0, HD_GAME_SCROLL_SPEED + 4);
			MovementPhase = 2;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(0, 8);
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		existence = 2;
	}
	return true;
}
bool HD_Object::Preset_MoveDown_StopOnce_BacktoUpperside(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		if (!MoveTimer.checkStart() && dstarea.y >= 160 && MovementPhase == 0){
			SetDirection(0, 0);
			MoveTimer.startTimer(5000);
			MovementPhase = 1;
		}
		else if (MoveTimer.checkStart() && MoveTimer.checkTimer() && MovementPhase == 1){
			SetDirection(0, -HD_GAME_SCROLL_SPEED);
			DestroyTimer = HD_Timer();
			DestroyTimer.startTimer(10000);
			MovementPhase = 2;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(0, HD_GAME_SCROLL_SPEED + 4); //downward x4 speed
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		existence = 2;
	}
	return true;
}
bool HD_Object::Preset_MoveLeftTilted(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(HD_GAME_SCROLL_SPEED + 4, HD_GAME_SCROLL_SPEED + 4); //downward x4 speed
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveRightTilted(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(-HD_GAME_SCROLL_SPEED - 4, HD_GAME_SCROLL_SPEED + 4); //downward x4 speed
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveDownwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_x){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y - 0.1);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(direction_x, 4); //downward x4 speed
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveUpwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_x){
	if (Condition == true){
		SetDirection(GetDirection().x, GetDirection().y + 0.1);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(direction_x, -4); //downward x4 speed
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveLeftwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_y){
	if (Condition == true){
		SetDirection(GetDirection().x - 0.1, GetDirection().y);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(4, direction_y); //downward x4 speed
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveRightwardCorn(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, double direction_y){
	if (Condition == true){
		SetDirection(GetDirection().x + 0.1, GetDirection().y);
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(-4, direction_y); //downward x4 speed
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveRight_ChargeTowardPlayer(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, HD_Object &Player){
	if (Condition == true){
		if (!MoveTimer.checkStart() && (GetCenterPosition().y - Player.GetCenterPosition().y) / (GetCenterPosition().x - Player.GetCenterPosition().x) >= 1 && MovementPhase == 0){
			SetDirection(10, 10);
			MovementPhase = 1;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(4, 0); //downward x4 speed
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveLeft_ChargeTowardPlayer(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype, HD_Object &Player){
	if (Condition == true){
		if (!MoveTimer.checkStart() && (GetCenterPosition().y - Player.GetCenterPosition().y) / (GetCenterPosition().x - Player.GetCenterPosition().x) >= -1 && MovementPhase == 0){
			SetDirection(-10, 10);
			MovementPhase = 1;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(-4, 0); //downward x4 speed
		AttackPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		existence = 2;
		DestroyTimer = HD_Timer();
		DestroyTimer.startTimer(300000);
	}
	return true;
}
bool HD_Object::Preset_MoveRight_ThenMoveDown(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		if (MovementPhase == 0 &&MoveTimer.checkTimer() ){
			SetDirection(0, 0);
			MoveTimer.startTimer(8000);
			MovementPhase = 1;
		}
		else if (MovementPhase == 1 && MoveTimer.checkTimer()){
			SetDirection(0, HD_GAME_SCROLL_SPEED + 2);
			MovementPhase = 2;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(4, 0); //downward x4 speed
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		anchor.x = dstarea.x + GetObjectCenter(false).x;
		anchor.y = dstarea.y + GetObjectCenter(false).y;
		MoveTimer = HD_Timer();
		MoveTimer.startTimer(800);
		MovementPhase = 0;
		existence = 2;
	}
	return true;
}
bool HD_Object::Preset_MoveLeft_ThenMoveDown(SDL_Renderer* renderer, bool Condition, HD_Rect PosRect, int sizetype){
	if (Condition == true){
		if (MovementPhase == 0 && MoveTimer.checkTimer()){
			SetDirection(0, 0);
			MoveTimer.startTimer(800);
			MovementPhase = 1;
		}
		else if (MovementPhase == 1 && MoveTimer.checkTimer()){
			SetDirection(0, HD_GAME_SCROLL_SPEED + 2);
			MovementPhase = 2;
		}
		else {
			SetDirection(GetDirection().x, GetDirection().y);
		}
	}
	else {
		switch (sizetype){
		case 1:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m1.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		case 2:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_l1.png", area, renderer);
			SetAniType(4, 0);
			health = 18;
			break;
		case 3:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s2.png", area, renderer);
			SetAniType(4, 0);
			health = 6;
			break;
		case 5:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 10000;
			break;
		case 6:
			area = { 0, 0, 128, 128 };
			dstarea = { 0, 0, 128, 128 };
			LoadSprite("resource/HD_Enemy_t1.png", area, renderer);
			SetAniType(2, 0);
			health = 20;
			break;
		case 7:
			area = { 0, 0, 96, 64 };
			dstarea = { 0, 0, 96, 64 };
			LoadSprite("resource/HD_Enemy_m2.png", area, renderer);
			SetAniType(4, 0);
			health = 12;
			break;
		default:
			area = { 0, 0, 64, 64 };
			dstarea = { 0, 0, 64, 64 };
			LoadSprite("resource/HD_Enemy_s1.png", area, renderer);
			SetAniType(2, 0);
			health = 4;
			break;
		}
		SetPosition(PosRect.x + 32, PosRect.y + 32);
		SetDirection(-4, 0); //downward x4 speed
		AttackPhase = 0;
		MovementPhase = 0;
		angle = 0;
		DestroyPhase = 0;
		MoveTimer = HD_Timer();
		MoveTimer.startTimer(1500);
		MovementPhase = 0;
		existence = 2;
	}
	return true;
}