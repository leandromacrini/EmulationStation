#ifndef _SETTINGSMENU_H_
#define _SETTINGSMENU_H_

#include "../GuiComponent.h"
#include "ComponentListComponent.h"
#include <vector>
#include "SwitchComponent.h"
#include "SliderComponent.h"
#include "TextComponent.h"
#include "GuiBox.h"

class GuiSettingsMenu : public GuiComponent
{
public:
	GuiSettingsMenu(Window* window);
	~GuiSettingsMenu();

	bool input(InputConfig* config, Input input) override;

private:
	void loadStates();
	void applyStates();

	ComponentListComponent mList;

	GuiBox mBox;

	SwitchComponent mDrawFramerateSwitch;
	SliderComponent mVolumeSlider;
	SwitchComponent mDisableSoundsSwitch;
	TextComponent mSaveLabel;

	std::vector<GuiComponent*> mLabels;
};

#endif
