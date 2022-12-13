/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "loggo.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Loggo character structure
enum
{
	Loggo_ArcMain_Idle0,
	Loggo_ArcMain_Idle1,
	Loggo_ArcMain_Left,
	Loggo_ArcMain_Down,
	Loggo_ArcMain_Up0,
	Loggo_ArcMain_Up1,
	Loggo_ArcMain_Right,
	
	Loggo_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Loggo_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Loggo;

//Loggo character definitions
static const CharFrame char_loggo_frame[] = {
	{Loggo_ArcMain_Idle0, {  0,  0,169,112}, {  0,  0}}, //0 idle 1
	{Loggo_ArcMain_Idle0, {  0,113,164,115}, { -2,  3}}, //1 idle 2
	{Loggo_ArcMain_Idle1, {  0,  0,156,119}, { -3,  7}}, //2 idle 3
	{Loggo_ArcMain_Idle1, {  0,120,154,119}, { -3,  7}}, //3 idle 4
	
	{Loggo_ArcMain_Left, {  0,  0,157,119}, { 12,  7}}, //4 left 1
	{Loggo_ArcMain_Left, {  0,120,156,119}, {  8,  7}}, //5 left 2
	
	{Loggo_ArcMain_Down, {  0,  0,158,106}, { -6, -7}}, //6 down 1
	{Loggo_ArcMain_Down, {  0,107,156,109}, { -8, -4}}, //7 down 2
	
	{Loggo_ArcMain_Up0, {  0,  0,152,134}, { -9, 22}}, //8 up 1
	{Loggo_ArcMain_Up1, {  0,  0,154,130}, { -8, 18}}, //9 up 2
	
	{Loggo_ArcMain_Right, {  0,  0,143,119}, {-20,  7}}, //10 right 1
	{Loggo_ArcMain_Right, {  0,120,148,119}, {-17,  7}}, //11 right 2
};

static const Animation char_loggo_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, 5, 5, 5, 5, 5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, 7, 7, 7, 7, 7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, 9, 9, 9, 9, 9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, 11, 11, 11, 11, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Loggo character functions
void Char_Loggo_SetFrame(void *user, u8 frame)
{
	Char_Loggo *this = (Char_Loggo*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_loggo_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Loggo_Tick(Character *character)
{
	Char_Loggo *this = (Char_Loggo*)character;
	
	//Camera stuff
	if (stage.stage_id == StageId_1_1)
	{
		if (stage.song_step == 192)
		{
			this->character.focus_x = FIXED_DEC(65,1);
			this->character.focus_y = FIXED_DEC(-1,1);	
			this->character.focus_zoom = FIXED_DEC(387,512);
		}
	}
	if (stage.stage_id == StageId_1_2)
	{
		if (stage.song_step == 96)
		{
			this->character.focus_x = FIXED_DEC(65,1);
			this->character.focus_y = FIXED_DEC(-1,1);	
			this->character.focus_zoom = FIXED_DEC(387,512);
		}
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Loggo_SetFrame);
	Character_Draw(character, &this->tex, &char_loggo_frame[this->frame]);
}

void Char_Loggo_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Loggo_Free(Character *character)
{
	Char_Loggo *this = (Char_Loggo*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Loggo_New(fixed_t x, fixed_t y)
{
	//Allocate loggo object
	Char_Loggo *this = Mem_Alloc(sizeof(Char_Loggo));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Loggo_New] Failed to allocate loggo object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Loggo_Tick;
	this->character.set_anim = Char_Loggo_SetAnim;
	this->character.free = Char_Loggo_Free;
	
	Animatable_Init(&this->character.animatable, char_loggo_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	this->character.idle2 = 0;
	
	this->character.health_i = 1;

	//health bar color
	this->character.health_bar = 0xFF289056;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(1,1);
	this->character.focus_zoom = FIXED_DEC(357,512);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\LOGGO.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Loggo_ArcMain_Idle0
		"idle1.tim", //Loggo_ArcMain_Idle1
		"left.tim",  //Loggo_ArcMain_Left
		"down.tim",  //Loggo_ArcMain_Down
		"up0.tim",    //Loggo_ArcMain_Up0
		"up1.tim",    //Loggo_ArcMain_Up1
		"right.tim", //Loggo_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
