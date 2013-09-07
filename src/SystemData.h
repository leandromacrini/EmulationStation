#ifndef _SYSTEMDATA_H_
#define _SYSTEMDATA_H_

#include <vector>
#include <string>
#include "FolderData.h"
#include "Window.h"

class GameData;

class SystemData
{
public:
	SystemData(const std::string& name, const std::string& descName, const std::string& startPath, const std::string& extension, const std::string& command, const std::string& image, const std::string& logo, const std::string& relaseDate, const std::string& manufacturer);
	~SystemData();

	FolderData* getRootFolder();
	std::string getName();
	std::string getImage();
	std::string getDescName();
	std::string getStartPath();
	std::string getExtension();
	std::string getGamelistPath();
	std::string getLogo();
	std::string getRelaseDate();
	std::string getManufacturer();
	
	bool hasGamelist();

	void launchGame(Window* window, GameData* game);

	static void deleteSystems();
	static bool loadConfig(const std::string& path, bool writeExampleIfNonexistant = true); //Load the system config file at getConfigPath(). Returns true if no errors were encountered. An example can be written if the file doesn't exist.
	static void writeExampleConfig(const std::string& path);
	static std::string getConfigPath();
	static std::string getBlankConsoleImagePath();

	static std::vector<SystemData*> sSystemVector;
private:
	std::string mName;
	std::string mDescName;
	std::string mStartPath;
	std::string mSearchExtension;
	std::string mLaunchCommand;
	std::string mImage;
	std::string mRelaseDate;
	std::string mManufacturer;
	std::string mLogo;

	void populateFolder(FolderData* folder);

	FolderData* mRootFolder;
};

#endif
