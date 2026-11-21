
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include <string.h>
#include "screen.h"
#include "stdbool.h"
#include "rng.h"
#include "input.h"
#include "sprites.h"

#define GAME_TICK ( 50 )
#define TICKS_PER_SEC ( 1000 / GAME_TICK )
#define PIECE_FALL_TICKS ( 500 / GAME_TICK )
#define PIECE_MOVE_TICKS ( 100 / GAME_TICK )
#define CLEAR_ANIMATION_TICKS ( 50 / GAME_TICK )

const static SpriteMap background = {
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,SpP_U,Sp1_U,SpS_U,SpC_U,SpO_U,SpR_U,SpE_U,SpCLN,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,SpP_L,Sp1_L,SpS_L,SpC_L,SpO_L,SpR_L,SpE_L,SpCLN,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,SpP_U,Sp2_U,SpS_U,SpC_U,SpO_U,SpR_U,SpE_U,SpCLN,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,SpP_L,Sp2_L,SpS_L,SpC_L,SpO_L,SpR_L,SpE_L,SpCLN,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07},
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09, 0x4A, 0x4B, 0x4C},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x0A, 0x07, 0x07, 0x07},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x0A, 0x07, 0x07, 0x07},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09, 0x4D, 0x4E, 0x4F},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x0A, 0x07, 0x07, 0x07},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x0A, 0x07, 0x07, 0x07},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  }
};

static SpriteMap foreground = { 0 };

// PIECE

typedef enum
{
    BLOCK_TYPE_EMPTY = 0,
    BLOCK_TYPE_1 = 1,
    BLOCK_TYPE_2,
    BLOCK_TYPE_3,
    BLOCK_TYPE_4,
} BlockType;

typedef struct
{
    int8_t x;
    int8_t y;
} PieceBlockOffset;

typedef struct
{
    PieceBlockOffset blocks[4];
} PieceStructure;

typedef struct
{
    PieceStructure rotations[4]; // Every index increase is a rotation 90 degrees CCW.
} Piece;

// Helpful: https://tetris.wiki/Super_Rotation_System

static const Piece iPiece = {
    {
        {{ {0, 0}, {-1, 0}, {1, 0}, {2, 0} }},
        {{ {1, 0}, {1, -1}, {1, 1}, {1, 2} }},
        {{ {0, 1}, {-1, 1}, {1, 1}, {2, 1} }},
        {{ {0, 0}, {0, -1}, {0, 1}, {0, 2} }},
    }
};

static const Piece jPiece = {
    {
        {{ {0, 0}, {-1, 0}, {1, 0}, {-1, -1} }},
        {{ {0, 0}, {0, -1}, {0, 1}, {1, -1} }},
        {{ {0, 0}, {-1, 0}, {1, 0}, {1, 1} }},
        {{ {0, 0}, {0, -1}, {0, 1}, {-1, 1} }},
    }
};

static const Piece lPiece = {
    {
        {{ {0, 0}, {-1, 0}, {1, 0}, {1, -1} }},
        {{ {0, 0}, {0, -1}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {-1, 0}, {1, 0}, {-1, 1} }},
        {{ {0, 0}, {0, -1}, {0, 1}, {-1, -1} }},
    }
};

static const Piece oPiece = {
    {
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }},
    }
};

static const Piece sPiece = {
    {
        {{ {0, 0},  {-1, 0},  {0, -1}, {1, -1} }},
        {{ {0, 0},  {0, -1},  {1, 0},  {1, 1} }},
        {{ {0, 0},  {0, 1},   {1, 0},  {-1, 1} }},
        {{ {0, 0},  {-1, -1}, {-1, 0}, {0, 1} }},
    }
};

static const Piece tPiece = {
    {
        {{ {0, 0}, {-1, 0}, {1, 0}, {0, -1} }},
        {{ {0, 0}, {0, -1}, {0, 1}, {1, 0} }},
        {{ {0, 0}, {1, 0}, {-1, 0}, {0, 1} }},
        {{ {0, 0}, {0, 1}, {0, -1}, {-1, 0} }}
    }
};

static const Piece zPiece = {
    {
        {{ {0, 0},  {1, 0},  {0, -1}, {-1, -1} }},
        {{ {0, 0},  {0, 1},  {1, 0},  {1, -1} }},
        {{ {0, 0},  {0, 1},   {-1, 0},  {1, 1} }},
        {{ {0, 0},  {-1, 1}, {-1, 0}, {0, -1} }},
    }
};

