/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "spooker.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Spooker character structure
enum
{
	Spooker_ArcMain_Idle0,
	Spooker_ArcMain_Idle1,
    Spooker_ArcMain_Idle2,
    Spooker_ArcMain_Idle3,
    Spooker_ArcMain_Idle4,
	Spooker_ArcMain_Left0,
    Spooker_ArcMain_Left1,
	Spooker_ArcMain_Down,
	Spooker_ArcMain_Up0,
	Spooker_ArcMain_Up1,
	Spooker_ArcMain_Right0,
    Spooker_ArcMain_Right1,
	
	Spooker_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Spooker_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Spooker;

//Spooker character definitions
static const CharFrame char_spooker_frame[] = {
	{Spooker_ArcMain_Idle0, {  0,  0,183,157}, {  0,  0}}, //0 idle 1
	{Spooker_ArcMain_Idle1, {  0,  0,178,163}, { -2,  6}}, //1 idle 2
	{Spooker_ArcMain_Idle2, {  0,  0,182,167}, {  0, 10}}, //2 idle 3
	{Spooker_ArcMain_Idle3, {  0,  0,176,167}, { -3, 10}}, //3 idle 4
    {Spooker_ArcMain_Idle4, {  0,  0,178,167}, { -2, 10}}, //4 idle 5
	
	{Spooker_ArcMain_Left0, {  0,  0,213,154}, { 34, -3}}, //5 left 1
	{Spooker_ArcMain_Left1, {  0,  0,203,155}, { 23, -2}}, //6 left 2
	
	{Spooker_ArcMain_Down, {  0,  0,187,111}, {  0,-44}}, //7 down 1
	{Spooker_ArcMain_Down, {  0,112,185,119}, {  0,-36}}, //8 down 2
	
	{Spooker_ArcMain_Up0, {  0,  0,181,186}, { -5, 30}}, //9 up 1
	{Spooker_ArcMain_Up1, {  0,  0,181,181}, { -3, 25}}, //10 up 2
	
	{Spooker_ArcMain_Right0, {  0,  0,201,162}, { -3,  7}}, //11 right 1
	{Spooker_ArcMain_Right1, {  0,  0,201,162}, { -2,  7}}, //12 right 2
};

static const Animation char_spooker_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5,  6, 6, 6, 6, 6, 6, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, 8, 8, 8, 8, 8, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 9,  10, 10, 10, 10, 10, 10, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 11, 12, 12, 12, 12, 12, 12, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Spooker character functions
void Char_Spooker_SetFrame(void *user, u8 frame)
{
	Char_Spooker *this = (Char_Spooker*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_spooker_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Spooker_Tick(Character *character)
{
	Char_Spooker *this = (Char_Spooker*)character;
	
	//Camera stuff
	if (stage.stage_id == StageId_1_3)
	{
		if (stage.song_step == 64)
		{
			this->character.focus_x = FIXED_DEC(119,1);
			this->character.focus_y = FIXED_DEC(47,1);	
			this->character.focus_zoom = FIXED_DEC(399,512);
		}
	}
	
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (stage.stage_id != StageId_1_3)
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
				character->animatable.anim != PlayerAnim_RightMiss) &&
				(stage.song_step & 0x7) == 0)
				character->set_anim(character, CharAnim_Idle);
		}
		else
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
				character->animatable.anim != PlayerAnim_RightMiss) &&
				(stage.song_step & 0xF) == 0)
				character->set_anim(character, CharAnim_Idle);
		}
			
		if (character->idle2 == 1)
		{
			if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 3)
			character->set_anim(character, CharAnim_LeftAlt);
		}
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Spooker_SetFrame);
	Character_Draw(character, &this->tex, &char_spooker_frame[this->frame]);
}

void Char_Spooker_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Spooker_Free(Character *character)
{
	Char_Spooker *this = (Char_Spooker*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Spooker_New(fixed_t x, fixed_t y)
{
	//Allocate spooker object
	Char_Spooker *this = Mem_Alloc(sizeof(Char_Spooker));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Spooker_New] Failed to allocate spooker object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Spooker_Tick;
	this->character.set_anim = Char_Spooker_SetAnim;
	this->character.free = Char_Spooker_Free;
	
	Animatable_Init(&this->character.animatable, char_spooker_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	this->character.idle2 = 0;
	
	this->character.health_i = 2;

	//health bar color
	this->character.health_bar = 0xFF289056;
	
	this->character.focus_x = FIXED_DEC(118,1);
	this->character.focus_y = FIXED_DEC(49,1);
	this->character.focus_zoom = FIXED_DEC(357,512);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SPOOKER.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Spooker_ArcMain_Idle0
		"idle1.tim", //Spooker_ArcMain_Idle1
        "idle2.tim", //Spooker_ArcMain_Idle2
        "idle3.tim", //Spooker_ArcMain_Idle3
        "idle4.tim", //Spooker_ArcMain_Idle4
		"left0.tim",  //Spooker_ArcMain_Left0
        "left1.tim",  //Spooker_ArcMain_Left1
		"down.tim",  //Spooker_ArcMain_Down
		"up0.tim",    //Spooker_ArcMain_Up0
		"up1.tim",    //Spooker_ArcMain_Up1
		"right0.tim", //Spooker_ArcMain_Right0
        "right1.tim", //Spooker_ArcMain_Right1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
