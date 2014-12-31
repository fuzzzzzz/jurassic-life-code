#include "cbase.h"
#include "FMODManager.h"

#include "fmod_errors.h"

#include <string>

using namespace FMOD;

EventSystem		*pEventSystem;
Event			*pEvent = NULL;

FMOD_RESULT		result;

CFMODManager gFMODMng;
CFMODManager* FMODManager()
{
	return &gFMODMng;
}

void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        Warning("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
    }
}

CFMODManager::CFMODManager()
{
}

CFMODManager::~CFMODManager()
{
}

// Starts FMOD
void CFMODManager::InitFMOD( void )
{
	ERRCHECK(result = FMOD::EventSystem_Create(&pEventSystem));
	ERRCHECK(result = pEventSystem->init(64, FMOD_INIT_NORMAL, 0, FMOD_EVENT_INIT_NORMAL));

	std::string path = engine->GetGameDirectory();
	path += "\\";

	DevMsg("FMOD path %s\n",path.c_str());

	ERRCHECK(result = pEventSystem->setMediaPath(path.c_str()));
}

// Stops FMOD
void CFMODManager::ExitFMOD( void )
{
	ERRCHECK(result = pEventSystem->release());
}

bool CFMODManager::LoadDesignerFile(const char *filename)
{
	if (m_sCurrentFile != filename)
	{
		m_sCurrentFile = filename;
		DevMsg("FMOD file : %s\n",filename);
		
		pEventSystem->unload();
		ERRCHECK(result = pEventSystem->load(filename, 0, 0));

		pEventSystem->update();
		return result != 0;
	}
	return false;
}

void CFMODManager::UnloadDesignerFile()
{
	pEventSystem->unload();
}

bool CFMODManager::SetEvent(const char *eventName)
{
	if (pEvent)
		pEvent->stop();
	m_sCurrentEvent = eventName;
	ERRCHECK(result = pEventSystem->getEvent(eventName, FMOD_EVENT_DEFAULT, &pEvent));
	if (pEvent)
		pEvent->start();
	pEventSystem->update();
	return true;
}

bool CFMODManager::SetParameter(const char *name, float value)
{
	if (pEvent)
	{
		FMOD::EventParameter *param;
		//float param_min, param_max;
		ERRCHECK(result = pEvent->getParameter(name, &param));
		//ERRCHECK(result = param->getRange(&param_min, &param_max));
		ERRCHECK(result = param->setValue(value));

		pEventSystem->update();
		return true;
	}
	return false;
}

void CFMODManager::Stop()
{
	if (pEvent)
		pEvent->stop();
}
