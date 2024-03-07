#include <stdio.h>
#include <stdint.h>
#include <pic32mx.h>
#include "graphics.h"
#include "inputoutput.h"
#include "player.h"
#include "utils.h"
#include "level.h"
#define sizez(sprite) (sizeof(sprite) / sizeof(sprite[0]))
char textbuffer[48];
struct Player *player;
struct level *currentLevel;
char highScoreList[20][16];
int highScoreIndex;
#define BUTTON1 0x8
#define BUTTON2 0x4
#define BUTTON3 0x2
#define BUTTON4 0x1

#define SWITCH1 0x80
#define SWITCH2 0x40
#define SWITCH3 0x20
#define SWITCH4 0x10

uint8_t numberBytes[] = {
	0xF8, 0x88, 0xF8,
	0x08, 0xF8, 0x00,
	0xE8, 0xA8, 0xB8,
	0x88, 0xA8, 0xF8,
	0x38, 0x20, 0xF8,
	0xB8, 0xA8, 0xE8,
	0xF8, 0xA8, 0xE8,
	0x08, 0xE8, 0x18,
	0xF8, 0xA8, 0xF8,
	0xB8, 0xA8, 0xF8};

// struct node with all the cordinates for the heart obejcts that will represent the amout of lives the player has left.
struct Node hearts[] =
	{

		{101, 1},
		{103, 1},
		{100, 2},
		{102, 2},
		{104, 2},
		{100, 3},
		{104, 3},
		{101, 4},
		{103, 4},
		{102, 5},

		{109, 1},
		{111, 1},
		{108, 2},
		{110, 2},
		{112, 2},
		{108, 3},
		{112, 3},
		{109, 4},
		{111, 4},
		{110, 5},

		{116, 1},
		{118, 1},
		{115, 2},
		{117, 2},
		{119, 2},
		{115, 3},
		{119, 3},
		{116, 4},
		{118, 4},
		{117, 5},

};

// struct object where we give the attributes for the heart object.
struct Object healthObject =
	{
		heartsN,
		hearts,
		sizez(hearts),
		0,
};

// Global varible 0 if game is not over and 1 if game is over.
int game_over = 0;
uint8_t mainBuffer[512];
double gravity = 0.0;
uint8_t input;
int cameraPosition = 0;
// how the camera should move right and left when there is movment.
int moveCameraLeft = 64;
int moveCameraRight = 64;
int score = 0;
int dir = 1;
struct Object *menu1;
struct Object *menu2;
struct Object *gameOver;
// counter for the amout of lives left
int chancesCounter = 0;
int levelIndex = 0;
int highScore = 9000;
int health = 3;
int lastLevels[4];
void damage();
void main_menu();
int8_t checkBounds(int playerB1, int playerB2, int colliderB2, int colliderB1);
int8_t on_floor();
void apply_gravity();
struct ColliderObject collision();
void update_player();
int8_t collisionY(int objectIndex, struct colliderY *collider);
int8_t collisionX(int objectIndex, struct colliderX *collider);

// function that applies gravity on the object. Checks the current position and applies gravity if the object is not on the floor. Uses help function on_floor
//  to determine the position of the player.
void apply_gravity()
{
	if (on_floor(player, currentLevel) == 0)
	{
		if (player->gravity.x >= 1)
		{
			if (player->gravity.y == 0)
			{
				player->gravity.x = 0;
				player->velocityY += 1;
			}
			else
				player->gravity.y--;
		}
		else
			player->gravity.x++;
	}
}

