/**
* @file  DataEngineLifecycle.h
* @brief Protocol/engine bring-up and teardown helpers for DataEngine
*/

#ifndef _DATA_ENGINE_LIFECYCLE_H
#define _DATA_ENGINE_LIFECYCLE_H

class DataEngine;

class DataEngineLifecycle {
public:
	explicit DataEngineLifecycle(DataEngine *engine);

	bool	start();
	void	stop();
	bool	findHPSDRDevices();
	bool	startDataEngineWithoutConnection();
	bool	initDataEngine();
	void	searchHpsdrNetworkDevices();

private:
	DataEngine *m_engine;
};

#endif // _DATA_ENGINE_LIFECYCLE_H
