/**
* @file  DataEngineThreadFactory.h
* @brief Worker thread create/start/stop helpers for DataEngine
*/

#ifndef _DATA_ENGINE_THREAD_FACTORY_H
#define _DATA_ENGINE_THREAD_FACTORY_H

#include <QThread>

class DataEngine;

class DataEngineThreadFactory {
public:
	explicit DataEngineThreadFactory(DataEngine *engine);

	void	createDiscoverer();
	bool	startDiscoverer(QThread::Priority prio);
	void	stopDiscoverer();

	void	createDataIO();
	bool	startDataIO(QThread::Priority prio);
	void	stopDataIO();

	void	createDataProcessor();
	bool	startDataProcessor(QThread::Priority prio);
	void	stopDataProcessor();

	void	createAudioOutProcessor();
	void	startAudioOutProcessor(QThread::Priority prio);
	void	stopAudioOutProcessor();

	void	createWideBandDataProcessor();
	bool	startWideBandDataProcessor(QThread::Priority prio);
	void	stopWideBandDataProcessor();

	void	createAudioReceiver();
	void	createAudioInputProcessor();

private:
	DataEngine *m_engine;
};

#endif // _DATA_ENGINE_THREAD_FACTORY_H
