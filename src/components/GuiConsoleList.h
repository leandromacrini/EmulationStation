#ifndef _GUICONSOLELIST_H_
#define _GUICONSOLELIST_H_

#include "../GuiComponent.h"
#include "GuiGameList.h"
#include "TextListComponent.h"
#include "ImageComponent.h"
#include "ThemeComponent.h"
#include "AnimationComponent.h"
#include "TextComponent.h"
#include <string>
#include <stack>
#include <vector>
#include "../SystemData.h"
#include "../GameData.h"
#include "../FolderData.h"

#define ANIMATION_MILLIS 250

//This is where the magic happens - GuiGameList is the parent of almost every graphical element in ES at the moment.
//It has a TextListComponent child that handles the game list, a ThemeComponent that handles the theming system, and an ImageComponent for game images.
class GuiConsoleList : public GuiComponent
{
public:
	GuiConsoleList(Window* window, GuiGameList* gameList);
	/*
	virtual ~GuiConsoleList();
	*/
	bool input(InputConfig* config, Input input);
	void update(int deltaTime);
	bool setCurrentIndex(unsigned int index);
	bool goToNext();
	bool goToPrev();
	void hideHUD();

protected:
	unsigned int mCurrentIndex;
	GuiGameList* mGameList;
	
	//HUD definition

	GuiComponent* slider;
	AnimationComponent* sliderAnimator;

	ImageComponent* logo;
	AnimationComponent* logoAnimator;

	GuiComponent* text;
	AnimationComponent* textAnimator;
	TextComponent* tName;
	TextComponent* tManufacturer;
	TextComponent* tDate;

	std::vector<ImageComponent*> mImageVector;

	void setText(bool animate);
	void setLogo(bool animate);
	void setImages(bool animate);
};

#endif
