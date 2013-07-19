#include "GuiConsoleList.h"
#include "../InputManager.h"
#include <iostream>
#include "GuiMenu.h"
#include "GuiFastSelect.h"
#include <boost/filesystem.hpp>
#include "../Log.h"
#include "../Settings.h"

GuiConsoleList::GuiConsoleList(Window* window, GuiGameList* gameList) : GuiComponent(window)
{
	mCurrentIndex = -1;
	mGameList = gameList;

	if(SystemData::sSystemVector.size() == 0)
	{
		LOG(LogError) << "Error - no systems found!";
		return;
	}

	//background
	ImageComponent* back = new ImageComponent(window);
	back->setTiling(true);
	back->setOrigin(0, 0);
	back->setImage("c:\\retromania\\consoles\\bg_pattern.png");
	back->setResize(Renderer::getScreenWidth(), Renderer::getScreenHeight(), false);
	this->addChild(back);

	//consoles slider
	slider = new GuiComponent(window);
	this->addChild(slider);
	sliderAnimator = new AnimationComponent();
	sliderAnimator->addChild(slider);

	for(unsigned int i = 0; i < SystemData::sSystemVector.size(); i++)
	{

		SystemData* data = SystemData::sSystemVector.at(i);

		std::string name = data->getDescName();

		std::string image = data->getImage();
		ImageComponent* img = NULL;
		
		if( image.compare("none") != 0 )
			img = new ImageComponent(window, Renderer::getScreenWidth()/2 + Renderer::getScreenWidth()/2 * i, Renderer::getScreenHeight() / 2, image, 0, (unsigned int)(Renderer::getScreenHeight()/2.5), true);
		else
			img = new ImageComponent(window, Renderer::getScreenWidth()/2 + Renderer::getScreenWidth()/2 * i, Renderer::getScreenHeight() / 2,  SystemData::getBlankConsoleImagePath(), 0,  (unsigned int)(Renderer::getScreenHeight()/2.5), true);

		slider->addChild( img );
	}

	//update slider total size
	slider->setSize(Renderer::getScreenWidth()/2 * (SystemData::sSystemVector.size()+1) , Renderer::getScreenHeight());

	//create loto e logoAnimator
	logo = new ImageComponent(window);
	logo->setTiling(false);
	logo->setOrigin(0.5, 0);
	this->addChild(logo);

	logoAnimator = new AnimationComponent();
	logoAnimator->addChild(logo);

	//create description text
	text = new GuiComponent(window);
	this->addChild(text);

	textAnimator = new AnimationComponent();
	textAnimator->addChild(text);

	tName = new TextComponent(window);
	tName->setColor(0xFFFFFFFF);
	tName->setOffset(10, 10);
	tName->setFont(Font::get(*window->getResourceManager(), getHomePath() + "/.emulationstation/fonts/pixeljosh6.ttf", FONT_SIZE_SMALL));

	tManufacturer = new TextComponent(window);
	tManufacturer->setColor(0xFFFFFFFF);
	tManufacturer->setOffset(10, 60);
	tManufacturer->setFont(Font::get(*window->getResourceManager(), getHomePath() + "/.emulationstation/fonts/pixeljosh6.ttf", FONT_SIZE_SMALL));

	tDate = new TextComponent(window);
	tDate->setColor(0xFFFFFFFF);
	tDate->setOffset(10, 110);
	tDate->setFont(Font::get(*window->getResourceManager(), getHomePath() + "/.emulationstation/fonts/pixeljosh6.ttf", FONT_SIZE_SMALL));

	text->addChild(tName);
	text->addChild(tManufacturer);
	text->addChild(tDate);

	//start at index = 0
	setCurrentIndex(0);
}

void GuiConsoleList::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	sliderAnimator->update(deltaTime);

	textAnimator->update(deltaTime);

	logoAnimator->update(deltaTime);
}

bool GuiConsoleList::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("right", input) && input.value != 0)
	{
		goToNext();
		return true;
	}

	if(config->isMappedTo("left", input)  && input.value != 0 )
	{
		goToPrev();
		return true;
	}

	if(config->isMappedTo("a", input))
	{
		mWindow->pushGui(mGameList);
		mGameList->setSystemId(mCurrentIndex);
		mGameList->doVerticalTransition(-1);
		return true;
	}

	//open the "start menu"
	if(config->isMappedTo("menu", input) && input.value != 0)
	{
		mWindow->pushGui(new GuiMenu(mWindow, mGameList));
		return true;
	}

	return false;
}

bool GuiConsoleList::goToNext()
{
	//avoid animation corruption
	if(sliderAnimator->isAnimating()) return false;

	if(mCurrentIndex+1 < SystemData::sSystemVector.size())
	{

		mCurrentIndex++;

		//animate slider
		sliderAnimator->move(-(Renderer::getScreenWidth() / 2), 0, ANIMATION_MILLIS);

		//animate logo
		setLogo(true);

		//animate text
		setText(true);

		return true;
	}

	return false;
}

bool GuiConsoleList::goToPrev()
{
	//avoid animation corruption
	if(sliderAnimator->isAnimating()) return false;

	if(mCurrentIndex > 0)
	{
		mCurrentIndex--;

		//animate slider
		sliderAnimator->move(+(Renderer::getScreenWidth() / 2), 0, ANIMATION_MILLIS);

		//animate logo
		setLogo(true);
		
		//animate text
		setText(true);

		return true;
	}

	return false;
}

bool GuiConsoleList::setCurrentIndex(unsigned int index)
{
	if(mCurrentIndex != index && index >= 0 && index < SystemData::sSystemVector.size())
	{
		mCurrentIndex = index;
		slider->setOffset(-(Renderer::getScreenWidth() / 2 * index), this->getOffset().y);
		
		setLogo(false);

		setText(false);

		return true;
	}

	return false;
}

void GuiConsoleList::setText(bool animate)
{
	//update text
	textAnimator->reset();
	tName->setText( "Name: " + SystemData::sSystemVector.at(mCurrentIndex)->getDescName());
	tManufacturer->setText( "Manufacturer: " + SystemData::sSystemVector.at(mCurrentIndex)->getManufacturer());
	tDate->setText( "Release date: " + SystemData::sSystemVector.at(mCurrentIndex)->getRelaseDate());

	int maxW = SDL_max( SDL_max(tName->getSize().x, tManufacturer->getSize().x), tDate->getSize().x);

	text->setSize(maxW, 200);

	if(animate)
	{
		text->setOffset(- text->getSize().x, 0);
		textAnimator->move(text->getSize().x, 0, ANIMATION_MILLIS);
	} else {
		text->setOffset(0,0);
	}
}

void GuiConsoleList::setLogo(bool animate)
{
	//update, reset and animate logo
	logoAnimator->reset();

	logo->setImage(SystemData::sSystemVector.at(mCurrentIndex)->getLogo()); //TODO DYNAMIC
	logo->setResize(Renderer::getScreenWidth() /2, 0, true);
	

	if(animate)
	{
		logo->setOffset(Renderer::getScreenWidth() / 2, Renderer::getScreenHeight());
		logoAnimator->move(0, -(Renderer::getScreenHeight()/4), ANIMATION_MILLIS);
	} else {
		logo->setOffset(Renderer::getScreenWidth() /2, Renderer::getScreenHeight() - Renderer::getScreenHeight()/4 );
	}
}