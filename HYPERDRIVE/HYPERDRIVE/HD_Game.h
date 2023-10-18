#ifndef SUB_HEADER_DEFINED
#define SUB_HEADER_DEFINED
#include "HD_Framework_Sub.h"
#endif
#include <vector>
using namespace std;
class HD_Game{
public:
	HD_Game();
	HD_Game(SDL_Window * Window);
	HD_Game(const HD_Game &src);
	HD_Game &HD_Game::operator =(const HD_Game &dummy);
	~HD_Game();
	//bool LoadStage(int stageNum);
	void readPattern();
	bool LoadStage(const char * filename);
	//bool LoadStage_v2(const char * filename);
	void addScore(unsigned int value);
	//member variable
	vector < vector< int > > pattern_ground;
	vector < int > pattern_background;
	unsigned int score; // score
	char scorebuf[9];
	unsigned int rank; // rank
	char rankbuf[3];
	int scrollSpeed;
	int currentPattern;
	int stageLength; //maximum limit of currentpattern
	int stageNum;
	std::ifstream stageData;
	bool stageExist;
	bool patternExist;
	HD_Timer spawnTimer;
	int stageState; // 0 : nothing, 1 : field battle, 2: boss battle, 3: stage end
	std::string stagefile;
	int totalpattern;
private:
	SDL_Window *window;
};