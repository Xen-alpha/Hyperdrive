#include "HD_Framework.h"
using namespace std;

// ---------------- HD_SYSTEM Definition -----------------------
HD_SYSTEM::HD_SYSTEM() {
	HD_window = NULL;
	init_Flag = true;
	KeyTimer = HD_Timer();
	KeyTimer.startTimer(500);
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("Failed to initiate HD Engine.\n");
		init_Flag = false;
	}
	HD_window = SDL_CreateWindow("HYPERDRIVE Beta build_180302", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, HD_SCREEN_WIDTH, HD_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (HD_window == NULL){
		cout << "Failed to generate HD Window :" << SDL_GetError() << endl;
		init_Flag = false;
	}
	else {
		SDL_DisplayMode displayMode;
		displayMode = { SDL_PIXELFORMAT_ARGB8888, HD_SCREEN_WIDTH, HD_SCREEN_HEIGHT, 0, 0 };
		SDL_SetWindowDisplayMode(HD_window, &displayMode);
	}
	HD_renderer = SDL_CreateRenderer(HD_window, -1, SDL_RENDERER_ACCELERATED ); // | SDL_RENDERER_PRESENTVSYNC
	if (HD_renderer == NULL){
		cout << "Failed to generate HD Renderer :" << SDL_GetError() << endl;
		init_Flag = false;
	} else { // didn't failed to initialize HD_renderer
		SDL_SetRenderDrawColor(HD_renderer, 0x00, 0x00, 0x04, 0xFF);
	}
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
		printf("Failed to initiate Image library in HD Engine.\n");
		init_Flag = false;
	}
	if (TTF_Init() == -1)
	{
		printf("Failed to initiate TTF library in HD Engine.\n");
		init_Flag = false;
	}
	else {
		font = TTF_OpenFont("resource/PressStart2P.ttf", 20);
		if (font == NULL){
			printf("Failed to initiate font in HD Engine.\n");
			init_Flag = false;
		}
		textColor = { 0xff, 0xff, 0xff, 0xff};
	}
	//Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("Fail to initialize Audio Mixer.\n");
		init_Flag = false;
	}
	HD_gameSys = HD_Game(HD_window);
	Display = HD_Display();
	// the whole Init functions are done
	if (init_Flag){ // no failure
		//Get window surface & Texture and initiate it
		HD_screenSurface = SDL_GetWindowSurface(HD_window);
		SDL_SetSurfaceBlendMode(HD_screenSurface, SDL_BLENDMODE_BLEND);
		HD_format = HD_screenSurface->format;
		//cout << SDL_GetPixelFormatName(HD_format->format) << endl;
		SDL_FillRect(HD_screenSurface, NULL, SDL_MapRGB(HD_format, 0x35, 0x24, 0x55)); // default background
		//Set game surface & Texture to null
		HD_gameSurface = SDL_CreateRGBSurface(0, HD_SCREEN_WIDTH, HD_SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		SDL_SetSurfaceBlendMode(HD_gameSurface, SDL_BLENDMODE_NONE);
		SDL_FillRect(HD_gameSurface, NULL, SDL_MapRGBA(HD_format, 0x35, 0x24, 0x55, 0xff));
		HD_gameTexture = SDL_CreateTexture(HD_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, HD_SCREEN_WIDTH, HD_SCREEN_HEIGHT);
		SDL_SetTextureBlendMode(HD_gameTexture, SDL_BLENDMODE_NONE);
		// Menu Initiation
		TitleMenu = HD_Menu(HD_format, HD_renderer);
		TitleMenu.InitMenu(HD_renderer);
		TitleMenu.AddMenu(HD_renderer, "Start", 3);
		TitleMenu.AddMenu(HD_renderer, "Help", 101);
		TitleMenu.AddMenu(HD_renderer, "Volume: 2", 121);
		TitleMenu.AddMenu(HD_renderer, "Credit", 111);
		TitleMenu.AddMenu(HD_renderer, "Exit", 1000);
		//TitleMenu.SetBackgroundSize(HD_SCREEN_WIDTH / 4, 144);
		PauseMenu = HD_Menu(HD_format, HD_renderer);
		//initializing Object[]
		Object = new HD_Object[HD_NUMBER_OF_OBJECT];
		for (int i = 0; i < HD_NUMBER_OF_OBJECT; i++) {
			Object[i] = HD_Object(HD_format, HD_renderer, 800, 600, 0);
		}
		Object_ingame = new HD_Object[HD_NUMBER_OF_OBJECT];
		for (int i = 0; i < HD_NUMBER_OF_OBJECT; i++) {
			Object_ingame[i] = HD_Object(HD_format, HD_renderer, 800, 600, 0);
		}
		Object_map.reserve(HD_MAX_MAPOBJECT);
		for (int i = 0; i < HD_MAX_MAPOBJECT; i++) {
			Object_map.push_back(HD_Object(HD_format, HD_renderer, 640, 64, 0));
		}
		MapTimer = HD_Timer();
		Bullet = new HD_Object[HD_NUMBER_OF_ENEMY_BULLET];
		for (int i = 0; i < HD_NUMBER_OF_ENEMY_BULLET; i++) {
			Bullet[i] = HD_Object(HD_format, HD_renderer, 64, 64, 0);
		}
		Enemy = new HD_Object[HD_NUMBER_OF_ENEMY];
		for (int i = 0; i < HD_NUMBER_OF_ENEMY; i++) {
			Enemy[i] = HD_Object(HD_format, HD_renderer, 128, 128, 0);
		}
		//initializing Player && boss
		Player = HD_Player(HD_screenSurface->format, HD_renderer, "resource/HD_Player2.png", 64, 64, 0);
		Player.chargeShot.LoadSprite("resource/HD_ChargeShot.png", { 0, 0, 32, 32 }, HD_renderer);
		Boss = HD_Object(HD_screenSurface->format, HD_renderer, 256, 256, 0);
		//Set default Ingame state
		scene = 0;
		quit_Flag = false;
		focusedObject = 0;
		scene_backup = 0;
		//default background
		HD_windowTexture = SDL_CreateTextureFromSurface(HD_renderer, HD_screenSurface);
		SDL_SetTextureBlendMode(HD_windowTexture, SDL_BLENDMODE_BLEND);
		//default BGM Load
		bgm = Mix_LoadMUS("resource/Title.wav");
		EnemySFX = NULL;
		SFXChannel_Enemy = -1;
	}
}
HD_SYSTEM::~HD_SYSTEM() {
}
void HD_SYSTEM::inGameTextureUpdate(){
	SDL_Rect srcRect, dstRect;
	int i;
	unsigned int j;
	// blit "object_ingame" object in HD_gameSurface
	for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
		//cout << "Managing Object_ingame" << i << endl;
		if (Object_ingame[i].spritebase != NULL && Object_ingame[i].existence != 0) {
			srcRect = RectTransform(Object_ingame[i].area);
			dstRect.x = (int)Object_ingame[i].dstarea.x + Display.GetGamePosition().x;
			dstRect.y = (int)Object_ingame[i].dstarea.y + Display.GetGamePosition().y;
			dstRect.w = (int)Object_ingame[i].dstarea.w;
			dstRect.h = (int)Object_ingame[i].dstarea.h;
			HD_BlitSprite(HD_renderer, Object_ingame[i], &srcRect, &dstRect);
			//cout << "Displaying Object_ingame" << i << endl;
		}
	}
	for (i = 0; i < HD_MAX_MAPOBJECT; i++){
		//cout << "Managing Object_map" << i << endl;
		if (Object_map[i].spritebase != NULL && Object_map[i].existence != 0) {
			srcRect = RectTransform(Object_map[i].area);
			dstRect.x = (int)Object_map[i].dstarea.x + Display.GetGamePosition().x;
			dstRect.y = (int)Object_map[i].dstarea.y + Display.GetGamePosition().y;
			dstRect.w = (int)Object_map[i].dstarea.w;
			dstRect.h = (int)Object_map[i].dstarea.h;
			HD_BlitSprite(HD_renderer, Object_map[i], &srcRect, &dstRect);
			//cout << "Displaying Object_map" << i << endl;
		}
	}
	for (i = 0; i < HD_NUMBER_OF_ENEMY; i++){
		//cout << "Managing Object_ingame" << i << endl;
		if (Enemy[i].spritebase != NULL && Enemy[i].existence != 0) {
			srcRect = RectTransform(Enemy[i].area);
			dstRect.x = (int)Enemy[i].dstarea.x + Display.GetGamePosition().x;
			dstRect.y = (int)Enemy[i].dstarea.y + Display.GetGamePosition().y;
			dstRect.w = (int)Enemy[i].dstarea.w;
			dstRect.h = (int)Enemy[i].dstarea.h;
			HD_BlitSprite(HD_renderer, Enemy[i], &srcRect, &dstRect);
			//cout << "Displaying Object_ingame" << i << endl;
		}
	}
	//cout << "Managing Boss" << endl;
	if (Boss.spritebase != NULL && Boss.existence != 0) {
		srcRect = RectTransform(Boss.area);
		dstRect.x = (int)Boss.dstarea.x + Display.GetGamePosition().x;
		dstRect.y = (int)Boss.dstarea.y + Display.GetGamePosition().y;
		dstRect.w = (int)Boss.dstarea.w;
		dstRect.h = (int)Boss.dstarea.h;
		HD_BlitSprite(HD_renderer, Boss, &srcRect, &dstRect);
	}
	//cout << "Managing player" << endl;
	if (Player.spritebase != NULL && Player.existence != 0) {
		srcRect = RectTransform(Player.area);
		dstRect.x = (int)Player.dstarea.x + Display.GetGamePosition().x;
		dstRect.y = (int)Player.dstarea.y + Display.GetGamePosition().y;
		dstRect.w = (int)Player.dstarea.w;
		dstRect.h = (int)Player.dstarea.h;
		HD_BlitSprite(HD_renderer, Player, &srcRect, &dstRect);
	}
	for (i = 0; i < 4; i++){
		if (Player.subweapon[i].existence != 0) {
			srcRect = RectTransform(Player.subweapon[i].area);
			dstRect.x = (int)Player.subweapon[i].dstarea.x + Display.GetGamePosition().x;
			dstRect.y = (int)Player.subweapon[i].dstarea.y + Display.GetGamePosition().y;
			dstRect.w = 32 + Display.GetGameArea().x;
			dstRect.h = 32 + Display.GetGameArea().y;
			HD_BlitSprite(HD_renderer, Player.subweapon[i], &srcRect, &dstRect);
		}
	}
	if (Player.chargeShot.spritebase != NULL && Player.chargeShot.existence != 0) {
		srcRect = RectTransform(Player.chargeShot.area);
		dstRect.x = (int)Player.chargeShot.dstarea.x + Display.GetGamePosition().x;
		dstRect.y = (int)Player.chargeShot.dstarea.y + Display.GetGamePosition().y;
		dstRect.w = (int)Player.chargeShot.dstarea.w;
		dstRect.h = (int)Player.chargeShot.dstarea.h;
		HD_BlitSprite(HD_renderer, Player.chargeShot, &srcRect, &dstRect);
	}
	if (Player.Shield.spritebase != NULL && Player.Shield.existence != 0) {
		srcRect = RectTransform(Player.Shield.area);
		dstRect.x = (int)Player.Shield.dstarea.x + Display.GetGamePosition().x;
		dstRect.y = (int)Player.Shield.dstarea.y + Display.GetGamePosition().y;
		dstRect.w = (int)Player.Shield.dstarea.w;
		dstRect.h = (int)Player.Shield.dstarea.h;
		HD_BlitSprite(HD_renderer, Player.Shield, &srcRect, &dstRect);
	}
	for (i = 0; i < HD_NUMBER_OF_MY_BULLET; i++) {
		if (Player.bullet[i].spritebase != NULL && Player.bullet[i].existence != 0) {
			srcRect = RectTransform(Player.bullet[i].area);
			dstRect.x = (int)Player.bullet[i].dstarea.x + Display.GetGamePosition().x;
			dstRect.y = (int)Player.bullet[i].dstarea.y + Display.GetGamePosition().y;
			dstRect.w = (int)Player.bullet[i].dstarea.w;
			dstRect.h = (int)Player.bullet[i].dstarea.h;
			HD_BlitSprite(HD_renderer, Player.bullet[i], &srcRect, &dstRect);
		}
	}
	//blit Enemy Bullet
	for (i = 0; i < HD_NUMBER_OF_ENEMY_BULLET; i++){
		//cout << "Managing Object_ingame" << i << endl;
		if (Bullet[i].spritebase != NULL && Bullet[i].existence != 0) {
			srcRect = RectTransform(Bullet[i].area);
			dstRect.x = (int)Bullet[i].dstarea.x + Display.GetGamePosition().x;
			dstRect.y = (int)Bullet[i].dstarea.y + Display.GetGamePosition().y;
			dstRect.w = (int)Bullet[i].dstarea.w;
			dstRect.h = (int)Bullet[i].dstarea.h;
			HD_BlitSprite(HD_renderer, Bullet[i], &srcRect, &dstRect);
			//cout << "Displaying Object_ingame" << i << endl;
		}
	}
	//cout << "Displaying Player" << endl;
	//copy screen and make a flip
	SDL_FillRect(HD_gameSurface, NULL, SDL_MapRGB(HD_format, 0x00, 0x00, 0x00));
	SDL_RenderReadPixels(HD_renderer, NULL, SDL_GetWindowPixelFormat(HD_window), HD_gameSurface->pixels, HD_gameSurface->pitch);
	SDL_UpdateTexture(HD_gameTexture, NULL, HD_gameSurface->pixels, HD_gameSurface->pitch);
	SDL_RenderClear(HD_renderer);
	SDL_RenderDrawRect(HD_renderer, NULL);
	SDL_Delay(20);
	srcRect = { (Uint32)Display.GetGamePosition().x, (Uint32)Display.GetGamePosition().y, HD_GAME_WIDTH, HD_GAME_HEIGHT };
	dstRect = { (Uint32)Display.GetGamePosition().x, (Uint32)Display.GetGamePosition().y, (Uint32)Display.GetGameArea().x, (Uint32)Display.GetGameArea().y };
	if (Display.Display_type == 2){
		if (Display.flip) SDL_RenderCopyEx(HD_renderer, HD_gameTexture, &srcRect, &dstRect, Display.angle, NULL, SDL_FLIP_HORIZONTAL);
		else SDL_RenderCopyEx(HD_renderer, HD_gameTexture, &srcRect, &dstRect, Display.angle, NULL, SDL_FLIP_NONE);
	}
	else {
		if (Display.flip) SDL_RenderCopyEx(HD_renderer, HD_gameTexture, &srcRect, &dstRect, 0, NULL, SDL_FLIP_HORIZONTAL);
		else SDL_RenderCopyEx(HD_renderer, HD_gameTexture, &srcRect, &dstRect, 0, NULL, SDL_FLIP_NONE);
	}
	// blit "object" object in HD_screenSurface
	for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
		//cout << "Managing Object" << i << endl;
		if (Object[i].spritebase != NULL && Object[i].existence != 0) {
			srcRect = RectTransform(Object[i].area);
			if (Object[i].attackType == 12 || Object[i].attackType == 5){
				dstRect.x = (int)Object[i].dstarea.x + Display.GetGamePosition().x; // no Display
				dstRect.y = (int)Object[i].dstarea.y + Display.GetGamePosition().y;
			}
			else{
				dstRect.x = (int)Object[i].dstarea.x; // no Display
				dstRect.y = (int)Object[i].dstarea.y;
			}
			dstRect.w = (int)Object[i].dstarea.w;
			dstRect.h = (int)Object[i].dstarea.h;
			HD_BlitSprite(HD_renderer, Object[i], &srcRect, &dstRect);
			//cout << "Displaying Object" << i << endl;
		}
	}
	return;
}
void HD_SYSTEM::exitGame(){
	int i;
	TTF_CloseFont(font);
	if (bgm != NULL){
		if (Mix_PlayingMusic() != 0){
			Mix_HaltMusic();
		}
		Mix_FreeMusic(bgm);
		bgm = NULL;
	}
	//remove all sound effects
	Mix_HaltChannel(-1);
	if (Player.PlayerSFX != NULL) {
		Mix_FreeChunk(Player.PlayerSFX);
		Player.PlayerSFX = NULL;
	}
	if (EnemySFX != NULL){
		Mix_FreeChunk(EnemySFX);
		EnemySFX= NULL;
		SFXChannel_Enemy = -1;
	}
	for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
		if (Object[i].spritebase_Surface != NULL) {
			SDL_FreeSurface(Object[i].spritebase_Surface);
			Object[i].spritebase_Surface = NULL;
			SDL_DestroyTexture(Object[i].spritebase);
			Object[i].spritebase = NULL;
		}
		if (Object[i].font != NULL) {
			TTF_CloseFont(Object[i].font);
			Object[i].font = NULL;
		}
	}
	delete[] Object;
	for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
		if (Object_ingame[i].spritebase_Surface != NULL) {
			SDL_FreeSurface(Object_ingame[i].spritebase_Surface);
			Object_ingame[i].spritebase_Surface = NULL;
			SDL_DestroyTexture(Object_ingame[i].spritebase);
			Object_ingame[i].spritebase = NULL;
		}
		if (Object_ingame[i].font != NULL) {
			TTF_CloseFont(Object_ingame[i].font);
			Object_ingame[i].font = NULL;
		}
	}
	delete[] Object_ingame;
	for (i = 0; i < HD_MAX_MAPOBJECT; i++){
		if (Object_map[i].spritebase_Surface != NULL) {
			SDL_FreeSurface(Object_map[i].spritebase_Surface);
			Object_map[i].spritebase_Surface = NULL;
			SDL_DestroyTexture(Object_map[i].spritebase);
			Object_map[i].spritebase = NULL;
		}
		if (Object_map[i].font != NULL) {
			TTF_CloseFont(Object_map[i].font);
			Object_map[i].font = NULL;
		}
	}
	SDL_FreeSurface(Player.spritebase_Surface);
	Player.spritebase_Surface = NULL;
	SDL_DestroyTexture(Player.spritebase);
	Player.spritebase = NULL;
	SDL_FreeSurface(Player.chargeShot.spritebase_Surface);
	Player.chargeShot.spritebase_Surface = NULL;
	SDL_DestroyTexture(Player.chargeShot.spritebase);
	Player.chargeShot.spritebase = NULL;
	SDL_FreeSurface(Player.Shield.spritebase_Surface);
	Player.Shield.spritebase_Surface = NULL;
	SDL_DestroyTexture(Player.Shield.spritebase);
	Player.Shield.spritebase = NULL;
	if (Player.bullet != NULL){ //delete bullet and its spritebases
		for (i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
			if (Player.bullet[i].spritebase != NULL) {
				SDL_FreeSurface(Player.bullet[i].spritebase_Surface);
				Player.bullet[i].spritebase_Surface = NULL;
				SDL_DestroyTexture(Player.bullet[i].spritebase);
				Player.bullet[i].spritebase = NULL;
			}
		}
		delete[] Player.bullet;
	}
	if (Player.subweapon != NULL){
		for (i = 0; i < 4; i++){
			if (Player.subweapon[i].spritebase != NULL) {
				SDL_FreeSurface(Player.subweapon[i].spritebase_Surface);
				Player.subweapon[i].spritebase_Surface = NULL;
				SDL_DestroyTexture(Player.subweapon[i].spritebase);
				Player.subweapon[i].spritebase = NULL;
			}
		}
		delete[] Player.subweapon;
	}
	for (i = 0; i < HD_NUMBER_OF_ENEMY_BULLET; i++){
		if (Bullet[i].spritebase != NULL) {
			SDL_FreeSurface(Bullet[i].spritebase_Surface);
			Bullet[i].spritebase_Surface = NULL;
			SDL_DestroyTexture(Bullet[i].spritebase);
			Bullet[i].spritebase = NULL;
		}
		if (Bullet[i].font != NULL) {
			TTF_CloseFont(Bullet[i].font);
			Bullet[i].font = NULL;
		}
	}
	delete[] Bullet;
	for (i = 0; i < HD_NUMBER_OF_ENEMY; i++){
		if (Enemy[i].spritebase != NULL) {
			SDL_FreeSurface(Enemy[i].spritebase_Surface);
			Enemy[i].spritebase_Surface = NULL;
			SDL_DestroyTexture(Enemy[i].spritebase);
			Enemy[i].spritebase = NULL;
		}
		if (Enemy[i].font != NULL) {
			TTF_CloseFont(Enemy[i].font);
			Enemy[i].font = NULL;
		}
	}
	delete[] Enemy;
	SDL_FreeSurface(HD_screenSurface);
	HD_screenSurface = NULL;
	unsigned int j;
	for (j = 0; j < TitleMenu.GetTotalItem(); j++){
		if (TitleMenu.Menutext[j].spritebase != NULL) {
			SDL_FreeSurface(TitleMenu.Menutext[j].spritebase_Surface);
			TitleMenu.Menutext[j].spritebase_Surface = NULL;
			SDL_DestroyTexture(TitleMenu.Menutext[j].spritebase);
			TitleMenu.Menutext[j].spritebase = NULL;
		}
		if (TitleMenu.Menutext[j].font != NULL) {
			TTF_CloseFont(TitleMenu.Menutext[j].font);
			TitleMenu.Menutext[j].font = NULL;
		}
	}
	if (TitleMenu.initiated != false) {
		if (TitleMenu.Cursor.spritebase != NULL) { 
			SDL_FreeSurface(TitleMenu.Cursor.spritebase_Surface);
			TitleMenu.Cursor.spritebase_Surface = NULL;
			SDL_DestroyTexture(TitleMenu.Cursor.spritebase);
			TitleMenu.Cursor.spritebase = NULL;
		}
		if (TitleMenu.Background.spritebase != NULL) { 
			SDL_FreeSurface(TitleMenu.Background.spritebase_Surface);
			TitleMenu.Background.spritebase_Surface = NULL;
			SDL_DestroyTexture(TitleMenu.Background.spritebase);
			TitleMenu.Background.spritebase = NULL;
		}
		TitleMenu.initiated = false;
	}
	for (j = 0; j < PauseMenu.GetTotalItem(); j++){
		if (PauseMenu.Menutext[j].spritebase != NULL) {
			SDL_FreeSurface(PauseMenu.Menutext[j].spritebase_Surface);
			PauseMenu.Menutext[j].spritebase_Surface = NULL;
			SDL_DestroyTexture(PauseMenu.Menutext[j].spritebase);
			PauseMenu.Menutext[j].spritebase = NULL;
		}
		if (PauseMenu.Menutext[j].font != NULL) {
			TTF_CloseFont(PauseMenu.Menutext[j].font);
			PauseMenu.Menutext[j].font = NULL;
		}
	}
	if (PauseMenu.initiated != false) {
		if (PauseMenu.Cursor.spritebase != NULL) {
			SDL_FreeSurface(PauseMenu.Cursor.spritebase_Surface);
			PauseMenu.Cursor.spritebase_Surface = NULL;
			SDL_DestroyTexture(PauseMenu.Cursor.spritebase);
			PauseMenu.Cursor.spritebase = NULL;
		}
		if (PauseMenu.Background.spritebase != NULL) {
			SDL_FreeSurface(PauseMenu.Background.spritebase_Surface);
			PauseMenu.Background.spritebase_Surface = NULL;
			SDL_DestroyTexture(PauseMenu.Background.spritebase);
			PauseMenu.Background.spritebase = NULL;
		}
		PauseMenu.initiated = false;
	}
	if (HD_gameSurface != NULL){
		SDL_FreeSurface(HD_gameSurface);
		HD_gameSurface = NULL;
	}
	if (HD_gameTexture != NULL){
		SDL_DestroyTexture(HD_gameTexture);
		HD_gameTexture = NULL;
	}
	SDL_DestroyTexture(HD_windowTexture);
	HD_windowTexture = NULL;
	SDL_DestroyRenderer(HD_renderer);
	HD_renderer = NULL;
	SDL_DestroyWindow(HD_window);
	HD_window = NULL;
	return;
}
void HD_SYSTEM::GameOver(){
	int i;
	Player.SetObjectAttribute(HD_renderer,false, "resource/explosion.png", { 0, 0, 64, 64 }, Player.dstarea, 3, 0, 7, 6);
	for (i = 0; i< 4; i++){
		Player.subweapon[i].existence = 0;
	}
	for (i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
		Player.bullet[i].existence = 0;
	}
	Object[3].existence = 0;
	Object[9].LoadText(HD_renderer,"Game Over");
	Object[9].dstarea = Object[9].area;
	Object[9].SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
	Object[9].SetAniType(0, -1);
	Object[9].existence = 2;
	Player.Shield.existence = 0;
	scene = 8;
	return;
}
void HD_SYSTEM::HD_BlitSprite(SDL_Renderer *renderer, HD_Object &src, SDL_Rect * srcRect, SDL_Rect * dstRect){
	// need to add some task related to the variable "existence"
	SDL_Rect tempRect = *srcRect;
	SDL_Rect tempRect2 = *dstRect;
	SDL_Point tempanchor = PointTransform(src.GetCenterPosition());
	double tempangle = 0;
	if (src.existence != 0){
		switch (src.GetRenderType()){
		case -1: // no animation
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_NONE);
			break;
		case 0: // animation with horizontally sucessive sprite
			if (src.currentFrame < src.totalAniFrame)
				tempRect.x += (src.currentFrame++ * src.area.w);
			else
				src.currentFrame = 0;
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_NONE);
			break;
		case 2: // stage scroll
			if (src.currentFrame < src.totalAniFrame){
				tempRect.y = 640 - (src.currentFrame * HD_GAME_SCROLL_SPEED);
				tempRect.w = 640;
				tempRect.h = 640;
				tempRect.x = 0;
				src.currentFrame++;
			}
			else{
				src.currentFrame = 0;
				tempRect.y = 640;
				tempRect.w = 640;
				tempRect.h = 640;
			}
			tempRect.x = 0;
			//Surface -> Texture
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_NONE);
			break;
		case 3: // stage scroll
			if (src.currentFrame < src.totalAniFrame / (int)Display.AreaDev){
				tempRect.y = 640 - (src.currentFrame * (int)Display.AreaDev);
				tempRect.w = 640;
				tempRect.h = 640;
				tempRect.x = 0;
				src.currentFrame++;
			}
			else{
				src.currentFrame = 0;
				tempRect.y = 640;
				tempRect.w = 640;
				tempRect.h = 640;
			}
			tempRect.x = 0;
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_NONE);
			break;
		case 7: // only for Destruction animation with horizontally sucessive sprite
			if (src.currentFrame < src.totalAniFrame)
				tempRect.x += (src.currentFrame++ * 64);
			else
				src.existence = 0;
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_NONE);
			break;
		case 8: // Background <- obsolete, will be eliminated in the future
			tempRect2.x = HD_GAME_WIDTH;
			tempRect2.y = 0;
			tempRect2.w = 160;
			tempRect2.h = 480;
			//Surface -> Texture
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_NONE);
			break;
		case 10: // Player subweapon with animation
			if (src.currentFrame < src.totalAniFrame)
				tempRect.x += (src.currentFrame++ * src.area.w);
			else
				src.currentFrame = 0;
			tempRect2.w = 32;
			tempRect2.h = 32;
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_VERTICAL);
			break;
		case 11: // Player subweapon
			tempRect2.w = 32;
			tempRect2.h = 32;
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_VERTICAL);
			break;
		case 12: // shoot animation with no-animation object
			//Texture -> Renderer
			SDL_RenderCopyEx(HD_renderer, src.spritebase, &tempRect, &tempRect2, src.angle, NULL, SDL_FLIP_NONE);
			src.area.y = 0; // go back to normal sprite
			src.SetAniType(0, -1);
			break;
		default:
			break;
		}
	}
	return;
}
void HD_SYSTEM::HD_ManageKeyboard(){
	int i, a;
	double keyangle = 0;
// Alert! SDL_PollEvent must be called before using this function!
	const Uint8* KeyState = SDL_GetKeyboardState(NULL);
	bool untouched = true;
	//reset player direction
	Player.area.y = 0; // default player sprite
	Player.SetDirection(0, 0); // reset Player's direction
	HD_Point PlayerCenterPosition = Player.GetCenterPosition();
	//bool PlayerDirectionDetermined = false;
	Player.spacePressed = false;
	Player.ctrlPressed = false;
	if (KeyState[SDL_SCANCODE_UP]){
		untouched = false;
		switch (scene){
		//case 0 is title init
		case 1: // title
			// selecting feature
			if (KeyTimer.checkTimer()){
				TitleMenu.SetCursorPosition(TitleMenu.GetCursorPosition() - 1);
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 2: // player type setting scene
			// do nothing - choosing player type using left/right key
			break;
		case 4: // stage1 intro
		case 5: // stage1 field
		case 6: // stage1 boss
		case 10: // stage2 field
		case 11: // stage2 boss
			//move player's aircraft in game surface
			if (Display.Display_type == 2){
				if (PlayerCenterPosition.y > 0 && PlayerCenterPosition.y < 500 && PlayerCenterPosition.x > 0 && PlayerCenterPosition.x < HD_GAME_WIDTH ){
					if (KeyState[SDL_SCANCODE_LEFT]) {
						keyangle = 135;
					}
					else if (KeyState[SDL_SCANCODE_RIGHT]) {
						keyangle = 45;
					}
					else {
						keyangle = 90;
					}
					if (Display.flip == false){
						Player.SetDirection(HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
					else {
						Player.SetDirection(-HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
				}
			} else {
				if (Player.GetCenterPosition().y > 0 && Player.GetDirection().y == 0){
					Player.SetDirection(Player.GetDirection().x, Player.GetDirection().y - HD_PLAYER_MOVE_SPEED);
				}
			}
			break;
		case 7: // stage1 result
		case 8: // game over
		case 12: // stage2 result
			//do nothing
			break;
		case 1003: // pausemenu
			// selecting feature
			if (KeyTimer.checkTimer()){
				PauseMenu.SetCursorPosition(PauseMenu.GetCursorPosition() - 1);
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		default:
			break;
			// do nothing
		}
	}
	if (KeyState[SDL_SCANCODE_DOWN]){
		untouched = false;
		switch (scene) {
		//case 0 is title init
		case 1: // title
			// selecting feature
			if (KeyTimer.checkTimer()){
				TitleMenu.SetCursorPosition(TitleMenu.GetCursorPosition() + 1);
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 2: // player type setting scene
			// do nothing - choosing player type using left/right key
			break;
		case 4: // stage1 intro
		case 5: // stage1 field
		case 6: // stage1 boss
		case 10: // stage2 field
		case 11: // stage2 boss
			//move player's aircraft in game surface
			if (Display.Display_type == 2){
				if (PlayerCenterPosition.y > 0 && PlayerCenterPosition.y < 500 && PlayerCenterPosition.x > 0 && PlayerCenterPosition.x < HD_GAME_WIDTH){
					if (KeyState[SDL_SCANCODE_LEFT]) {
						keyangle = -135;
					}
					else if (KeyState[SDL_SCANCODE_RIGHT]) {
						keyangle = -45;
					}
					else {
						keyangle = -90;
					}
					if (Display.flip == false){
						Player.SetDirection(HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
					else {
						Player.SetDirection(-HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
				}
			}
			else{
				if (Player.GetCenterPosition().y < 460 && Player.GetDirection().y == 0){
					Player.SetDirection(Player.GetDirection().x, Player.GetDirection().y + HD_PLAYER_MOVE_SPEED);
				}
			}
			break;
		case 7: // stage1 result
		case 12: // stage2 result
		case 13:
			//do nothing
			break;
		case 1003: // pausemenu
			// selecting feature
			if (KeyTimer.checkTimer()){
				PauseMenu.SetCursorPosition(PauseMenu.GetCursorPosition() + 1);
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		default:
			break;
			// do nothing
		}
	}
	if (KeyState[SDL_SCANCODE_LEFT]){
		untouched = false;
		switch (scene){
			//case 0 is title init
		case 1: // title
			// do nothing
			break;
		case 2: // player type setting scene
			// choosing player type using left/right key
			/*
			if (Player.attackType != -1) {
				Player.attackType = -1;
			} else {
				Player.attackType = -2;
			}
			*/
			break;
		case 4: // stage1 intro
		case 5: // stage1 field
		case 6: // stage1 boss
		case 10:
		case 11:
			if (Display.Display_type == 2){
				if (PlayerCenterPosition.y > 0 && PlayerCenterPosition.y < 500 && PlayerCenterPosition.x > 0 && PlayerCenterPosition.x < HD_GAME_WIDTH){
					if (KeyState[SDL_SCANCODE_UP]) {
						keyangle = 135;
					}
					else if (KeyState[SDL_SCANCODE_DOWN]) {
						keyangle = -135;
					}
					else {
						keyangle = 180;
					}
					if (Display.flip == false){
						Player.SetDirection(HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
					else {
						Player.SetDirection(-HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
				}
			} else{
				// move player's aircraft in game surface
				if (Display.flip == false && Player.GetCenterPosition().x > 0 && Player.GetDirection().x == 0){
					Player.SetDirection(Player.GetDirection().x - HD_PLAYER_MOVE_SPEED, Player.GetDirection().y);
					Player.area.y = 64;
				}
				else if (Display.flip == true && Player.GetCenterPosition().x < HD_GAME_WIDTH && Player.GetDirection().x == 0){
					Player.SetDirection(Player.GetDirection().x + HD_PLAYER_MOVE_SPEED, Player.GetDirection().y);
					Player.area.y = 128;
				}
			}
			break;
		case 7: // stage1 result
			// do nothing
			break;
		default:
			// do nothing
			break;
		}
	}
	if (KeyState[SDL_SCANCODE_RIGHT]){
		untouched = false;
		switch (scene){
		//case 0 is title init
		case 1: // title
			// do nothing - choosing button using up/down key
			break;
		case 2: // player type setting scene
			// choosing player type using left/right key
			/*
			if (Player.attackType != -2) {
				Player.attackType = -2;
			} else {
				Player.attackType = -1;
			}
			*/
			break;
		case 4: // stage1 intro
		case 5: // stage1 field
		case 6: // stage1 boss
		case 10:
		case 11:
			if (Display.Display_type == 2){
				if (PlayerCenterPosition.y > 0 && PlayerCenterPosition.y < 500 && PlayerCenterPosition.x > 0 && PlayerCenterPosition.x < HD_GAME_WIDTH){
					if (KeyState[SDL_SCANCODE_UP]) {
						keyangle = 45;
					}
					else if (KeyState[SDL_SCANCODE_DOWN]) {
						keyangle = -45;
					}
					else {
						keyangle = 0;
					}
					if (Display.flip == false){
						Player.SetDirection(HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
					else {
						Player.SetDirection(-HD_PLAYER_MOVE_SPEED * cos((keyangle + Display.angle)* M_PI / 180), -HD_PLAYER_MOVE_SPEED * sin((keyangle + Display.angle)* M_PI / 180));
					}
				}
			}
			else {
				// move player's aircraft in game surface
				if (Display.flip == false && Player.GetCenterPosition().x < HD_GAME_WIDTH && Player.GetDirection().x == 0){
					Player.SetDirection(Player.GetDirection().x + HD_PLAYER_MOVE_SPEED, Player.GetDirection().y);
					Player.area.y = 128;
				}
				else if (Display.flip == true && Player.GetCenterPosition().x >0 && Player.GetDirection().x == 0){
					Player.SetDirection(Player.GetDirection().x - HD_PLAYER_MOVE_SPEED, Player.GetDirection().y);
					Player.area.y = 64;
				}
			}
			break;
		case 7: // stage1 result
			// do nothing
			break;
		default:
			// do nothing
			break;
		}
	}
	if (KeyState[SDL_SCANCODE_SPACE] || KeyState[SDL_SCANCODE_C]){
		untouched = false;
		Player.spacePressed = true;
		switch (scene){
		//case 0 is title init
		case 1: // title scene
			// selecting feature
			if (KeyTimer.checkTimer()){
				switch (TitleMenu.GetMenuReference()){
				case 3:
					//start game
					//before do that, initialize game surface(move to scene 3 first)
					scene = 3;
					SDL_Delay(50);
					break;
				case 101:
					scene = 101;
					break;
				case 111:
					scene = 111;
					break;
				case 121:
					scene = 121;
					break;
				case 1000:
					//exit
					scene = 1000;
					break;
				default:
					break;
				}
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 2: // player type setting scene
			// determine player type and set player's initial position
			if (KeyTimer.checkTimer()){
				SDL_Delay(50);
				scene = 4;
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 4: // stage1 intro
		case 5: // stage1 field
		case 6: // stage1 boss
		case 10: // stage 2 field
		case 11: // stage 2 boss
			// charge shot
			if (Player.chargeGauge < HD_MAX_CHARGE_GAUGE)
				Player.chargeGauge++;
			break;
		case 7: // stage1 clear
			if (KeyTimer.checkTimer()){
				focusedObject = 0;
				scene_backup = 9;
				scene = 9;
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 8: // game over
			// scene skip
			if (KeyTimer.checkTimer()){
				focusedObject = 0;
				scene_backup = 0;
				scene = 0;
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 12: // stage2 clear
			if (KeyTimer.checkTimer()){
				focusedObject = 0;
				scene_backup = 0;
				scene = 0;
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 99:
			if (KeyTimer.checkTimer()){
				Object[1].existence = 2;
				Object[6].existence = 0;
				TitleMenu.Display = true;
				scene = 1;
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		case 1003: // pausemenu
			// choosing button in pause menu
			if (KeyTimer.checkTimer()){
				switch (PauseMenu.GetMenuReference()) {
					//stage
				case 0:
				case 2:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 10:
				case 11:
				case 12:
					if (Mix_PlayingMusic() != 0 && Mix_PausedMusic() != 0){
						Mix_ResumeMusic();
					}
					if (PauseMenu.GetCursorPosition() == 0){
						scene = PauseMenu.GetMenuReference();
						Player.attackTimer.resumeTimer();
						HD_gameSys.spawnTimer.resumeTimer();
						Player.CrashTimer.resumeTimer();
						for (i = 0; i < 4; i++){
							Player.subweapon[i].attackTimer.resumeTimer();
							Player.subweapon[i].DestroyTimer.resumeTimer();
						}
						Object[3].existence = 2;
						Object[5].existence = 2;
					}
					scene = PauseMenu.GetMenuReference();
					PauseMenu.Display = false;
					break;
				case 1000:
					quit_Flag = true;
					break;
				default:
					break; // do nothing
				}
				KeyTimer = HD_Timer();
				KeyTimer.startTimer(500);
			}
			break;
		default:
			// do nothing
			break;
		}
	}
	if (KeyState[SDL_SCANCODE_LCTRL] || KeyState[SDL_SCANCODE_Z]){
		untouched = false;
		Player.ctrlPressed = true;
		switch (scene){
		case 5:
		case 6:
		case 10:
		case 11:
			Player.SetDirection(Player.GetDirection().x/2, Player.GetDirection().y/2);
			break;
		default:
			break;
		}
	}
	if (KeyState[SDL_SCANCODE_X]){
		untouched = false;
		if (KeyTimer.checkTimer()){
			switch (scene){
			case 5:
			case 6:
			case 10:
			case 11:
				if (HD_gameSys.rank > 0) {
					Player.dstarea.x = 2 * (Display.GetGameCenter().x - HD_INGAME_DISPLAY_CORRECTION_X) - Player.dstarea.x - Player.area.w;
					if (Display.flip == true){
						Display.flip = false;
					}
					else {
						Display.flip = true;
					}
					HD_gameSys.rank--;
					Player.CrashTimer = HD_Timer();
					Player.CrashTimer.startTimer(4000);
					Player.Shield.existence = 2;
				}
				break;
			default:
				break;
			}
			KeyTimer = HD_Timer();
			KeyTimer.startTimer(500);
		}
	}
	if (KeyState[SDL_SCANCODE_ESCAPE]){ // may be changed: frome switch to if - else if - else
		untouched = false;
		if (KeyTimer.checkTimer()){
			switch (scene){
				// case 0 is title init
			case 1: // title
				// just quit the game
				quit_Flag = true;
				break;
			case 2: // player type setting scene
			case 4: // stage 1 intro
			case 5: // stage 1 field
			case 6: // stage 1 boss
			case 7: // stage 1 clear
			case 8: // stage 1 result
			case 10: // stage 2 field
			case 11: // stage 2 boss
			case 12: // stage 2 clear
				// go to pause Menu
				if (Mix_PlayingMusic() != 0 && Mix_PausedMusic() == 0){
					Mix_PauseMusic();
				}
				//Pause Game state
				Player.attackTimer.pauseTimer();
				Player.CrashTimer.pauseTimer();
				HD_gameSys.spawnTimer.pauseTimer();
				for (i = 0; i < 4; i++){
					Player.subweapon[i].attackTimer.pauseTimer();
					Player.subweapon[i].DestroyTimer.pauseTimer();
				}
				//open pause menu
				PauseMenu.InitMenu(HD_renderer); // reset
				PauseMenu.AddMenu(HD_renderer, "Resume", scene);
				PauseMenu.AddMenu(HD_renderer, "Go to title", 0);
				PauseMenu.AddMenu(HD_renderer, "Exit", 1000);
				PauseMenu.SetCursorPosition(0);
				PauseMenu.SetBackgroundSize(HD_SCREEN_WIDTH / 4, 192);
				PauseMenu.Display = true;
				Object[3].existence = 0;
				Object[5].existence = 0;
				scene_backup = scene;
				scene = 1003;
				break;
			case 99:
				TitleMenu.Display = true;
				Object[1].existence = 2;
				Object[6].existence = 0;
				scene = 1;
				break;
			case 1003: // pausemenu - do nothing
				break;
			default:
				// do nothing
				break;
			}
			KeyTimer = HD_Timer();
			KeyTimer.startTimer(500);
		}
	}
	return;
}
bool HD_SYSTEM::SetMapObject_sub(int i) {
	// Do not directly use this function!
	// This function will determined the background sprite pattern
	bool bg_pattern_determined = false;
	switch (HD_gameSys.pattern_background[i]){
	case 0:
		Object_map[i].LoadSprite("resource/default2.png", {0,0,640, 64}, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 1:
		Object_map[i].LoadSprite("resource/pattern1.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 2:
		Object_map[i].LoadSprite("resource/pattern1_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 3:
		Object_map[i].LoadSprite("resource/pattern1_3.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 4:
		Object_map[i].LoadSprite("resource/pattern1A.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 5:
		Object_map[i].LoadSprite("resource/pattern1B.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 6:
		Object_map[i].LoadSprite("resource/pattern1A_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 7:
		Object_map[i].LoadSprite("resource/pattern1B_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 9:
		Object_map[i].LoadSprite("resource/pattern2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 10:
		Object_map[i].LoadSprite("resource/pattern2_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 11:
		Object_map[i].LoadSprite("resource/pattern2_1.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 12:
		Object_map[i].LoadSprite("resource/pattern2_1_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 13:
		Object_map[i].LoadSprite("resource/pattern2_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 14:
		Object_map[i].LoadSprite("resource/pattern2_3.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 15:
		Object_map[i].LoadSprite("resource/pattern2_4.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 16:
		Object_map[i].LoadSprite("resource/pattern2_5.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 17:
		Object_map[i].LoadSprite("resource/pattern2_5_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 18:
		Object_map[i].LoadSprite("resource/pattern2_6.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 19:
		Object_map[i].LoadSprite("resource/pattern2_7.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 20:
		Object_map[i].LoadSprite("resource/pattern2_7_a.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 21:
		Object_map[i].LoadSprite("resource/pattern2_8.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 22:
		Object_map[i].LoadSprite("resource/pattern2_8_a.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 23:
		Object_map[i].LoadSprite("resource/pattern2_9.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 24:
		Object_map[i].LoadSprite("resource/pattern2_9_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 25:
		Object_map[i].LoadSprite("resource/pattern2_10.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 26:
		Object_map[i].LoadSprite("resource/pattern2_10_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 27:
		Object_map[i].LoadSprite("resource/pattern2_11.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 28:
		Object_map[i].LoadSprite("resource/pattern2_11_d.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 29:
		Object_map[i].LoadSprite("resource/pattern2_11_u.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 30:
		Object_map[i].LoadSprite("resource/pattern_3_1.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 31:
		Object_map[i].LoadSprite("resource/pattern_3_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 32:
		Object_map[i].LoadSprite("resource/pattern_3_3.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 33:
		Object_map[i].LoadSprite("resource/pattern_4.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 34:
		Object_map[i].LoadSprite("resource/pattern_4_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 35:
		Object_map[i].LoadSprite("resource/pattern_4_3.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 36:
		Object_map[i].LoadSprite("resource/pattern_4_4.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 37:
		Object_map[i].LoadSprite("resource/pattern_4_5.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 38:
		Object_map[i].LoadSprite("resource/pattern_4_6.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 39:
		Object_map[i].LoadSprite("resource/pattern_4_7.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 40:
		Object_map[i].LoadSprite("resource/pattern_4_8.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 41:
		Object_map[i].LoadSprite("resource/pattern_4_9.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 42:
		Object_map[i].LoadSprite("resource/pattern_4_10.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 49:
		Object_map[i].LoadSprite("resource/pattern_5.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 50:
		Object_map[i].LoadSprite("resource/pattern_5_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 51:
		Object_map[i].LoadSprite("resource/pattern_5_1.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 52:
		Object_map[i].LoadSprite("resource/pattern_5_1_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 53:
		Object_map[i].LoadSprite("resource/pattern_5_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 54:
		Object_map[i].LoadSprite("resource/pattern_5_2_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 55:
		Object_map[i].LoadSprite("resource/pattern_5_3.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 56:
		Object_map[i].LoadSprite("resource/pattern_5_3_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 60:
		Object_map[i].LoadSprite("resource/pattern1C.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 61:
		Object_map[i].LoadSprite("resource/pattern1C_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 62:
		Object_map[i].LoadSprite("resource/pattern_3_4.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 63:
		Object_map[i].LoadSprite("resource/pattern_3_5.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;		
	case 69:
		Object_map[i].LoadSprite("resource/pattern_0.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 70:
		Object_map[i].LoadSprite("resource/pattern_6.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 71:
		Object_map[i].LoadSprite("resource/pattern_6_1.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 72:
		Object_map[i].LoadSprite("resource/pattern_6_1_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 73:
		Object_map[i].LoadSprite("resource/pattern_6_2.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 74:
		Object_map[i].LoadSprite("resource/pattern_6_2_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 75:
		Object_map[i].LoadSprite("resource/pattern_6_3.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 76:
		Object_map[i].LoadSprite("resource/pattern_6_3_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 77:
		Object_map[i].LoadSprite("resource/pattern_6_4.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 78:
		Object_map[i].LoadSprite("resource/pattern_6_4_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 79:
		Object_map[i].LoadSprite("resource/pattern_6_5.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 80:
		Object_map[i].LoadSprite("resource/pattern_6_5_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 81:
		Object_map[i].LoadSprite("resource/pattern_6_6.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 82:
		Object_map[i].LoadSprite("resource/pattern_6_6_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 83:
		Object_map[i].LoadSprite("resource/pattern_6_7.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 84:
		Object_map[i].LoadSprite("resource/pattern_6_7_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 85:
		Object_map[i].LoadSprite("resource/pattern_6_8.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 86:
		Object_map[i].LoadSprite("resource/pattern_6_8_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 87:
		Object_map[i].LoadSprite("resource/pattern_6_9.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 88:
		Object_map[i].LoadSprite("resource/pattern_6_9_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 89:
		Object_map[i].LoadSprite("resource/pattern_6_10.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	case 90:
		Object_map[i].LoadSprite("resource/pattern_6_10_i.png", { 0, 0, 640, 64 }, HD_renderer);
		bg_pattern_determined = true;
		break;
	default:
		break;
	}
	return bg_pattern_determined;
}
void HD_SYSTEM::SetMapObject(){
	for (int i = 0; i < HD_MAX_MAPOBJECT; i++){
		Object_map[i].existence = 0;
		Object_map[i].SetCenterPosition(-320, -320);
	}
	for (int i = 0; i < HD_MAX_MAPOBJECT; i++){
		if (i < HD_gameSys.stageLength){
			SetMapObject_sub(i);
			Object_map[i].area = { 0, 0, 640, 64 };
			Object_map[i].dstarea = { 0, -i * 64, 640, 64 };
			Object_map[i].existence = 2;
			Object_map[i].SetDirection(0, HD_MAP_SCROLL_SPEED);
			Object_map[i].AttackPhase = 0;
			Object_map[i].SetAniType(0, -1);
			Object_map[i].attackType = 1;
			Object_map[i].angle = 0;
			Object_map[i].anchor.x = Object_map[i].dstarea.x + Object_map[i].GetObjectCenter(false).x;
			Object_map[i].anchor.y = Object_map[i].dstarea.y + Object_map[i].GetObjectCenter(false).y;
		}
	}
	HD_gameSys.spawnTimer = HD_Timer();
	HD_gameSys.spawnTimer.startTimer(100);
	HD_gameSys.currentPattern = 0;
	return;
}
void HD_SYSTEM::spawnEnemy(){
	int i;
	int a = 0;
	bool bg_pattern_determined = false;
	//spawn ground pattern
	if (Boss.existence == 0 && HD_gameSys.currentPattern < HD_MAX_MAPOBJECT){ // field pattern - no spawning mob in boss battle
		//field spawn
		for (i = 0; i < 10; i++)
		{
			if (HD_gameSys.pattern_ground[HD_gameSys.currentPattern][i] == 1000){
				Boss.area = { 0, 0, 256, 256 };
				Boss.dstarea.w = 256;
				Boss.dstarea.h = 256;
				Boss.LoadSprite("resource/HD_Boss1.png", Boss.area, HD_renderer);
				Boss.existence = 2;
				Boss.AttackPhase = 0;
				Boss.SetAniType(2, 0);
				Boss.SetDirection(0, -1);
				Boss.SetCenterPosition(Display.GetGameCenter().x - 80, -270);
				Boss.SetDirection(0, 8);
				Boss.MovementPhase = 0;
				Boss.attackType = 1000;
				Boss.health = HD_BOSS1_HEALTH;
				Boss.attackTimer = HD_Timer();
				Boss.attackTimer.startTimer(900);
				Boss.DestroyTimer = HD_Timer();
				Boss.DestroyTimer.startTimer(600000);
				bossTimer = HD_Timer();
				bossTimer.startTimer(4000);
				Object[12].area = { 0, 0, 64, 64 };
				Object[12].dstarea.w = 560;
				Object[12].dstarea.h = 16;
				Object[12].LoadSprite("resource/default.png", Object[12].area, HD_renderer);
				Object[12].existence = 2;
				Object[12].SetCenterPosition(HD_GAME_WIDTH / 2, 40);
				Object[12].SetDirection(0, 0);
				Object[12].AttackPhase = 0;
				Object[12].SetAniType(0, -1);
				Object[12].attackType = 1010; // attackType = 1010 -> health bar for boss 1
				Object[12].health = -1;
				Object[12].angle = 0;
				boss_phase = 0;
				// random spawn timer
				HD_gameSys.spawnTimer = HD_Timer();
				HD_gameSys.spawnTimer.startTimer(5000);
				scene++; //move to boss scene
			}
			else if (HD_gameSys.pattern_ground[HD_gameSys.currentPattern][i] == 1001){
				Boss.area = { 0, 0, 256, 160 };
				Boss.dstarea.w = 256;
				Boss.dstarea.h = 160;
				Boss.LoadSprite("resource/HD_Boss2.png", Boss.area, HD_renderer);
				Boss.existence = 2;
				Boss.AttackPhase = 0;
				Boss.SetAniType(2, 0);
				Boss.SetDirection(0, -1);
				Boss.SetCenterPosition(Display.GetGameCenter().x - 80, -270);
				Boss.SetDirection(0, 8);
				Boss.MovementPhase = 0;
				Boss.attackType = 1001;
				Boss.health = HD_BOSS2_HEALTH;
				Boss.attackTimer = HD_Timer();
				Boss.attackTimer.startTimer(900);
				Boss.DestroyTimer = HD_Timer();
				Boss.DestroyTimer.startTimer(600000);
				bossTimer = HD_Timer();
				bossTimer.startTimer(4000);
				Object[12].area = { 0, 0, 64, 64 };
				Object[12].dstarea.w = 560;
				Object[12].dstarea.h = 16;
				Object[12].LoadSprite("resource/default.png", Object[12].area, HD_renderer);
				Object[12].existence = 2;
				Object[12].SetCenterPosition(HD_GAME_WIDTH / 2, 40);
				Object[12].SetDirection(0, 0);
				Object[12].AttackPhase = 0;
				Object[12].SetAniType(0, -1);
				Object[12].attackType = 1011; // attackType = 1011 -> health bar for boss 2
				Object[12].health = -1;
				Object[12].angle = 0;
				boss_phase = 0;
				// random spawn timer
				HD_gameSys.spawnTimer = HD_Timer();
				HD_gameSys.spawnTimer.startTimer(5000);
				scene++; //move to boss scene
			}
			else if (HD_gameSys.pattern_ground[HD_gameSys.currentPattern][i] == 16){
				Object[10].LoadText(HD_renderer,"Stage 1");
				Object[10].SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
				Object[10].DestroyTimer.startTimer(3000);
				Object[10].existence = 2;
			}
			else if (HD_gameSys.pattern_ground[HD_gameSys.currentPattern][i] == 17){
				Object[10].LoadText(HD_renderer,"Stage 2");
				Object[10].SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
				Object[10].DestroyTimer.startTimer(3000);
				Object[10].existence = 2;
			}
			else if (HD_gameSys.pattern_ground[HD_gameSys.currentPattern][i] == 999){
				//boss alert sound
				if (Mix_PlayingMusic() != 0){
					Mix_FadeOutMusic(2500);
				}
			}
			else if (HD_gameSys.pattern_ground[HD_gameSys.currentPattern][i] == 998) {
				if (bgm != NULL){
					Mix_FreeMusic(bgm);
				}
				SDL_Delay(20);
				bgm = Mix_LoadMUS("resource/Boss.wav");
				Mix_PlayMusic(bgm, 0);
				Object[10].LoadText(HD_renderer, "WARNING!!!");
				Object[10].SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
				Object[10].DestroyTimer.startTimer(3000);
				Object[10].existence = 2;
			}
			else{
				spawnEnemy_Process(HD_gameSys.pattern_ground[HD_gameSys.currentPattern][i], i);
			}
		}
		HD_gameSys.currentPattern++;
	}
	return;
}
bool HD_SYSTEM::spawnEnemy_Process(int trigger, int index){
	int j, k;
	int a = 0;
	switch (trigger){
	case 11:
		for (j = 0; j < HD_NUMBER_OF_ENEMY; j++){
			if (Enemy[j].existence == 0) {
				//cout << "Ground enemy 11 spawned" << endl;
				Enemy[j].area = { 0, 0, 64, 64 };
				Enemy[j].dstarea = { 0, 0, 64, 64 };
				Enemy[j].LoadSprite("resource/HD_GroundEnemy.png", Enemy[j].area, HD_renderer);
				Enemy[j].existence = 2;
				Enemy[j].SetPosition(index * 64, Object_map[HD_gameSys.currentPattern].dstarea.y);
				Enemy[j].SetDirection(0, HD_MAP_SCROLL_SPEED);
				Enemy[j].AttackPhase = 0;
				Enemy[j].SetAniType(2, 0);
				Enemy[j].attackType = 11;
				Enemy[j].health = 10;
				Enemy[j].angle = 0;
				Enemy[j].DestroyTimer = HD_Timer();
				Enemy[j].DestroyTimer.startTimer(300000);
				Enemy[j].MovementPhase = HD_gameSys.currentPattern;
				Enemy[j].anchor.x = Enemy[j].GetCenterPosition().x;
				Enemy[j].anchor.y = Enemy[j].GetCenterPosition().y;
				Enemy[j].attackTimer.startTimer(1500);
				break;
			}
		}
		break;
	case 12:
		for (j = 0; j < HD_NUMBER_OF_ENEMY; j++){
			if (Enemy[j].existence == 0) {
				//cout << "Ground enemy 11 spawned" << endl;
				Enemy[j].area = { 0, 0, 64, 64 };
				Enemy[j].dstarea = { 0, 0, 64, 64 };
				Enemy[j].LoadSprite("resource/HD_LaserEnemy1.png", Enemy[j].area, HD_renderer);
				Enemy[j].existence = 2;
				Enemy[j].SetPosition(index * 64, Object_map[HD_gameSys.currentPattern].dstarea.y);
				Enemy[j].SetDirection(0, HD_MAP_SCROLL_SPEED);
				Enemy[j].AttackPhase = 0;
				Enemy[j].MovementPhase = HD_gameSys.currentPattern;
				Enemy[j].SetAniType(2,0);
				Enemy[j].attackType = 12;
				Enemy[j].health = 10;
				Enemy[j].angle = 0;
				Enemy[j].DestroyTimer = HD_Timer();
				Enemy[j].DestroyTimer.startTimer(300000);
				Enemy[j].anchor.x = Enemy[j].GetCenterPosition().x;
				Enemy[j].anchor.y = Enemy[j].GetCenterPosition().y;
				Enemy[j].attackTimer.startTimer(1500);
				break;
			}
		}
		break;
	case 21:
		//enemy s1 spawn
		//cout << "21 enemy spawned" << endl;
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_TrackPlayer(HD_renderer, false, { index * 64, -64, 64, 64 }, 0, Player);
				Enemy[k].attackTimer.startTimer(500);
				Enemy[k].attackType = 21;
				break;
			}
		}
		break;
	case 121:
		a = 0;
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_TrackPlayer(HD_renderer, false, { index * 64, -64 - a / 5, 64, 64 }, 0, Player);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 21;
				a += 360;
			}
			if (a >= 720) break;
		}
		break;
	case 22:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_TrackPlayer(HD_renderer, false, { index * 64, -64, 96, 64 }, 7, Player);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 22;
				break;
			}
		}
		break;
	case 122:
		a = 0;
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_TrackPlayer(HD_renderer, false, { index * 64, -64 - a / 5, 96, 64 }, 7, Player);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 22;
				a += 360;
			}
			if (a >= 720) break;
		}
		break;
	case 23:
		//cout << "23 enemy spawned" << endl;
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 96, 64 }, 7);
				Enemy[k].attackTimer.startTimer(2000);
				Enemy[k].attackType = 23;
				break;
			}
		}
		break;
	case 24:
	case 58:
		//cout << "24 enemy spawned" << endl;
		//enemy l1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 128, 128 }, 2);
				Enemy[k].attackTimer.startTimer(2000);
				Enemy[k].attackType = trigger;
				break;
			}
		}
		break;
	case 25:
		//cout << "25 enemy spawned" << endl;
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_TrackPlayer(HD_renderer, false, { index * 64, -64, 64, 64 }, 0, Player);
				Enemy[k].attackType = 25;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 26:
		//cout << "26 enemy spawned" << endl;
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 96, 64 }, 1);
				Enemy[k].attackType = 26;
				Enemy[k].angle = 0;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 27:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 96, 64 }, 1);
				Enemy[k].attackType = 27;
				Enemy[k].angle = 0;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 28:
		//cout << "28 enemy spawned" << endl;
		//enemy l1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 128, 128 }, 2);
				Enemy[k].attackType = 28;
				Enemy[k].angle = 0;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 29:
		//enemy s1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_Accelerate(HD_renderer, false, { index * 64, -64, 64, 64 }, 0);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 29;
				break;
			}
		}
		break;
	case 30:
		//LaserEnemy
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_StopOnce_Long_Upperbound(HD_renderer, false, { HD_GAME_WIDTH, 128, 128, 128 }, 6);
				Enemy[k].attackType = 30;
				Enemy[k].attackTimer.startTimer(3000);
				break;
			}
		}
		break;
	case 31:
		//enemy s1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_TrackPlayer(HD_renderer, false, { index * 64, -64, 64, 64 }, 0, Player);
				Enemy[k].attackType = 31;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 32:
		//cout << "32 enemy spawned" << endl;
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_TrackPlayer(HD_renderer, false, { index * 64, -64, 96, 64 }, 1, Player);
				Enemy[k].attackType = 32;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;

	case 33:
		//enemy spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRight(HD_renderer, false, { -64, 64 * (index + 1), 64, 64 }, 3);
				Enemy[k].attackType = 33;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 34:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRight(HD_renderer, false, { -64, 64 * (index + 1), 96, 64 }, 7);
				Enemy[k].angle = -90;
				Enemy[k].attackType = 34;
				Enemy[k].attackTimer.startTimer(2000);
				break;
			}
		}
		break;
	case 35:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeft(HD_renderer, false, { HD_GAME_WIDTH, 64 * (index + 1), 64, 64 }, 3);
				Enemy[k].attackType = 35;
				Enemy[k].attackTimer.startTimer(2000);
				break;
			}
		}
		break;
	case 36:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeft(HD_renderer, false, { HD_GAME_WIDTH, 64 * (index + 1), 96, 64 }, 7);
				Enemy[k].attackType = 36;
				Enemy[k].attackTimer.startTimer(2000);
				break;
			}
		}
		break;
	case 37:
		a = 0;
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRight_ThenMoveDown(HD_renderer, false, { -64, 128 * a / 360, 64, 64 }, 3);
				Enemy[k].SetDirection(2 * (1 + a / 360), 0);
				Enemy[k].attackType = 37;
				Enemy[k].attackTimer.startTimer(1000);
				if (a >= 720)	{
					break;
				}
				else {
					a += 360;
				}
			}
		}
		break;
	case 38:
		a = 0;
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeft_ThenMoveDown(HD_renderer, false, { HD_GAME_WIDTH, 128 * a / 360, 64, 64 }, 3);
				Enemy[k].SetDirection(-2 * (1 + a / 360), 0);
				Enemy[k].attackType = 38;
				Enemy[k].attackTimer.startTimer(1000);
				if (a >= 720)	{
					break;
				}
				else {
					a += 360;
				}
			}
		}
		break;
	case 39:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRight_ChargeTowardPlayer(HD_renderer, false, { -64, index * 64, 64, 64 }, 3, Player);
				Enemy[k].attackType = 39;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 40:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeft_ChargeTowardPlayer(HD_renderer, false, { HD_GAME_WIDTH, index * 64, 64, 64 }, 3, Player);
				Enemy[k].attackType = 40;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 41:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRight(HD_renderer, false, { -64, index * 64, 96, 64 }, 7);
				Enemy[k].attackType = 41;
				Enemy[k].attackTimer.startTimer(2000);
				break;
			}
		}
		break;
	case 42:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeft(HD_renderer, false, { HD_GAME_WIDTH, index * 64, 96, 64 }, 7);
				Enemy[k].attackType = 42;
				Enemy[k].attackTimer.startTimer(2000);
				break;
			}
		}
		break;
	case 43:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeftwardCorn(HD_renderer, false, { index * 64, -64, 96, 64 }, 2, 12);
				Enemy[k].attackType = 43;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 44:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRightwardCorn(HD_renderer, false, { index * 64, -64, 96, 64 }, 2, 12);
				Enemy[k].attackType = 44;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 45:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeftwardCorn(HD_renderer, false, { index * 64, -64, 64, 64 }, 0, 16);
				Enemy[k].attackType = 45;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 46:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRightwardCorn(HD_renderer, false, { index * 64, -64, 64, 64 }, 0, 15);
				Enemy[k].attackType = 46;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 47:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRight_ThenMoveDown(HD_renderer, false, { -64, index * 64, 96, 64 }, 1);
				Enemy[k].attackType = 47;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 48:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeft_ThenMoveDown(HD_renderer, false, { HD_GAME_WIDTH, index * 64, 96, 64 }, 1);
				Enemy[k].attackType = 48;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 49:
		//LaserEnemy
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_StopOnce_Long_Upperbound(HD_renderer, false, { HD_GAME_WIDTH, 128, 128, 128 }, 5);
				Enemy[k].attackType = 49;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 50:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeftTilted(HD_renderer, false, { index * 64, -64, 96, 64 }, 1);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 50;
				break;
			}
		}
		break;
	case 51:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeftTilted(HD_renderer, false, { index * 64, -64, 128, 128 }, 2);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 51;
				break;
			}
		}
		break;
	case 52:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 96, 64 }, 1);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 52;
				break;
			}
		}
		break;
	case 53:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 128, 128 }, 2);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 53;
				break;
			}
		}
		break;
	case 54:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_StopOnce(HD_renderer, false, { index * 64, -64, 96, 64 }, 1);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 54;
				break;
			}
		}
		break;
	case 55:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown_StopOnce(HD_renderer, false, { index * 64, -64, 128, 128 }, 2);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 55;
				break;
			}
		}
		break;
	case 56:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRightTilted(HD_renderer, false, { index * 64, -64, 96, 64 }, 1);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 56;
				break;
			}
		}
		break;
	case 57:
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRightTilted(HD_renderer, false, { index * 64, -64, 128, 128 }, 2);
				Enemy[k].attackTimer.startTimer(1000);
				Enemy[k].attackType = 57;
				break;
			}
		}
		break;
	case 65:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDown(HD_renderer, false, { index * 64, -64, 128, 128 }, 2);
				Enemy[k].health = 32;
				Enemy[k].attackType = 65;
				Enemy[k].attackTimer.startTimer(1000);
				break;
			}
		}
		break;
	case 66:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveRight(HD_renderer, false, { HD_GAME_WIDTH, 128, 96, 64 }, 7);
				Enemy[k].attackType = 66;
				Enemy[k].attackTimer.startTimer(2000);
				break;
			}
		}
		break;
	case 67:
		//enemy m1 spawn
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveLeft(HD_renderer, false, { HD_GAME_WIDTH, 128, 96, 64 }, 7);
				Enemy[k].attackType = 67;
				Enemy[k].attackTimer.startTimer(2000);
				break;
			}
		}
		break;
	case 70:
		//enemy s1 spawn x4
		a = 0;
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDownwardCorn(HD_renderer, false, { index * 64 - a / 3, -64, 64, 64 }, 0, 6);
				Enemy[k].attackType = 70;
				Enemy[k].attackTimer.startTimer(1000);
				if (a >= 720)	{
					break;
				}
				else {
					a += 180;
				}
			}
		}
		break;
	case 71:
		a = 0;
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				Enemy[k].Preset_MoveDownwardCorn(HD_renderer, false, { index * 64 - a / 3, -64, 64, 64 }, 0, -6);
				Enemy[k].attackType = 70;
				Enemy[k].attackTimer.startTimer(1000);
				if (a >= 720)	{
					break;
				}
				else {
					a += 180;
				}
			}
		}
		break;
	case 101: // item 1
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				//cout << "Ground item 101 spawned" << endl;
				Enemy[k].SetObjectAttribute(HD_renderer, false, "resource/HD_item1.png", { 0, 0, 64, 64 }, { 0, 0, 64, 64 }, 2, 101, -1, 0);
				Enemy[k].SetPosition(index * 64, Object_map[HD_gameSys.currentPattern].dstarea.y);
				Enemy[k].SetDirection(0, HD_MAP_SCROLL_SPEED);
				Enemy[k].DestroyTimer = HD_Timer();
				Enemy[k].DestroyTimer.startTimer(300000);
				Enemy[k].health = 1;
				break;
			}
		}
		break;
	case 102: // item 1
		for (k = 0; k < HD_NUMBER_OF_ENEMY; k++){
			if (Enemy[k].existence == 0) {
				//cout << "Ground item 101 spawned" << endl;
				Enemy[k].SetObjectAttribute(HD_renderer, false, "resource/HD_item2.png", { 0, 0, 64, 64 }, { 0, 0, 64, 64 }, 2, 102, 0, 2);
				Enemy[k].SetPosition(index * 64, Object_map[HD_gameSys.currentPattern].dstarea.y);
				Enemy[k].SetDirection(0, 0.2);
				Enemy[k].DestroyTimer = HD_Timer();
				Enemy[k].DestroyTimer.startTimer(300000);
				Enemy[k].health = 1;
				break;
			}
		}
		break;
	default:
		break;
	}
	return true;
}
void HD_SYSTEM::manageObject(){ //managing ingame object
	int i;
	for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
		//delete object_ingame or move it
		if (Object_ingame[i].existence > 0) {
			if (Object_ingame[i].dstarea.y < -64 || Object_ingame[i].dstarea.y >(HD_GAME_HEIGHT + 64) || Object_ingame[i].dstarea.x < -64 || Object_ingame[i].dstarea.x >(HD_GAME_WIDTH + 64)){
				Object_ingame[i].existence = 0; //delete
			}
			Object_ingame[i].SetPosition(Object_ingame[i].dstarea.x + Object_ingame[i].GetDirection().x, Object_ingame[i].dstarea.y + Object_ingame[i].GetDirection().y);
			Object_ingame[i].anchor.x = Object_ingame[i].dstarea.x + Object_ingame[i].GetObjectCenter(false).x;
			Object_ingame[i].anchor.y = Object_ingame[i].dstarea.y + Object_ingame[i].GetObjectCenter(false).y;
		}
		if (Object[i].attackType == 1010) { //boss scene
			Object[i].dstarea.w = 560 * Boss.health / HD_BOSS1_HEALTH;
			Object[i].dstarea.h = 16;
			Object[i].SetCenterPosition(HD_SCREEN_WIDTH / 2, 40);
		}
		else if (Object[i].attackType == 1011) {
			Object[i].dstarea.w = 560 * Boss.health / HD_BOSS2_HEALTH;
			Object[i].dstarea.h = 16;
			Object[i].SetCenterPosition(HD_SCREEN_WIDTH / 2, 40);
		}
		//manage object[]
		if (Object[i].existence > 0) {
			if (Object[i].DestroyTimer.checkTimer()){
				Object[i].existence = 0;
			}
		}
		if (Object[i].attackType == 12) { // charge bar
			if (!Display.flip){
				Object[i].SetCenterPosition(Player.GetCenterPosition().x, Player.GetCenterPosition().y + 40);
			}
			else{
				Object[i].SetCenterPosition(HD_GAME_WIDTH - Player.GetCenterPosition().x, Player.GetCenterPosition().y + 40);
			}
		} else if (Object[i].attackType == 5){
			Object[i].SetPosition(16, 460);
		}
	}
	for (i = 0; i < HD_MAX_MAPOBJECT; i++){
		Object_map[i].SetPosition(Object_map[i].dstarea.x + Object_map[i].GetDirection().x, Object_map[i].dstarea.y + Object_map[i].GetDirection().y);
	}
	// update score object
	sprintf_s(Player.healthbuf, "%02d", Player.health);
	Object[16].LoadText(HD_renderer, Player.healthbuf);
	Object[16].SetCenterPosition(248, 540);
	Object[16].AttackPhase = 0;
	Object[16].SetDirection(0, 0);
	Object[16].SetAniType(0, -1);
	Object[16].existence = 2;
	sprintf_s(HD_gameSys.rankbuf, "%02d", HD_gameSys.rank);
	Object[14].LoadText(HD_renderer, HD_gameSys.rankbuf);
	Object[14].SetPosition(532, 540);
	Object[14].AttackPhase = 0;
	Object[14].SetDirection(0, 0);
	Object[14].SetAniType(0, -1);
	Object[14].existence = 2;
	sprintf_s(HD_gameSys.scorebuf, "%08d", HD_gameSys.score);
	Object[15].LoadText(HD_renderer, HD_gameSys.scorebuf);
	Object[15].SetCenterPosition(400, 540);
	Object[15].AttackPhase = 0;
	Object[15].SetDirection(0, 0);
	Object[15].SetAniType(0, -1);
	Object[15].existence = 2;
	return;
}
void HD_SYSTEM::manageEnemy(){
	int i, j;
	int a = 0;
	int direction_rand;
	HD_Point EnemyCenter;
	for (i = 0; i < HD_NUMBER_OF_ENEMY; i++){
		//delete enemy or move it
		if (Enemy[i].existence == 2) {
			//shared condition control
			if ((Enemy[i].DestroyTimer.checkStart() && Enemy[i].DestroyTimer.checkTimer()) || Enemy[i].dstarea.y > HD_GAME_HEIGHT + 320 || Enemy[i].dstarea.x < -320 || Enemy[i].dstarea.x > HD_GAME_WIDTH + 320){
				Enemy[i].existence = 0; //delete
				Enemy[i].AttackPhase = 0;
				Enemy[i].MovementPhase = 0;
				Enemy[i].DestroyPhase = 0;
				Enemy[i].attackTimer = HD_Timer();
				Enemy[i].MoveTimer = HD_Timer();
				Enemy[i].DestroyTimer = HD_Timer();
			}
			else if (Enemy[i].attackType != 49 && Enemy[i].health == 0){
				if (Enemy[i].attackType == 65) {
					Enemy[i].SetObjectAttribute(HD_renderer, false, "resource/HD_item2.png", { 0, 0, 64, 64 }, { Enemy[i].dstarea.x, Enemy[i].dstarea.y, 64, 64 }, 2, 102, 0, 2);
					Enemy[i].SetDirection(0, -3);
					Enemy[i].attackType = 102; // attackType = 101;
					Enemy[i].health = 1;
				}
				else {
					if (scene != 6 && scene != 11){ // not in boss phase
						switch (Enemy[i].attackType) {
						case 11:
						case 21:
						case 25:
						case 29:
						case 31:
						case 33:
						case 35:
						case 37:
						case 38:
						case 39:
						case 40:
						case 45:
						case 46:
							HD_gameSys.addScore(100);
							break;
						case 12:
						case 22:
						case 23:
						case 26:
						case 27:
						case 32:
						case 34:
						case 36:
						case 41:
						case 42:
						case 50:
						case 52:
						case 54:
						case 56:
						case 66:
						case 67:
						case 70:
						case 71:
							HD_gameSys.addScore(200);
							break;
						case 24:
						case 28:
						case 30:
						case 43:
						case 44:
						case 47:
						case 48:
						case 51:
						case 53:
						case 55:
						case 57:
						case 58:
						case 65:
							HD_gameSys.addScore(500);
							break;
						default:
							HD_gameSys.addScore(0);
							break;
						}
					}
					EnemyCenter = Enemy[i].GetCenterPosition();
					Enemy[i].currentFrame = 0;
					Enemy[i].AttackPhase = 0;
					Enemy[i].MovementPhase = 0;
					Enemy[i].DestroyPhase = 0;
					Enemy[i].SetObjectAttribute(HD_renderer, false, "resource/explosion.png", { 0, 0, 64, 64 }, { EnemyCenter.x, EnemyCenter.y, 64, 64 }, 3, 0, 7, 6);
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
				}
				break;
			}
			//individual enemy condition control
			//movement changes
			if (Enemy[i].attackType > 10 && Enemy[i].attackType <= 15) {
				Enemy[i].dstarea.y = Object_map[Enemy[i].MovementPhase].dstarea.y;
			}
			else {
				switch (Enemy[i].attackType){
				case 29:
					Enemy[i].Preset_MoveDown_Accelerate(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 21:
				case 22:
				case 25:
					Enemy[i].Preset_MoveDown_TrackPlayer(HD_renderer, true, Enemy[i].dstarea, 0, Player);
					break;
				case 54:
				case 55:
					Enemy[i].Preset_MoveDown_StopOnce_BacktoUpperside(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 31:
				case 32:
					Enemy[i].Preset_MoveDown_StopOnce_Long(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 30:
				case 49:
					Enemy[i].Preset_MoveDown_StopOnce_Long_Upperbound(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 70:
				case 71:
					Enemy[i].Preset_MoveDownwardCorn(HD_renderer, true, Enemy[i].dstarea, 0, 0);
					break;
				case 43:
				case 45:
					Enemy[i].Preset_MoveLeftwardCorn(HD_renderer, true, Enemy[i].dstarea, 0, 0);
					break;
				case 44:
				case 46:
					Enemy[i].Preset_MoveRightwardCorn(HD_renderer, true, Enemy[i].dstarea, 0, 0);
					break;
				case 37:
				case 47:
					Enemy[i].Preset_MoveRight_ThenMoveDown(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 38:
				case 48:
					Enemy[i].Preset_MoveLeft_ThenMoveDown(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 39:
					Enemy[i].Preset_MoveRight_ChargeTowardPlayer(HD_renderer, true, Enemy[i].dstarea, 0, Player);
					break;
				case 40:
					Enemy[i].Preset_MoveLeft_ChargeTowardPlayer(HD_renderer, true, Enemy[i].dstarea, 0, Player);
					break;
				case 50:
				case 51:
					Enemy[i].Preset_MoveLeftTilted(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 56:
				case 57:
					Enemy[i].Preset_MoveRightTilted(HD_renderer, true, Enemy[i].dstarea, 0);
					break;
				case 102:
					if (Enemy[i].GetDirection().y < 3)Enemy[i].SetDirection(Enemy[i].GetDirection().x, Enemy[i].GetDirection().y + 0.2);
					break;
				default:
					break;
				}
				//Apply enemy position changes
				Enemy[i].SetPosition(Enemy[i].dstarea.x + Enemy[i].GetDirection().x, Enemy[i].dstarea.y + Enemy[i].GetDirection().y);
				Enemy[i].anchor = PointTransform(Enemy[i].GetCenterPosition());
			}
			//create enemy's bullet
			if (Enemy[i].existence == 2 && Enemy[i].attackTimer.checkTimer()){
				a = 0;
				for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
					if (Bullet[j].existence == 0 && Enemy[i].dstarea.y > 0 && Enemy[i].dstarea.y < 360 && Enemy[i].dstarea.x < HD_GAME_WIDTH-96 && Enemy[i].dstarea.x > 0) {
						HD_Point differ;
						differ.x = Player.GetCenterPosition().x - Enemy[i].GetCenterPosition().x;
						differ.y = Player.GetCenterPosition().y - Enemy[i].GetCenterPosition().y;
						double distance = sqrt(differ.x * differ.x + differ.y * differ.y);
						switch (Enemy[i].attackType) {
						case 11:
						case 21:
						case 22:
						case 33:
						case 35:
						case 65:
						case 70:
							//aiming shot, constant speed
							differ.x = 8*differ.x /distance;
							differ.y = 8 * differ.y / distance;
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet3.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, -1, 0);
							Bullet[j].SetDirection(differ.x, differ.y);
							differ.x /= 8;
							if (differ.y > 0)Bullet[j].angle = -M_PI+(180 / M_PI) * acos(differ.x);
							else Bullet[j].angle = M_PI - (180 / M_PI) * acos(differ.x);
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a = 720;
							break;
						case 54:
						case 55:
							// downward *2, constant speed
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							Bullet[j].SetDirection(0, HD_GAME_SCROLL_SPEED+ 8);
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16 + 32 * a / 360, Enemy[i].GetCenterPosition().y);
							a += 360;
							break;
						case 50:
						case 51:
						case 56:
						case 57:
							//4-way + shot, constant speed
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							switch (a){
							case 0:
								Bullet[j].SetDirection(0, HD_GAME_SCROLL_SPEED + 8);
								break;
							case 180:
								Bullet[j].SetDirection(HD_GAME_SCROLL_SPEED + 8, 0);
								break;
							case 360:
								Bullet[j].SetDirection(0, (HD_GAME_SCROLL_SPEED + 8)*-1);
								break;
							case 540:
								Bullet[j].SetDirection(-1 * (HD_GAME_SCROLL_SPEED + 8), 0);
								break;
							default:
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a += 180;
							break;
						case 52:
						case 53:
							//4-way x shot, constant speed
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							switch (a){
							case 0:
								Bullet[j].SetDirection(4 * cos(M_PI / 4), 4 * sin(M_PI / 4));
								break;
							case 180:
								Bullet[j].SetDirection(4 * cos(3 * M_PI / 4), 4 * sin(3 * M_PI / 4));
								break;
							case 360:
								Bullet[j].SetDirection(4 * cos(-3 * M_PI / 4), 4 * sin(-3 * M_PI / 4));
								break;
							case 540:
								Bullet[j].SetDirection(4 * cos(-M_PI / 4), 4 * sin(-M_PI / 4));
								break;
							default:
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a += 180;
							break;
						case 23:
						case 24:
							//8-way, constant speed
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							switch (a){
							case 0:
								Bullet[j].SetDirection(0, 4);
								break;
							case 90:
								Bullet[j].SetDirection(0.7071*4, 0.7071*4);
								break;
							case 180:
								Bullet[j].SetDirection(4, 0);
								break;
							case 270:
								Bullet[j].SetDirection(0.7071*4, -0.7071*4);
								break;
							case 360:
								Bullet[j].SetDirection(0, 4*-1);
								break;
							case 450:
								Bullet[j].SetDirection(-0.7071*4, -0.7071*4);
								break;
							case 540:
								Bullet[j].SetDirection(-1 * 4, 0);
								break;
							case 630:
								Bullet[j].SetDirection(-0.7071*4, 0.7071*4);
								break;
							default:
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a += 90;
							break;
						case 25:
							//3-way downward, constant speed
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							switch (a){
							case 0:
								Bullet[j].SetDirection(0, HD_GAME_SCROLL_SPEED + 8);
								break;
							case 240:
								Bullet[j].SetDirection(0.7071*(HD_GAME_SCROLL_SPEED + 8), 0.7071*(HD_GAME_SCROLL_SPEED + 8));
								break;
							case 480:
								Bullet[j].SetDirection(-0.7071*(HD_GAME_SCROLL_SPEED + 8), 0.7071*(HD_GAME_SCROLL_SPEED + 8));
								break;
							default:
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a += 240;
							break;
						case 26:
							//5-way downward: / ||| \, constant speed
							switch (a){
							case 0:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(-0.7071*3, 0.7071*3);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16, Enemy[i].GetCenterPosition().y + 16);
								break;
							case 144:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(0, 5);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16, Enemy[i].GetCenterPosition().y + 16);
								break;
							case 288:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(0, 6);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
								break;
							case 432:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(0, 5);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x + 16, Enemy[i].GetCenterPosition().y + 16);
								break;
							case 576:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(0.7071 * 3, 0.7071 * 3);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x + 16, Enemy[i].GetCenterPosition().y + 16);
								break;
							default:
								break;
							}
							a += 144;
							break;
						case 27:
						case 28:
							//+ <-> x
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							if (Enemy[i].AttackPhase != 1){
								switch (a){
								case 0:
									Bullet[j].SetDirection(0, 4);
									break;
								case 180:
									Bullet[j].SetDirection(4, 0);
									break;
								case 360:
									Bullet[j].SetDirection(0, 4 * -1);
									break;
								case 540:
									Bullet[j].SetDirection(-1 * 4, 0);
									break;
								default:
									break;
								}
							}
							else {
								switch (a){
								case 0:
									Bullet[j].SetDirection(0.7071 * 4, 0.7071*(4));
									break;
								case 180:
									Bullet[j].SetDirection(0.7071 * 4, -0.7071 * 4);
									break;
								case 360:
									Bullet[j].SetDirection(-0.7071 * 4, -0.7071 * 4);
									break;
								case 540:
									Bullet[j].SetDirection(-0.7071 * 4, 0.7071 * 4);
									break;
								default:
									break;
								}
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
							a += 180;
							break;
						case 47:
						case 48:
							//+ <-> x with 3 shot
							if (Enemy[i].AttackPhase != 1){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								switch (a){
								case 0:
									Bullet[j].SetDirection(4 * cos(-M_PI / 12), 4 * sin(-M_PI / 12));
									break;
								case 60:
									Bullet[j].SetDirection(4 * cos(0), 4 * sin(0));
									break;
								case 120:
									Bullet[j].SetDirection(4 * cos(M_PI / 12), 4 * sin(M_PI / 12));
									break;
								case 180:
									Bullet[j].SetDirection(4 * cos(5 * M_PI / 12), 4 * sin(5 * M_PI / 12));
									break;
								case 240:
									Bullet[j].SetDirection(4 * cos(M_PI / 2), 4 * sin(M_PI / 2));
									break;
								case 300:
									Bullet[j].SetDirection(4 * cos(7 * M_PI / 12), 4 * sin(7 * M_PI / 12));
									break;
								case 360:
									Bullet[j].SetDirection(4 * cos(11 * M_PI / 12), 4 * sin(11 * M_PI / 12));
									break;
								case 420:
									Bullet[j].SetDirection(4 * cos(M_PI), 4 * sin(M_PI));
									break;
								case 480:
									Bullet[j].SetDirection(4 * cos(-11 * M_PI / 12), 4 * sin(-11 * M_PI / 12));
									break;
								case 540:
									Bullet[j].SetDirection(4 * cos(-7 * M_PI / 12), 4 * sin(-7 * M_PI / 12));
									break;
								case 600:
									Bullet[j].SetDirection(4 * cos(-M_PI / 2), 4 * sin(-M_PI / 2));
									break;
								case 660:
									Bullet[j].SetDirection(4 * cos(-5 * M_PI / 12), 4 * sin(-5 * M_PI / 12));
									break;
								default:
									break;
								}
							}
							else {
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								switch (a){
								case 0:
									Bullet[j].SetDirection(4 * cos(M_PI / 6), 4 * sin(M_PI / 6));
									break;
								case 60:
									Bullet[j].SetDirection(4 * cos(M_PI / 4), 4 * sin(M_PI / 4));
									break;
								case 120:
									Bullet[j].SetDirection(4 * cos(M_PI / 3), 4 * sin(M_PI / 3));
									break;
								case 180:
									Bullet[j].SetDirection(4 * cos(2 * M_PI / 3), 4 * sin(2* M_PI / 3));
									break;
								case 240:
									Bullet[j].SetDirection(4 * cos(3*M_PI / 4), 4 * sin(3* M_PI / 4));
									break;
								case 300:
									Bullet[j].SetDirection(4 * cos(5 * M_PI / 6), 4 * sin(5 * M_PI / 6));
									break;
								case 360:
									Bullet[j].SetDirection(4 * cos(-5 * M_PI / 6), 4 * sin(-5 * M_PI / 6));
									break;
								case 420:
									Bullet[j].SetDirection(4 * cos(-3 * M_PI / 4), 4 * sin(-3 * M_PI / 4));
									break;
								case 480:
									Bullet[j].SetDirection(4 * cos(-2 * M_PI / 3), 4 * sin(-2 * M_PI / 3));
									break;
								case 540:
									Bullet[j].SetDirection(4 * cos(-M_PI / 3), 4 * sin(-M_PI / 3));
									break;
								case 600:
									Bullet[j].SetDirection(4 * cos(-M_PI / 4), 4 * sin(-M_PI / 4));
									break;
								case 660:
									Bullet[j].SetDirection(4 * cos(-M_PI / 6), 4 * sin(-M_PI / 6));
									break;
								default:
									break;
								}
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
							a += 60;
							break;
						case 30:
						case 49:
							if (Enemy[i].AttackPhase == 0){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Laser.png", { 0, 0, 64, 64 }, { 0, 0, 64, 64 }, 2, 1, 0, 2);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
								Bullet[j].MovementPhase = i; // Enemy index
								Enemy[i].AttackPhase = 1;
							}
							a = 720;
							break;
						case 58:
							//+ <-> x
							if (Enemy[i].AttackPhase != 1){
								switch (a){
								case 0:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(-0.7071 * 3, 0.7071 * 3);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16, Enemy[i].GetCenterPosition().y + 16);
									break;
								case 144:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(0, 5);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16, Enemy[i].GetCenterPosition().y + 16);
									break;
								case 288:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(0, 6);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
									break;
								case 432:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(0, 5);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x + 16, Enemy[i].GetCenterPosition().y + 16);
									break;
								case 576:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(0.7071 * 3, 0.7071 * 3);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x + 16, Enemy[i].GetCenterPosition().y + 16);
									break;
								default:
									break;
								}
								a += 144;
							}
							else {
								switch (a){
								case 0:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(-4,0);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
									break;
								case 180:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(0.7071 * 4, -0.7071 * 4);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
									break;
								case 360:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(-0.7071 * 4, 0.7071 * 4);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
									break;
								case 540:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(4, 0);
									Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y + 16);
									break;
								default:
									break;
								}
								a += 180;
							}
							break;
						case 29:
							//drop shot
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 3, 0, 2);
							Bullet[j].SetDirection(0, (HD_GAME_SCROLL_SPEED + 2));
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a = 720;
							break;
						case 31:
							//drop shot x2
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 3, 0, 2);
							Bullet[j].SetDirection(0, -1 * (HD_GAME_SCROLL_SPEED + 2));
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a = 720;
							break;
						case 32:
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 3, 0, 2);
							Bullet[j].SetDirection(0, -1 * (HD_GAME_SCROLL_SPEED + 2));
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x-16 + 32*a/360, Enemy[i].GetCenterPosition().y);
							a += 360;
							break;
						case 41:
						case 43:
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							Bullet[j].SetDirection(4 * cos(M_PI * (1 - ((double)Enemy[i].AttackPhase) / 8)), 4 * sin(M_PI * (1 - ((double)Enemy[i].AttackPhase) / 8)));
							a = 720;
							break;
						case 42:
						case 44:
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							Bullet[j].SetDirection(4 * cos(M_PI*(-1 + ((double)Enemy[i].AttackPhase) / 8)), 4 * sin(M_PI*(-1 + ((double)Enemy[i].AttackPhase) / 8)));
							a = 720;
							break;
						case 45:
						case 46:
							//3 aiming shot with various speed x2
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							switch (a){
							case 0:
								Bullet[j].SetDirection(4 * differ.x / distance, 4 * differ.y / distance);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16, Enemy[i].GetCenterPosition().y);
								break;
							case 120:
								Bullet[j].SetDirection(4 * differ.x / distance, 4 * differ.y / distance);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x + 16, Enemy[i].GetCenterPosition().y);
								break;
							case 240:
								Bullet[j].SetDirection(7 * differ.x / distance, 7 * differ.y / distance);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16, Enemy[i].GetCenterPosition().y);
								break;
							case 360:
								Bullet[j].SetDirection(7 * differ.x / distance, 7 * differ.y / distance);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x + 16, Enemy[i].GetCenterPosition().y);
								break;
							case 480:
								Bullet[j].SetDirection(10 * differ.x / distance, 10 * differ.y / distance);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x - 16, Enemy[i].GetCenterPosition().y);
								break;
							case 600:
								Bullet[j].SetDirection(10 * differ.x / distance, 10 * differ.y / distance);
								Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x + 16, Enemy[i].GetCenterPosition().y);
								break;
							default:
								break;
							}
							a += 120;
							break;
						case 12:
							if (differ.y > 0)Enemy[i].angle = -M_PI + (180 / M_PI) * acos(differ.x/distance);
							else Enemy[i].angle = M_PI - (180 / M_PI) * acos(differ.x/distance);
							Enemy[i].area.y += 64;
							Enemy[i].SetAniType(0,12);
							//2 aiming shot with various speed
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							switch (a){
							case 0:
								Bullet[j].SetDirection(4 * differ.x / distance, 4 * differ.y / distance);
								break;
							case 360:
								Bullet[j].SetDirection(8 * differ.x / distance, 7 * differ.y / distance);
								break;
							default:
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a += 360;
							break;
						case 37:
						case 38:
						case 39:
						case 40:
							//2 aiming shot with various speed
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
							switch (a){
							case 0:
								Bullet[j].SetDirection(4 * differ.x / distance, 4 * differ.y / distance);
								break;
							case 360:
								Bullet[j].SetDirection(8 * differ.x / distance, 7 * differ.y / distance);
								break;
							default:
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a += 360;
							break;
						case 34:
						case 36:
						case 66:
						case 67:
							//3 aiming shot with various speed
							switch (a){
							case 0:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(4 * differ.x / distance, 4 * differ.y / distance);
								break;
							case 240:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(7 * differ.x / distance, 7 * differ.y / distance);
								break;
							case 480:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(10 * differ.x / distance, 10 * differ.y / distance);
								break;
							default:
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a += 240;
							break;
						case 71:
							Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 4, 0, 2);
							direction_rand = rand() % 4;
							switch (direction_rand){
							case 0:
								Bullet[j].SetDirection(4 * cos(M_PI/4), 4 * sin(M_PI/4));
								break;
							case 1:
								Bullet[j].SetDirection(4 * cos(3* M_PI / 4), 4 * sin(3* M_PI / 4));
								break;
							case 2:
								Bullet[j].SetDirection(4 * cos(-3* M_PI / 4), 4 * sin(-3* M_PI / 4));
								break;
							case 3:
								Bullet[j].SetDirection(4 * cos(-M_PI / 4), 4 * sin(-M_PI / 4));
								break;
							}
							Bullet[j].SetCenterPosition(Enemy[i].GetCenterPosition().x, Enemy[i].GetCenterPosition().y);
							a = 720;
							break;
						default:
							break;
						}
						if (a >= 720){
							switch (Enemy[i].attackType){
							case 65:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(500);
								break;
							case 25:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(1000);
								break;
							case 21:
							case 22:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(1500);
								break;
							case 11:
							case 31:
							case 33:
							case 35:
							case 37:
							case 38:
							case 39:
							case 40:
							case 50:
							case 51:
							case 52:
							case 53:
							case 56:
							case 57:
							case 70:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(2000);
								break;
							case 12:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(2000);
								Enemy[i].angle = acos(differ.x / distance);
								break;
							case 26:
							case 29:
							case 32:
							case 34:
							case 36:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(2500);
								break;
							case 23:
							case 24:
							case 45:
							case 46:
							case 66:
							case 67:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(3000);
								break;
							case 41:
							case 42:
								if (Enemy[i].AttackPhase != 16) {
									Enemy[i].AttackPhase += 1;
									Enemy[i].attackTimer = HD_Timer();
									Enemy[i].attackTimer.startTimer(50);
								}
								else { 
									Enemy[i].AttackPhase = 0; 
									Enemy[i].attackTimer = HD_Timer();
									Enemy[i].attackTimer.startTimer(3000);
								}
								break;
							case 43:
							case 44:
								if (Enemy[i].AttackPhase != 32) {
									Enemy[i].AttackPhase += 1;
									Enemy[i].attackTimer = HD_Timer();
									Enemy[i].attackTimer.startTimer(50);
								}
								else {
									Enemy[i].AttackPhase = 0;
									Enemy[i].attackTimer = HD_Timer();
									Enemy[i].attackTimer.startTimer(3000);
								}
								break;
							case 27:
							case 28:
								if (Enemy[i].AttackPhase != 1) {
									Enemy[i].AttackPhase = 1;
									Enemy[i].attackTimer = HD_Timer();
									Enemy[i].attackTimer.startTimer(500);
								}
								else {
									Enemy[i].AttackPhase = 0;
									Enemy[i].attackTimer = HD_Timer();
									Enemy[i].attackTimer.startTimer(2000);
								}
								break;
							case 47:
							case 48:
								if (Enemy[i].AttackPhase != 1) {
									Enemy[i].AttackPhase = 1;
									Enemy[i].attackTimer = HD_Timer();
								}
								else {
									Enemy[i].AttackPhase = 0;
									Enemy[i].attackTimer = HD_Timer();
								}
								Enemy[i].attackTimer.startTimer(1000);
								break;
							case 58:
								if (Enemy[i].AttackPhase == 1) Enemy[i].AttackPhase = 0;
								else Enemy[i].AttackPhase = 1;
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(3000);
								break;
							default:
								Enemy[i].attackTimer = HD_Timer();
								Enemy[i].attackTimer.startTimer(5000);
								break;
							}
							break;
						}
					}
				}
			}
		}
	}
	return;
}
bool HD_SYSTEM::manageBoss(){
	int i, j;
	HD_Point differ = {0.00, 0.00};
	int randomNum = 0;
	int probability = 0;
	int PlayerSubweaponNum = 0;
	for (i = 0; i < 4; i++){
		if (Player.subweapon[i].existence == 2) PlayerSubweaponNum++;
	}
	double distance = 0.00;
	differ.x = Player.GetCenterPosition().x - Boss.GetCenterPosition().x;
	differ.y = Player.GetCenterPosition().y - Boss.GetCenterPosition().y;
	distance = sqrt(differ.x * differ.x + differ.y * differ.y);
	differ.x /= distance;
	differ.y /= distance;
	HD_Point tempPoint;
	// manage boss
	Boss.area.y = 0;
	for (i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
		if (CollisionCheck(Boss, Player.bullet[i])){
			if (Boss.GetHealth() > 0 && Boss.GetHealth() >= HD_PLAYER_ATTACK_POWER )
				Boss.SetHealth(Boss.GetHealth() - HD_PLAYER_ATTACK_POWER);
			else if (Boss.GetHealth() < 0) //immortal
				Boss.SetHealth(Boss.GetHealth());
			else
				Boss.SetHealth(0);
			Player.bullet[i].existence = 0;
			Boss.area.y += Boss.area.h;
		}
	}
	//boss pattern managemnet
	if (Boss.existence == 2){
		if (CollisionCheck(Player, Boss)){
			if (Player.CrashTimer.checkTimer()){
				//Player vs boss collision
				if (Player.health >= 1){
					for (i = 0; i < 4; i++){
						if (Player.subweapon[i].existence != 0){
							Player.subweapon[i].existence = 0;
							Player.subweapon[i].DestroyTimer = HD_Timer();
							break;
						}
					}
				}
				if (Player.GetHealth() > 0 && Player.GetHealth() >= 1)
					Player.SetHealth(Player.GetHealth() - 1);
				else if (Player.GetHealth() < 0) //no immortality
					Player.SetHealth(0);
				else
				{// Enemy[i].Gethealth == 0 ||(Enemy[i].GetHealth >0 && Enemy[i].GetHealth() < HD_PLAYER_ATTACK_POWER)
					Player.SetHealth(0);
				}
				if (Player.health == 0){
					GameOver();
				}
				else {
					Player.CrashTimer = HD_Timer();
					Player.CrashTimer.startTimer(1500);
					Player.Shield.existence = 2;
				}
			}
		}
		// manage boss
		switch (scene){
		case 11:
			//screen changes & bullet shooting
			if (Boss.GetHealth() >= HD_BOSS2_HEALTH * 3 / 4){
				//move
				if (boss_phase < 1 && bossTimer.checkStart()){ //phase 0
					if (Boss.MovementPhase == 0 && bossTimer.checkTimer()){
						if (Boss.GetCenterPosition().y < 150) {
							tempPoint = Boss.GetCenterPosition();
							tempPoint.y += Boss.GetDirection().y;
							Boss.SetCenterPosition(tempPoint.x, tempPoint.y);
							Display.Shake();
						}
						else {
							Boss.MovementPhase = 1;
							boss_phase = 1;
							Display.DisplayReset();
							bossTimer = HD_Timer();
							bossTimer.startTimer(300);
						}
					}
				}
				else Display.CrossShrink(false,30, 3);
				//shoot
				if (bossTimer.checkStart() && bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							if (Boss.AttackPhase >= 0 && Boss.AttackPhase < 4){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint.x = (double)a;
								Bullet[j].SetCenterPosition(tempPoint.x, 0.0);
								switch (Boss.AttackPhase){
								case 0:
									Bullet[j].SetDirection(4 * cos(3 * M_PI / 8), 4);
									break;
								case 2:
									Bullet[j].SetDirection(4 * cos(5 * M_PI / 8), 4);
									break;
								default:
									Bullet[j].SetDirection(0, 4.0 * cos (3));
								}
								a += 80;
							}
							else if (Boss.AttackPhase >= 5 && Boss.AttackPhase < 9){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint.x = (double)a + 20;
								Bullet[j].SetCenterPosition(tempPoint.x, 0);
								switch (Boss.AttackPhase){
								case 5:
									Bullet[j].SetDirection(4 * cos(3 * M_PI / 8), 4);
									break;
								case 7:
									Bullet[j].SetDirection(4 * cos(5 * M_PI / 8), 4);
									break;
								default:
									Bullet[j].SetDirection(0, 4.0);
								}
								a += 80;
							}
							else if (Boss.AttackPhase >= 10 && Boss.AttackPhase < 14){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint.x = (double)a + 40;
								Bullet[j].SetCenterPosition(tempPoint.x, 0);
								switch (Boss.AttackPhase){
								case 10:
									Bullet[j].SetDirection(4 * cos(3 * M_PI / 8), 4);
									break;
								case 12:
									Bullet[j].SetDirection(4 * cos(5 * M_PI / 8), 4);
									break;
								default:
									Bullet[j].SetDirection(0, 4.0);
								}
								a += 80;
							}
							else if (Boss.AttackPhase >= 15 && Boss.AttackPhase < 19){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint.x = (double)a + 60;
								Bullet[j].SetCenterPosition(tempPoint.x, 0);
								switch (Boss.AttackPhase){
								case 15:
									Bullet[j].SetDirection(4 * cos(3 * M_PI / 8), 4 );
									break;
								case 17:
									Bullet[j].SetDirection(4 * cos(5 * M_PI / 8), 4);
									break;
								default:
									Bullet[j].SetDirection(0, 4.0);
								}
								a += 80;
							}
							else {
								switch (a){
								case 0:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(5.0* differ.x, 5.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
									break;
								case 240:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(5.0* differ.x, 5.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 32 * differ.y - 2 * differ.x, Boss.GetCenterPosition().y + 32 * differ.x - 2 * differ.y);
									break;
								case 480:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(5.0* differ.x, 5.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 32 * differ.y - 2 * differ.x, Boss.GetCenterPosition().y - 32 * differ.x - 2 * differ.y);
									break;
								default:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(4*differ.x, 4*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
									break;
								}
								a += 240;
							}
							if (a >= 640){
								switch (Boss.AttackPhase){
								case 3:
								case 4:
								case 8:
								case 9:
								case 13:
								case 14:
								case 18:
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(375);
									break;
								case 19:
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(375);
									break;
								default:
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(750);
									break;
								}
								break;
							}
						}
					}
				}
			}
			else if (Boss.GetHealth() >= HD_BOSS2_HEALTH / 2 && Boss.GetHealth() < HD_BOSS2_HEALTH * 3 / 4) {
				// phase change
				if (boss_phase < 2){
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						Bullet[j].existence = 0;
					}
					bossTimer = HD_Timer();
					bossTimer.startTimer(300);
					Boss.attackTimer = HD_Timer();
					Boss.attackTimer.startTimer(600);
					Boss.AttackPhase = 0;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					boss_phase = 2;
				}
				else {
					if (bossTimer.checkStart() && bossTimer.checkTimer()){
						if (boss_phase < 3) {
							boss_phase = 3;
							Display.DisplayReset();
						}
						else Display.Swing8();
					}
					else{
						Display.Shake();
	
					}
				}
				//shoot
				if (bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							if (Boss.AttackPhase == 0){
								if (a < 720){
									tempPoint.x = 3.000 * cos((-1 + (double)a / 360) * M_PI);
									tempPoint.y = 3.000 * sin((-1 + (double)a / 360) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								}
								else {
									tempPoint.x = 4 * differ.x;
									tempPoint.y = 4 * differ.y;
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								}
								a += 60;
							}
							else if (Boss.AttackPhase == 8){
								if (a < 720) {
									tempPoint.x = 3.000 * cos((-1 + (double)(a + 30) / 360) * M_PI);
									tempPoint.y = 3.000 * sin((-1 + (double)(a + 30) / 360) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									tempPoint.x = -32 * sin(Display.AreaDev* M_PI / 90) + Boss.GetCenterPosition().x;
									tempPoint.y = -32 * sin(Display.AreaDev *M_PI / 180) + Boss.GetCenterPosition().y;
									Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								}
								else {
									tempPoint.x = 4 * differ.x;
									tempPoint.y = 4 * differ.y;
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								}
								a += 60;
							}
							else if (Boss.AttackPhase == 16){
								if (a < 720) {
									tempPoint.x = 3.000 * cos((-1 + (double)(a + 30) / 360) * M_PI);
									tempPoint.y = 3.000 * sin((-1 + (double)(a + 30) / 360) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								}
								else {
									tempPoint.x = 4 * differ.x;
									tempPoint.y = 4 * differ.y;
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								}
								a += 60;
							}
							else if (Boss.AttackPhase == 24){
								if (a < 720){
									tempPoint.x = 3.000 * cos((-1 + (double)a / 360) * M_PI);
									tempPoint.y = 3.000 * sin((-1 + (double)a / 360) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									tempPoint.x = -32 * sin(Display.AreaDev* M_PI / 90) + Boss.GetCenterPosition().x;
									tempPoint.y = -32 * sin(Display.AreaDev *M_PI / 180) + Boss.GetCenterPosition().y;
									Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								}
								else {
									tempPoint.x = 4 * differ.x;
									tempPoint.y = 4 * differ.y;
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								}
								a += 60;
							}
							else {
								tempPoint.x = 4 * differ.x;
								tempPoint.y = 4 * differ.y;
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								a += 780;
							}
							if (a >= 780){
								if (Boss.AttackPhase >= 31) {
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(300);
								}
								else {
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(300);
								}
								break;
							}
						}
					}
				}
			}
			else if (Boss.GetHealth() >= HD_BOSS2_HEALTH / 4 && Boss.GetHealth() < HD_BOSS2_HEALTH / 2){
				if (boss_phase < 4){
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						Bullet[j].existence = 0;
					}
					bossTimer = HD_Timer();
					bossTimer.startTimer(300);
					Boss.attackTimer = HD_Timer();
					Boss.attackTimer.startTimer(600);
					Boss.AttackPhase = 0;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					boss_phase = 4;
				}
				else {
					if (bossTimer.checkStart() && bossTimer.checkTimer()){
						if (boss_phase < 5) {
							boss_phase = 5;
							Display.DisplayReset();
						}
						else Display.Spin180();
					}
					else{
						Display.Shake();
					}
				}
				// Shoot
				if (bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					srand(SDL_GetTicks() + 5);
					probability = rand() % 2;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							switch (Boss.AttackPhase){
							case 0:
							case 2:
							case 4:
							case 6:
							case 8:
							case 10:
							case 12:
							case 14:
								if (a < 240){
									tempPoint.x = 2.000 * cos(((double)Boss.AttackPhase*22.5 + (double)a / 4) * M_PI / 180);
									tempPoint.y = 2.000 * sin(((double)Boss.AttackPhase*22.5 + (double)a / 8) * M_PI / 180);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								else if (a >= 240 && a < 480) {
									tempPoint.x = 2.000 * cos((((double)Boss.AttackPhase*22.5 + (double)a / 4) - 180) * M_PI / 180);
									tempPoint.y = 2.000 * sin((((double)Boss.AttackPhase*22.5 + (double)a / 4) - 180) * M_PI / 180);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								else {
									a += 240;
								}
								tempPoint = Boss.GetCenterPosition();
								Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								a += 12;
								break;
							case 20:
							case 22:
							case 24:
							case 26:
							case 28:
							case 30:
							case 32:
							case 34:
								tempPoint.x = 2.000 * cos(((double)(Boss.AttackPhase - 20)*22.5 - (double)a / 16) * M_PI / 180);
								tempPoint.y = 2.000 * sin(((double)(Boss.AttackPhase - 20)*22.5 - (double)a / 16) * M_PI / 180);
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								tempPoint = Boss.GetCenterPosition();
								Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								a += 48;
								break;
							case 16:
							case 17:
							case 18:
							case 19:
							case 36:
							case 37:
							case 38:
							case 39:
							case 40:
								srand(SDL_GetTicks() + 5);
								if (rand() % 2 != 0) {
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetCenterPosition(a * 640 / 720, 0.0);
									switch (probability){
									case 0:
										Bullet[j].SetDirection(4 * cos(7 * M_PI / 16), 4);
										break;
									case 1:
										Bullet[j].SetDirection(4 * cos(9 * M_PI / 16), 4);
										break;
									default:
										Bullet[j].SetDirection(0, 4.0 * cos(3));
									}
								}
								a += 144;
								break;
							default:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint = Boss.GetCenterPosition();
								Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								differ.x *= 4.0;
								differ.y *= 4.0;
								Bullet[j].SetDirection(differ.x, differ.y);
								a += 720;
								break;
							}
							if (a >= 720){
								if (Boss.AttackPhase >= 0 && Boss.AttackPhase < 15){
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(1000);
								}
								else if (Boss.AttackPhase == 15 || Boss.AttackPhase == 35){
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
								}
								else if (Boss.AttackPhase == 40) {
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(750);
								}
								else if (Boss.AttackPhase == 20) {
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(750);
								}
								else {
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(750);
								}
								break;
							}
						}
					}
				}
				// ----end----
			}
			else if (Boss.GetHealth() > 0 && Boss.GetHealth() < HD_BOSS2_HEALTH / 4){
				//last pattern
				//move and change display
				if (boss_phase < 6){
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						Bullet[j].existence = 0;
					}
					bossTimer = HD_Timer();
					bossTimer.startTimer(300);
					Boss.attackTimer = HD_Timer();
					Boss.attackTimer.startTimer(600);
					Boss.AttackPhase = 0;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					boss_phase = 6;
				}
				else {
					if (bossTimer.checkStart() && bossTimer.checkTimer()){
						if (boss_phase < 7) {
							boss_phase = 7;
							Display.DisplayReset();
						}
						else {
							Display.Spin();
						};
					}
					else{
						Display.Shake();
					}
				}
				//shoot
				if (bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							if (Boss.AttackPhase % 30 == 0){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								if (a < 360) {
									Bullet[j].SetDirection(2 * cos((((double)Boss.AttackPhase / 360) + 0.5) * M_PI), 2 * sin((((double)Boss.AttackPhase / 360) + 0.5) * M_PI));
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 80, Boss.GetCenterPosition().y);
								}
								else {
									Bullet[j].SetDirection(2 * cos((-((double)Boss.AttackPhase / 360) + 0.5) * M_PI), 2 * sin((-((double)Boss.AttackPhase / 360) + 0.5) * M_PI));
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 80, Boss.GetCenterPosition().y);
								}
								a += 360;
							}
							else if (Boss.AttackPhase % 82 == 0 ){
								switch (a){
								case 0:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 3.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
									break;
								case 120:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 3.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 64 * differ.y + 2 * differ.x, Boss.GetCenterPosition().y + 64 * differ.x + 2 * differ.y);
									break;
								case 240:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 3.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 32 * differ.y + 2 * differ.x, Boss.GetCenterPosition().y + 32 * differ.x + 2 * differ.y);
									break;
								case 360:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 3.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 64 * differ.y + 2 * differ.x, Boss.GetCenterPosition().y - 32 * differ.x + 2 * differ.y);
									break;
								case 480:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 3.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 32 * differ.y + 2 * differ.x, Boss.GetCenterPosition().y - 64 * differ.x + 2 * differ.y);
									break;
								case 600:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(5.0* differ.x, 5.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
									break;
								default:
									a += 720;
									break;
								}
								a += 120;
							}
							else{
								a += 720;
							}
							if (a >= 720){
								if (Boss.AttackPhase < 360){
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(10);
								}
								else {
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(10);
								}
								break;
							}
						}
					}
				}
				// ---- end
			}
			else if (Boss.GetHealth() == 0) {
				if (boss_phase < 8){
					Display.DisplayReset();
					boss_phase = 8;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					Boss.AttackPhase = 0;
				}
			}
			break;
		case 6:
			//screen changes & bullet shooting
			if (Boss.GetHealth() >= HD_BOSS1_HEALTH * 3 / 4){
				//move
				if (Boss.MovementPhase == 0 && bossTimer.checkStart() && bossTimer.checkTimer()){ //phase 0
					if (Boss.GetCenterPosition().y < 150) {
						tempPoint= Boss.GetCenterPosition();
						tempPoint.y += Boss.GetDirection().y;
						Boss.SetCenterPosition(tempPoint.x, tempPoint.y);
						Display.Shake();
					}
					else {
						Boss.MovementPhase = 1;
						boss_phase = 1;
						Display.DisplayReset();
					}
				}
				else if (Boss.MovementPhase == 0 && bossTimer.checkStart() && !bossTimer.checkTimer()){ //phase 0
					boss_phase = 0;
				}
				else Display.HorizontalShrink(30, 3);
				//shoot
				if (bossTimer.checkStart() && bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							if (Boss.AttackPhase == 0){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint.x = (double)a;
								Bullet[j].SetCenterPosition(tempPoint.x, 0.0);
								Bullet[j].SetDirection(0, 4.0);
								a += 80;
							}
							else if (Boss.AttackPhase == 2){
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint.x = (double)a + 40;
								Bullet[j].SetCenterPosition(tempPoint.x, 0);
								Bullet[j].SetDirection(0, 4);
								a += 80;
							}
							else {
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint = Boss.GetCenterPosition();
								Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								differ.x *= 4.0;
								differ.y *= 4.0;
								Bullet[j].SetDirection(differ.x, differ.y);
								a = 640;
							}
							if (a >= 640){
								if (Boss.AttackPhase != 3){
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(750);
								}
								else {
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(750);
								}
								break;
							}
						}
					}
				}
			}
			else if (Boss.GetHealth() >= HD_BOSS1_HEALTH / 2 && Boss.GetHealth() < HD_BOSS1_HEALTH * 3 / 4) {
				// phase change
				if (boss_phase < 2){
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						Bullet[j].existence = 0;
					}
					bossTimer = HD_Timer();
					bossTimer.startTimer(300);
					Boss.attackTimer = HD_Timer();
					Boss.attackTimer.startTimer(600);
					Boss.AttackPhase = 0;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					boss_phase = 2;
				}
				else {
					if (bossTimer.checkStart() && bossTimer.checkTimer()){
						if (boss_phase < 3) {
							boss_phase = 3;
							Display.DisplayReset();
						}
						else Display.HorizontalMove();
					}
					else{
						Display.Shake();
					}
				}
				//shoot
				if (bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							if (Boss.AttackPhase == 0){
								tempPoint.x = 4.000 * cos((-1 + (double)a / 360) * M_PI);
								tempPoint.y = 4.000 * sin((-1 + (double)a / 360) * M_PI);
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								a += 30;
								Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
							}
							else if (Boss.AttackPhase == 4){
								if (a >= 0 && a < 360){
									tempPoint.x = 4.000 * cos((-1 + (double)(a + 15) / 360) * M_PI);
									tempPoint.y = 4.000 * sin((-1 + (double)(a + 15) / 360) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								else {
									tempPoint.x = 4.000 * cos((-1 + (double)(a + 15) / 360) * M_PI);
									tempPoint.y = 4.000 * sin((-1 + (double)(a + 15) / 360) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								a += 30;
								Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
							}
							else {
								tempPoint.x = 4 * differ.x ;
								tempPoint.y = 4 * differ.y ;
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								if (a < 360) Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 8 * differ.y , Boss.GetCenterPosition().y + 8 * differ.x );
								else Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 8 * differ.y, Boss.GetCenterPosition().y - 8 * differ.x);
								a += 360;
							}
							if (a >= 720){
								if (Boss.AttackPhase == 7) {
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
								}
								else {
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
								}
								break;
							}
						}
					}
				}
			}
			else if (Boss.GetHealth() >= HD_BOSS1_HEALTH / 4 && Boss.GetHealth() < HD_BOSS1_HEALTH / 2){
				if (boss_phase < 4){
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						Bullet[j].existence = 0;
					}
					bossTimer = HD_Timer();
					bossTimer.startTimer(300);
					Boss.attackTimer = HD_Timer();
					Boss.attackTimer.startTimer(600);
					Boss.AttackPhase = 0;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					boss_phase = 4;
				}
				else {
					if (bossTimer.checkStart() && bossTimer.checkTimer()){
						if (boss_phase < 5) {
							boss_phase = 5;
							Display.DisplayReset();
						}
						else Display.HorizontalShrink(80, 5);
					}
					else{
						Display.Shake();
					}
				}
				// Shoot
				if (bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							if (Boss.AttackPhase == 0){
								randomNum = rand() % 60;
								if (a >= 0 && a < 360){
									tempPoint.x = 4.000 * cos((-1 + (double)randomNum / 180) * M_PI);
									tempPoint.y = 4.000 * sin((-1 + (double)randomNum / 180) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 5, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								else {
									tempPoint.x = 4.000 * cos((1 - (double)randomNum / 180) * M_PI);
									tempPoint.y = 4.000 * sin((1 - (double)randomNum / 180) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 5, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								a += 60;
							}
							else if (Boss.AttackPhase == 1){
								randomNum = rand() % 60;
								if (a >= 0 && a < 360){
									tempPoint.x = 4.000 * cos(((double)randomNum / 180) * M_PI);
									tempPoint.y = 4.000 * sin(((double)randomNum / 180) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 5, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								else {
									tempPoint.x = 4.000 * cos(((double)-randomNum / 180) * M_PI);
									tempPoint.y = 4.000 * sin(((double)-randomNum / 180) * M_PI);
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 5, 0, 2);
									Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								}
								a += 60;
							}
							else {
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint = Boss.GetCenterPosition();
								Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								differ.x *= 4.0;
								differ.y *= 4.0;
								Bullet[j].SetDirection(differ.x, differ.y);
								a = 720;
							}
							tempPoint = Boss.GetCenterPosition();
							Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
							if (a >= 720){
								if (Boss.AttackPhase == 0){
									Boss.AttackPhase = 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(1000);
								}
								else if (Boss.AttackPhase == 1){
									Boss.AttackPhase = 2;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
								}
								else {
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
								}
								break;
							}
						}
					}
				}
				// ----end----
			}
			else if (Boss.GetHealth() > 0 && Boss.GetHealth() < HD_BOSS1_HEALTH / 4){ 
				//last pattern
				//move and change display
				if (boss_phase < 6){
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						Bullet[j].existence = 0;
					}
					bossTimer = HD_Timer();
					bossTimer.startTimer(300);
					Boss.attackTimer = HD_Timer();
					Boss.attackTimer.startTimer(600);
					Boss.AttackPhase = 0;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					boss_phase = 6;
				}
				else {
					if (bossTimer.checkStart() && bossTimer.checkTimer()){
						if (boss_phase < 7) {
							boss_phase = 7;
							Display.DisplayReset();
						}
						else {
							Display.Swing();
						};
					}
					else{
						Display.Shake();
					}
				}
				//shoot
				if (bossTimer.checkTimer() && Boss.attackTimer.checkTimer()){
					int a = 0;
					for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++){
						if (Bullet[j].existence == 0){
							switch (Boss.AttackPhase){
							case 0:
								tempPoint.x = 4.000 * cos((-1 + (double)a / 360) * M_PI);
								tempPoint.y = 4.000 * sin((-1 + (double)a / 360) * M_PI);
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								a += 30;
								Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								break;
							case 2:
								tempPoint.x = 4.000 * cos((-1 + (double)(a+10) / 360) * M_PI);
								tempPoint.y = 4.000 * sin((-1 + (double)(a+10) / 360) * M_PI);
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								a += 30;
								Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								break;
							case 4:
								tempPoint.x = 4.000 * cos((-1 + ((double)a + 20) / 360) * M_PI);
								tempPoint.y = 4.000 * sin((-1 + ((double)a + 20) / 360) * M_PI);
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								Bullet[j].SetDirection(tempPoint.x, tempPoint.y);
								a += 30;
								Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
								break;
							case 6:
							case 8:
								switch (a){
								case 0:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(5.0* differ.x, 5.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
									break;
								case 120:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(4.0* differ.x, 4.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 32 * differ.y, Boss.GetCenterPosition().y + 32 * differ.x);
									break;
								case 240:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 3.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 32 * differ.y, Boss.GetCenterPosition().y + 32 * differ.x);
									break;
								case 360:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(4.0* differ.x, 4.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 32 * differ.y, Boss.GetCenterPosition().y - 32 * differ.x);
									break;
								case 480:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 3.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 32 * differ.y, Boss.GetCenterPosition().y - 32 * differ.x);
									break;
								case 600:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									Bullet[j].SetDirection(3.0* differ.x, 4.0*differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
									break;
								default:
									Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
									tempPoint = Boss.GetCenterPosition();
									Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
									differ.x *= 4.0;
									differ.y *= 4.0;
									Bullet[j].SetDirection(differ.x, differ.y);
									Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x, Boss.GetCenterPosition().y);
									break;
								}
								a += 120;
								break;
							case 1:
							case 3:
							case 5:
							case 7:
							case 9:
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet1.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								if (a <360) Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x - 64, Boss.GetCenterPosition().y - 80);
								else Bullet[j].SetCenterPosition(Boss.GetCenterPosition().x + 64, Boss.GetCenterPosition().y -80);
								Bullet[j].SetDirection(0, 6);
								a += 360;
								break;
							default: // case 10
								Bullet[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Bullet2.png", { 0, 0, 32, 32 }, { 0, 0, 32, 32 }, 2, 0, 0, 2);
								tempPoint = Boss.GetCenterPosition();
								Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								differ.x *= 4.0;
								differ.y *= 4.0;
								Bullet[j].SetDirection(differ.x, differ.y);
								a = 720;
								tempPoint = Boss.GetCenterPosition();
								Bullet[j].SetCenterPosition(tempPoint.x, tempPoint.y);
								break;
							}
							if (a >= 720){
								switch (Boss.AttackPhase){
								case 0:
									Boss.AttackPhase = 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								case 1:
									Boss.AttackPhase = 2;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								case 2:
									Boss.AttackPhase = 3;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								case 4:
								case 8:
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								case 3:
									Boss.AttackPhase = 4;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								case 5:
								case 7:
								case 9:
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								case 6:
									Boss.AttackPhase += 1;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								default:
									Boss.AttackPhase = 0;
									Boss.attackTimer = HD_Timer();
									Boss.attackTimer.startTimer(500);
									break;
								}
								break;
							}
						}
					}
				}
				// ---- end
			}
			else if (Boss.GetHealth() == 0) {
				if (boss_phase < 8){
					Display.DisplayReset();
					boss_phase = 8;
					if (SFXChannel_Enemy != -1) Mix_HaltChannel(SFXChannel_Enemy);
					if (EnemySFX != NULL){
						Mix_FreeChunk(EnemySFX);
						EnemySFX = NULL;
					}
					EnemySFX = Mix_LoadWAV("resource/Object_destruction.wav");
					if (EnemySFX != NULL) SFXChannel_Enemy = Mix_PlayChannel(-1, EnemySFX, 0);
					Boss.AttackPhase = 0;
				}
			}
			break;
		default:
			//ignore
			break;
		}
		//manage randomly spawned enemy
		if (HD_gameSys.spawnTimer.checkTimer()){
			srand(SDL_GetTicks());
			probability = rand() % 100;
			srand(SDL_GetTicks());
			randomNum = rand() % 10;
			if (probability >= 96){
				spawnEnemy_Process(102, randomNum);
			}
			else if (probability >= 50 && probability < 96){
				spawnEnemy_Process(21, randomNum);
			}
			else if (probability < 5){
				spawnEnemy_Process(65, randomNum);
			}
			else if (probability >= 5 && probability < 20){
				spawnEnemy_Process(27, randomNum);
			}
			else {
				spawnEnemy_Process(25, randomNum);
			}
			HD_gameSys.spawnTimer = HD_Timer();
			HD_gameSys.spawnTimer.startTimer(6000);
		}
	}
	if (!Boss.DestroyTimer.checkTimer() && Boss.existence != 0 && Boss.GetHealth() == 0){
		for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++) Bullet[j].existence = 0;
		Boss.existence = 0;
		HD_gameSys.addScore(100000 * HD_gameSys.stageNum);
		scene++;
	}
	else if (Boss.DestroyTimer.checkTimer()){
		for (j = 0; j < HD_NUMBER_OF_ENEMY_BULLET; j++) Bullet[j].existence = 0;
		Boss.existence = 0;
		scene++;
	}
	return true;
}
void HD_SYSTEM::managePlayer(){ // chargeshot -> normal shot -> subweapon shot -> player vs enemy collision -> player vs enemy bullet colision -> player & relavant object position
	int i, j, a= 0;
	if (!Player.spacePressed && (Player.chargeGauge > 0)) { // space control
		if (Player.chargeGauge == HD_MAX_CHARGE_GAUGE && Player.health >= 2){
			//spawn charge shot
			Player.chargeShot.area = { 0, 0, 32, 32 };
			Player.chargeShot.dstarea = { 0, 0, 32, 32 };
			Player.chargeShot.SetPosition(Player.dstarea.x + (Player.area.w / 2) - (Player.chargeShot.area.w / 2), Player.dstarea.y + 8);
			Player.chargeShot.SetDirection(0, -32);
			Player.chargeShot.existence = 2;
			if (Player.SFXChannel != -1) Mix_HaltChannel(Player.SFXChannel);
			if (Player.PlayerSFX != NULL){
				Mix_FreeChunk(Player.PlayerSFX);
				Player.PlayerSFX = NULL;
			}
			Player.PlayerSFX = Mix_LoadWAV("resource/chargeShot_effect.wav");
			if (Player.PlayerSFX != NULL) Player.SFXChannel = Mix_PlayChannel(-1,Player.PlayerSFX,0);
		}
		Player.chargeGauge = 0;
		Player.attackTimer = HD_Timer();
		Player.attackTimer.startTimer(1000);
		for (i = 0; i < 4; i++){
			Player.subweapon[i].attackTimer = HD_Timer();
			Player.subweapon[i].attackTimer.startTimer(1000);
		}
	}
	else if (!Player.spacePressed && (scene == 5 || scene == 6|| scene == 10 || scene == 11)){
		if (Player.attackTimer.checkTimer()){
			a = 0;
			for (i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
				if (Player.bullet[i].existence == 0) {
					Player.bullet[i].angle = 0;
					Player.bullet[i].existence = 2;
					Player.bullet[i].area = {0, 0, 32, 64};
					Player.bullet[i].dstarea = { 0, 0, 32, 64 };
					Player.bullet[i].SetPosition(Player.dstarea.x + a * 32, Player.dstarea.y);
					Player.bullet[i].SetDirection(0, -48);
					Player.bullet[i].AttackPhase = 0;
					Player.bullet[i].SetAniType(0, -1);
					if (a >= 1) {
						break;
					}
					else {
						a++;
					}
				}
			}
			Player.attackTimer = HD_Timer();
			Player.attackTimer.startTimer(250);
		}
	}
	//subweapon management
	for (j = 0; j < 4; j++){
		if (Player.subweapon[j].existence != 0){
			if (j % 2 == 0) {
				Player.subweapon[j].SetCenterPosition(Player.GetCenterPosition().x + 64, Player.dstarea.y + 32 + 32 * (j / 2));
			}
			else {
				Player.subweapon[j].SetCenterPosition(Player.GetCenterPosition().x - 32, Player.dstarea.y + 32 + 32 * (j / 2));
			}
			if (Player.spacePressed == false && Player.subweapon[j].attackTimer.checkTimer()){
				a = 0;
				for (i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
					if (Player.bullet[i].existence == 0) {
						switch (Player.subweapon[j].attackType){
						case 21:
						case 22:
						case 31:
						case 33:
						case 34:
						case 35:
						case 36:
						case 37:
						case 38:
						case 39:
						case 40:
						case 66:
						case 67:
						case 70:
						case 71:
							Player.bullet[i].SetObjectAttribute(HD_renderer, false, "resource/HD_NormalShot.png", { 0, 0, 32, 64 }, { 0, 0, 32, 64 }, 2, 0, -1, 0);
							Player.bullet[i].SetCenterPosition(Player.subweapon[j].dstarea.x + 16, Player.subweapon[j].dstarea.y);
							Player.bullet[i].angle = 0;
							Player.bullet[i].SetDirection(0, -48);
							a += 720;
							break;
						case 23:
						case 41:
						case 42:
							Player.bullet[i].SetObjectAttribute(HD_renderer, false, "resource/HD_NormalShot.png", { 0, 0, 32, 64 }, { 0, 0, 32, 64 }, 2, 0, -1, 0);
							Player.bullet[i].SetCenterPosition(Player.subweapon[j].dstarea.x + 16, Player.subweapon[j].dstarea.y);
							switch (a) {
							case 0:
								Player.bullet[i].SetDirection(24 * cos(-M_PI / 2), 24 * sin(-M_PI / 2));
								break;
							case 90:
								Player.bullet[i].SetDirection(24 * cos(-M_PI / 4), 24 * sin(-M_PI / 4));
								break;
							case 180:
								Player.bullet[i].SetDirection(24 * cos(0), 24 * sin(0));
								break;
							case 270:
								Player.bullet[i].SetDirection(24 * cos(M_PI / 4), 24 * sin(M_PI / 4));
								break;
							case 360:
								Player.bullet[i].SetDirection(24 * cos(M_PI / 2), 24 * sin(M_PI / 2));
								break;
							case 450:
								Player.bullet[i].SetDirection(24 * cos(3* M_PI / 4), 24 * sin(3* M_PI / 4));
								break;
							case 540:
								Player.bullet[i].SetDirection(24 * cos(M_PI), 24 * sin(M_PI));
								break;
							case 630:
								Player.bullet[i].SetDirection(24 * cos(-3 * M_PI / 4), 24 * sin(-3 * M_PI / 4));
								break;
							default:
								a = 720;
							}
							Player.bullet[i].angle = a/2;
							a += 90;
							break;
						case 25:
						case 26:
							Player.bullet[i].SetObjectAttribute(HD_renderer, false, "resource/HD_NormalShot.png", { 0, 0, 32, 64 }, { 0, 0, 32, 64 }, 2, 0, -1, 0);
							Player.bullet[i].SetCenterPosition(Player.subweapon[j].dstarea.x + 16, Player.subweapon[j].dstarea.y);
							switch (a) {
							case 0:
								Player.bullet[i].SetDirection(24 * cos(-M_PI *3/ 4), 24 * sin(-M_PI*3 / 4));
								Player.bullet[i].angle = -45;
								break;
							case 240:
								Player.bullet[i].SetDirection(24 * cos(-M_PI / 2), 24 * sin(-M_PI / 2));
								Player.bullet[i].angle = 0;
								break;
							case 480:
								Player.bullet[i].SetDirection(24 * cos(-M_PI / 4), 24 * sin(-M_PI / 4));
								Player.bullet[i].angle = 45;
								break;
							default:
								a = 720;
							}
							a += 240;
							break;
						case 27:
							Player.bullet[i].SetObjectAttribute(HD_renderer, false, "resource/HD_NormalShot.png", { 0, 0, 32, 64 }, { 0, 0, 32, 64 }, 2, 0, -1, 0);
							Player.bullet[i].SetCenterPosition(Player.subweapon[j].dstarea.x + 16, Player.subweapon[j].dstarea.y);
							if (Player.subweapon[j].AttackPhase != 0){
								switch (a) {
								case 0:
									Player.bullet[i].SetDirection(24 * cos(-M_PI / 2), 24 * sin(-M_PI / 2));
									break;
								case 180:
									Player.bullet[i].SetDirection(24 * cos(0), 24 * sin(0));
									break;
								case 360:
									Player.bullet[i].SetDirection(24 * cos(M_PI / 2), 24 * sin(M_PI / 2));
									break;
								case 540:
									Player.bullet[i].SetDirection(24 * cos(M_PI), 24 * sin(M_PI));
									break;
								default:
									a = 720;
								}
								Player.bullet[i].angle = a / 2;
							}
							else {
								switch (a) {
								case 0:
									Player.bullet[i].SetDirection(24 * cos(-M_PI / 4), 24 * sin(-M_PI / 4));
									break;
								case 180:
									Player.bullet[i].SetDirection(24 * cos(M_PI / 4), 24 * sin(M_PI / 4));
									break;
								case 360:
									Player.bullet[i].SetDirection(24 * cos(3 * M_PI / 4), 24 * sin(3 * M_PI / 4));
									break;
								case 540:
									Player.bullet[i].SetDirection(24 * cos(-3 * M_PI / 4), 24 * sin(-3 * M_PI / 4));
									break;
								default:
									a = 720;
								}
								Player.bullet[i].angle = a / 2 + 45;
							}
							a += 180;
							break;
						case 50:
							Player.bullet[i].SetObjectAttribute(HD_renderer, false, "resource/HD_NormalShot.png", { 0, 0, 32, 64 }, { 0, 0, 32, 64 }, 2, 0, -1, 0);
							Player.bullet[i].SetCenterPosition(Player.subweapon[j].dstarea.x + 16, Player.subweapon[j].dstarea.y);
							switch (a) {
							case 0:
								Player.bullet[i].SetDirection(24 * cos(-M_PI / 2), 24 * sin(-M_PI / 2));
								break;
							case 180:
								Player.bullet[i].SetDirection(24 * cos(0), 24 * sin(0));
								break;
							case 360:
								Player.bullet[i].SetDirection(24 * cos(M_PI / 2), 24 * sin(M_PI / 2));
								break;
							case 540:
								Player.bullet[i].SetDirection(24 * cos(M_PI), 24 * sin(M_PI));
								break;
							default:
								a = 720;
							}
							Player.bullet[i].angle = a / 2;
							a += 180;
							break;
						case 52:
						case 56:
							Player.bullet[i].SetObjectAttribute(HD_renderer, false, "resource/HD_NormalShot.png", { 0, 0, 32, 64 }, { 0, 0, 32, 64 }, 2, 0, -1, 0);
							Player.bullet[i].SetCenterPosition(Player.subweapon[j].dstarea.x + 16, Player.subweapon[j].dstarea.y);
							switch (a) {
							case 0:
								Player.bullet[i].SetDirection(24 * cos(-M_PI / 4), 24 * sin(-M_PI / 4));
								break;
							case 180:
								Player.bullet[i].SetDirection(24 * cos(M_PI / 4), 24 * sin(M_PI / 4));
								break;
							case 360:
								Player.bullet[i].SetDirection(24 * cos(3 * M_PI / 4), 24 * sin(3 * M_PI / 4));
								break;
							case 540:
								Player.bullet[i].SetDirection(24 * cos(-3 * M_PI / 4), 24 * sin(-3 * M_PI / 4));
								break;
							default:
								a = 720;
							}
							Player.bullet[i].angle = a / 2 + 45;
							a += 180;
							break;
						default:
							Player.bullet[i].SetObjectAttribute(HD_renderer, false, "resource/HD_NormalShot.png", { 0, 0, 32, 64 }, { 0, 0, 32, 64 }, 2, 0, -1, 0);
							Player.bullet[i].SetCenterPosition(Player.subweapon[j].dstarea.x + 16, Player.subweapon[j].dstarea.y);
							Player.bullet[i].SetDirection(0, -48);
							a += 360;
							break;
						}
						if (a >= 720) {
							switch (Player.subweapon[j].attackType){
							case 21:
							case 22:
							case 33:
							case 34:
							case 35:
							case 36:
							case 37:
							case 38:
							case 39:
							case 40:
							case 66:
							case 67:
							case 70:
								Player.subweapon[j].attackTimer = HD_Timer();
								Player.subweapon[j].attackTimer.startTimer(250);
								break;
							case 27:
							case 28:
								Player.subweapon[j].attackTimer = HD_Timer();
								Player.subweapon[j].attackTimer.startTimer(750);
								break;
							case 23:
							case 41:
							case 42:
								Player.subweapon[j].attackTimer = HD_Timer();
								Player.subweapon[j].attackTimer.startTimer(1000);
								break;
							case 25:
							case 26:
								if (Player.subweapon[j].AttackPhase != 0){
									Player.subweapon[j].attackTimer = HD_Timer();
									Player.subweapon[j].attackTimer.startTimer(500);
									Player.subweapon[j].AttackPhase = 0;
								}
								else {
									Player.subweapon[j].attackTimer = HD_Timer();
									Player.subweapon[j].attackTimer.startTimer(1000);
									Player.subweapon[j].AttackPhase = 1;
								}
								break;
							default:
								Player.subweapon[j].attackTimer = HD_Timer();
								Player.subweapon[j].attackTimer.startTimer(500);
								break;
							}
							break;
						}
					}
				}
			}
		}
	}
	for (i = 0; i < HD_NUMBER_OF_ENEMY; i++){
		//Player's bullet vs enemy collision
		for (j = 0; j < HD_NUMBER_OF_MY_BULLET; j++){
			if (CollisionCheck(Enemy[i], Player.bullet[j])){
				if (Enemy[i].attackType == 101 || Enemy[i].attackType == 102) {
					continue; // ignore the collision check
				}
				else{
					if (Enemy[i].GetHealth() >0 && Enemy[i].GetHealth() >= HD_PLAYER_ATTACK_POWER)
						Enemy[i].SetHealth(Enemy[i].GetHealth() - HD_PLAYER_ATTACK_POWER);
					else if (Enemy[i].GetHealth() < 0) //immortal
						Enemy[i].SetHealth(Enemy[i].GetHealth());
					else // Enemy[i].Gethealth == 0 ||(Enemy[i].GetHealth >0 && Enemy[i].GetHealth() < HD_PLAYER_ATTACK_POWER)
						Enemy[i].SetHealth(0); // bug prevention -- since negative hp == immortal --
					Player.bullet[j].existence = 0;
				}
			}
		}
		//Player's charged bullet vs enemy collision
		if (CollisionCheck(Enemy[i], Player.chargeShot) && Player.health >= 2){
			bool allocated = false;
			switch (Enemy[i].attackType){
			case 21:
			case 25:
			case 29:
			case 31:
			case 45:
			case 46:
				for (j = 0; j < 4; j++){
					if (Player.subweapon != NULL && Player.subweapon[j].existence == 0){
						Player.subweapon[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Enemy_s1.png", { 0, 0, 64, 64 }, { 0, 0, 64, 64 }, 2, Enemy[i].attackType, 11, 0);
						Player.subweapon[j].SetDirection(Player.GetDirection().x, Player.GetDirection().y);
						if (j % 2 == 0) {
							Player.subweapon[j].SetCenterPosition(Player.dstarea.x + Player.area.w, Player.dstarea.y + 32 + 32 * j / 2);
						}
						else {
							Player.subweapon[j].SetCenterPosition(Player.dstarea.x, Player.dstarea.y + 32 + 32 * j / 2);
						}
						Player.subweapon[j].attackTimer = HD_Timer();
						Player.subweapon[j].attackTimer.startTimer(250);
						Player.subweapon[j].DestroyTimer = HD_Timer();
						Player.subweapon[j].DestroyTimer.startTimer(10000);
						allocated = true;
						break;
					}
				}
				Enemy[i].existence = 0;
				Player.chargeShot.existence = 0;
				if (Player.subwpnorder < 3)Player.subwpnorder++;
				Player.health--; // decrease player health
				break;
			case 22:
			case 34:
			case 36:
			case 41:
			case 42:
			case 66:
			case 67:
			case 70:
			case 71:
				for (j = 0; j < 4; j++){
					if (Player.subweapon != NULL && Player.subweapon[j].existence == 0){
						Player.subweapon[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Enemy_m2.png", { 0, 0, 96, 64 }, { 0, 0, 96, 64 }, 2, Enemy[i].attackType, 11, 0);
						Player.subweapon[j].SetDirection(Player.GetDirection().x, Player.GetDirection().y);
						if (j % 2 == 0) {
							Player.subweapon[j].SetPosition(Player.dstarea.x + Player.area.w, Player.dstarea.y + 32 + 32 * j / 2);
						}
						else {
							Player.subweapon[j].SetPosition(Player.dstarea.x, Player.dstarea.y + 32 + 32 * j / 2);
						}
						Player.subweapon[j].existence = 2;
						Player.subweapon[j].attackTimer = HD_Timer();
						Player.subweapon[j].attackTimer.startTimer(750);
						Player.subweapon[j].DestroyTimer = HD_Timer();
						Player.subweapon[j].DestroyTimer.startTimer(10000);
						allocated = true;
						break;
					}
				}
				Enemy[i].existence = 0;
				Player.chargeShot.existence = 0;
				if (Player.subwpnorder < 3)Player.subwpnorder++;
				Player.health--; // decrease player health
				break;
			case 23:
			case 26:
			case 27:
			case 32:
			case 47:
			case 50:
			case 52:
			case 54:
			case 56:
				for (j = 0; j < 4; j++){
					if (Player.subweapon != NULL && Player.subweapon[j].existence == 0){
						Player.subweapon[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Enemy_m1.png", { 0, 0, 96, 64 }, { 0, 0, 96, 64 }, 2, Enemy[i].attackType, 11, 0);
						Player.subweapon[j].SetDirection(Player.GetDirection().x, Player.GetDirection().y);
						if (j % 2 == 0) {
							Player.subweapon[j].SetPosition(Player.dstarea.x + Player.area.w, Player.dstarea.y + 32 + 32 * j / 2);
						}
						else {
							Player.subweapon[j].SetPosition(Player.dstarea.x, Player.dstarea.y + 32 + 32 * j / 2);
						}
						Player.subweapon[j].existence = 2;
						Player.subweapon[j].attackTimer = HD_Timer();
						Player.subweapon[j].attackTimer.startTimer(750);
						Player.subweapon[j].DestroyTimer = HD_Timer();
						Player.subweapon[j].DestroyTimer.startTimer(10000);
						allocated = true;
						break;
					}
				}
				Enemy[i].existence = 0;
				Player.chargeShot.existence = 0;
				if (Player.subwpnorder < 3)Player.subwpnorder++;
				Player.health--; // decrease player health
				break;
			case 33:
			case 35:
			case 37:
			case 38:
			case 39:
			case 40:
				for (j = 0; j < 4; j++){
					if (Player.subweapon != NULL && Player.subweapon[j].existence == 0){
						Player.subweapon[j].SetObjectAttribute(HD_renderer, false, "resource/HD_Enemy_s2.png", { 0, 0, 64, 64 }, { 0, 0, 64, 64 }, 2, Enemy[i].attackType, 10,2);
						Player.subweapon[j].SetDirection(Player.GetDirection().x, Player.GetDirection().y);
						if (j % 2 == 0) {
							Player.subweapon[j].SetCenterPosition(Player.dstarea.x + Player.area.w, Player.dstarea.y + 32 + 32 * j / 2);
						}
						else {
							Player.subweapon[j].SetCenterPosition(Player.dstarea.x, Player.dstarea.y + 32 + 32 * j / 2);
						}
						Player.subweapon[j].attackTimer = HD_Timer();
						Player.subweapon[j].attackTimer.startTimer(250);
						Player.subweapon[j].DestroyTimer = HD_Timer();
						Player.subweapon[j].DestroyTimer.startTimer(10000);
						allocated = true;
						break;
					}
				}
				Enemy[i].existence = 0;
				Player.chargeShot.existence = 0;
				if (Player.subwpnorder < 3)Player.subwpnorder++;
				Player.health--; // decrease player health
				break;
			default:
				break;
			}
		}
		if (CollisionCheck(Player, Enemy[i])){
			//map item collision
			if (Enemy[i].attackType == 101){
				Player.dstarea.x = 2 * (Display.GetGameCenter().x - HD_INGAME_DISPLAY_CORRECTION_X) - Player.dstarea.x - Player.area.w;
				if (Display.flip){
					Display.flip = false;
				}
				else {
					Display.flip = true;
				}
				HD_gameSys.score += 1000;
				Enemy[i].existence = 0;
				if (Player.SFXChannel != -1) Mix_HaltChannel(Player.SFXChannel);
				if (Player.PlayerSFX != NULL){
					Mix_FreeChunk(Player.PlayerSFX);
					Player.PlayerSFX = NULL;
				}
				Player.PlayerSFX = Mix_LoadWAV("resource/Item_Sound.wav");
				if (Player.PlayerSFX != NULL) Player.SFXChannel = Mix_PlayChannel(-1, Player.PlayerSFX, 0);
			}
			else if (Enemy[i].attackType == 102) {
				if (HD_gameSys.rank < 3){
					HD_gameSys.rank++;
				}
				else {
					HD_gameSys.score += 1000;
				}
				if (Player.health < 4) Player.health++;
				Enemy[i].existence = 0;
				if (Player.SFXChannel != -1) Mix_HaltChannel(Player.SFXChannel);
				if (Player.PlayerSFX != NULL){
					Mix_FreeChunk(Player.PlayerSFX);
					Player.PlayerSFX = NULL;
				}
				Player.PlayerSFX = Mix_LoadWAV("resource/Item_Sound.wav");
				if (Player.PlayerSFX != NULL) Player.SFXChannel = Mix_PlayChannel(-1, Player.PlayerSFX, 0);
			}
			else if (Player.CrashTimer.checkTimer() && Enemy[i].attackType >= 21 && Enemy[i].attackType <= 100){
				//Player vs enemy collision
				if (Player.health >= 1){
					for (i = 0; i < 4; i++){
						if (Player.subweapon[i].existence != 0){
							Player.subweapon[i].existence = 0;
							Player.subweapon[i].DestroyTimer = HD_Timer();
							break;
						}
					}
				}
				if (Player.GetHealth() > 0 && Player.GetHealth() >= 1)
					Player.SetHealth(Player.GetHealth() - 1);
				else if (Player.GetHealth() < 0) //no immortality
					Player.SetHealth(0);
				else 
				{// Enemy[i].Gethealth == 0 ||(Enemy[i].GetHealth >0 && Enemy[i].GetHealth() < HD_PLAYER_ATTACK_POWER)
					Player.SetHealth(0);
				}
				if (Player.health == 0){
					GameOver();
				}
				else {
					Player.CrashTimer = HD_Timer();
					Player.CrashTimer.startTimer(1500);
					Player.Shield.existence = 2;
				}
				if (Player.SFXChannel != -1) Mix_HaltChannel(Player.SFXChannel);
				if (Player.PlayerSFX != NULL){
					Mix_FreeChunk(Player.PlayerSFX);
					Player.PlayerSFX = NULL;
				}
				Player.PlayerSFX = Mix_LoadWAV("resource/Object_destruction.wav");
				if (Player.PlayerSFX != NULL) Player.SFXChannel = Mix_PlayChannel(-1, Player.PlayerSFX, 0);
			}
		}
	}
	//Enemy bullet collision
	for (i = 0; i < HD_NUMBER_OF_ENEMY_BULLET; i++){
		if (Player.CollisionCheck(Bullet[i]) && Player.CrashTimer.checkTimer()){
			if (Player.health >= 1){
				for (i = 0; i < 4; i++){
					if (Player.subweapon[i].existence != 0){
						Player.subweapon[i].existence = 0;
						Player.subweapon[i].DestroyTimer = HD_Timer();
						break;
					}
				}
			}
			if (Player.GetHealth() > 0 && Player.GetHealth() >= 1)
				Player.SetHealth(Player.GetHealth() - 1);
			else if (Player.GetHealth() < 0) //no immortality
				Player.SetHealth(0);
			else // Enemy[i].Gethealth == 0 ||(Enemy[i].GetHealth >0 && Enemy[i].GetHealth() < HD_PLAYER_ATTACK_POWER)
				Player.SetHealth(0);
			if (Player.health == 0){
				GameOver();
			}
			else {
				Player.CrashTimer = HD_Timer();
				Player.CrashTimer.startTimer(1500);
				Player.Shield.existence = 2;
			}
			Bullet[i].existence = 0;
		}
		else if (Player.CollisionCheck(Bullet[i]) && !Player.CrashTimer.checkTimer()) {
			if (Bullet[i].attackType == 1) {//laser collision while player is invincible
				Bullet[i].dstarea.h = Player.GetCenterPosition().y - Enemy[Bullet[i].MovementPhase].GetCenterPosition().y;
			}
			else {
				Bullet[i].existence = 0;
			}
		}
	}
	HD_Point PlayerNextPoint = Player.GetCenterPosition();
	PlayerNextPoint.x += Player.GetDirection().x;
	PlayerNextPoint.y += Player.GetDirection().y;
	if (PlayerNextPoint.y > 0 && PlayerNextPoint.y < 500 && PlayerNextPoint.x > 0 && PlayerNextPoint.x < HD_GAME_WIDTH) {
		Player.SetCenterPosition(PlayerNextPoint.x, PlayerNextPoint.y);
	}
	Object[3].dstarea.w = Player.chargeGauge * 4;
	Object[5].dstarea.w = 8 * Player.health;
	//disable the shield when crashTimer expires
	if (Player.CrashTimer.checkStart() && !Player.CrashTimer.checkTimer()){
		Player.Shield.SetCenterPosition(Player.GetCenterPosition().x, Player.GetCenterPosition().y);
	}
	else {
		Player.Shield.existence = 0;
	}
	return;
}
void HD_SYSTEM::manageBullet(){
	int i;
	for (i = 0; i < HD_NUMBER_OF_ENEMY_BULLET; i++){
		if (Bullet[i].existence > 0) {
			if (Bullet[i].GetCenterPosition().y < -32 || Bullet[i].GetCenterPosition().y > 512 || Bullet[i].GetCenterPosition().x < -32 || Bullet[i].GetCenterPosition().x > HD_GAME_WIDTH + 32){
				Bullet[i].existence = 0; //delete
				Bullet[i].attackTimer = HD_Timer();
				Bullet[i].MoveTimer = HD_Timer();
				Bullet[i].DestroyTimer = HD_Timer();
			}
			//bullet state changes
			HD_Point tempdirection = Bullet[i].GetDirection();
			switch (Bullet[i].attackType){
			case 1:
				if (Enemy[Bullet[i].MovementPhase].existence == 0){
					Bullet[i].existence = 0;
				}
				Bullet[i].SetPosition(Bullet[i].dstarea.x, Enemy[Bullet[i].MovementPhase].GetCenterPosition().y + 16);
				Bullet[i].SetCenterPosition(Enemy[Bullet[i].MovementPhase].GetCenterPosition().x, Bullet[i].GetCenterPosition().y);
				if (Bullet[i].dstarea.h < HD_GAME_HEIGHT *2)Bullet[i].dstarea.h += 24;
			case 3:
				Bullet[i].SetDirection(tempdirection.x, tempdirection.y + 0.1);
				Bullet[i].SetCenterPosition(Bullet[i].GetCenterPosition().x + tempdirection.x, Bullet[i].GetCenterPosition().y + tempdirection.y);
				break;
			case 4:
				if (Bullet[i].GetCenterPosition().x < 16 && Bullet[i].GetCenterPosition().x >= HD_GAME_WIDTH - 16){
					Bullet[i].SetDirection(Bullet[i].GetDirection().x* (-1), Bullet[i].GetDirection().y);
				}
				Bullet[i].SetCenterPosition(Bullet[i].GetCenterPosition().x + tempdirection.x, Bullet[i].GetCenterPosition().y + tempdirection.y);
				break;
			case 5:
				if (Bullet[i].GetCenterPosition().x < 16 && Bullet[i].GetDirection().x < 0 && Bullet[i].MovementPhase == 0){
					Bullet[i].LoadSprite("resource/HD_Bullet1.png", { 0, 0, 32, 32 }, HD_renderer);
					Bullet[i].SetDirection(Bullet[i].GetDirection().x* (-1), 0);
					Bullet[i].MovementPhase = 1;
				}
				else if (Bullet[i].GetCenterPosition().x >= HD_GAME_WIDTH - 16 && Bullet[i].GetDirection().x > 0 && Bullet[i].MovementPhase == 0) {
					Bullet[i].LoadSprite("resource/HD_Bullet1.png", { 0, 0, 32, 32 }, HD_renderer);
					Bullet[i].SetDirection(Bullet[i].GetDirection().x* (-1), 0);
					Bullet[i].MovementPhase = 1;
				}
				Bullet[i].SetCenterPosition(Bullet[i].GetCenterPosition().x + tempdirection.x, Bullet[i].GetCenterPosition().y + tempdirection.y);
				break;
			default:
				Bullet[i].SetCenterPosition(Bullet[i].GetCenterPosition().x + tempdirection.x, Bullet[i].GetCenterPosition().y + tempdirection.y);
				break;
			}
			Bullet[i].anchor.x = Bullet[i].GetObjectCenter(false).x;
			Bullet[i].anchor.y = Bullet[i].GetObjectCenter(false).y;
		}
	}
	return;
}

//--------------------------------Normal function-------------------------------------
SDL_Rect RectTransform(HD_Rect src){
	//warning! do not use this function in calculation!
	SDL_Rect temp;
	temp.w = src.w;
	temp.h = src.h;
	temp.x = (int)src.x;
	temp.y = (int)src.y;
	return temp;
}
SDL_Point PointTransform(HD_Point src){
	//warning! do not use this function in calculation!
	SDL_Point temp;
	temp.x = (int)src.x;
	temp.y = (int)src.y;
	return temp;
}
bool CollisionCheck(HD_Object &src, HD_Object &target){
	if (src.existence != 2 || target.existence != 2){
		return false;
	}
	switch (target.attackType){
	case 1000: //only for boss
		if ((src.dstarea.x + src.dstarea.w / 4 < target.dstarea.x + target.dstarea.w * 3 / 4) && (src.dstarea.x + src.dstarea.w * 3 / 4 > target.dstarea.x + target.dstarea.w / 4) && (src.dstarea.y + src.dstarea.h / 4 < target.dstarea.y + target.dstarea.h * 3 / 4) && (src.dstarea.y + src.dstarea.h * 3 / 4 > target.dstarea.y + target.dstarea.h / 4)){
			return true;
		}
		else{
			return false;
		}
		break;
	default:
		if ((src.dstarea.x + src.dstarea.w / 4 < target.dstarea.x + target.dstarea.w * 3 / 4) && (src.dstarea.x + src.dstarea.w * 3 / 4 > target.dstarea.x + target.dstarea.w / 4) && (src.dstarea.y + src.dstarea.h / 4 < target.dstarea.y + target.dstarea.h * 3 / 4) && (src.dstarea.y + src.dstarea.h * 3 / 4 > target.dstarea.y + target.dstarea.h / 4)){
			return true;
		}
		else{
			return false;
		}
		break;
	}
}