static const Piece *pieceBag[] = {
    &iPiece, &jPiece, &lPiece, &oPiece, &sPiece, &tPiece, &zPiece
};
#define NUMBER_OF_PIECES ( sizeof(pieceBag) / sizeof(Piece *) )

/**
 * @brief Shuffles the pieces in the pieces bag
 * @note TETRIS uses a random bag, so this is needed.
 */
static void shufflePieces()
{
    for (uint32_t i = NUMBER_OF_PIECES - 1; i > 0; i--) {
        uint32_t j = pcg32() % (i + 1);

        const Piece * temp = pieceBag[i];
        pieceBag[i] = pieceBag[j];
        pieceBag[j] = temp;
    }
}

/**
 * @brief Gets the next piece for queueing
 * @note Automatically pulls from piece bag, and shuffles.
 * @note Just keep calling this, no need to manually shuffle.
 */
static const Piece *getNextPiece()
{
    static uint32_t currentPieceIndex = NUMBER_OF_PIECES;

    if( currentPieceIndex == NUMBER_OF_PIECES )
    {
        shufflePieces();
        currentPieceIndex = 0;
    }
    
    const Piece *piece = pieceBag[currentPieceIndex];
    currentPieceIndex++;

    return piece;
}

typedef struct
{
    uint8_t x;
    uint8_t y;
} Position;

typedef struct
{
    BlockType blockType;
    Position pos; // Relative to the board
    uint8_t rotation; // Must be between 0 to 3
    const Piece *structure;
} PieceActive;

PieceActive nextPiece = { 0 };

PieceActive heldPiece = { 0 };
bool heldPieceAvailable = false;

PieceActive activePiece = { 0 };

// BOARD

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

typedef BlockType Board[20][10];

static Board board = { 0 };
static const Position boardPos = { 2, 9 };

static uint8_t fullLines[BOARD_HEIGHT];
static uint8_t fullLinesSize;
static uint8_t fullLinesAvailable = false;

static const void drawBoard()
{
    for( uint8_t y = 0; y < BOARD_HEIGHT; y++ )
    {
        for( uint8_t x = 0; x < BOARD_WIDTH; x++ )
        {
            screenPutSprite( foreground, x + boardPos.x, y + boardPos.y, board[y][x] );
        }
    }
}

static void pieceAbsolute( PieceActive *piece, PieceStructure *abs_position )
{
    // Get rotated structure of piece
    const PieceStructure *piece_structure = &piece->structure->rotations[piece->rotation];

    for( int i = 0; i < 4; i++ )
    {
        abs_position->blocks[i].x = piece_structure->blocks[i].x + piece->pos.x;
        abs_position->blocks[i].y = piece_structure->blocks[i].y + piece->pos.y;
    }
}

static const void drawPiece( PieceActive *piece )
{
    PieceStructure abs_position;
    pieceAbsolute( piece, &abs_position );

    for( uint8_t i = 0; i < 4; i++ )
    {
        int8_t block_x = abs_position.blocks[i].x;
        int8_t block_y = abs_position.blocks[i].y;

        if( block_y >= 0 )
        {
            screenPutSprite( foreground, block_x + boardPos.x, block_y + boardPos.y, piece->blockType );
        }
    }
}

#define DISPLAY_ROTATION ( 1 )

#define NXT_LOCATION_X 13
#define NXT_LOCATION_Y 13
#define HLD_LOCATION_X 13
#define HLD_LOCATION_Y 21

static void showPieceInDisplay( PieceActive * piece, uint8_t x, uint8_t y )
{
    const PieceStructure *piece_disp = &piece->structure->rotations[DISPLAY_ROTATION];
    for( uint8_t i = 0; i < 4; i++ )
    {
        int8_t block_x = piece_disp->blocks[i].x;
        int8_t block_y = piece_disp->blocks[i].y;

        screenPutSprite( foreground, block_x + x, block_y + y, piece->blockType );
    }
}