void update_player()
{
	// if the player is under the floor, call the damage function which will take a live and reset the position of the player.
	if (player->y > 39)
	{
		damage();
	}

	int xPrev = player->x;
	int Prev = player->y;

	player->x += player->velocityX;
	player->y += player->velocityY;

	struct ColliderObject collisions = collision();

	// checks for collisions in the y-axis
	if (collisions.y != -1)
	{

		int sign;
		if (player->velocityY > 0)
			sign = -1;
		else
			sign = 1;
		player->y = collisions.y + sign;
		player->velocityY = -(player->velocityY / 4);
		player->action = isOnFloor;
	}

	// checks for collisions in the y-axis and applies the damage function if the player collided with a damagin object.
	if (collisions.x != -1)
	{
		if (collisions.obj->object == rectangleN)
			damage();
		else
		{
			player->x = xPrev;
		}
	}

	// if player reaches ending then increment the level aswell as reset the position of the player.
	if (collisions.obj->object == endingN)
	{
		levelIndex++;
		player->x = 3;
		player->y = 28;
	}
	// if the player collides with a triangle object, then apply damage.
	if (collisions.obj->object == triangleN)
	{
		damage();
	}

	player->velocityX = 0;
	if (player->y - player->height <= 0)
		player->y = player->height;
}

// function that returns a struct Colliderobject.
struct ColliderObject collision()
{
	int currentObject = player->x / 22;
	int i, cX, cY;

	struct ColliderObject collisionc = {-1, -1, 0};

	for (i = (currentObject)-3; i < (currentObject + 5); i++)
	{
		if (i < 0)
			continue;
		if (i >= currentLevel->numObjects)
			continue;

		cX = collisionX(i * 22, currentLevel->levelObjects[i]->colliders->colliderX);
		cY = collisionY(i * 22, currentLevel->levelObjects[i]->colliders->colliderY);

		if (cX != -1)
		{
			collisionc.x = cX;
			if (currentLevel->levelObjects[i]->object == rectangleN)
				collisionc.obj = currentLevel->levelObjects[i];
		}

		if (cY != -1)
		{
			collisionc.y = cY;
			if (currentLevel->levelObjects[i]->object == endingN || currentLevel->levelObjects[i]->object == triangleN)
				collisionc.obj = currentLevel->levelObjects[i];
		}
	}

	return collisionc;
}

// function that returns
int8_t collisionY(int objectIndex, struct colliderY *collider)
{
	if (!collider)
		return -1;

	if (bounded(collider->y, player->y, player->y - player->height))
	{
		if (checkBounds(player->x, player->x + player->width, collider->leftX + objectIndex, collider->rightX + objectIndex))
			return collider->y;
	}
	return collisionY(objectIndex, collider->next);
}

// A helper function for the collision function.
int bounded(int a, int x1, int x2)
{
	int min, max;

	if (x1 > x2)
	{
		max = x1;
		min = x2;
	}
	else
	{
		min = x1;
		max = x2;
	}

	if ((a == x1) || (a == x2))
		return 1;
	if ((a > min) && (a < max))
		return 1;
	return 0;
}

int8_t collisionX(int objectIndex, struct colliderX *collider)
{
	if (!collider)
		return -1;
	if (bounded(collider->x + objectIndex, player->x, player->x + player->width))
	{
		if (checkBounds(player->y, player->y - player->height, collider->upY, collider->bottomY))
			return collider->x;
	}
	return collisionX(objectIndex, collider->next);
}

// A function that checks the bounds of the screen
int8_t checkBounds(int playerB1, int playerB2, int colliderB2, int colliderB1)
{
	if (bounded(playerB1, colliderB2, colliderB1))
		return 1;
	if (bounded(playerB2, colliderB2, colliderB1))
		return 1;
	if (bounded(colliderB2, playerB1, playerB2) && bounded(colliderB1, playerB1, playerB2))
		return 1;
	return 0;
}

int8_t find_floor(int objectIndex, struct colliderY *collider)
{
	if (!collider)
		return 0;

	int floor = player->y + 1;

	int sign = 1;
	if (player->direction == left)
		sign = -1;
	else if (player->direction == right)
		sign = 1;

	if (floor == collider->y)
	{
		int playerLeft = player->x;
		int playerRight = player->x + sign * player->width;
		int collliderLeft = collider->leftX + objectIndex;
		int colliderRight = collider->rightX + objectIndex;

		if (bounded(playerLeft, collliderLeft, colliderRight))
			return 1;
		if (bounded(playerRight, collliderLeft, colliderRight))
			return 1;
		if (bounded(collliderLeft, playerLeft, playerRight) && bounded(colliderRight, playerLeft, playerRight))
			return 1;
	}

	return find_floor(objectIndex, collider->next);
}

