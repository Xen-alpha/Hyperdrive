#ifndef SUB_HEADER_DEFINED
#define SUB_HEADER_DEFINED
#include "HD_Framework_Sub.h"
#endif
#ifndef OBJECT_HEADER_DEFINED
#define OBJECT_HEADER_DEFINED
#include "HD_Object.h"
#endif
class HD_Menu {
	// do not put this data type in the parameter of the function
public:
	HD_Menu();
	HD_Menu(SDL_PixelFormat* Screenformat, SDL_Renderer *render);
	~HD_Menu();
	void InitMenu(SDL_Renderer *renderer);
	bool AddMenu(SDL_Renderer *renderer,const char *text, unsigned int referencestate);
	unsigned int GetCursorPosition();
	unsigned int GetMenuReference();
	unsigned int GetTotalItem();
	void SetCursorPosition(unsigned int num);
	void SetBackgroundSize(int w, int h);
	void ChangeMenuName(SDL_Renderer *renderer,int itemindex, char * text);
	HD_Object Menutext[8];
	HD_Object Cursor;
	HD_Object Background;
	bool Display;
	bool initiated;
	unsigned int totalitem;
	unsigned int currentitem; // position of cursor
	unsigned int state[8];
};
