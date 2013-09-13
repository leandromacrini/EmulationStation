#ifndef _ANIMATIONCOMPONENT_H_
#define _ANIMATIONCOMPONENT_H_

#include "../GuiComponent.h"
#include <vector>

#ifndef WIN32
	#include <tr1/functional>
#else
	#include <functional>
#endif

//just fyi, this is easily the worst animation system i've ever written.
//it was mostly written during a single lecture and it really shows in how un-thought-out it is
//it also hasn't been converted to use floats or vectors yet
class AnimationComponent
{
public:
	AnimationComponent();

	void move(int x, int y, int speed);//, std::tr1::function<void()> callback = nullptr);
	void fadeIn(int time);//, std::tr1::function<void()> callback = nullptr);
	void fadeOut(int time);//, std::tr1::function<void()> callback = nullptr);

	void update(int deltaTime);
	void reset();

	bool isAnimating();

	void addChild(GuiComponent* gui);

private:
	unsigned char mOpacity;

	std::vector<GuiComponent*> mChildren;

	void moveChildren(int offsetx, int offsety);
	void setChildrenOpacity(unsigned char opacity);

	int mFadeRate;
	int mMoveX, mMoveY, mMoveSpeed;

	std::tr1::function<void()> mCallback;
};

#endif
