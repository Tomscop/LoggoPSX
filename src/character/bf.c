/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

//Boyfriend player types
enum
{
	BF_ArcMain_Idle0,
	BF_ArcMain_Idle1,
	BF_ArcMain_Idle2,
	BF_ArcMain_Idle3,
	BF_ArcMain_Left0,
	BF_ArcMain_Left1,
	BF_ArcMain_Down,
	BF_ArcMain_Up,
	BF_ArcMain_Right0,
	BF_ArcMain_Right1,
	BF_ArcMain_LeftM0,
	BF_ArcMain_LeftM1,
	BF_ArcMain_DownM,
	BF_ArcMain_UpM,
	BF_ArcMain_RightM0,
	BF_ArcMain_RightM1,
	
	BF_ArcMain_Max,
};

#define BF_Arc_Max BF_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BF_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
} Char_BF;

//Boyfriend player definitions
static const CharFrame char_bf_frame[] = {
	{BF_ArcMain_Idle0, {  0,  0,133,132}, {  0,  0}}, //0 idle 1
	{BF_ArcMain_Idle1, {  0,  0,132,133}, {  0,  1}}, //1 idle 2
	{BF_ArcMain_Idle2, {  0,  0,132,137}, {  1,  5}}, //2 idle 3
	{BF_ArcMain_Idle3, {  0,  0,130,139}, {  1,  7}}, //3 idle 4
	{BF_ArcMain_Idle3, {  0,  0,130,139}, {  1,  7}}, //4 idle 5
	
	{BF_ArcMain_Left0, {  0,  0,136,139}, {  8,  7}}, //5 left 1
	{BF_ArcMain_Left1, {  0,  0,134,139}, {  6,  7}}, //6 left 2
	
	{BF_ArcMain_Down, {  0,  0,144,125}, {  2, -7}}, //7 down 1
	{BF_ArcMain_Down, {  0,126,142,128}, {  4, -4}}, //8 down 2
	
	{BF_ArcMain_Up, {  0,  0,118,153}, { -7, 20}}, //9 up 1
	{BF_ArcMain_Up, {119,  0,123,150}, { -4, 17}}, //10 up 2
	
	{BF_ArcMain_Right0, {  0,  0,132,137}, { -4,  4}}, //11 right 1
	{BF_ArcMain_Right1, {  0,  0,129,137}, { -3,  4}}, //12 right 2
	
	{BF_ArcMain_LeftM0, {  0,  0,136,139}, {  8,  7}}, //13 left miss 1
	{BF_ArcMain_LeftM1, {  0,  0,134,139}, {  6,  7}}, //14 left miss 2
	
	{BF_ArcMain_DownM, {  0,  0,144,125}, {  2, -7}}, //15 down miss 1
	{BF_ArcMain_DownM, {  0,126,142,128}, {  4, -4}}, //16 down miss 2
	
	{BF_ArcMain_UpM, {  0,  0,118,153}, { -7, 20}}, //17 up miss 1
	{BF_ArcMain_UpM, {119,  0,123,150}, { -4, 17}}, //18 up miss 2
	
	{BF_ArcMain_RightM0, {  0,  0,132,137}, { -4,  4}}, //19 right miss 1
	{BF_ArcMain_RightM1, {  0,  0,129,137}, { -3,  4}}, //20 right miss 2
};

static const Animation char_bf_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5, 6, 6, 6, 6, 6, 6, ASCR_BACK, 1}},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 7, 8, 8, 8, 8, 8, 8, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 9, 10, 10, 10, 10, 10, 10, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){ 11, 12, 12, 12, 12, 12, 12, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{2, (const u8[]){ 13, 14, 14, 14, 14, 14, 14, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{2, (const u8[]){ 15, 16, 16, 16, 16, 16, 16, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{2, (const u8[]){ 17, 18, 18, 18, 18, 18, 18, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{2, (const u8[]){ 19, 20, 20, 20, 20, 20, 20, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
};

//Boyfriend player functions
void Char_BF_SetFrame(void *user, u8 frame)
{
	Char_BF *this = (Char_BF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BF_Tick(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Camera stuff
	if (stage.stage_id == StageId_1_1)
	{
		if (stage.song_step == 196)
		{
			this->character.focus_x = FIXED_DEC(-5,1);
			this->character.focus_y = FIXED_DEC(13,1);	
			this->character.focus_zoom = FIXED_DEC(399,512);
		}
	}
	if (stage.stage_id == StageId_1_2)
	{
		if (stage.song_step == 100)
		{
			this->character.focus_x = FIXED_DEC(-5,1);
			this->character.focus_y = FIXED_DEC(13,1);	
			this->character.focus_zoom = FIXED_DEC(399,512);
		}
	}
	if (stage.stage_id == StageId_1_3)
	{
		if (stage.song_step == 70)
		{
			this->character.focus_x = FIXED_DEC(-5,1);
			this->character.focus_y = FIXED_DEC(13,1);	
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
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BF_SetFrame);
	Character_Draw(character, &this->tex, &char_bf_frame[this->frame]);
}

void Char_BF_SetAnim(Character *character, u8 anim)
{
	Char_BF *this = (Char_BF*)character;
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BF_Free(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BF_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BF *this = Mem_Alloc(sizeof(Char_BF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BF_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BF_Tick;
	this->character.set_anim = Char_BF_SetAnim;
	this->character.free = Char_BF_Free;
	
	Animatable_Init(&this->character.animatable, char_bf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	this->character.idle2 = 0;
	
	this->character.health_i = 0;

	//health bar color
	this->character.health_bar = 0xFF31B0D1;
	
	this->character.focus_x = FIXED_DEC(11,1);
	this->character.focus_y = FIXED_DEC(15,1);
	this->character.focus_zoom = FIXED_DEC(357,512);
	
	this->character.size = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BF.ARC;1");
		
	const char **pathp = (const char *[]){
		"idle0.tim",   //BF_ArcMain_Idle0
		"idle1.tim",   //BF_ArcMain_Idle1
		"idle2.tim",   //BF_ArcMain_Idle2
		"idle3.tim",   //BF_ArcMain_Idle3
		"left0.tim",   //BF_ArcMain_Left0
		"left1.tim",   //BF_ArcMain_Left1
		"down.tim",   //BF_ArcMain_Down
		"up.tim",   //BF_ArcMain_Up
		"right0.tim",   //BF_ArcMain_Right0
		"right1.tim",   //BF_ArcMain_Right1
		"leftm0.tim",   //BF_ArcMain_LeftM0
		"leftm1.tim",   //BF_ArcMain_LeftM1
		"downm.tim",   //BF_ArcMain_DownM
		"upm.tim",   //BF_ArcMain_UpM
		"rightm0.tim",   //BF_ArcMain_RightM0
		"rightm1.tim",   //BF_ArcMain_RightM1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
