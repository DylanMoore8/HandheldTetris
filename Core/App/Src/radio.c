/**
 * Author: Dylan Moore
 * Date: Nov 13, 2026
 * 
 * Description:
 *  Contains implementation of the interface for radio module.
 *  ref: https://github.com/wdomski/SX1278-example/blob/master/Core/Src/main.c
 */

#include <string.h>

#include "SX1278.h"

#include "radio.h"

typedef struct 
{
    bool initialized;
    radiohw_t hw;
    SX1278_t module;
} radio_t;

typedef struct
{
    sessionState_t state;
    uint8_t opponentId; 
} sessionInfo_t;

__weak uint8_t getId( void ) {return BOARD_ID;}

static radio_t radio = {0};
static SX1278_hw_t moduleHw = {0};

static sessionInfo_t session = {0};

static unsigned long receivedPacketCount = 0;
static unsigned long sentPacketCount = 0;

#define SESSION_TIMEOUT 3000 // after 3 seconds of no pings, the session times out

#define RADIO_MUTEX_TIMOUT_MS 1000

#define RADIO_TX_TIMEOUT_MS 2000 // max wait for TxDone (0 would wait forever)
#define RADIO_RX_TIMEOUT_MS 2000 // max wait for the chip to enter RX
#define RADIO_RX_POLL_MS    5    // receiver task poll period

availableOpponent availableOpponents[MAX_OPPONENTS] = {0};

radioError_t radioInit( radiohw_t * hw )
{
    /* get the radio hardware */
    if ( hw == NULL || hw->radioMutex == NULL ) return RADIO_ERR;
    radio.hw = *hw;
    
    // initialize LoRa module
    moduleHw.dio0.port = RADIO_DIO0_GPIO_Port;
	moduleHw.dio0.pin = RADIO_DIO0_Pin;
	moduleHw.nss.port = RADIO_NSS_GPIO_Port;
	moduleHw.nss.pin = RADIO_NSS_Pin;
	moduleHw.reset.port = RADIO_RST_GPIO_Port;
	moduleHw.reset.pin = RADIO_RST_Pin;
	moduleHw.spi = hw->spi;
    
    radio.module.hw = &moduleHw;

    SX1278_init( &radio.module, 434000000, SX1278_POWER_20DBM, SX1278_LORA_SF_7,
	    SX1278_LORA_BW_125KHZ, SX1278_LORA_CR_4_5, SX1278_LORA_CRC_EN, 10);

    SX1278_receive( &radio.module, RADIO_MESSAGE_SIZE, RADIO_RX_TIMEOUT_MS );

    /* set last: the other functions only touch the module once this is true,
       so init doesn't need the mutex */
    radio.initialized = true;

    return RADIO_OK;
}

/**
 * @brief calculate crc for given array of bytes
 * 
 * @param[in] bytes array of bytes 
 * @param[in] len length of bytes array
 * 
 * @return calculated crc
 */
static uint8_t calculateCRC( uint8_t * bytes, int len )
{
    uint8_t crc = 0;
    for ( int i = 0; i < len; i++ )
    {
        crc = (crc + bytes[i]) % 256;
    }
    return crc;
}

/**
 * @brief checks for validity of packet check sum 
 * 
 * @param[in] msg the message to check 
 */
static bool isValidPacket( radioMessage_t msg )
{
    /* calculate crc up to last byte */
    uint8_t crc = calculateCRC( (uint8_t*)&msg, RADIO_MESSAGE_SIZE - 1 );
    // TODO: will I be checking the byte order? Probably, but not here though
    return ( crc == msg.crc );
}

static unsigned long getPacketId()
{
    unsigned long id = sentPacketCount;
    sentPacketCount++;
    return id;
}

/**
 * @brief find an opponent in the available opponents table
 *
 * @param[in] opponentId id of the opponent
 *
 * @return pointer to the table entry, NULL if not found
 */
static availableOpponent * findOpponent( uint8_t opponentId )
{
    if ( opponentId == 0 ) return NULL; // 0 marks an empty slot
    for ( int i = 0; i < MAX_OPPONENTS; i++ )
    {
        if ( availableOpponents[i].id == opponentId ) return &availableOpponents[i];
    }
    return NULL;
}

/**
 * @brief add/update an opponent from their beacon, and enter the session
 *        if the opponent we requested has requested us back
 *
 * @param[in] msg a valid beacon message
 */
static void handleBeacon( const radioMessage_t * msg )
{
    availableOpponent * op = findOpponent( msg->header.id );

    /* new opponent, take the first free slot */
    if ( op == NULL )
    {
        for ( int i = 0; i < MAX_OPPONENTS; i++ )
        {
            if ( availableOpponents[i].id == 0 )
            {
                op = &availableOpponents[i];
                op->id = msg->header.id;
                break;
            }
        }
        if ( op == NULL ) return; // too many available opponents
    }

    /* always refresh with the latest beacon */
    memcpy( &op->info, msg->body, BEACON_PKT_LEN );
    op->lastPing = HAL_GetTick(); // for timeout detection

    /* check if the opponent we requested has requested us back */
    if ( session.state == WAITING &&
         session.opponentId == op->id &&
         op->info.opponentId == getId() &&
         ( op->info.state == WAITING || op->info.state == IN_SESSION ) )
    {
        /* we enter the session with them */
        session.state = IN_SESSION;
    }
}

