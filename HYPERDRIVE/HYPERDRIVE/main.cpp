#include "HD_Framework.h"
using namespace std;

int main(int argc, char* arg[] ) {
	HD_SYSTEM hdSys = HD_SYSTEM(); // Initiate HD_Framework with SDL configuration
	SDL_Rect ScreenPoint;
	SDL_Rect srcRect;
	SDL_Rect dstRect; // may be used later
	int i, animation = 0;
	Uint32 Key_Delay = 0;
	unsigned int highscore = 0;
	char highscorebuf[12];
	for (i = 0; i < 12; i++) {
		highscorebuf[i] = 0;
	}
	unsigned int j;
	Uint32 fps = 0, fps_backup = 30;
	int MusicVolume = MIX_MAX_VOLUME / 2;
	Mix_VolumeMusic(MusicVolume);
	int Channel_Num;
	HD_Timer fps_counter = HD_Timer();
	HD_Timer fps_limiter = HD_Timer();
	fps_counter.startTimer(1000);
	if (hdSys.init_Flag) { // Successfully initialized
		//Start main loop
		while (!hdSys.quit_Flag){
			srand(SDL_GetTicks());
			SDL_PollEvent(&hdSys.HD_event);
			hdSys.HD_ManageKeyboard();
			if (hdSys.HD_event.type == SDL_QUIT) {
				hdSys.quit_Flag = true;
			}				
			else { // mouse event, joystick event, etc.
				// do nothing for now
			}
			if (hdSys.scene >= 4 && hdSys.scene < 100){
				if (fps_counter.checkTimer()){
					cout << fps << endl;
					fps_backup = fps;
					fps_counter = HD_Timer(); //reset counter
					fps_counter.startTimer(1000);
					fps = 0; 
				}
				else fps++;
			}
			SDL_RenderClear(hdSys.HD_renderer);
			//printf("scene %d.\n", hdSys.scene);
			switch (hdSys.scene) {
			case 0:
				//initialize ingame variable
				hdSys.Display = HD_Display();
				hdSys.KeyTimer = HD_Timer();
				hdSys.KeyTimer.startTimer(500);
				animation = 0;
				hdSys.Boss.existence = 0;
				hdSys.boss_phase = 0;
				hdSys.HD_gameSys.rank = 0;
				hdSys.Player.attackType = 0;
				hdSys.Player.attackTimer = HD_Timer();
				hdSys.Player.health = 4;
				hdSys.Player.subwpnorder = 0;
				if (Mix_PlayingMusic() != 0){
					Mix_HaltMusic();
				}
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					hdSys.Object_ingame[i].existence = 0;
					hdSys.Object[i].existence = 0;
				}
				for (i = 0; i < HD_NUMBER_OF_MY_BULLET; i++){
					hdSys.Player.bullet[i].existence = 0;
				}
				for (i = 0; i < HD_NUMBER_OF_ENEMY; i++){
					hdSys.Enemy[i].existence = 0;
				}
				for (i = 0; i < HD_NUMBER_OF_ENEMY_BULLET; i++){
					hdSys.Bullet[i].existence = 0;
				}
				for (i = 0; i < HD_MAX_MAPOBJECT; i++){
					hdSys.Object_map[i].existence = 0;
				}
				hdSys.Player.existence = 0;
				for (i = 0; i < 4; i++){
					hdSys.Player.subweapon[i].existence = 0;
				}
				//initialize title scene
				hdSys.Object[0].SetObjectAttribute(hdSys.HD_renderer,false, "resource/BG1.png", { 0, 0, 640, 640 }, { 0, 0, 800, 600 }, 2, 0, 2, 640);
				hdSys.Object[1].SetObjectAttribute(hdSys.HD_renderer, false, "resource/HD_Title.png", { 0, 0, 800, 600 }, { 0, 0, 800, 600 }, 2, 0, 0, 2);
				//show title menu
				hdSys.TitleMenu.SetCursorPosition(0);
				hdSys.TitleMenu.SetBackgroundSize(HD_SCREEN_WIDTH / 4, 192);
				hdSys.TitleMenu.Display = true;
				//BGM
				if (hdSys.bgm != NULL){
					Mix_FreeMusic(hdSys.bgm);
					SDL_Delay(20);
				}
				hdSys.bgm = Mix_LoadMUS("resource/Title.wav");
				SDL_Delay(50);
				hdSys.Player.Shield.SetObjectAttribute(hdSys.HD_renderer, false, "resource/margin.png", { 0, 0, 64, 64 }, hdSys.Player.dstarea, 0, 0, 0, 4);
				Mix_PlayMusic(hdSys.bgm, 0);
				fps_limiter = HD_Timer();
				hdSys.bossTimer = HD_Timer();
				if (hdSys.HD_gameSys.score > highscore) highscore = hdSys.HD_gameSys.score;
				sprintf_s(highscorebuf, "HI %08d", highscore);
				// hi-score
				hdSys.Object[5].LoadText(hdSys.HD_renderer, highscorebuf);
				hdSys.Object[5].SetCenterPosition(400, 540);
				hdSys.Object[5].AttackPhase = 0;
				hdSys.Object[5].SetDirection(0, 0);
				hdSys.Object[5].SetAniType(0, -1);
				hdSys.Object[5].existence = 2;
				//Go to next scene
				hdSys.scene = 1;
				break;
			case 1:
				// title - blit all object
				SDL_Delay(30); // slow down
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					if (hdSys.Object[i].spritebase != NULL && hdSys.Object[i].existence != 0) {
						srcRect = RectTransform(hdSys.Object[i].area);
						dstRect.x = (int)hdSys.Object[i].dstarea.x;
						dstRect.y = (int)hdSys.Object[i].dstarea.y;
						dstRect.w = (int)hdSys.Object[i].dstarea.w;
						dstRect.h = (int)hdSys.Object[i].dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.Object[i], &srcRect, &dstRect);
					}
				}
				// display title menu
				if (hdSys.TitleMenu.Display == true){
					if (hdSys.TitleMenu.Background.spritebase != NULL) {
						srcRect = RectTransform(hdSys.TitleMenu.Background.area);
						dstRect.x = (int)hdSys.TitleMenu.Background.dstarea.x;
						dstRect.y = (int)hdSys.TitleMenu.Background.dstarea.y;
						dstRect.w = (int)hdSys.TitleMenu.Background.dstarea.w;
						dstRect.h = (int)hdSys.TitleMenu.Background.dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.TitleMenu.Background, &srcRect, &dstRect);
					}
					for (j = 0; j < hdSys.TitleMenu.GetTotalItem(); j++){
						if (hdSys.TitleMenu.Menutext[j].spritebase != NULL) {
							srcRect = RectTransform(hdSys.TitleMenu.Menutext[j].area);
							dstRect.x = (int)hdSys.TitleMenu.Menutext[j].dstarea.x;
							dstRect.y = (int)hdSys.TitleMenu.Menutext[j].dstarea.y;
							dstRect.w = (int)hdSys.TitleMenu.Menutext[j].dstarea.w;
							dstRect.h = (int)hdSys.TitleMenu.Menutext[j].dstarea.h;
							hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.TitleMenu.Menutext[j], &srcRect, &dstRect);
						}
					}
					if (hdSys.TitleMenu.Cursor.spritebase != NULL) {
						srcRect = RectTransform(hdSys.TitleMenu.Cursor.area);
						dstRect.x = (int)hdSys.TitleMenu.Cursor.dstarea.x;
						dstRect.y = (int)hdSys.TitleMenu.Cursor.dstarea.y;
						dstRect.w = (int)hdSys.TitleMenu.Cursor.dstarea.w;
						dstRect.h = (int)hdSys.TitleMenu.Cursor.dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.TitleMenu.Cursor, &srcRect, &dstRect);
					}
				}
				break;
			case 2:
				// initial game setting display(scene 0 ->3 ->2->4)
				hdSys.inGameTextureUpdate();
				break;
			case 3:
				// initial game setting
				hdSys.Object[5].existence = 0;
				SDL_FillRect(hdSys.HD_gameSurface, NULL, SDL_MapRGB(hdSys.HD_format, 0x00, 0x00, 0x04));
				if (SDL_UpdateTexture(hdSys.HD_gameTexture, NULL, hdSys.HD_gameSurface->pixels, hdSys.HD_gameSurface->pitch) == -1) {
					cout << "Problem occured in updating game texture" << endl;
				}
				//and initailize screen surface again
				SDL_FillRect(hdSys.HD_screenSurface, NULL, SDL_MapRGB(hdSys.HD_format, 0x00, 0x00, 0x00));
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					hdSys.Object[i].existence = 0;
				}
				for (i = 0; i < HD_MAX_MAPOBJECT; i++){
					hdSys.Object_map[i].existence = 0;
				}
				hdSys.Object[0].AttackPhase = 0;
				hdSys.Object[0].SetDirection(0, 0);
				hdSys.Object[0].SetObjectAttribute(hdSys.HD_renderer, false, "resource/sideBG.png", { 0, 0, 800, 600 }, { 0, 0, 800, 600 }, 2, 0, -1, 0);
				hdSys.Object[1].LoadSprite("resource/HD_GameUI.png", { 0, 0, 800, 600 }, hdSys.HD_renderer);
				hdSys.Object[1].area = { 0, 0, 800, 600 };
				hdSys.Object[1].dstarea.w = 800;
				hdSys.Object[1].dstarea.h = 600;
				hdSys.Object[1].SetPosition(0, 0);
				hdSys.Object[1].AttackPhase = 0;
				hdSys.Object[1].SetDirection(0, 0);
				hdSys.Object[1].SetAniType(0, -1);
				hdSys.Object[1].existence = 2;
				hdSys.Object[8].LoadText(hdSys.HD_renderer, "Press Space to start");
				hdSys.Object[8].SetPosition(hdSys.Display.GetScreenCenter().x - hdSys.Object[8].area.w / 2.0, hdSys.Display.GetScreenCenter().y - hdSys.Object[8].area.h / 2.0);
				hdSys.Object[8].AttackPhase = 0;
				hdSys.Object[8].SetDirection(0, 0);
				hdSys.Object[8].SetAniType(0, -1);
				hdSys.Object[8].dstarea.w = hdSys.Object[8].area.w;
				hdSys.Object[8].dstarea.h = hdSys.Object[8].area.h;
				hdSys.Object[8].existence = 2;
 				hdSys.Player.attackType = -1; // default value of player type
				hdSys.HD_gameSys.score = 0;
				if (Mix_PlayingMusic() != 0){
					Mix_HaltMusic();
				}
				fps_limiter.startTimer(40);
				hdSys.Object_ingame[0].currentFrame = 0;
				hdSys.Player.area = { 0, 0, 64, 64 };
				hdSys.Player.LoadSprite("resource/HD_Player.png", hdSys.Player.area, hdSys.HD_renderer);
				hdSys.Player.dstarea = { 0, 0, 64, 64 };
				hdSys.Player.SetCenterPosition(hdSys.Display.GetScreenCenter().x - 80, HD_GAME_HEIGHT);
				hdSys.Player.AttackPhase = 0;
				hdSys.Player.SetAniType(2,0);
				//Object_ingame became invisible and HD_gameSurface is reset
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					hdSys.Object_ingame[i].existence = 0;
				}
				for (i = 0; i < 4; i++){
					hdSys.Player.subweapon[i].existence = 0;
				}
				hdSys.Object_ingame[0].area = { 0, 0, 640, 640 };
				hdSys.Object_ingame[0].LoadSprite("resource/BG1.png", hdSys.Object_ingame[0].area, hdSys.HD_renderer);
				hdSys.Object_ingame[0].dstarea.w = 640;
				hdSys.Object_ingame[0].dstarea.h = 640;
				hdSys.Object_ingame[0].SetPosition(0, 0);
				hdSys.Object_ingame[0].SetAniType(640, 3);
				hdSys.Object_ingame[0].attackType = 1;
				hdSys.Object_ingame[0].currentFrame = 0;
				hdSys.Object_ingame[0].existence = 0;
				hdSys.Display.AreaDev = 5;
				hdSys.scene = 2;
				SDL_Delay(50);
				break;
			case 4:
				//stage 1 bgm start
				if (Mix_PlayingMusic() == 0){
					if (hdSys.bgm != NULL){
						Mix_FreeMusic(hdSys.bgm);
						SDL_Delay(20);
					}
					hdSys.bgm = Mix_LoadMUS("resource/stage 1.wav");
					Mix_PlayMusic(hdSys.bgm, 0);
				}
				hdSys.Player.existence = 2;
				//stage 1 intro animation && read stage 1's stagepattern
				if (animation < 250){
					hdSys.Object[8].existence = 0; // delete player select guideline
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, animation);
					hdSys.Player.dstarea.y -= 6;
					hdSys.Display.AreaDev = 1;
					animation += 5;
				}
				else{
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, 0xff);
					//load stage
					hdSys.HD_gameSys.LoadStage("resource/stage1.txt");
					hdSys.SetMapObject();
					for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
						hdSys.Object_map[i].existence = 2;
					}
					hdSys.Object_ingame[0].SetAniType(640, 2);
					hdSys.Display.AreaDev = 1;
					hdSys.Player.health = 4;
					hdSys.HD_gameSys.rank = 2; // 2 shields
					//display charge bar
					hdSys.Object[3].area = { 0, 0, 64, 64 };
					hdSys.Object[3].LoadSprite("resource/default.png", hdSys.Object[3].area, hdSys.HD_renderer);
					hdSys.Object[3].dstarea.w = 0;
					hdSys.Object[3].dstarea.h = 10;
					hdSys.Object[3].attackType = 12;
					hdSys.Object[3].SetAniType(0, -1);
					hdSys.Object[3].SetCenterPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 40);
					hdSys.Object[3].existence = 2;
					//display player Health
					hdSys.Object[4].area = { 0, 0, 64, 64 };
					hdSys.Object[4].LoadSprite("resource/default.png", hdSys.Object[4].area, hdSys.HD_renderer);
					hdSys.Object[4].dstarea = {16,460,8 * hdSys.Player.health,10};
					hdSys.Object[4].attackType = 5;
					hdSys.Object[4].SetAniType(0, -1);
					hdSys.Object[4].SetPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 60);
					hdSys.Object[4].existence = 2;
					hdSys.Player.attackTimer.startTimer(250);
					hdSys.Player.CrashTimer = HD_Timer();
					hdSys.Player.CrashTimer.startTimer(1000);
					hdSys.HD_gameSys.spawnTimer.startTimer(0);
					hdSys.boss_phase = 0;
					hdSys.scene = 5;
					animation = 0;
					fps_limiter = HD_Timer();
				}
				hdSys.Object_ingame[0].existence = 2;
				hdSys.inGameTextureUpdate();
				break;
			case 5:
				//stage 1 field battle
				//manage my bullets
				if (hdSys.HD_gameSys.spawnTimer.checkTimer()){
					hdSys.spawnEnemy();
					hdSys.HD_gameSys.spawnTimer = HD_Timer(); //reset timer
					if (hdSys.Player.ctrlPressed)  hdSys.HD_gameSys.spawnTimer.startTimer(fps_backup * 64 * 1.5 / HD_MAP_SCROLL_SPEED);
					else hdSys.HD_gameSys.spawnTimer.startTimer(25 * 64 / HD_MAP_SCROLL_SPEED);
				}
				hdSys.manageObject();
				if (hdSys.Display.Display_type != 2) hdSys.Player.correctMoving();
				hdSys.managePlayer();
				hdSys.manageEnemy();
				hdSys.manageBullet();
				hdSys.Player.manageBullet();
				fps_limiter = HD_Timer();
				// ctrl key/z key control
				if (!hdSys.Player.ctrlPressed){
					if (fps_backup != 0) Key_Delay = 1000 / fps_backup;
					else Key_Delay = 33;
				}
				else {
					SDL_Delay(Key_Delay);
				}
				hdSys.inGameTextureUpdate();
 				break;
			case 6:
				//stage1 boss battle
				hdSys.manageObject();
				if (hdSys.Display.Display_type != 2)hdSys.Player.correctMoving();
				hdSys.managePlayer();
				hdSys.manageEnemy();
				hdSys.manageBoss();
				hdSys.manageBullet();
				hdSys.Player.manageBullet();
				fps_limiter = HD_Timer();
				// ctrl key/z key control
				if (!hdSys.Player.ctrlPressed){
					if (fps_backup != 0) Key_Delay = 1000 / fps_backup;
					else Key_Delay = 33;
				}
				else {
					SDL_Delay(Key_Delay);
				}
				hdSys.inGameTextureUpdate();
				break;
			case 7:
				//stage 1 result
				if (Mix_PlayingMusic() != 0 && Mix_FadingMusic() == 0){
					Mix_FadeOutMusic(3000);
				}
				if (hdSys.boss_phase != 0){
					hdSys.Object[9].LoadText(hdSys.HD_renderer, "Stage 1 Clear!");
					hdSys.Object[9].SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
					hdSys.Object[9].existence = 2;
					hdSys.boss_phase = 0;
				}
				hdSys.Player.CrashTimer = HD_Timer();
				hdSys.Player.CrashTimer.startTimer(100000);
				hdSys.manageObject();
				hdSys.manageEnemy();
				hdSys.manageBullet();
				// ctrl key/z key control
				if (!hdSys.Player.ctrlPressed){
					if (fps_backup != 0) Key_Delay = 1000 / fps_backup;
					else Key_Delay = 33;
				}
				else {
					SDL_Delay(Key_Delay);
				}
				if (animation < 250){
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, 255 - animation);
					animation += 5;
				}
				else{
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, 0x00);
					for (i = 0; i < HD_MAX_MAPOBJECT; i++){
						hdSys.Object_map[i].existence = 0;
					}
					//hide charge bar
					hdSys.Object[3].existence = 0;
					//hide player Health bar
					hdSys.Object[4].existence = 0;
					hdSys.boss_phase = 0;
					animation = 0;
				}
				hdSys.inGameTextureUpdate();
				break;
			case 8:
				//Game over
				if (Mix_PlayingMusic() != 0){
					Mix_HaltMusic();
				}
				hdSys.Display.DisplayReset();
				hdSys.manageObject();
				hdSys.managePlayer();
				hdSys.manageEnemy();
				hdSys.manageBullet();
				hdSys.Player.manageBullet();
				// ctrl key/z key control
				if (!hdSys.Player.ctrlPressed){
					if (fps_backup != 0) Key_Delay = 1000 / fps_backup;
					else Key_Delay = 33;
				}
				else {
					SDL_Delay(Key_Delay);
				}
				hdSys.inGameTextureUpdate();
				break;
			case 9:
				//stage 2 bgm start
				if (Mix_PlayingMusic() != 0) Mix_HaltMusic();
				if (hdSys.bgm != NULL){
					Mix_FreeMusic(hdSys.bgm);
					hdSys.bgm = NULL;
				}
				hdSys.bgm = Mix_LoadMUS("resource/stage 2.wav");
				Mix_PlayMusic(hdSys.bgm, 0);
				//stage 2 intro animation && read stage 2's stagepattern
				if (animation < 250){
					hdSys.Object[8].existence = 0; // delete player select guideline
					hdSys.Object[9].existence = 0;
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, animation);
					animation += 5;
				}
				else{
					hdSys.HD_gameSys.LoadStage("resource/stage2.txt");
					hdSys.Object_ingame[0].area = { 0, 0, 640, 640 };
					hdSys.Object_ingame[0].LoadSprite("resource/BG2.png", hdSys.Object_ingame[0].area, hdSys.HD_renderer);
					hdSys.Object_ingame[0].dstarea.w = 640;
					hdSys.Object_ingame[0].dstarea.h = 640;
					hdSys.Object_ingame[0].SetPosition(0, 0);
					hdSys.Object_ingame[0].SetAniType(640, 2);
					hdSys.Object_ingame[0].attackType = 1;
					hdSys.Object_ingame[0].currentFrame = 0;
					hdSys.Object_ingame[0].existence = 2;
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, 0xff);
					//load stage
					hdSys.SetMapObject();
					hdSys.HD_gameSys.currentPattern = 0;
					for (i = 0; i < HD_MAX_MAPOBJECT; i++){
						hdSys.Object_map[i].existence = 2;
					}
					hdSys.Object_ingame[0].SetAniType(640, 2);
					for (i = 0; i < HD_NUMBER_OF_ENEMY; i++){
						hdSys.Enemy[i].existence = 0;
					}
					for (i = 0; i < HD_NUMBER_OF_ENEMY_BULLET; i++){
						hdSys.Bullet[i].existence = 0;
					}
					//display charge bar
					hdSys.Object[3].area = { 0, 0, 64, 64 };
					hdSys.Object[3].LoadSprite("resource/default.png", hdSys.Object[3].area, hdSys.HD_renderer);
					hdSys.Object[3].dstarea.w = 0;
					hdSys.Object[3].dstarea.h = 10;
					hdSys.Object[3].attackType = 12;
					hdSys.Object[3].SetAniType(0, -1);
					hdSys.Object[3].SetCenterPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 40);
					hdSys.Object[3].existence = 2;
					//display player Health
					hdSys.Object[4].area = { 0, 0, 64, 64 };
					hdSys.Object[4].LoadSprite("resource/default.png", hdSys.Object[4].area, hdSys.HD_renderer);
					hdSys.Object[4].dstarea = { 16, 460, 8 * hdSys.Player.health, 10 };
					hdSys.Object[4].attackType = 5;
					hdSys.Object[4].SetAniType(0, -1);
					hdSys.Object[4].SetPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 60);
					hdSys.Object[4].existence = 2;
					hdSys.Player.attackTimer = HD_Timer();
					hdSys.Player.attackTimer.startTimer(250);
					hdSys.Player.CrashTimer = HD_Timer();
					hdSys.Player.CrashTimer.startTimer(1000);
					hdSys.HD_gameSys.spawnTimer = HD_Timer();
					hdSys.HD_gameSys.spawnTimer.startTimer(10);
					hdSys.boss_phase = 0;
					hdSys.scene = 10;
					animation = 0;
					fps_limiter = HD_Timer();
				}
				hdSys.inGameTextureUpdate();
				break;
			case 10:
				//stage 1 field battle
				//manage my bullets
				if (hdSys.HD_gameSys.spawnTimer.checkTimer()){
					hdSys.spawnEnemy();
					hdSys.HD_gameSys.spawnTimer = HD_Timer(); //reset timer
					if (hdSys.Player.ctrlPressed)  hdSys.HD_gameSys.spawnTimer.startTimer(25 * 64 * 1.5 / HD_MAP_SCROLL_SPEED);
					else hdSys.HD_gameSys.spawnTimer.startTimer(25 * 64 / HD_MAP_SCROLL_SPEED);
				}
				hdSys.manageObject();
				if (hdSys.Display.Display_type != 2) hdSys.Player.correctMoving();
				hdSys.managePlayer();
				hdSys.manageEnemy();
				hdSys.manageBullet();
				hdSys.Player.manageBullet();
				fps_limiter = HD_Timer();
				// ctrl key/z key control
				if (!hdSys.Player.ctrlPressed){
					if (fps_backup != 0) Key_Delay = 1000 / fps_backup;
					else Key_Delay = 33;
				}
				else {
					SDL_Delay(Key_Delay);
				}
				hdSys.inGameTextureUpdate();
				break;
			case 11:
				//stage1 boss battle
				hdSys.manageObject();
				if (hdSys.Display.Display_type != 2) hdSys.Player.correctMoving();
				hdSys.managePlayer();
				hdSys.manageEnemy();
				hdSys.manageBoss();
				hdSys.manageBullet();
				hdSys.Player.manageBullet();
				fps_limiter = HD_Timer();
				// ctrl key/z key control
				if (!hdSys.Player.ctrlPressed){
					if (fps_backup != 0) Key_Delay = 1000 / fps_backup;
					else Key_Delay = 33;
				}
				else {
					SDL_Delay(Key_Delay);
				}
				hdSys.inGameTextureUpdate();
				break;
			case 12:
				//stage 2 result
				if (hdSys.boss_phase != 0){
					hdSys.Object[9].LoadText(hdSys.HD_renderer, "Stage 2 Clear!");
					hdSys.Object[9].SetCenterPosition(HD_SCREEN_WIDTH / 2, HD_SCREEN_HEIGHT / 2);
					hdSys.Object[9].existence = 2;
					hdSys.boss_phase = 0;
				}
				hdSys.Player.CrashTimer = HD_Timer();
				hdSys.Player.CrashTimer.startTimer(100000);
				hdSys.manageObject();
				hdSys.manageEnemy();
				hdSys.manageBullet();
				// ctrl key/z key control
				if (!hdSys.Player.ctrlPressed){
					if (fps_backup != 0) Key_Delay = 1000 / fps_backup;
					else Key_Delay = 33;
				}
				else {
					SDL_Delay(Key_Delay);
				}
				if (animation < 250){
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, 255 - animation);
					animation += 5;
				}
				else{
					SDL_SetTextureAlphaMod(hdSys.Object_ingame[0].spritebase, 0x00);
					for (i = 0; i < HD_MAX_MAPOBJECT; i++){
						hdSys.Object_map[i].existence = 0;
					}
					//hide charge bar
					hdSys.Object[3].existence = 0;
					//hide player Health bar
					hdSys.Object[4].existence = 0;
					hdSys.boss_phase = 0;
					animation = 0;
				}
				hdSys.inGameTextureUpdate();
				break;
			case 99:
				//Help
				// title - blit all object
				SDL_Delay(30); // slow down
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					if (hdSys.Object[i].spritebase != NULL && hdSys.Object[i].existence != 0) {
						srcRect = RectTransform(hdSys.Object[i].area);
						dstRect.x = (int)hdSys.Object[i].dstarea.x;
						dstRect.y = (int)hdSys.Object[i].dstarea.y;
						dstRect.w = (int)hdSys.Object[i].dstarea.w;
						dstRect.h = (int)hdSys.Object[i].dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.Object[i], &srcRect, &dstRect);
					}
				}
				// display title menu
				if (hdSys.TitleMenu.Display == true){
					if (hdSys.TitleMenu.Background.spritebase != NULL) {
						srcRect = RectTransform(hdSys.TitleMenu.Background.area);
						dstRect.x = (int)hdSys.TitleMenu.Background.dstarea.x;
						dstRect.y = (int)hdSys.TitleMenu.Background.dstarea.y;
						dstRect.w = (int)hdSys.TitleMenu.Background.dstarea.w;
						dstRect.h = (int)hdSys.TitleMenu.Background.dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.TitleMenu.Background, &srcRect, &dstRect);
					}
					for (j = 0; j < hdSys.TitleMenu.GetTotalItem(); j++){
						if (hdSys.TitleMenu.Menutext[j].spritebase != NULL) {
							srcRect = RectTransform(hdSys.TitleMenu.Menutext[j].area);
							dstRect.x = (int)hdSys.TitleMenu.Menutext[j].dstarea.x;
							dstRect.y = (int)hdSys.TitleMenu.Menutext[j].dstarea.y;
							dstRect.w = (int)hdSys.TitleMenu.Menutext[j].dstarea.w;
							dstRect.h = (int)hdSys.TitleMenu.Menutext[j].dstarea.h;
							hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.TitleMenu.Menutext[j], &srcRect, &dstRect);
						}
					}
					if (hdSys.TitleMenu.Cursor.spritebase != NULL) {
						srcRect = RectTransform(hdSys.TitleMenu.Cursor.area);
						dstRect.x = (int)hdSys.TitleMenu.Cursor.dstarea.x;
						dstRect.y = (int)hdSys.TitleMenu.Cursor.dstarea.y;
						dstRect.w = (int)hdSys.TitleMenu.Cursor.dstarea.w;
						dstRect.h = (int)hdSys.TitleMenu.Cursor.dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.TitleMenu.Cursor, &srcRect, &dstRect);
					}
				}
				break;
			case 101:
				hdSys.Object[1].existence = 0;
				hdSys.Object[6].SetObjectAttribute(hdSys.HD_renderer, false, "resource/HD_help.png", { 0, 0, 640, 480 }, { HD_INGAME_DISPLAY_CORRECTION_X, 0, 640, 480 }, 2, 0, -1, 0);
				hdSys.Object[6].SetDirection(0, 0);
				hdSys.TitleMenu.Display = false;
				hdSys.scene = 99;
				SDL_Delay(50);
				break;
			case 102:
				//Display Test 1
				if (hdSys.Display.animation_test_counter.checkTimer() == true){
					hdSys.Display.DisplayReset();
					hdSys.Display.animation_test_counter = HD_Timer();
					hdSys.Display.animation_test_counter.startTimer(20000);
					hdSys.scene = 103;
				}
				else {
					hdSys.Display.Shake();
				}
				hdSys.inGameTextureUpdate();
				break;
			case 103:
				//Display Test 1
				if (hdSys.Display.animation_test_counter.checkTimer() == true){
					hdSys.Display.DisplayReset();
					hdSys.Display.animation_test_counter = HD_Timer();
					hdSys.scene = 0;
				}
				else {
					hdSys.Display.Swing();
				}
				hdSys.inGameTextureUpdate();
				break;
			case 104:
				SDL_FillRect(hdSys.HD_gameSurface, NULL, SDL_MapRGB(hdSys.HD_format, 0x00, 0x00, 0x04));
				if (SDL_UpdateTexture(hdSys.HD_gameTexture, NULL, hdSys.HD_gameSurface->pixels, hdSys.HD_gameSurface->pitch) == -1) {
					cout << "Problem occured in updating game texture" << endl;
				}
				//and initailize screen surface again
				SDL_FillRect(hdSys.HD_screenSurface, NULL, SDL_MapRGB(hdSys.HD_format, 0x00, 0x00, 0x00));
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					hdSys.Object[i].existence = 0;
				}
				if (Mix_PlayingMusic() != 0){
					Mix_HaltMusic();
				}
				fps_backup = 30;
				hdSys.HD_gameSys.spawnTimer = HD_Timer();
				hdSys.HD_gameSys.spawnTimer.startTimer(8000);
				hdSys.Object[0].area = { 0, 0, 800, 600 };
				hdSys.Object[0].LoadSprite("resource/sideBG.png", hdSys.Object[0].area, hdSys.HD_renderer);
				hdSys.Object[0].dstarea = { 0, 0, 800, 600 };
				hdSys.Object[0].SetPosition(0, 0);
				hdSys.Object[0].AttackPhase = 0;
				hdSys.Object[0].SetDirection(0, 0);
				hdSys.Object[0].SetAniType(0, -1);
				hdSys.Object[0].existence = 2;
				hdSys.Object[1].area = { 0, 0, 800, 600 };
				hdSys.Object[1].LoadSprite("resource/HD_GameUI.png", hdSys.Object[1].area, hdSys.HD_renderer);
				hdSys.Object[1].dstarea.w = 800;
				hdSys.Object[1].dstarea.h = 600;
				hdSys.Object[1].SetPosition(0, 0);
				hdSys.Object[1].AttackPhase = 0;
				hdSys.Object[1].SetDirection(0, 0);
				hdSys.Object[1].SetAniType(0, -1);
				hdSys.Object[1].existence = 2;
				hdSys.Object_ingame[0].area = { 0, 0, 640, 640 };
				hdSys.Object_ingame[0].LoadSprite("resource/BG1.png", hdSys.Object_ingame[0].area, hdSys.HD_renderer);
				hdSys.Object_ingame[0].dstarea = { 0, 0, HD_GAME_WIDTH, HD_GAME_HEIGHT };
				hdSys.Object_ingame[0].SetAniType(0, -1);
				hdSys.Object_ingame[0].attackType = 0;
				hdSys.Object_ingame[0].currentFrame = 0;
				hdSys.Object_ingame[0].existence = 2;
				hdSys.Boss.area = { 0, 0, 256, 256 };
				hdSys.Boss.LoadSprite("resource/HD_Boss1.png", hdSys.Boss.area, hdSys.HD_renderer);
				hdSys.Boss.dstarea.w = 256;
				hdSys.Boss.dstarea.h = 256;
				hdSys.Boss.existence = 2;
				hdSys.Boss.AttackPhase = 0;
				hdSys.Boss.SetAniType(2, 0);
				hdSys.Boss.SetDirection(0, -1);
				hdSys.Boss.SetCenterPosition(hdSys.Display.GetGameCenter().x - 80, -270);
				hdSys.Boss.SetDirection(0, 8);
				hdSys.Boss.MovementPhase = 0;
				hdSys.Boss.attackType = 1000;
				hdSys.Boss.health = HD_BOSS1_HEALTH;
				hdSys.Boss.attackTimer = HD_Timer();
				hdSys.Boss.attackTimer.startTimer(900);
				hdSys.Boss.DestroyTimer = HD_Timer();
				hdSys.Boss.DestroyTimer.startTimer(600000);
				hdSys.boss_phase = 0;
				hdSys.Object[12].area = { 0, 0, 64, 64 };
				hdSys.Object[12].LoadSprite("resource/default.png", hdSys.Object[12].area, hdSys.HD_renderer);
				hdSys.Object[12].dstarea.w = 560;
				hdSys.Object[12].dstarea.h = 16;
				hdSys.Object[12].existence = 2;
				hdSys.Object[12].SetCenterPosition(HD_GAME_WIDTH / 2, 40);
				hdSys.Object[12].SetDirection(0, 0);
				hdSys.Object[12].AttackPhase = 0;
				hdSys.Object[12].SetAniType(0, -1);
				hdSys.Object[12].attackType = 1010; // attackType = 1010 -> health bar
				hdSys.Object[12].health = -1;
				hdSys.Object[12].angle = 0;
				hdSys.Display.animation_test_counter = HD_Timer();
				hdSys.Display.animation_test_counter.startTimer(1000);
				hdSys.Object_ingame[0].currentFrame = 0;
				hdSys.Player.area = { 0, 0, 64, 64 };
				hdSys.Player.LoadSprite("resource/HD_Player.png", hdSys.Player.area, hdSys.HD_renderer);
				hdSys.Player.existence = 2;
				hdSys.Player.dstarea = { 0, 0, 64, 64 };
				hdSys.Player.SetCenterPosition(hdSys.Display.GetScreenCenter().x - 80, 400);
				hdSys.Player.AttackPhase = 0;
				hdSys.Player.SetAniType(0, -1);
				hdSys.Object[8].existence = 0; // delete player select guideline
				//Object_ingame became invisible and HD_gameSurface is reset
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					hdSys.Object_ingame[i].existence = 0;
				}
				for (i = 0; i < 4; i++){
					hdSys.Player.subweapon[i].existence = 0;
				}
				hdSys.Object_ingame[0].area = { 0, 0, 640, 640 };
				hdSys.Object_ingame[0].LoadSprite("resource/BG1.png", hdSys.Object_ingame[0].area, hdSys.HD_renderer);
				hdSys.Object_ingame[0].dstarea.w = 640;
				hdSys.Object_ingame[0].dstarea.h = 640;
				hdSys.Object_ingame[0].SetPosition(0, 0);
				hdSys.Object_ingame[0].SetAniType(640, 3);
				hdSys.Object_ingame[0].attackType = 1;
				hdSys.Object_ingame[0].currentFrame = 0;
				hdSys.Object_ingame[0].existence = 2;
				hdSys.Object_ingame[0].SetAniType(640, 2);
				hdSys.Display.AreaDev = 1;
				hdSys.Player.health = 4;
				hdSys.Player.existence = 2;
				//display charge bar
				hdSys.Object[3].area = { 0, 0, 64, 64 };
				hdSys.Object[3].LoadSprite("resource/default.png", hdSys.Object[3].area, hdSys.HD_renderer);
				hdSys.Object[3].dstarea.w = 0;
				hdSys.Object[3].dstarea.h = 10;
				hdSys.Object[3].attackType = 12;
				hdSys.Object[3].SetAniType(0, -1);
				hdSys.Object[3].SetCenterPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 40);
				hdSys.Object[3].existence = 2;
				//display player Health
				hdSys.Object[4].area = { 0, 0, 64, 64 };
				hdSys.Object[4].LoadSprite("resource/default.png", hdSys.Object[4].area, hdSys.HD_renderer);
				hdSys.Object[4].dstarea = { 16, 460, 8 * hdSys.Player.health, 10 };
				hdSys.Object[4].attackType = 5;
				hdSys.Object[4].SetAniType(0, -1);
				hdSys.Object[4].SetPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 60);
				hdSys.Object[4].existence = 2;
				hdSys.Player.attackTimer.startTimer(250);
				hdSys.Player.CrashTimer = HD_Timer();
				hdSys.Player.CrashTimer.startTimer(1000);
				hdSys.bossTimer = HD_Timer();
				hdSys.bossTimer.startTimer(4000);
				hdSys.scene = 6;
				SDL_Delay(50);
				break;
				case 105:
					hdSys.Object[4].existence = 0;
					SDL_FillRect(hdSys.HD_gameSurface, NULL, SDL_MapRGB(hdSys.HD_format, 0x00, 0x00, 0x04));
					if (SDL_UpdateTexture(hdSys.HD_gameTexture, NULL, hdSys.HD_gameSurface->pixels, hdSys.HD_gameSurface->pitch) == -1) {
						cout << "Problem occured in updating game texture" << endl;
					}
					//and initailize screen surface again
					SDL_FillRect(hdSys.HD_screenSurface, NULL, SDL_MapRGB(hdSys.HD_format, 0x00, 0x00, 0x00));
					for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
						hdSys.Object[i].existence = 0;
					}
					if (Mix_PlayingMusic() != 0){
						Mix_HaltMusic();
					}
					fps_backup = 30;
					hdSys.HD_gameSys.spawnTimer = HD_Timer();
					hdSys.HD_gameSys.spawnTimer.startTimer(8000);
					hdSys.Object[0].area = { 0, 0, 800, 600 };
					hdSys.Object[0].LoadSprite("resource/sideBG.png", hdSys.Object[0].area, hdSys.HD_renderer);
					hdSys.Object[0].dstarea = { 0, 0, 800, 600 };
					hdSys.Object[0].SetPosition(0, 0);
					hdSys.Object[0].AttackPhase = 0;
					hdSys.Object[0].SetDirection(0, 0);
					hdSys.Object[0].SetAniType(0, -1);
					hdSys.Object[0].existence = 2;
					hdSys.Object[1].area = { 0, 0, 800, 600 };
					hdSys.Object[1].LoadSprite("resource/HD_GameUI.png", hdSys.Object[1].area, hdSys.HD_renderer);
					hdSys.Object[1].dstarea.w = 800;
					hdSys.Object[1].dstarea.h = 600;
					hdSys.Object[1].SetPosition(0, 0);
					hdSys.Object[1].AttackPhase = 0;
					hdSys.Object[1].SetDirection(0, 0);
					hdSys.Object[1].SetAniType(0, -1);
					hdSys.Object[1].existence = 2;
					hdSys.Boss.area = { 0, 0, 256, 160 };
					hdSys.Boss.LoadSprite("resource/HD_Boss2.png", hdSys.Boss.area, hdSys.HD_renderer);
					hdSys.Boss.dstarea.w = 256;
					hdSys.Boss.dstarea.h = 160;
					hdSys.Boss.existence = 2;
					hdSys.Boss.AttackPhase = 0;
					hdSys.Boss.SetAniType(2, 0);
					hdSys.Boss.SetDirection(0, -1);
					hdSys.Boss.SetCenterPosition(hdSys.Display.GetGameCenter().x - 80, -270);
					hdSys.Boss.SetDirection(0, 8);
					hdSys.Boss.MovementPhase = 0;
					hdSys.Boss.attackType = 1000;
					hdSys.Boss.health = HD_BOSS2_HEALTH;
					hdSys.Boss.attackTimer = HD_Timer();
					hdSys.Boss.attackTimer.startTimer(900);
					hdSys.Boss.DestroyTimer = HD_Timer();
					hdSys.Boss.DestroyTimer.startTimer(600000);
					hdSys.boss_phase = 0;
					hdSys.Object[12].area = { 0, 0, 64, 64 };
					hdSys.Object[12].LoadSprite("resource/default.png", hdSys.Object[12].area, hdSys.HD_renderer);
					hdSys.Object[12].dstarea.w = 560;
					hdSys.Object[12].dstarea.h = 16;
					hdSys.Object[12].existence = 2;
					hdSys.Object[12].SetCenterPosition(HD_GAME_WIDTH / 2, 40);
					hdSys.Object[12].SetDirection(0, 0);
					hdSys.Object[12].AttackPhase = 0;
					hdSys.Object[12].SetAniType(0, -1);
					hdSys.Object[12].attackType = 1010; // attackType = 1010 -> health bar
					hdSys.Object[12].health = -1;
					hdSys.Object[12].angle = 0;
					hdSys.Display.animation_test_counter = HD_Timer();
					hdSys.Display.animation_test_counter.startTimer(1000);
					hdSys.Object_ingame[0].currentFrame = 0;
					hdSys.Player.area = { 0, 0, 64, 64 };
					hdSys.Player.LoadSprite("resource/HD_Player.png", hdSys.Player.area, hdSys.HD_renderer);
					hdSys.Player.existence = 2;
					hdSys.Player.dstarea = { 0, 0, 64, 64 };
					hdSys.Player.SetCenterPosition(hdSys.Display.GetScreenCenter().x - 80, 400);
					hdSys.Player.AttackPhase = 0;
					hdSys.Player.SetAniType(0, -1);
					hdSys.Object[8].existence = 0; // delete player select guideline
					//Object_ingame became invisible and HD_gameSurface is reset
					for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
						hdSys.Object_ingame[i].existence = 0;
					}
					for (i = 0; i < 4; i++){
						hdSys.Player.subweapon[i].existence = 0;
					}
					hdSys.Object_ingame[0].area = { 0, 0, 640, 640 };
					hdSys.Object_ingame[0].LoadSprite("resource/BG2.png", hdSys.Object_ingame[0].area, hdSys.HD_renderer);
					hdSys.Object_ingame[0].dstarea.w = 640;
					hdSys.Object_ingame[0].dstarea.h = 640;
					hdSys.Object_ingame[0].SetPosition(0, 0);
					hdSys.Object_ingame[0].SetAniType(640, 3);
					hdSys.Object_ingame[0].attackType = 1;
					hdSys.Object_ingame[0].currentFrame = 0;
					hdSys.Object_ingame[0].existence = 2;
					hdSys.Object_ingame[0].SetAniType(640, 2);
					hdSys.Display.AreaDev = 1;
					hdSys.Player.health = 4;
					hdSys.Player.existence = 2;
					//display charge bar
					hdSys.Object[3].area = { 0, 0, 64, 64 };
					hdSys.Object[3].LoadSprite("resource/default.png", hdSys.Object[3].area, hdSys.HD_renderer);
					hdSys.Object[3].dstarea.w = 0;
					hdSys.Object[3].dstarea.h = 10;
					hdSys.Object[3].attackType = 12;
					hdSys.Object[3].SetAniType(0, -1);
					hdSys.Object[3].SetCenterPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 40);
					hdSys.Object[3].existence = 2;
					//display player Health
					hdSys.Object[4].area = { 0, 0, 64, 64 };
					hdSys.Object[4].LoadSprite("resource/default.png", hdSys.Object[4].area, hdSys.HD_renderer);
					hdSys.Object[4].dstarea = { 16, 460, 8 * hdSys.Player.health, 10 };
					hdSys.Object[4].attackType = 5;
					hdSys.Object[4].SetAniType(0, -1);
					hdSys.Object[4].SetPosition(hdSys.Player.GetCenterPosition().x, hdSys.Player.GetCenterPosition().y + 60);
					hdSys.Object[4].existence = 2;
					hdSys.Player.attackTimer.startTimer(250);
					hdSys.Player.CrashTimer = HD_Timer();
					hdSys.Player.CrashTimer.startTimer(1000);
					hdSys.bossTimer = HD_Timer();
					hdSys.bossTimer.startTimer(4000);
					hdSys.scene = 11;
					SDL_Delay(50);                           
					break;
			case 111:
				hdSys.Object[1].existence = 0;
				hdSys.Object[6].SetObjectAttribute(hdSys.HD_renderer, false, "resource/HD_credit.png", { 0, 0, 640, 480 }, { HD_INGAME_DISPLAY_CORRECTION_X, 0, 640, 480 }, 2, 0, -1, 0);
				hdSys.Object[6].SetDirection(0, 0);
				hdSys.TitleMenu.Display = false;
				hdSys.scene = 99;
				SDL_Delay(50);
				break;
			case 121:
				if (MusicVolume >= MIX_MAX_VOLUME) {
					MusicVolume = 0;
					sprintf_s(hdSys.volumeText, "Volume: %d", MusicVolume);
				}
				else {
					MusicVolume += MIX_MAX_VOLUME / 4;
					sprintf_s(hdSys.volumeText, "Volume: %d", MusicVolume / (MIX_MAX_VOLUME / 4));
				}
				Mix_VolumeMusic(MusicVolume);
				hdSys.TitleMenu.ChangeMenuName(hdSys.HD_renderer, 2, hdSys.volumeText);
				hdSys.scene = 1;
				break;
			case 1000:
				hdSys.quit_Flag = true;
				break;
			case 1003:
				//pause
				SDL_FillRect(hdSys.HD_gameSurface, NULL, SDL_MapRGB(hdSys.HD_format, 0x00, 0x00, 0x04));
				for (i = 0; i < HD_NUMBER_OF_OBJECT; i++){
					if (hdSys.Object[i].spritebase != NULL && hdSys.Object[i].existence != 0) {
						srcRect = RectTransform(hdSys.Object[i].area);
						dstRect.x = (int)hdSys.Object[i].dstarea.x;
						dstRect.y = (int)hdSys.Object[i].dstarea.y;
						dstRect.w = (int)hdSys.Object[i].dstarea.w;
						dstRect.h = (int)hdSys.Object[i].dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.Object[i], &srcRect, &dstRect);
					}
				}
				//pause
				if (hdSys.PauseMenu.Display == true){
					if (hdSys.PauseMenu.Background.spritebase != NULL) {
						srcRect = RectTransform(hdSys.PauseMenu.Background.area);
						dstRect.x = (int)hdSys.PauseMenu.Background.dstarea.x;
						dstRect.y = (int)hdSys.PauseMenu.Background.dstarea.y;
						dstRect.w = (int)hdSys.PauseMenu.Background.dstarea.w;
						dstRect.h = (int)hdSys.PauseMenu.Background.dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.PauseMenu.Background, &srcRect, &dstRect);
					}
					for (j = 0; j < hdSys.PauseMenu.GetTotalItem(); j++){
						if (hdSys.PauseMenu.Menutext[j].spritebase != NULL) {
							srcRect = RectTransform(hdSys.PauseMenu.Menutext[j].area);
							dstRect.x = (int)hdSys.PauseMenu.Menutext[j].dstarea.x;
							dstRect.y = (int)hdSys.PauseMenu.Menutext[j].dstarea.y;
							dstRect.w = (int)hdSys.PauseMenu.Menutext[j].dstarea.w;
							dstRect.h = (int)hdSys.PauseMenu.Menutext[j].dstarea.h;
							hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.PauseMenu.Menutext[j], &srcRect, &dstRect);
						}
					}
					if (hdSys.PauseMenu.Cursor.spritebase != NULL) {
						srcRect = RectTransform(hdSys.PauseMenu.Cursor.area);
						dstRect.x = (int)hdSys.PauseMenu.Cursor.dstarea.x;
						dstRect.y = (int)hdSys.PauseMenu.Cursor.dstarea.y;
						dstRect.w = (int)hdSys.PauseMenu.Cursor.dstarea.w;
						dstRect.h = (int)hdSys.PauseMenu.Cursor.dstarea.h;
						hdSys.HD_BlitSprite(hdSys.HD_renderer, hdSys.PauseMenu.Cursor, &srcRect, &dstRect);
					}
				}
				//Update Window Texture
				break;
			default:
				//error - wrong scene choosed
				//if some memories are allocated by making In-game property, delete/free them
				cout << "Error managing Scene in HD_SYSTEM." << endl;
				hdSys.quit_Flag = true;
			}
			//Update Window Texture
			SDL_RenderPresent(hdSys.HD_renderer);
		}
	}
	//exit
	hdSys.exitGame();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0; //automatic deletion of HD_Framework's custom resources
}
