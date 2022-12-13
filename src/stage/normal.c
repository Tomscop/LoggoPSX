/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "normal.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Normal background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_people, arc_people_ptr[2];
	IO_Data arc_person, arc_person_ptr[1];
	IO_Data arc_fire, arc_fire_ptr[1];
	
	Gfx_Tex tex_back0; //back0
	Gfx_Tex tex_back1; //back1
	
	//People state
	Gfx_Tex tex_people;
	u8 people_frame, people_tex_id;

	Animatable people_animatable;
	
	//Person state
	Gfx_Tex tex_person;
	u8 person_frame, person_tex_id;

	Animatable person_animatable;
	
	//Fire state
	Gfx_Tex tex_fire;
	u8 fire_frame, fire_tex_id;

	Animatable fire_animatable;

} Back_Normal;

//People animation and rects
static const CharFrame people_frame[] = {
	{0, {  0,  0,189,107}, {  0,  0}}, //0 idle 1
	{0, {  0,108,187,109}, { -2,  2}}, //1 idle 2
	{1, {  0,  0,186,110}, { -2,  3}}, //2 idle 3
	{1, {  0,111,185,110}, { -3,  3}}, //3 idle 4
};

static const Animation people_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_BACK, 1}}, //Idle
};

//People functions
void Normal_People_SetFrame(void *user, u8 frame)
{
	Back_Normal *this = (Back_Normal*)user;
	
	//Check if this is a new frame
	if (frame != this->people_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &people_frame[this->people_frame = frame];
		if (cframe->tex != this->people_tex_id)
			Gfx_LoadTex(&this->tex_people, this->arc_people_ptr[this->people_tex_id = cframe->tex], 0);
	}
}

void Normal_People_Draw(Back_Normal *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &people_frame[this->people_frame];
    
    fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = { ox, oy, src.w << FIXED_SHIFT ,src.h<< FIXED_SHIFT };
	Debug_StageMoveDebug(&dst, 7, stage.camera.x, stage.camera.y);
	Stage_DrawTex(&this->tex_people, &src, &dst, stage.camera.bzoom);
}

//Person animation and rects
static const CharFrame person_frame[] = {
	{0, {  0,  0,107, 98}, {  0,  0}}, //0 idle 1
	{0, {108,  0,105,100}, { -1,  2}}, //1 idle 2
	{0, {  0,101,103,103}, { -2,  5}}, //2 idle 3
	{0, {104,101,101,104}, { -3,  6}}, //3 idle 4
};

static const Animation person_anim[] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_BACK, 1}}, //Idle
};

//Person functions
void Normal_Person_SetFrame(void *user, u8 frame)
{
	Back_Normal *this = (Back_Normal*)user;
	
	//Check if this is a new frame
	if (frame != this->person_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &person_frame[this->person_frame = frame];
		if (cframe->tex != this->person_tex_id)
			Gfx_LoadTex(&this->tex_person, this->arc_person_ptr[this->person_tex_id = cframe->tex], 0);
	}
}

void Normal_Person_Draw(Back_Normal *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &person_frame[this->person_frame];
    
    fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = { ox, oy, src.w << FIXED_SHIFT ,src.h<< FIXED_SHIFT };
	Debug_StageMoveDebug(&dst, 8, stage.camera.x, stage.camera.y);
	Stage_DrawTex(&this->tex_person, &src, &dst, stage.camera.bzoom);
}

//Fire animation and rects
static const CharFrame fire_frame[] = {
	{0, {  0,  0,112, 74}, {  0,  0}}, //0 idle 1
	{0, {  0, 75,112, 74}, {  0,  0}}, //1 idle 2
	{0, {  0,150,116, 74}, {  2,  0}}, //2 idle 3
};

static const Animation fire_anim[] = {
	{2, (const u8[]){0, 1, 2, 1, ASCR_CHGANI, 0}}, //Idle
};

//Fire functions
void Normal_Fire_SetFrame(void *user, u8 frame)
{
	Back_Normal *this = (Back_Normal*)user;
	
	//Check if this is a new frame
	if (frame != this->fire_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &fire_frame[this->fire_frame = frame];
		if (cframe->tex != this->fire_tex_id)
			Gfx_LoadTex(&this->tex_fire, this->arc_fire_ptr[this->fire_tex_id = cframe->tex], 0);
	}
}