// a booelan function that returns 0 if the object is not on the floor and 1 if the object is on the floor.
int8_t on_floor()
{
	int currentObject = player->x / 22;
	int i;

	for (i = (currentObject - 1); i < (currentObject + 1); i++)
	{
		if (i < 0)
			continue;
		if (i >= currentLevel->numObjects)
			continue;

		if (find_floor(i * 22, currentLevel->levelObjects[i]->colliders->colliderY))
			return 1;
	}
	return 0;
}

// Initiate all the nessecary components of the game such as display, timers and i/o ports
void init_game()
{
	init_io();
	init_timers();
	init_all();
}

void user_isr()
{
	if (IFS(0) & 0x10000)
	{
		IFS(0) &= ~0x10000;
		tick();
	}
}

// function that updates the camera based on the current poistion of the player. This function will later be called in a game loop to be checked constantly
// so that the camera tracks and follows the players movement.
void update_camera()
{
	if (player->x + player->width / 2 < (cameraPosition + moveCameraLeft))
	{
		cameraPosition = player->x + player->width / 2 - moveCameraLeft;
	}
	else if (player->x + player->width / 2 > (cameraPosition + moveCameraRight))
	{
		cameraPosition = player->x + player->width / 2 - moveCameraRight;
	}
	if (cameraPosition < 0)
		cameraPosition = 0;
}

// this function updateds position of the player based on the buttons that was clicked.
void updateAction()
{
	currentLevel = getLevel(levelIndex);

	// checks if game over counter is equal to one then reset all the buffers and return back to the main menu.
	if (game_over == 1)
	{
		reset_buffer(mainBuffer);
		game_over = 0;
		main_menu();
	}

	// button1 will move the player to the left
	if (input & BUTTON1)
	{

		player->direction = left;
		dir = -1;
		player->velocityX--;
	}

	// button2 will move the player to the right
	if (input & BUTTON2)
	{
		player->direction = right;
		dir = 1;
		player->velocityX++;
	}

	if (player->action == isOnFloor)
	{
		// button4 will have the player jump, but only when it is on the floor.
		if (input & BUTTON3)
		{

			player->velocityY -= 5;
			player->gravity.x = 0;
			player->gravity.y = 0;
			player->action = isJumping;
		}
	}
	// button4 will make the user go back to the main menu.
	if (input & BUTTON4)
	{

		main_menu();
		return;
	}
	// apply gravity on the player.
	apply_gravity(player, currentLevel);
	update_player(player, currentLevel);
}

// update the camera and position of the player. Will be used in a game loop.
void update()
{
	updateAction(player, currentLevel);
	update_camera();
}

// inputhandler will make sure the lihgts matches the buttons.
void inputHandler()
{
	input = get_io();
	match_leds(input);
}

// function that renders the player.
void renderPlayer()
{
	int byteIndex, bitIndex;
	int i;
	int xPosition, yPosition;

	for (i = 0; i < player->spriteSize; i++)
	{

		xPosition = player->sprite[i].x + player->x - cameraPosition;
		yPosition = player->sprite[i].y + player->y;
		byteIndex = (yPosition / 8) * 128 + xPosition;
		bitIndex = yPosition % 8;

		if (xPosition > 128 || xPosition < 0 || yPosition > 511 || yPosition < 0)
			continue;
		if (bitIndex > 7 || bitIndex < 0 || byteIndex > 511 || byteIndex < 0)
			continue;

		(mainBuffer[byteIndex] |= ((uint8_t)1 << bitIndex));
	}
}

