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
	back->setTiling(false);
	back->setOrigin(0, 0);
#ifdef WIN32
	back->setImage("c:\\retromania\\consoles\\bg.jpg");
#else //RASPBERRY PI
	back->setImage("/home/pi/.emulationstation/consoles/bg.jpg");
#endif 

	back->setResize(Renderer::getScreenWidth(), Renderer::getScreenHeight(), true);
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

		mImageVector.push_back(img);
	}

	//update slider total size
	slider->setSize(Renderer::getScreenWidth()/2 * (SystemData::sSystemVector.size()+1) , Renderer::getScreenHeight());

	//create logo e logoAnimator
	logo = new ImageComponent(window);
	logo->setSize(0.3, 0.2);
	//logo->setResize(540,180, true);
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
	tName->setColor(0x000000FF);
	tName->setPosition(10, 10);
	tName->setFont(Font::get(*window->getResourceManager(), getHomePath() + "/.emulationstation/fonts/pixeljosh6.ttf", FONT_SIZE_SMALL));

	tManufacturer = new TextComponent(window);
	tManufacturer->setColor(0x000000FF);
	tManufacturer->setPosition(10, 60);
	tManufacturer->setFont(Font::get(*window->getResourceManager(), getHomePath() + "/.emulationstation/fonts/pixeljosh6.ttf", FONT_SIZE_SMALL));

	tDate = new TextComponent(window);
	tDate->setColor(0x000000FF);
	tDate->setPosition(10, 110);
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
		//play theme sound
		mGameList->getTheme()->getSound("menuScroll")->play();
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

	//play theme sound
	mGameList->getTheme()->getSound("menuScroll")->play();

	if(mCurrentIndex+1 < SystemData::sSystemVector.size())
	{

		mCurrentIndex++;

		hideHUD();

		//animate slider
		sliderAnimator->move(-(Renderer::getScreenWidth() / 2), 0, ANIMATION_MILLIS,
			[this] ()
		{
		//animate logo
		setLogo(true);

		//animate text
		setText(true);

			//resize current console
			setImages(true);
		});

		return true;
	}

	return false;
}

bool GuiConsoleList::goToPrev()
{
	//avoid animation corruption
	if(sliderAnimator->isAnimating()) return false;

	//play theme sound
	mGameList->getTheme()->getSound("menuScroll")->play();

	if(mCurrentIndex > 0)
	{
		mCurrentIndex--;

		hideHUD();

		//animate slider
		sliderAnimator->move(Renderer::getScreenWidth() / 2, 0, ANIMATION_MILLIS,
			[this] ()
		{
		//animate logo
		setLogo(true);
		
		//animate text
		setText(true);

			//resize current console
			setImages(true);
		});

		return true;
	}

	return false;
}

bool GuiConsoleList::setCurrentIndex(unsigned int index)
{
	if(mCurrentIndex != index && index >= 0 && index < SystemData::sSystemVector.size())
	{
		mCurrentIndex = index;
		slider->setPosition(-(Renderer::getScreenWidth() / 2 * index), this->getPosition().y());
		
		setLogo(false);

		setText(false);

		//resize current console
		setImages(false);

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

	int maxW = SDL_max( SDL_max(tName->getSize().x(), tManufacturer->getSize().x()), tDate->getSize().x());

	text->setSize(maxW + 10, 200); //10 is the internal text left margin

	if(animate)
	{
		hideHUD();
		textAnimator->move(text->getSize().x(), 0, ANIMATION_MILLIS*2);
	} else {
		text->setPosition(0,0);
	}
}

void GuiConsoleList::setLogo(bool animate)
{
	//update, reset and animate logo
	logoAnimator->reset();

	logo->setImage(SystemData::sSystemVector.at(mCurrentIndex)->getLogo());
	
	if(animate)
	{
		hideHUD();
		logoAnimator->move(0, -(Renderer::getScreenHeight()*0.225), ANIMATION_MILLIS/4);
	} else {
		logo->setPosition(Renderer::getScreenWidth() /2, Renderer::getScreenHeight() - Renderer::getScreenHeight()/4 );
	}
}

void GuiConsoleList::setImages(bool animate)
{
	for(unsigned int i = 0; i < SystemData::sSystemVector.size(); i++)
	{
		ImageComponent* img = mImageVector.at(i);

		if(i == mCurrentIndex)
		{
			img->setResize(0, (unsigned int)(Renderer::getScreenHeight()/2), true);
			img->setOpacity(255);
		} else {
			img->setResize(0, (unsigned int)(Renderer::getScreenHeight()/5), true);
			img->setOpacity(150);
		}
	}
}

void GuiConsoleList::hideHUD()
{
	text->setPosition(- text->getSize().x(), 0);
	logo->setPosition(Renderer::getScreenWidth() / 2, Renderer::getScreenHeight());
}