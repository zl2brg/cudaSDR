/**
* @file  DataEngineSoapy.h
* @brief SoapySDR discovery/start helpers for DataEngine
*/

#ifndef _DATA_ENGINE_SOAPY_H
#define _DATA_ENGINE_SOAPY_H

class DataEngine;

class DataEngineSoapy {
public:
	explicit DataEngineSoapy(DataEngine *engine);

#ifdef HAVE_SOAPYSDR
	void	searchSoapyDevices();
	bool	startSoapyEngine();
#endif

private:
	DataEngine *m_engine;
};

#endif // _DATA_ENGINE_SOAPY_H