void Normal_Fire_Draw(Back_Normal *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &fire_frame[this->fire_frame];
    
    fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = { ox, oy, src.w << FIXED_SHIFT ,src.h<< FIXED_SHIFT };
	Debug_StageMoveDebug(&dst, 9, stage.camera.x, stage.camera.y);
	Stage_DrawTex(&this->tex_fire, &src, &dst, stage.camera.bzoom);
}

void Back_Normal_DrawBG(StageBack *back)
{
	Back_Normal *this = (Back_Normal*)back;
	
	fixed_t fx, fy;
	
	//Animate and draw people
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP && (stage.song_step & 0x7) == 0)
		Animatable_SetAnim(&this->people_animatable, 0);
	
	Animatable_Animate(&this->people_animatable, (void*)this, Normal_People_SetFrame);
	
	Normal_People_Draw(this, FIXED_DEC(0 - 3,1) - fx, FIXED_DEC(0 + 3,1) - fy);
	
	//Animate and draw person
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP && (stage.song_step & 0x7 )== 0)
		Animatable_SetAnim(&this->person_animatable, 0);
	
	Animatable_Animate(&this->person_animatable, (void*)this, Normal_Person_SetFrame);
	
	Normal_Person_Draw(this, FIXED_DEC(0 - 3,1) - fx, FIXED_DEC(0 + 6,1) - fy);
	
	//Animate and draw fire
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP && (stage.song_step == -29))
		Animatable_SetAnim(&this->fire_animatable, 0);
	
	Animatable_Animate(&this->fire_animatable, (void*)this, Normal_Fire_SetFrame);
	
	Normal_Fire_Draw(this, FIXED_DEC(0,1) - fx, FIXED_DEC(0,1) - fy);
	
	//Draw back
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT back0_src = {  0,  0,254,255};
	RECT_FIXED back0_dst = {
		FIXED_DEC(0 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(369 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(370,1)
	};
	
	RECT back1_src = {  0,  0,243,255};
	RECT_FIXED back1_dst = {
		FIXED_DEC(367 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(353 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(370,1)
	};
	
	Debug_StageMoveDebug(&back0_dst, 5, fx, fy);
	Debug_StageMoveDebug(&back1_dst, 6, fx, fy);
	Stage_DrawTex(&this->tex_back0, &back0_src, &back0_dst, stage.camera.bzoom);
	Stage_DrawTex(&this->tex_back1, &back1_src, &back1_dst, stage.camera.bzoom);
}

void Back_Normal_Free(StageBack *back)
{
	Back_Normal *this = (Back_Normal*)back;
	
	//Free people archive
	Mem_Free(this->arc_people);
	
	//Free person archive
	Mem_Free(this->arc_person);
	
	//Free fire archive
	Mem_Free(this->arc_fire);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Normal_New(void)
{
	//Allocate background structure
	Back_Normal *this = (Back_Normal*)Mem_Alloc(sizeof(Back_Normal));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Normal_DrawBG;
	this->back.free = Back_Normal_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\BG\\NORMAL.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Mem_Free(arc_back);
	
	//Load people textures
	this->arc_people = IO_Read("\\BG\\PEOPLE.ARC;1");
	this->arc_people_ptr[0] = Archive_Find(this->arc_people, "people0.tim");
	this->arc_people_ptr[1] = Archive_Find(this->arc_people, "people1.tim");
	
	//Load person textures
	this->arc_person = IO_Read("\\BG\\PERSON.ARC;1");
	this->arc_person_ptr[0] = Archive_Find(this->arc_person, "person.tim");
	
	//Load fire textures
	this->arc_fire = IO_Read("\\BG\\FIRE.ARC;1");
	this->arc_fire_ptr[0] = Archive_Find(this->arc_fire, "fire.tim");
	
	//Initialize people state
	Animatable_Init(&this->people_animatable, people_anim);
	Animatable_SetAnim(&this->people_animatable, 0);
	this->people_frame = this->people_tex_id = 0xFF; //Force art load
	
	//Initialize person state
	Animatable_Init(&this->person_animatable, person_anim);
	Animatable_SetAnim(&this->person_animatable, 0);
	this->person_frame = this->person_tex_id = 0xFF; //Force art load
	
	//Initialize fire state
	Animatable_Init(&this->fire_animatable, fire_anim);
	Animatable_SetAnim(&this->fire_animatable, 0);
	this->fire_frame = this->fire_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