static bool pieceOverlapping( PieceActive *piece )
{
    PieceStructure abs_position;
    pieceAbsolute( piece, &abs_position );

    bool overlapping = false;

    for( int i = 0; i < 4; i++ )
    {
        int8_t block_x = abs_position.blocks[i].x;
        int8_t block_y = abs_position.blocks[i].y;

        // If piece is outside horizontal boundaries:
        if( block_x < 0 || block_x >= BOARD_WIDTH )
        {
            overlapping = true;
            break;
        }
        // If block is below the floor:
        else if( block_y >= BOARD_HEIGHT ) // We won't check if the block is above the board.
        {
            overlapping = true;
            break;
        }
        // If block is clipping block on board:
        else if( block_y >= 0 && board[block_y][block_x] != BLOCK_TYPE_EMPTY )
        {
            overlapping = true;
            break;
        }
    }

    return overlapping;
}

static void copyPieceToBoard( PieceActive *piece )
{
    PieceStructure abs_position;
    pieceAbsolute( piece, &abs_position );

    BlockType blockType = piece->blockType;

    for( int i = 0; i < 4; i++ )
    {
        int8_t block_x = abs_position.blocks[i].x;
        int8_t block_y = abs_position.blocks[i].y;

        if( block_y >= 0 )
        {
            board[block_y][block_x] = blockType;
        }
    }
}

typedef enum
{
    PIECE_UP = 0,
    PIECE_DOWN,
    PIECE_LEFT,
    PIECE_RIGHT,
    PIECE_CW,
    PIECE_CCW,    
} PieceMovements;

#define INC_ROTATION(rot) ( rot == 3 ? 0 : ( rot + 1 ) )
#define DEC_ROTATION(rot) ( rot == 0 ? 3 : ( rot - 1 ) )

/**
 * @brief Attempts to move the piece.
 * 
 * @param piece Piece to move
 * @param movement Type of movement
 * 
 * @return true Piece moved successfully
 * @return false Could not move piece (overlap)
 */
static bool pieceMove( PieceActive *piece, PieceMovements movement )
{
    bool rc = true;
    switch(movement)
    {
        case(PIECE_UP):
        {
            piece->pos.y--;
            if( pieceOverlapping( piece ) ) {
                piece->pos.y++;
                rc = false;
            }
        } break;
        case(PIECE_DOWN):
        {
            piece->pos.y++;
            if( pieceOverlapping( piece ) ) {
                piece->pos.y--;
                rc = false;
            }
        } break;
        case(PIECE_LEFT):
        {
            piece->pos.x--;
            if( pieceOverlapping( piece ) ) {
                piece->pos.x++;
                rc = false;
            }
        } break;
        case(PIECE_RIGHT):
        {
            piece->pos.x++;
            if( pieceOverlapping( piece ) ) {
                piece->pos.x--;
                rc = false;
            }
        } break;
        case(PIECE_CW):
        {
            piece->rotation = INC_ROTATION(piece->rotation);
            if( pieceOverlapping( piece ) ) {
                piece->rotation = DEC_ROTATION(piece->rotation);
                rc = false;
            }
        } break;
        case(PIECE_CCW):
        {
            piece->rotation = DEC_ROTATION(piece->rotation);
            if( pieceOverlapping( piece ) ) {
                piece->rotation = INC_ROTATION(piece->rotation);
                rc = false;
            }
        } break;
    }

    return rc;
}

static const Position startingPiecePos = (Position){ 4, 0 };
static const uint8_t startingPieceRotation = 0;

static void placeNewPiece( PieceActive * piece )
{
    piece->blockType = (pcg32() % 4) + 1; // Random color (1-4)
    piece->pos = startingPiecePos; // Place in center top
    piece->rotation = startingPieceRotation; // Initial rotation
    piece->structure = getNextPiece(); // Grab from random bag
}

static void resetPieceLocation( PieceActive * piece )
{
    piece->pos = startingPiecePos;
    piece->rotation = startingPieceRotation;
}

void holdPiece()
{
    PieceActive tmp;

    if( heldPieceAvailable )
    {
        // Swap held with active

        resetPieceLocation( &heldPiece );
        resetPieceLocation( &activePiece );

        tmp = heldPiece;
        heldPiece = activePiece;
        activePiece = tmp;
    }
    else {
        // Active to held, next to active, new to next
        resetPieceLocation( &heldPiece );
        resetPieceLocation( &activePiece );
        resetPieceLocation( &nextPiece );

        heldPiece = activePiece;
        activePiece = nextPiece; 
        placeNewPiece(&nextPiece);

        heldPieceAvailable = true;
    }
}

