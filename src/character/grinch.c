/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "grinch.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Grinch character structure
enum
{
	Grinch_ArcMain_Idle0,
	Grinch_ArcMain_Idle1,
	Grinch_ArcMain_Idle2,
	Grinch_ArcMain_Left,
	Grinch_ArcMain_Down,
	Grinch_ArcMain_Up,
	Grinch_ArcMain_Right,
	
	Grinch_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Grinch_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Grinch;

//Grinch character definitions
static const CharFrame char_grinch_frame[] = {
	{Grinch_ArcMain_Idle0, {  0,  0,119,185}, {  0,  0}}, //0 idle 1
	{Grinch_ArcMain_Idle0, {120,  0,120,186}, {  0,  1}}, //1 idle 2
	{Grinch_ArcMain_Idle1, {  0,  0,120,188}, {  0,  3}}, //2 idle 3
	{Grinch_ArcMain_Idle1, {121,  0,120,189}, {  0,  4}}, //3 idle 4
	{Grinch_ArcMain_Idle2, {  0,  0,120,189}, {  0,  4}}, //4 idle 5
	
	{Grinch_ArcMain_Left, {  0,  0,112,187}, {  0,  2}}, //5 left 1
	{Grinch_ArcMain_Left, {113,  0,112,188}, {  0,  3}}, //6 left 2
	
	{Grinch_ArcMain_Down, {  0,  0,124,180}, {  0, -5}}, //7 down 1
	{Grinch_ArcMain_Down, {125,  0,124,181}, {  0, -4}}, //8 down 2
	
	{Grinch_ArcMain_Up, {  0,  0,115,191}, {  0,  5}}, //9 up 1
	{Grinch_ArcMain_Up, {116,  0,115,190}, {  0,  4}}, //10 up 2
	
	{Grinch_ArcMain_Right, {  0,  0,125,187}, {  0,  2}}, //11 right 1
	{Grinch_ArcMain_Right, {126,  0,124,187}, {  0,  2}}, //12 right 2
};

static const Animation char_grinch_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{1, (const u8[]){ 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{1, (const u8[]){ 7, 7, 8, 8, 8, 8, 8, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{1, (const u8[]){ 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{1, (const u8[]){ 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Grinch character functions
void Char_Grinch_SetFrame(void *user, u8 frame)
{
	Char_Grinch *this = (Char_Grinch*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_grinch_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Grinch_Tick(Character *character)
{
	Char_Grinch *this = (Char_Grinch*)character;
	
	//Camera stuff
	if (stage.stage_id == StageId_1_4)
	{
		if (stage.song_step == 0)
		{
			this->character.focus_x = FIXED_DEC(107,1);
			this->character.focus_y = FIXED_DEC(66,1);	
			this->character.focus_zoom = FIXED_DEC(530,512);
		}
		if ((stage.song_step == 88) || (stage.song_step == 192) || (stage.song_step == 381) || (stage.song_step == 532))
		{
			this->character.focus_x = FIXED_DEC(107,1);
			this->character.focus_y = FIXED_DEC(66,1);	
			this->character.focus_zoom = FIXED_DEC(259,256);
		}
		if ((stage.song_step == 31) || (stage.song_step == 176) || (stage.song_step == 350) || (stage.song_step == 500))
		{
			this->character.focus_x = FIXED_DEC(112,1);
			this->character.focus_y = FIXED_DEC(100,1);	
			this->character.focus_zoom = FIXED_DEC(710,512);
		}
	}
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Grinch_SetFrame);
	Character_Draw(character, &this->tex, &char_grinch_frame[this->frame]);
}

void Char_Grinch_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Grinch_Free(Character *character)
{
	Char_Grinch *this = (Char_Grinch*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Grinch_New(fixed_t x, fixed_t y)
{
	//Allocate grinch object
	Char_Grinch *this = Mem_Alloc(sizeof(Char_Grinch));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Grinch_New] Failed to allocate grinch object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Grinch_Tick;
	this->character.set_anim = Char_Grinch_SetAnim;
	this->character.free = Char_Grinch_Free;
	
	Animatable_Init(&this->character.animatable, char_grinch_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	this->character.idle2 = 0;
	
	this->character.health_i = 3;

	//health bar color
	this->character.health_bar = 0xFF289056;
	
	this->character.focus_x = FIXED_DEC(104,1);
	this->character.focus_y = FIXED_DEC(65,1);
	this->character.focus_zoom = FIXED_DEC(487,512);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GRINCH.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Grinch_ArcMain_Idle0
		"idle1.tim", //Grinch_ArcMain_Idle1
		"idle2.tim", //Grinch_ArcMain_Idle2
		"left.tim",  //Grinch_ArcMain_Left
		"down.tim",  //Grinch_ArcMain_Down
		"up.tim",    //Grinch_ArcMain_Up
		"right.tim", //Grinch_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
