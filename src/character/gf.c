/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//GF character structure
enum
{
	GF_ArcMain_Loggo,
	GF_ArcMain_GF1,
	GF_ArcMain_GF2,
	
	GF_ArcScene_0, //tut0
	GF_ArcScene_1, //tut1
	
	GF_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_scene;
	IO_Data arc_ptr[GF_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
} Char_GF;

//GF character definitions
static const CharFrame char_gf_frame[] = {
	{GF_ArcMain_Loggo, {  0,  0,120, 82}, {  0,  0}}, //0 bop left
	{GF_ArcMain_Loggo, {121,  0,120, 82}, {  0,  0}}, //1 bop right
	
	{GF_ArcMain_GF2, {  0,   0,  73, 100}, { 34,  71}}, //2 cry 1
	{GF_ArcMain_GF2, { 74,   0,  73, 102}, { 35,  72}}, //3 cry 2
	{GF_ArcMain_GF2, {148,   0,  73, 102}, { 34,  72}}, //4 cry 3
	{GF_ArcMain_GF2, {  0, 101,  74, 102}, { 35,  72}}, //5 cry 4
	{GF_ArcMain_GF2, { 75, 102,  73, 102}, { 34,  72}}, //6 cry 5
	
	{GF_ArcScene_0, {  0,   0,  75, 102}, { 39,  71}}, //7 left 1
	{GF_ArcScene_0, { 76,   0,  77, 103}, { 41,  72}}, //8 left 2
	
	{GF_ArcScene_0, {154,   0,  79, 102}, { 37,  71}}, //9 down 1
	{GF_ArcScene_0, {  0, 103,  78, 104}, { 37,  72}}, //10 down 2
	
	{GF_ArcScene_0, { 79, 104,  79, 108}, { 39,  78}}, //11 up 1
	{GF_ArcScene_0, {159, 104,  79, 109}, { 39,  78}}, //12 up 2
	
	{GF_ArcScene_1, {  0,   0,  81, 102}, { 41,  71}}, //13 right 1
	{GF_ArcScene_1, { 81,   0,  76, 103}, { 36,  72}}, //14 right 2
	
	{GF_ArcScene_1, {158,   0,  75, 108}, { 36,  78}}, //15 cheer 1
	{GF_ArcScene_1, {  0, 103,  77, 107}, { 37,  77}}, //16 cheer 2
};

static const Animation char_gf_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                        //CharAnim_Idle
	{2, (const u8[]){ 7, 8, ASCR_BACK, 1}},                                 //CharAnim_Left
	{3, (const u8[]){ 0, ASCR_CHGANI, CharAnim_RightAlt}}, //CharAnim_LeftAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},                                 //CharAnim_Down
	{1, (const u8[]){ 2, 3, 4, 5, 6, ASCR_REPEAT}},                      //CharAnim_DownAlt
	{2, (const u8[]){ 11, 12, ASCR_BACK, 1}},                                 //CharAnim_Up
	{2, (const u8[]){ 15, 16, ASCR_BACK, 1}},                                 //CharAnim_UpAlt
	{2, (const u8[]){ 13, 14, ASCR_BACK, 1}},                                 //CharAnim_Right
	{3, (const u8[]){ 1, ASCR_CHGANI, CharAnim_LeftAlt}}, //CharAnim_RightAlt
};

//GF character functions
void Char_GF_SetFrame(void *user, u8 frame)
{
	Char_GF *this = (Char_GF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GF_Tick(Character *character)
{
	Char_GF *this = (Char_GF*)character;
	
	//Initialize Pico test
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{			
		//Perform dance
	    if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
		{
			//Switch animation
			if (character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_Right)
				character->set_anim(character, CharAnim_RightAlt);
			else
				character->set_anim(character, CharAnim_LeftAlt);
		}
	}

	//Get parallax
	fixed_t parallax;
	parallax = FIXED_UNIT;
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GF_SetFrame);
	Character_DrawParallax(character, &this->tex, &char_gf_frame[this->frame], parallax);
}

void Char_GF_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim == CharAnim_Left || anim == CharAnim_Down || anim == CharAnim_Up || anim == CharAnim_Right || anim == CharAnim_UpAlt)
		character->sing_end = stage.note_scroll + FIXED_DEC(22,1); //Nearly 2 steps
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GF_Free(Character *character)
{
	Char_GF *this = (Char_GF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_scene);
}

Character *Char_GF_New(fixed_t x, fixed_t y)
{
	//Allocate gf object
	Char_GF *this = Mem_Alloc(sizeof(Char_GF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GF_New] Failed to allocate gf object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GF_Tick;
	this->character.set_anim = Char_GF_SetAnim;
	this->character.free = Char_GF_Free;
	
	Animatable_Init(&this->character.animatable, char_gf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	this->character.idle2 = 0;
	
	this->character.health_i = 2;

	//health bar color
	this->character.health_bar = 0xFFA5004A;
	
	this->character.focus_x = FIXED_DEC(2,1);
	this->character.focus_y = FIXED_DEC(-40,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	this->character.size = FIXED_DEC(6680,10000);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\GF.ARC;1");
		
	const char **pathp = (const char *[]){
		"loggo.tim", //GF_ArcMain_Loggo
		"gf1.tim", //GF_ArcMain_GF1
		"gf2.tim", //GF_ArcMain_GF2
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Load scene specific art
	switch (stage.stage_id)
	{
//		case StageId_1_4: //Tutorial
//		{
//			this->arc_scene = IO_Read("\\CHAR\\GFTUT.ARC;1");
//			
//			const char **pathp = (const char *[]){
//				"tut0.tim", //GF_ArcScene_0
//				"tut1.tim", //GF_ArcScene_1
//				NULL
//			};
//			IO_Data *arc_ptr = &this->arc_ptr[GF_ArcScene_0];
//			for (; *pathp != NULL; pathp++)
//				*arc_ptr++ = Archive_Find(this->arc_scene, *pathp);
//			break;
//		}
		default:
			this->arc_scene = NULL;
			break;
	}
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
