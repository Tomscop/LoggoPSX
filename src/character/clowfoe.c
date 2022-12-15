/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "clowfoe.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Clowfoe character structure
enum
{
	Clowfoe_ArcMain_Clow0,
	Clowfoe_ArcMain_Clow1,
	Clowfoe_ArcMain_Clow2,
	Clowfoe_ArcMain_Clow3,
	
	Clowfoe_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Clowfoe_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Clowfoe;

//Clowfoe character definitions
static const CharFrame char_clowfoe_frame[] = {
	{Clowfoe_ArcMain_Clow3, {176, 83, 58, 69}, {  1,-20}}, //0 idle
	
	{Clowfoe_ArcMain_Clow0, {  0,  0,114, 89}, {  0,  0}}, //1 bark 1
	{Clowfoe_ArcMain_Clow0, {115,  0,115, 92}, {  0,  3}}, //2 bark 2
	{Clowfoe_ArcMain_Clow0, {  0, 93,115, 92}, {  0,  3}}, //3 bark 3
	{Clowfoe_ArcMain_Clow0, {116, 93,115, 93}, {  0,  4}}, //4 bark 4
	{Clowfoe_ArcMain_Clow1, {  0,  0,116, 93}, {  1,  4}}, //5 bark 5
	{Clowfoe_ArcMain_Clow1, {117,  0,116, 94}, {  1,  5}}, //6 bark 6
	{Clowfoe_ArcMain_Clow1, {  0, 95,116, 94}, {  1,  5}}, //7 bark 7
	{Clowfoe_ArcMain_Clow1, {117, 95,116, 93}, {  1,  4}}, //8 bark 8
	{Clowfoe_ArcMain_Clow2, {  0,  0,116, 93}, {  1,  4}}, //9 bark 9
	{Clowfoe_ArcMain_Clow2, {117,  0,116, 91}, {  1,  2}}, //10 bark 10
	{Clowfoe_ArcMain_Clow2, {  0, 94,116, 89}, {  1,  0}}, //11 bark 11
	{Clowfoe_ArcMain_Clow2, {117, 94,116, 85}, {  1, -4}}, //12 bark 12
	{Clowfoe_ArcMain_Clow3, {  0,  0,116, 82}, {  1, -7}}, //13 bark 13
	{Clowfoe_ArcMain_Clow3, {117,  0,116, 77}, {  1,-12}}, //14 bark 14
	{Clowfoe_ArcMain_Clow3, {  0, 83,116, 72}, {  1,-17}}, //15 bark 15
	{Clowfoe_ArcMain_Clow3, {117, 83, 58, 69}, {  1,-20}}, //16 bark 16
};

static const Animation char_clowfoe_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, ASCR_BACK, 1}}, //CharAnim_Idle
	{1, (const u8[]){ 1, 2, 3, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{1, (const u8[]){ 1, 2, 3, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{1, (const u8[]){ 1, 2, 3, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{1, (const u8[]){ 1, 2, 3, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, ASCR_CHGANI, CharAnim_Idle}},    //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Clowfoe character functions
void Char_Clowfoe_SetFrame(void *user, u8 frame)
{
	Char_Clowfoe *this = (Char_Clowfoe*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_clowfoe_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Clowfoe_Tick(Character *character)
{
	Char_Clowfoe *this = (Char_Clowfoe*)character;
	
	//Camera stuff
	if (stage.stage_id == StageId_1_4)
	{
		if (stage.song_step == 192)
		{
			this->character.focus_x = FIXED_DEC(93,1);
			this->character.focus_y = FIXED_DEC(1,1);
			this->character.focus_zoom = FIXED_DEC(357,512);
		}
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
	{
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
			character->animatable.anim != CharAnim_LeftAlt &&
			character->animatable.anim != PlayerAnim_LeftMiss &&
			character->animatable.anim != CharAnim_Down &&
			character->animatable.anim != CharAnim_DownAlt &&
			character->animatable.anim != PlayerAnim_DownMiss &&
			character->animatable.anim != CharAnim_Up &&
			character->animatable.anim != CharAnim_UpAlt &&
			character->animatable.anim != PlayerAnim_UpMiss &&
			character->animatable.anim != CharAnim_Right &&
			character->animatable.anim != CharAnim_RightAlt &&
			character->animatable.anim != PlayerAnim_RightMiss))
				Character_PerformIdle(character);
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Clowfoe_SetFrame);
	Character_Draw(character, &this->tex, &char_clowfoe_frame[this->frame]);
}

void Char_Clowfoe_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Clowfoe_Free(Character *character)
{
	Char_Clowfoe *this = (Char_Clowfoe*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Clowfoe_New(fixed_t x, fixed_t y)
{
	//Allocate clowfoe object
	Char_Clowfoe *this = Mem_Alloc(sizeof(Char_Clowfoe));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Clowfoe_New] Failed to allocate clowfoe object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Clowfoe_Tick;
	this->character.set_anim = Char_Clowfoe_SetAnim;
	this->character.free = Char_Clowfoe_Free;
	
	Animatable_Init(&this->character.animatable, char_clowfoe_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	this->character.idle2 = 0;
	
	this->character.health_i = 3;

	//health bar color
	this->character.health_bar = 0xFF289056;
	
	this->character.focus_x = FIXED_DEC(93,1);
	this->character.focus_y = FIXED_DEC(1,1);
	this->character.focus_zoom = FIXED_DEC(357,512);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CLOWFOE.ARC;1");
	
	const char **pathp = (const char *[]){
		"clow0.tim", //Clowfoe_ArcMain_Clow0
		"clow1.tim", //Clowfoe_ArcMain_Clow1
		"clow2.tim", //Clowfoe_ArcMain_Clow2
		"clow3.tim", //Clowfoe_ArcMain_Clow3
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
