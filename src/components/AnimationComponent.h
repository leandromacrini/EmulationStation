#ifndef _ANIMATIONCOMPONENT_H_
#define _ANIMATIONCOMPONENT_H_

#include "../GuiComponent.h"
#include <vector>
#include <functional>

class AnimationComponent
{
public:
	AnimationComponent();

	void move(int x, int y, int speed, std::tr1::function<void()> callback = nullptr);
	void fadeIn(int time, std::tr1::function<void()> callback = nullptr);
	void fadeOut(int time, std::tr1::function<void()> callback = nullptr);

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
