/**
 * Author: Dylan Moore
 * Date: Nov 13, 2026
 * 
 * Description:
 *  Contains interface for radio module.
 */

#include "SX1278_hw.h"

#include "cmsis_os2.h"
#include "spi.h"
#include "cmsis_os.h"

#define DISPLAY_NAME_LEN 3

typedef struct
{
    SPI_HandleTypeDef * spi;
    osMessageQueueId_t  rxQueue; // handle is already a pointer type
    osMessageQueueId_t  txQueue;
    osMutexId_t         radioMutex;
} radiohw_t;

typedef enum
{
    BEACON = 0,
    DATA   = 1
} radioMessageType_t; 

typedef enum
{
    NONE       = 0, // idle
    WAITING,        // requested to join 
    IN_SESSION,     // in a session with another plater
} sessionState_t;

/* structs sent over the air are packed so their byte layout has no padding */
typedef struct __attribute__((packed))
{
    uint8_t state;
    uint8_t opponentId;
    uint8_t displayName[DISPLAY_NAME_LEN];
} radioBeaconMessageContents_t;
#define BEACON_PKT_LEN sizeof( radioBeaconMessageContents_t )

typedef struct __attribute__((packed))
{
    uint8_t  gameState;
    uint16_t score;
    uint16_t lines;
} radioDataMessageContents_t;
#define DATA_PKT_LEN sizeof( radioDataMessageContents_t )

#define MAX_BODY_LEN ( ( BEACON_PKT_LEN > DATA_PKT_LEN ) ? BEACON_PKT_LEN : DATA_PKT_LEN )

typedef struct __attribute__((packed))
{
    struct
    {
        uint8_t id;
        uint8_t type; // radioMessageType_t
        uint8_t packetId; // for syncronization if needed
    } header;
    uint8_t body[MAX_BODY_LEN];
    uint8_t crc;
} radioMessage_t;
#define RADIO_HEADER_LEN 3
#define RADIO_MESSAGE_SIZE sizeof(radioMessage_t)

typedef enum
{
    RADIO_OK = 0,
    RADIO_ERR
} radioError_t;

/**
 * @brief initialize radio module
 * 
 * @param[in] hw radio hardware descriptor
 */
radioError_t radioInit( radiohw_t * hw );

/**
 * @brief Broadcast beacon message to look for lobby
 */
void radioBroadcast( char * displayName );

/**
 * @brief Request to play with an opponent
 * 
 * @param[in] opponentId the opponent id of the opponent you want to play with
 * @return err
 */
radioError_t radioSessionRequestJoin( uint8_t opponentId );

/**
 * @brief sends bytes over radio connection
 * 
 * @param[in] bytes array of bytes being sent
 * @param[in] len length od bytes
 */
radioError_t radioSessionSendPacket( uint8_t * bytes, int len );

/**
 * @brief receives bytes from radio connection
 * 
 * @param[in] bytes array of bytes received
 * @param[in] len number of bytes received
 */
radioError_t radioSessionReceivePacket( uint8_t * bytes, int len );

/**
 * @brief implementation of radio receiver function
 */
void radioReceiverTaskRunner( void );
/**
 * @brief implementation of radio transmitter function
 */
void radioTransmitterTaskRunner( void );

/**
 * @brief get our session state
 * 
 * @return session state
 */
sessionState_t getSessionState( void );

/**
 * @brief get our targeted opponent id
 * 
 * @return targeted opponent id
 */
uint8_t getOpponentId( void );