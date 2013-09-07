#include "AnimationComponent.h"

AnimationComponent::AnimationComponent()
{
	mMoveX = 0;
	mMoveY = 0;
	mMoveSpeed = 0;
	mFadeRate = 0;
	mOpacity = 0;
}



void AnimationComponent::move(int x, int y, int speed, std::tr1::function<void()> callback)
{
	mCallback = callback;

	mMoveX = x;
	mMoveY = y;
	mMoveSpeed = speed;
}

void AnimationComponent::fadeIn(int time, std::tr1::function<void()> callback)
{
	mCallback = callback;

	mOpacity = 0;
	setChildrenOpacity(0);

	mFadeRate = time;
}

void AnimationComponent::fadeOut(int time, std::tr1::function<void()> callback)
{
	mCallback = callback;

	mOpacity = 255;
	setChildrenOpacity(255);

	mFadeRate = -time;
}

//this should really be fixed at the system loop level...
void AnimationComponent::update(int deltaTime)
{
	if(mMoveX != 0 || mMoveY != 0)
	{
		//avoid division by zero and make a instant animation
		if(mMoveSpeed == 0) mMoveSpeed = 1;

		int deltaX = mMoveX * deltaTime / mMoveSpeed;
		int deltaY = mMoveY * deltaTime / mMoveSpeed;

		//if is last time we enter complete animation and erase data
		if( abs(deltaX) > abs(mMoveX) || abs(deltaY) > abs(mMoveY) )
		{
			deltaX = mMoveX;
			deltaY = mMoveY;

			mMoveSpeed = 0;
		}
		else
		{	
			mMoveX -= deltaX;
			mMoveY -= deltaY;

			mMoveSpeed -= deltaTime;
		}

		if(mMoveSpeed == 0 ){
			if(mCallback != nullptr) mCallback();

			reset();
		}

		moveChildren(deltaX, deltaY);
	}

	if(mFadeRate != 0)
	{
		int deltaOpacity = 0;
		
		if(mFadeRate < 0)
		{
			deltaOpacity = mOpacity * deltaTime / mFadeRate;
			mFadeRate += deltaTime;
		}
		else
		{
			deltaOpacity = (255 - mOpacity) * deltaTime / mFadeRate;
			mFadeRate -= deltaTime;
		}

		int opacity = (int)mOpacity + deltaOpacity;

		if(opacity > 255)
		{
			mFadeRate = 0;
			opacity = 255;

			//if we have a callback, call it
			if(mCallback != NULL) mCallback();
			
			reset();

		}

		if(opacity < 0)
		{
			mFadeRate = 0;
			opacity = 0;

			//if we have a callback, call it
			if(mCallback != NULL) mCallback();
			
			reset();

		}

		mOpacity = (unsigned char)opacity;
		setChildrenOpacity((unsigned char)opacity);
	}
}

bool AnimationComponent::isAnimating()
{
	if(mMoveX != 0 || mMoveY != 0 || mFadeRate != 0) return true;

	return false;
}

void AnimationComponent::reset()
{
	mCallback = nullptr;

	mMoveX = 0;
	mMoveY = 0;
	mMoveSpeed = 0;

	mFadeRate = 0;
}

void AnimationComponent::addChild(GuiComponent* gui)
{
	mChildren.push_back(gui);
}

void AnimationComponent::moveChildren(int offsetx, int offsety)
{
	Eigen::Vector3f move(offsetx, offsety, 0);
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		GuiComponent* comp = mChildren.at(i);
		comp->setPosition(comp->getPosition() + move);
	}
}

void AnimationComponent::setChildrenOpacity(unsigned char opacity)
{
	for(unsigned int i = 0; i < mChildren.size(); i++)
	{
		mChildren.at(i)->setOpacity(opacity);
	}
}
