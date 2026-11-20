
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "screen.h"
#include "stdbool.h"
#include "rng.h"

#define GAME_TICK ( 100 )
#define TICKS_PER_SEC ( 1000 / GAME_TICK )
#define PIECE_FALL_TICKS ( 500 / GAME_TICK )

const static SpriteMap background = {
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07},
  {  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09, 0x4D, 0x4E, 0x4F},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x0A, 0x07, 0x07, 0x07},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09,  0  ,  0  ,  0  },
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x0A, 0x07, 0x07, 0x07},
  {  0  , 0x05,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  ,  0  , 0x09, 0x4A, 0x4B, 0x4C},
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

// (1, 0) -> (0, 1)
// (0, 1) -> (-1, 0)
// (-1, 0) -> (0, -1)
// (0, -1) -> (1, 0)

static const Piece t_piece = {
    {
        {{ {0, 0}, {-1, 0}, {1, 0}, {0, -1} }},
        {{ {0, 0}, {0, -1}, {0, 1}, {1, 0} }},
        {{ {0, 0}, {1, 0}, {-1, 0}, {0, 1} }},
        {{ {0, 0}, {0, 1}, {0, -1}, {-1, 0} }}
    }
};

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

PieceActive activePiece = { 0 };

// BOARD

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

typedef BlockType Board[20][10];

static Board board = { 0 };
static const Position boardPos = { 2, 9 };

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
        else if( block_y > 0 && board[block_y][block_x] != BLOCK_TYPE_EMPTY )
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

        if( block_y > 0 )
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
 * @brief Attempts to move the piece
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

void gameTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

    board[19][5] = 1;
    board[19][4] = 1;
    board[19][3] = 1;
    board[18][3] = 1;

    // typedef struct
    // {
    //     BlockType blockType;
    //     Position pos; // Relative to the board
    //     uint8_t rotation; // Must be between 0 to 3
    //     Piece *structure;
    // } PieceActive;

    pcg32_init(44022);

    activePiece = (PieceActive){
        .blockType = (pcg32() % 4) + 1,
        .pos = (Position){ 4, 0 },
        .rotation = (pcg32() % 4),
        .structure = &t_piece
    };

    uint32_t tickCount = 0;
    uint32_t pieceFallTickCount = 0;

    uint32_t osTick = osKernelGetTickCount();

    for(;;)
    {
        
        if( pieceFallTickCount == PIECE_FALL_TICKS )
        {
            bool hit_block = !pieceMove(&activePiece, PIECE_DOWN);
            if( hit_block )
            {
                copyPieceToBoard(&activePiece);
                activePiece = (PieceActive){
                    .blockType = (pcg32() % 4) + 1,
                    .pos = (Position){ (pcg32() % 6) + 2, 0 },
                    .rotation = (pcg32() % 4),
                    .structure = &t_piece
                };
            }
            pieceFallTickCount = 0;
        }
        
        screenReset(foreground);
        drawBoard();
        drawPiece(&activePiece);
        screenDrawBFDebug(background, foreground);
        
        osDelayUntil( osTick + GAME_TICK );
        osTick = osKernelGetTickCount();
        tickCount++;

        pieceFallTickCount++;
    }
    /* USER CODE END StartDefaultTask */
}

void initGame()
{
    gameTaskHandle = osThreadNew(gameTask, NULL, &gameTask_attributes);
}