bool isLineFull( uint8_t lineInx )
{
    bool lineFull = true;
    for( int x = 0; x < BOARD_WIDTH; x++ ) // For every block on row 'lineInx'
    {
        if( board[lineInx][x] == BLOCK_TYPE_EMPTY ) // If the block is empty
        {
            lineFull = false;
            break;
        }
    }
    return lineFull;
}

/**
 * @brief Checks if any lines (along the vertical range of a piece) are full.
 * 
 * @param piece Piece to search along
 * @param linesFull Buffer to store list of line indicies that are full (max: BOARD_HEIGHT)
 * @param linesFullSize Size of `linesFull`.
 * @return true Full line(s) found.
 * @return false No full lines found.
 */
static bool checkFullLinesAlongPiece(PieceActive *piece, uint8_t *linesFull, uint8_t *linesFullSize)
{
    bool lineFull = false;

    PieceStructure absPosition;
    pieceAbsolute( piece, &absPosition );

    bool foundLine[BOARD_HEIGHT]; // Used to prevent duplicates from being listed.
    memset(foundLine, false, BOARD_HEIGHT);

    uint8_t numLinesFull = 0;
    for( int i = 0; i < 4; i++ )
    {
        int8_t block_y = absPosition.blocks[i].y;

        if( isLineFull(block_y) && !foundLine[block_y] )
        {
            linesFull[numLinesFull] = block_y;

            // sort (for line falling purposes)
            for( int y = numLinesFull; y > 0; y-- )
            {
                if( linesFull[y] < linesFull[y-1] )
                {
                    uint8_t tmp = linesFull[y];
                    linesFull[y] = linesFull[y-1];
                    linesFull[y-1] = tmp;
                }
                else
                {
                    break;
                }
            }

            foundLine[block_y] = true;
            numLinesFull++;

            lineFull = true;
        }
    }

    *linesFullSize = numLinesFull;
    
    return lineFull;
}

void landPieceAndCheckLines()
{
    copyPieceToBoard(&activePiece);

    // Check for full lines
    fullLinesAvailable = checkFullLinesAlongPiece(&activePiece, fullLines, &fullLinesSize);

    activePiece = nextPiece;
    placeNewPiece(&nextPiece);
}

void fallLine(uint8_t lineInx)
{
    for( uint8_t i = lineInx; i > 0; i-- )
    {
        memcpy(&board[i], &board[i-1], BOARD_WIDTH);
    }
    return;
}

void fallFullLines()
{
    for(uint8_t i = 0; i < fullLinesSize; i++)
    {
        fallLine(fullLines[i]);
    }
}

// GAME TASK

