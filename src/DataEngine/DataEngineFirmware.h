/**
* @file  DataEngineFirmware.h
* @brief Firmware version probe/check helpers for DataEngine
*/

#ifndef _DATA_ENGINE_FIRMWARE_H
#define _DATA_ENGINE_FIRMWARE_H

class DataEngine;

class DataEngineFirmware {
public:
	explicit DataEngineFirmware(DataEngine *engine);

	bool	getFirmwareVersions();
	bool	checkFirmwareVersions();

private:
	DataEngine *m_engine;
};

#endif // _DATA_ENGINE_FIRMWARE_H
