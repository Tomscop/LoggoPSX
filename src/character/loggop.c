/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "loggop.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Loggo Playable character structure
enum
{
	LoggoP_ArcMain_Loggo0,
	LoggoP_ArcMain_Loggo1,
	LoggoP_ArcMain_Loggo2,
	
	LoggoP_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[LoggoP_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_LoggoP;

//Loggo Playable character definitions
static const CharFrame char_loggop_frame[] = {
	{LoggoP_ArcMain_Loggo0, {  0,  0, 86,105}, {  0,  0}}, //0 idle 1
	{LoggoP_ArcMain_Loggo0, { 87,  0, 84,108}, { -1,  3}}, //1 idle 2
	{LoggoP_ArcMain_Loggo0, {172,  0, 83,108}, { -1,  4}}, //2 idle 3
	{LoggoP_ArcMain_Loggo0, {  0,109, 82,107}, { -1,  4}}, //3 idle 4
	{LoggoP_ArcMain_Loggo0, { 83,109, 81,107}, { -1,  4}}, //4 idle 5
	
	{LoggoP_ArcMain_Loggo0, {165,109, 83,101}, {  2,  3}}, //5 left 1
	{LoggoP_ArcMain_Loggo1, {  0,  0, 82,100}, {  0,  3}}, //6 left 2
	
	{LoggoP_ArcMain_Loggo1, { 83,  0,105, 86}, { 10,-10}}, //7 down 1
	{LoggoP_ArcMain_Loggo1, {  0,101,105, 87}, {  9, -9}}, //8 down 2
	
	{LoggoP_ArcMain_Loggo1, {106,101, 90,101}, { -1,  9}}, //9 up 1
	{LoggoP_ArcMain_Loggo2, {  0,  0, 92, 99}, {  0,  7}}, //10 up 2
	
	{LoggoP_ArcMain_Loggo2, { 93,  0, 80,101}, { -3,  3}}, //11 right 1
	{LoggoP_ArcMain_Loggo2, {174,  0, 77,101}, { -2,  4}}, //12 right 2
};

static const Animation char_loggop_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5, 6, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7, 8, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 11, 12, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Loggo Playable character functions
void Char_LoggoP_SetFrame(void *user, u8 frame)
{
	Char_LoggoP *this = (Char_LoggoP*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_loggop_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_LoggoP_Tick(Character *character)
{
	Char_LoggoP *this = (Char_LoggoP*)character;
	
	//Camera stuff
	if (stage.stage_id == StageId_1_1)
	{
		if (stage.song_step == 192)
		{
			this->character.focus_x = FIXED_DEC(94,1);
			this->character.focus_y = FIXED_DEC(-1,1);	
			this->character.focus_zoom = FIXED_DEC(399,512);
		}
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_LoggoP_SetFrame);
	Character_Draw(character, &this->tex, &char_loggop_frame[this->frame]);
}

void Char_LoggoP_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_LoggoP_Free(Character *character)
{
	Char_LoggoP *this = (Char_LoggoP*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_LoggoP_New(fixed_t x, fixed_t y)
{
	//Allocate loggop object
	Char_LoggoP *this = Mem_Alloc(sizeof(Char_LoggoP));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_LoggoP_New] Failed to allocate loggop object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_LoggoP_Tick;
	this->character.set_anim = Char_LoggoP_SetAnim;
	this->character.free = Char_LoggoP_Free;
	
	Animatable_Init(&this->character.animatable, char_loggop_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	this->character.idle2 = 0;
	
	this->character.health_i = 3;

	//health bar color
	this->character.health_bar = 0xFF289056;
	
	this->character.focus_x = FIXED_DEC(-1,1);
	this->character.focus_y = FIXED_DEC(15,1);
	this->character.focus_zoom = FIXED_DEC(357,512);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\LOGGOP.ARC;1");
	
	const char **pathp = (const char *[]){
		"loggo0.tim", //LoggoP_ArcMain_Loggo0
		"loggo1.tim", //LoggoP_ArcMain_Loggo1
		"loggo2.tim",  //LoggoP_ArcMain_Loggo2
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
