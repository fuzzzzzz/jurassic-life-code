
#ifndef FMOD_MANAGER_H
#define FMOD_MANAGER_H

#include "fmod_event.hpp"

#include <string>

class CFMODManager
{
public:
	CFMODManager();
	~CFMODManager();
 
	void InitFMOD();
	void ExitFMOD();

	//FMOD Designer
	bool LoadDesignerFile(const char *filename);
	void UnloadDesignerFile();
	bool SetEvent(const char *eventName);

	bool SetParameter(const char *name, float value);

	void Stop();
protected:
	std::string m_sCurrentFile;
	std::string m_sCurrentEvent;
};

extern CFMODManager* FMODManager();
 
#endif //FMOD_MANAGER_H
