
#ifndef OBJECTS_H
#define OBJECTS_H

#include <stdint.h>

struct Node
{
    int x;
    int y;
};


enum ObjectType
{
    reward,
    obstacle,
    damaging
};
enum ObjectName 
{
    middleFloorN,
    triangleN,
    upFloorN,
    endingN,
    cloudFloorN,
    rectangleN,
    heartsN
};

struct colliderX
{
    int x;
    int upY;
    int bottomY;
    struct colliderX *next;
   
};
struct colliderY
{
    int y;
    int leftX;
    int rightX;
    struct colliderY *next;
};

struct colliders
{
   struct colliderX *colliderX ;
   struct colliderY *colliderY;

};

struct Object
{
    enum ObjectName object;
    struct Node *sprite;
    int spriteSize;
    struct colliders *colliders;
};

struct ColliderObject
{
    int x;
    int y;
    struct Object* obj;
};

enum LevelCompleted
{
    yes,
    no
};






enum PlayerState
{
    isAlive,
    isOnFloor,
    speedX2,
    isJumping,
    isBeingRevived,
    isRecovering,
    isRecovered
};
enum Direction
{
    left,
    right
};

struct Player
{
    enum PlayerState action;
    int x;
    int y;
    int velocityX;
    int velocityY;
    int height;
    int width;
    enum Direction direction;
    struct Node gravity;

    // Player action sprites ans its sizes
    struct Node *sprite;
    struct Node *spriteJump;
    struct Node *spriteAir;
    struct Node *spriteFall;
    struct Node *spriteDash;
    int spriteSize;
    int spriteJumpSize;
    int spriteAirSize;
    int spriteFallSize;
    int spriteDashSize;
};

struct level
{
    int levelLength;
    int numObjects;
    struct Object *levelObjects[];
};


#endif


