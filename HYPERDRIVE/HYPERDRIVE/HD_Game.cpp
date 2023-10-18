#include "HD_Game.h"

//--------------------------------HD_Game definition----------------------------------
HD_Game::HD_Game(){
	int i, j, k;
	score = 0;
	rank = 0;
	scrollSpeed = 0;
	currentPattern = 0;
	stageLength = 0;
	stageExist = false;
	for (i = 0; i < 9; i++){
		scorebuf[i] = 0;
	}
	for (i = 0; i < 3; i++){
		rankbuf[i] = 0;
	}
	stageNum = 0;
	spawnTimer = HD_Timer();
	stageState = 0;
	stagefile = "resource/stage1.txt";
}
HD_Game::HD_Game(SDL_Window * Window) : HD_Game(){
	window = Window;
}
HD_Game::~HD_Game(){
	if (stageData.is_open()) stageData.close();
}
HD_Game::HD_Game(const HD_Game &src){
	int i, j;
	score = src.score;
	rank = src.rank;
	scrollSpeed = src.scrollSpeed;
	currentPattern = src.currentPattern;
	pattern_background = src.pattern_background;
	pattern_ground = src.pattern_ground;
	stageLength = src.stageLength; //maximum limit of currentpattern
	stageData.open(src.stagefile.c_str());
	stageExist = src.stageExist;
	stageNum = src.stageNum;
	spawnTimer = src.spawnTimer;
	stageState = src.stageState;
	stagefile = src.stagefile;
	for (i = 0; i < 9; i++){
		scorebuf[i] = src.scorebuf[i];
	}
	for (i = 0; i < 3; i++){
		rankbuf[i] = src.rankbuf[i];
	}
}
HD_Game &HD_Game::operator =(const HD_Game & src){
	if (this == &src){
		return *this;
	}
	else{
		int i, j;
		score = src.score;
		rank = src.rank;
		scrollSpeed = src.scrollSpeed;
		currentPattern = src.currentPattern;
		stageLength = src.stageLength; //maximum limit of currentpattern
		stageData.open(src.stagefile.c_str());
		stageExist = src.stageExist;
		pattern_background = src.pattern_background;
		pattern_ground = src.pattern_ground;
		stageNum = src.stageNum;
		spawnTimer = src.spawnTimer;
		stageState = src.stageState;
		stagefile = src.stagefile;
		for (i = 0; i < 9; i++){
			scorebuf[i] = src.scorebuf[i];
		}
		for (i = 0; i < 3; i++){
			rankbuf[i] = src.rankbuf[i];
		}
		return *this;
	}
}
bool HD_Game::LoadStage(const char * filename){
	int i, j;
	int data;
	vector<int> data_row;
	pattern_ground.clear();
	pattern_background.clear();
	if (stageData.is_open()){
		stageData.close();
	}
	stageData.open(filename);
	if (!stageData.is_open()){
		stageExist = false;
		return false;
	}
	else {
		stageData >> stageNum;
		stageData >> stageLength;
		cout << stageNum << endl;
		cout << stageLength << endl;
		for (i = 0; i < stageLength; i++){
			stageData >> data;
			cout << i << "th Row :";
			cout << data << " " << endl;
			pattern_background.push_back(data);
			for (j = 0; j < 10; j++){
				stageData >> data;
				cout << "column " << j << " = "<< data << "/";
				data_row.push_back(data);
			}
			cout << endl;
			pattern_ground.push_back(data_row);
			data_row.clear();
		}
		currentPattern = 0;
		stageExist = true;
		stagefile = std::string(filename);
		spawnTimer = HD_Timer();
		return true;
	}
}
void HD_Game::addScore(unsigned int value){
	if ((score + value) < 100000000){
		score += value;
	}
	else{
		score = 99999999;
	}
	return;
}