// function that renders the obejcts.
void renderObject(struct Object *object, int offset)
{
	int16_t xPosition;
	int8_t yPosition;
	int byteIndex, bitIndex;
	int i;

	for (i = 0; i < object->spriteSize; i++)
	{
		xPosition = object->sprite[i].x + offset - cameraPosition;
		yPosition = object->sprite[i].y;
		byteIndex = (yPosition / 8) * 128 + xPosition;
		bitIndex = yPosition % 8;
		if (xPosition > 127 || xPosition < 0 || yPosition > 511 || yPosition < 0)
			continue;
		if (bitIndex > 7 || bitIndex < 0 || byteIndex > 511 || byteIndex < 0)
			continue;

		(mainBuffer[byteIndex] |= ((uint8_t)1 << bitIndex));
	}
}

// renders the lives left of the player and the highscore.
void renderInfo(struct Object *object)
{
	int16_t xPosition;
	int8_t yPosition;
	int byteIndex, bitIndex;
	int i;
	if (object->object == heartsN)
		i = 30 - health * 10;
	else
		i = 0;
	for (i; i < object->spriteSize; i++)
	{
		xPosition = object->sprite[i].x;
		yPosition = object->sprite[i].y;
		byteIndex = (yPosition / 8) * 128 + xPosition;
		bitIndex = yPosition % 8;
		if (xPosition > 127 || xPosition < 0 || yPosition > 511 || yPosition < 0)
			continue;
		if (bitIndex > 7 || bitIndex < 0 || byteIndex > 511 || byteIndex < 0)
			continue;

		(mainBuffer[byteIndex] |= ((uint8_t)1 << bitIndex));
	}
}

// fucntion that renders the score and the level that was reached. Will be used later to display the score and level on the highscore menu.
void render_score_level(int score, int level, int line, int col)
{
	level = level + 1;
	int digits[4];
	digits[4] = 0;
	int i;

	int numDigits = 0;
	while (score > 0)
	{
		int digit = score % 10;
		digits[numDigits] = digit;
		numDigits++;
		score /= 10;
	}

	int row = line * 128;

	for (i = 0; i < 3; i++)
		mainBuffer[row + i + col] |= numberBytes[digits[4] * 3 + i];

	for (i = 0; i < 3; i++)
		mainBuffer[row + i + 4 + col] |= numberBytes[digits[3] * 3 + i];

	for (i = 0; i < 3; i++)
		mainBuffer[row + i + 8 + col] |= numberBytes[(digits[2]) * 3 + i];

	for (i = 0; i < 3; i++)
		mainBuffer[row + i + 12 + col] |= numberBytes[(digits[1]) * 3 + i];

	for (i = 0; i < 3; i++)
		mainBuffer[row + i + 16 + col] |= numberBytes[(digits[0]) * 3 + i];
}

// renders the level. This will use the renderobject() funciton to render all the objects that belongs to each and every level.
void renderlevel(struct level *level, struct Player *player, uint8_t *mainBuffer)
{
	int i;
	int currentObject = (int)player->x / 22;
	for (i = currentObject - 30; i < currentObject + 30; i++)
	{
		if (i < 0 || i > currentLevel->numObjects)
			continue;
		int offset = i * 22;
		if (offset > cameraPosition + 127 || offset + 21 < cameraPosition)
		{
			continue;
		}
		renderObject(currentLevel->levelObjects[i], offset);
	}
}

// function that implements the damage. If the player hit an damaging object will the lives counter decrement and score will be reset. If the player
// takes damamge 3 times, meaning loosing all lives will the game terminate and the user will be sent back to the main menu.
void damage()
{
	if (chancesCounter != 2)
	{
		chancesCounter++;

		health--;
		player->x -= 12;
		player->y = 10;
		if (highScore > 0)
			highScore -= 1000;
	}
	// if there are no more lives left and the game is over.
	else
	{
		chancesCounter = 0;
		game_over = 1;
		gameOver = getGameOver();
		reset_buffer(mainBuffer);
		renderInfo(gameOver);
		update_display(mainBuffer);
		delay_milliseconds(5000);
		save_high_score(highScore);
		player->x = 3;
		player->y = 28;
		levelIndex = 0;
		currentLevel = getLevel(levelIndex);
		highScore = 10000;
		health = 0;
	}
}