void radioGetAvailableOpponents( availableOpponent * ops, int len )
{
    if ( ops == NULL ) return;
    if ( MAX_OPPONENTS == len )
    {
        memcpy(ops, availableOpponents, sizeof(availableOpponents));
    }
}

/**
 * @brief remove opponents we haven't heard from in SESSION_TIMEOUT ms.
 *        if it was our opponent, the session ends
 */
static void expireOpponents( void )
{
    uint32_t now = HAL_GetTick();
    for ( int i = 0; i < MAX_OPPONENTS; i++ )
    {
        availableOpponent * op = &availableOpponents[i];
        if ( op->id != 0 && ( now - op->lastPing ) > SESSION_TIMEOUT )
        {
            if ( session.opponentId == op->id )
            {
                session.opponentId = 0;
                session.state      = NONE;
            }
            memset( op, 0, sizeof( *op ) );
        }
    }
}

void radioReceiverTaskRunner( void )
{
    if ( radio.initialized )
    {
        if ( osMutexAcquire( radio.hw.radioMutex, RADIO_MUTEX_TIMOUT_MS) == osOK )
        {
            uint8_t size = SX1278_available( &radio.module );
            radioMessage_t msg;
    
            /* always read when a packet is available so the driver resets readBytes.
               SX1278_read copies the whole packet regardless of the length passed and
               appends a '\0', so read into a buffer large enough for any packet */
            uint8_t buf[SX1278_MAX_PACKET + 1];
            if ( size > 0 ) SX1278_read( &radio.module, buf, size );
    
            /* only accept packets that are exactly one of our messages */
            if ( size == RADIO_MESSAGE_SIZE )
            {
                memcpy( &msg, buf, RADIO_MESSAGE_SIZE );
            }
    
            if ( size == RADIO_MESSAGE_SIZE && isValidPacket( msg ) && msg.header.id != 0 )
            {
                // received a valid packet
                receivedPacketCount++;
    
                if ( msg.header.type == BEACON )
                {
                    /* beacons are handled here and not queued, so the lobby keeps
                       working even when nothing is reading the rx queue */
                    handleBeacon( &msg );
                }
                else if ( msg.header.type == DATA &&
                          session.state == IN_SESSION &&
                          msg.header.id == session.opponentId )
                {
                    /* only game data from our opponent goes to the game loop */
                    osMessageQueuePut( radio.hw.rxQueue, &msg, 0, 0 );
                }
            }
    
            expireOpponents();

            osMutexRelease( radio.hw.radioMutex );
        }
    }

    /* poll period; a packet takes ~30ms on air so this won't miss any */
    osDelay( RADIO_RX_POLL_MS );
}

void radioTransmitterTaskRunner( void )
{
    if ( radio.initialized )
    {
        radioMessage_t msg;
        /* block until the game loop queues something to send */
        if ( osMessageQueueGet( radio.hw.txQueue, &msg, NULL, osWaitForever ) == osOK )
        {
            /* if we can't get the radio this message is dropped; game data is
               sent continuously so the next one replaces it */
            if ( osMutexAcquire( radio.hw.radioMutex, RADIO_MUTEX_TIMOUT_MS ) == osOK )
            {
                /* push messages from our queue to the radio module */
                SX1278_transmit( &radio.module, (uint8_t *)&msg, RADIO_MESSAGE_SIZE, RADIO_TX_TIMEOUT_MS );
                /* back to listening */
                SX1278_receive( &radio.module, RADIO_MESSAGE_SIZE, RADIO_RX_TIMEOUT_MS );

                osMutexRelease( radio.hw.radioMutex );
            }
        }
    }
    else
    {
        osDelay( RADIO_RX_POLL_MS ); // don't spin before init
    }
}


/**
 * @brief build a radio message around a body
 *
 * @param[in] body contents to place in the message body
 * @param[in] len  number of bytes in body (must be <= MAX_BODY_LEN)
 * @param[in] type message type
 * @param[out] msg the built message
 *
 * @return RADIO_OK if built, RADIO_ERR if body is NULL or too long
 */
static radioError_t radioBuildMessage( const void * body, size_t len, radioMessageType_t type, radioMessage_t * msg )
{
    if ( body == NULL || msg == NULL || len > MAX_BODY_LEN ) return RADIO_ERR;

    memset( msg, 0, sizeof( *msg ) );
    memcpy( msg->body, body, len ); // unused body bytes stay 0

    msg->header.id       = getId();
    msg->header.packetId = (uint8_t)getPacketId();
    msg->header.type     = type;

    msg->crc = calculateCRC( (uint8_t*)msg, RADIO_MESSAGE_SIZE - 1 );

    return RADIO_OK;
}
/**
 * @brief determine the state of target opponent
 * 
 * @param[in] opponentId id of opponent to check availability of
 * @param[out] state state of the oppoent
 * @return state == NONE    && false -> opponent does not exist, 
 *         state == WAITING && false -> opponent is waiting for someone else,  
 *         state == WAITING && true  -> opponent is waiting for you,
 *         stae == IN_SESSION        -> opponent is in a game 
 * 
 */
