/*
 * bms.h
 *
 * Driver for the BMS. This controls all data access between the VCM
 * and BMS
 *
 *  Created on: Mar 9, 2019
 *      Author: Duemmer
 */

#ifndef SRC_DEVICE_BMS_H_
#define SRC_DEVICE_BMS_H_

#define BMS_NUM_CELLS               	20

// BMS CAN base ID. BMS frame numbers are added to this to build the actual ID
#define BMS_ID_BASE						0x6B3

// Timeouts for frames. If the frame isn't received for a period at least this
// this long, CVNP will flag an error. Units in ms.
#define BMS_FRAME0_TIMEOUT				50
#define BMS_FRAME1_TIMEOUT				2000
#define BMS_CELL_BROAD_TIMEOUT			100

// Fault Thresholds
#define BMS_CHG_OVERCURRENT_THRESH		60.0f	// Magnitude of maximum allowed charging current
#define BMS_DIS_OVERCURRENT_THRESH		-60.0f	// Magnitude of maximum allowed discharge current
#define BMS_TRANSIENT_OVERCURRENT_TIME	120		// number of milliseconds to allow a transient current greater than the max to occur before asserting a fault
#define BMS_MIN_CELL_WARN_VOLTS			3.2f
#define BMS_MIN_CELL_VOLTS				3.0f
#define BMS_MAX_CELL_WARN_VOLTS			3.9f
#define BMS_MAX_CELL_VOLTS				4.05f
#define BMS_PACK_MIN_VOLTS				62.0f
#define BMS_PACK_MIN_WARN_VOLTS			69.0f
#define BMS_PACK_MAX_WARN_VOLTS			80.0f
#define BMS_PACK_MAX_VOLTS				81.0f
#define BMS_IMBALANCE_THRESH			0.5f
#define BMS_CELL_MAX_TEMP				65.0f
#define BMS_CELL_MIN_TEMP				1.0f // set above 0 to catch potential issues with 0 degree listed temperatures

/**
 * Data for one cell
 */
typedef struct {
    float voltage;
    float resistance;
    bool valid;
} tCellData;


/**
 * Custom flag 0 for the BMS
 */
typedef struct {
    uint8_t rlyChgFault : 1;
    uint8_t rlyDisFault : 1;
    uint8_t cellOVFault : 1;
    uint8_t cellUVFault : 1;
    uint8_t thermistorFault : 1;
    uint8_t overTempFault : 1;
    uint8_t internalTempFault : 1;
    uint8_t cellOpenWiringFault : 1;
} tBmsFlag0;


/**
 * Custom flag 4 for the BMS
 */
typedef struct {
	uint8_t heatsinkThermistorFault : 1;
    uint8_t weakCellFault : 1;
    uint8_t currentSenseFault : 1;
    uint8_t isolationFault : 1;
    uint8_t internalCellCommFault : 1;
    uint8_t internalLogicFault : 1;
    uint8_t internalHardwareFault : 1;
} tBmsFlag4;


/**
 * Data for the BMS
 */
typedef struct {
    tCellData cellData[BMS_NUM_CELLS];
    tBmsFlag0 flag0;
    tBmsFlag4 flag4;
    float vCellAvg;
    float tAvg; // TODO: Implement average temperature
    float vBat;
    float iBat;
    float soc;
    float vInput;
    uint8_t vMaxIdx;
    uint8_t vMinIdx;
    uint8_t tMaxIdx;
    uint8_t tMinIdx;
    uint8_t tMin;
    uint8_t tMax;
    uint8_t rlyDis : 1;
    uint8_t rlyChg : 1;

} tBMSData;


/**
 * Initializes the motor controller driver
 */
void bms_init();


/**
 * Tick routine to be run every ~10ms. This is where this api can
 * periodically query, check stuff, etc.
 */
void bms_tick();


/**
 * Retrieves the instantaneous data for the BMS.
 */
tBMSData *bms_data();



/**
 * Function to be called when the dedicated DCL line from the BMS changes
 * state. This usually means that the short circuit current threshold has
 * been reached, and thus needs immediate action to be taken.
 */
void bms_onDCLGpioChange(bool isOn);


#endif /* SRC_DEVICE_BMS_H_ */