// function that triggers all the rendering functions. will be used in the game loop.
void render_game()
{
	reset_buffer(mainBuffer);
	render_score_level(highScore, levelIndex, 0, 0);
	renderInfo(&healthObject);
	renderlevel(currentLevel, player, mainBuffer);
	renderPlayer(player, mainBuffer);
	update_display(mainBuffer);
}

// game loop is where the game happens
void game_loop()
{
	player = getPlayer();
	currentLevel = getLevel(levelIndex);
	while (1)
	{

		inputHandler();
		update(player, currentLevel);
		delay_milliseconds(16);
		render_game();
	}
}

void reset_text_buffer()
{
	int j = 0;
	for (j = 0; j < 48; j++)
	{
		textbuffer[j] = 0x00;
	}
}

void save_high_score(int score)
{

	int letter = 65;
	int i = 0;
	int j = 0;
	char *scoreasc = itoaconv(score);
	int t = 1;

	while (t == 1)
	{
		input = 0;
		inputHandler();
		textbuffer[i] = letter;
		if (i > 4)
		{
			for (j = 0; j < 5; j++)
			{
				highScoreList[highScoreIndex][j] = textbuffer[j];
			}

			for (j = 0; j < 5; j++)
			{
				highScoreList[highScoreIndex][j + 5] = 0x00;
			}

			for (j = 0; j < 5; j++)
			{
				highScoreList[highScoreIndex][10 + j] = scoreasc[j];
			}

			highScoreIndex++;
			reset_text_buffer();

			reset_buffer(mainBuffer);

			t = 0;
		}

		if (letter < 90)
		{

			if (input & BUTTON1)
			{

				textbuffer[i] = letter;
				letter++;
				delay_milliseconds(300);
			}

			if (input & BUTTON2)
			{
				letter--;
				textbuffer[i] = letter;
				delay_milliseconds(300);
			}

			if (input & BUTTON3)
			{
				delay_milliseconds(200);
				textbuffer[i] = letter;
				letter = 65;
				i++;
			}
			reset_buffer(mainBuffer);
			delay_milliseconds(50);
			display_text("Enter you name:", mainBuffer, 1, 0);
			display_text(textbuffer, mainBuffer, 1, 2 * 128 + 30);
			update_display(mainBuffer);
		}

		else
		{
			letter = 65;
		}
	}
}

// a function to list the score in the score menu that can be choosen to enter from the main menu.
void scoreMenu()
{
	int i;
	reset_buffer(mainBuffer);
	for (i = 0; i < 4; i++)
	{
		display_text(highScoreList[i], mainBuffer, 1, i * 128 + 3);
	}
	update_display(mainBuffer);

	delay_milliseconds(10000);
	main_menu();
}

// a function for the main menu. This screen will redirect the user to either the game or the highscore menu.
void main_menu()
{

	player->x = 3;
	player->y = 28;
	levelIndex = 0;
	currentLevel = getLevel(levelIndex);
	reset_buffer(mainBuffer);
	menu1 = getMenu();
	renderInfo(menu1);
	update_display(mainBuffer);
	int selected = 0;
	game_over = 0;
	struct Object *curseurRi = getCurseur(1);
	struct Object *curseurLe = getCurseur(2);

	input = 0;

	while (1)
	{
		renderInfo(menu1);
		inputHandler();

		if (input & BUTTON1)
		{
			reset_buffer(mainBuffer);
			renderInfo(menu1);
			renderInfo(curseurLe);
			selected = 1;
		}

		if (input & BUTTON2)
		{
			selected = 2;
			reset_buffer(mainBuffer);
			renderInfo(menu1);
			renderInfo(curseurRi);
		}

		if (input & BUTTON3)
		{
			switch (selected)
			{
			case 2:
				scoreMenu();

			case 1:
				game_loop();
			}
		}

		update_display(mainBuffer);
	}
}

int main()
{
	init_game();
	main_menu();

	return 0;
}