static bool getOpponentState( uint8_t opponentId, sessionState_t * state )
{
    bool ret = false;
    *state = NONE;
    for ( int i = 0; i < MAX_OPPONENTS; i++ )
    {
        /* does opponent exist */
        if ( availableOpponents[i].id == opponentId )
        {
            *state = availableOpponents[i].info.state;
            if ( availableOpponents[i].info.opponentId == getId() )
            {
                ret = true;
            }
            else
            {
                ret = false;
            }
        }
    }
    return ret;
}

void radioBroadcast( char * displayName )
{
    if ( radio.initialized && displayName != NULL &&
         osMutexAcquire( radio.hw.radioMutex, RADIO_MUTEX_TIMOUT_MS ) == osOK )
    {
        radioBeaconMessageContents_t beacon = {0};
        strncpy( (char *)beacon.displayName, displayName, DISPLAY_NAME_LEN );

        /* if we are in a session, say so (read under the mutex so the
           receiver task can't change the session halfway through) */
        switch( session.state )
        {
            case NONE:
            {
                beacon.state = NONE;
            } break;
            case WAITING:
            {
                beacon.state = WAITING;
                beacon.opponentId = session.opponentId;
            } break;
            case IN_SESSION:
            {
                beacon.state = IN_SESSION;
                beacon.opponentId = session.opponentId;
            } break;
        }

        /* build message */
        radioMessage_t msg;
        if ( radioBuildMessage( &beacon, BEACON_PKT_LEN, BEACON, &msg ) == RADIO_OK )
        {
            /* send beacon */
            SX1278_transmit( &radio.module, (uint8_t*) &msg, RADIO_MESSAGE_SIZE, RADIO_TX_TIMEOUT_MS );
            /* back to listening */
            SX1278_receive( &radio.module, RADIO_MESSAGE_SIZE, RADIO_RX_TIMEOUT_MS );
        }

        osMutexRelease( radio.hw.radioMutex );
    }
}

radioError_t radioSessionRequestJoin( uint8_t opponentId )
{
    radioError_t err = RADIO_ERR;
    /* the session and opponent table are also updated by the receiver task */
    if ( radio.initialized &&
         osMutexAcquire( radio.hw.radioMutex, RADIO_MUTEX_TIMOUT_MS ) == osOK )
    {
        sessionState_t opState = NONE;
        bool waitingForUs = getOpponentState( opponentId, &opState );

        if ( findOpponent( opponentId ) == NULL )
        {
            /* opponent doesn't exist */
        }
        else if ( opState == WAITING && waitingForUs )
        {
            /* they already requested us, so we can join the session */
            session.opponentId = opponentId;
            session.state      = IN_SESSION;
            err                = RADIO_OK;
        }
        else if ( opState == NONE )
        {
            /* they are idle, request them and wait for them to request us back */
            session.opponentId = opponentId;
            session.state      = WAITING;
            err                = RADIO_OK;
        }
        /* else they are waiting for someone else or in a game */

        osMutexRelease( radio.hw.radioMutex );
    }
    return err;
}

void radioSessionWithdrawlRequestJoin( void )
{
    if ( radio.initialized &&
         osMutexAcquire( radio.hw.radioMutex, RADIO_MUTEX_TIMOUT_MS ) == osOK )
    {
        session.opponentId = 0;
        session.state      = NONE;

        osMutexRelease( radio.hw.radioMutex );
    }
}

radioError_t radioSendData( radioDataMessageContents_t * body )
{
    radioError_t err = RADIO_ERR;
    if ( radio.initialized )
    {
        radioMessage_t msg;
        if ( radioBuildMessage( body, DATA_PKT_LEN, DATA, &msg ) == RADIO_OK &&
             osMessageQueuePut( radio.hw.txQueue, &msg, 0, 0 ) == osOK )
        {
            err = RADIO_OK;
        }
    }
    return err;
}


radioError_t radioGetData( radioDataMessageContents_t * body )
{
    radioError_t err = RADIO_ERR;
    if ( radio.initialized && body != NULL )
    {
        radioMessage_t msg;
        if ( osMessageQueueGet( radio.hw.rxQueue, &msg, NULL, 0 ) == osOK )
        {
            if ( msg.header.type == DATA )
            {
                memcpy( body, msg.body, DATA_PKT_LEN );
                err = RADIO_OK;
            }
        }
    }
    return err;
}

sessionState_t getSessionState( void )
{
    return session.state;
}

uint8_t getOpponentId( void )
{
    return session.opponentId;
}