osThreadId_t gameTaskHandle;
uint32_t gameTaskBuffer[ 512 ];
StaticTask_t gameTaskControlBlock;
const osThreadAttr_t gameTask_attributes = {
  .name = "gameTask",
  .cb_mem = &gameTaskControlBlock,
  .cb_size = sizeof(gameTaskControlBlock),
  .stack_mem = &gameTaskBuffer[0],
  .stack_size = sizeof(gameTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};

// INPUTS

extern osMessageQueueId_t inputQueue; // from input.c

bool pieceLanded = false;
bool holdPieceLock = false;

#define INPUT_HOLD_NONE ( 255 )
InputType inputHoldKey = INPUT_HOLD_NONE; // Nothing

static uint32_t tickCount = 0;
static uint32_t pieceFallTickCount = 0;
static uint32_t pieceMoveTickCount = 0;
static uint32_t clearAnimationTickCount = 0;

static void handleInput()
{
    InputEvent inputEvent;
    if( osMessageQueueGet(inputQueue, &inputEvent, 0, 0 ) == osOK ) // If an input is queued:
    {
        if( inputEvent.state == INPUT_PRESSED )
        {
            switch(inputEvent.type) {
                case(INPUT_DOWN):
                case(INPUT_LEFT):
                case(INPUT_RIGHT): {
                    inputHoldKey = inputEvent.type; // Key to move repeatedly on hold
                    pieceMoveTickCount = PIECE_MOVE_TICKS; // Sets the piece to move immediately on input
                } break;
                case(INPUT_A): {
                    pieceMove(&activePiece, PIECE_CW);
                } break;
                case(INPUT_B): {
                    if( holdPieceLock == false )
                    {
                        holdPiece();
                        holdPieceLock = true; // Lock the hold functionality;
                    }
                } break;
                default: {} break;
            }
        }
        else // Key was released:
        {
            if( inputEvent.type == inputHoldKey )
            {
                inputHoldKey = INPUT_HOLD_NONE;
            }
        }

    }
}

static void handleMovement()
{
    switch( inputHoldKey ) {
        case(INPUT_DOWN): { 
            bool hit_block = !pieceMove(&activePiece, PIECE_DOWN);
            if( hit_block )
            {
                pieceLanded = true; // trigger land flag
            }
        } break;
        case(INPUT_LEFT): {
            pieceMove(&activePiece, PIECE_LEFT);
        } break;
        case(INPUT_RIGHT): {
            pieceMove(&activePiece, PIECE_RIGHT);
        } break;
        default: {} break;
    }
}

typedef enum
{
    GAME_NORMAL,
    GAME_CLEAR_LINES,
    GAME_GAME_OVER,
} GameState;

static GameState gameState = GAME_NORMAL;

void gameTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

    pcg32_init(4402);

    placeNewPiece(&nextPiece);
    placeNewPiece(&activePiece);

    tickCount = 0;
    pieceFallTickCount = 0;
    pieceMoveTickCount = 0;
    clearAnimationTickCount = 0;

    uint8_t lineClearBlockAnimation = 0;

    uint32_t osTick = osKernelGetTickCount();

    for(;;)
    {
        screenReset(foreground);

        switch( gameState )
        {
            case(GAME_NORMAL):
            {
                // Move pieces downwards
                if( pieceFallTickCount == PIECE_FALL_TICKS )
                {
                    bool hit_block = !pieceMove(&activePiece, PIECE_DOWN);
                    if( hit_block )
                    {
                        pieceLanded = true;
                    }
        
                    pieceFallTickCount = 0;
                }
        
                if( !pieceLanded ) // Stop input if a piece lands
                {
                    handleInput();
                    if( pieceMoveTickCount == PIECE_MOVE_TICKS )
                    {
                        handleMovement();
                        pieceMoveTickCount = 0;
                    }
                    pieceMoveTickCount++;
                }
                else // If a piece lands
                {
                    landPieceAndCheckLines();
                    if( fullLinesAvailable ) // If a full line is available:
                    {
                        clearAnimationTickCount = 0;
                        lineClearBlockAnimation = 0;
                        gameState = GAME_CLEAR_LINES; // Transition to show clear lines animation
                    }
                    if( pieceOverlapping( &activePiece ) ) // Piece is clipping immediately after spawning
                    {
                        gameState = GAME_GAME_OVER;
                    }
                    pieceLanded = false; // Reset the flag
                    holdPieceLock = false; // Release the hold functionality
                }

                pieceFallTickCount++;
            } break;
            case(GAME_CLEAR_LINES):
            {
                if( clearAnimationTickCount == CLEAR_ANIMATION_TICKS ) // If the animation tick is triggered:
                {
                    if( lineClearBlockAnimation != BOARD_WIDTH ) // If the animation is not finished:
                    {
                        for( uint8_t i = 0; i < fullLinesSize; i++ ) // For every full line:
                        {
                            uint8_t lineIndex = fullLines[i]; // Get the full line index:
                            board[lineIndex][lineClearBlockAnimation] = BLOCK_TYPE_EMPTY; // Set the current animation block to clear.
                        }

                        lineClearBlockAnimation++;
                        clearAnimationTickCount = 0;
                    }
                    else
                    {
                        fallFullLines();

                        pieceFallTickCount = 0;
                        pieceMoveTickCount = 0;
                        gameState = GAME_NORMAL;
                    }
                }

                clearAnimationTickCount++;
            } break;
            case(GAME_GAME_OVER):
            {
                // TODO
            } break;
        }

        drawBoard();
        drawPiece(&activePiece);
        showPieceInDisplay(&nextPiece, NXT_LOCATION_X, NXT_LOCATION_Y);
        if( heldPieceAvailable )
        {
            showPieceInDisplay(&heldPiece, HLD_LOCATION_X, HLD_LOCATION_Y);
        }
        screenDrawBFDebug(background, foreground);
        
        osDelayUntil( osTick + GAME_TICK );
        osTick = osKernelGetTickCount();

        tickCount++;
    }

}

void gameInit()
{
    gameTaskHandle = osThreadNew(gameTask, NULL, &gameTask_attributes);
}