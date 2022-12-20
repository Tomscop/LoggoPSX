/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gronch.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

//Gronch background structure
typedef struct
{
	//Stage background base structure
	StageBack back;
	
	//Textures
	IO_Data arc_fire, arc_fire_ptr[1];
	
	Gfx_Tex tex_back0; //back0
	Gfx_Tex tex_back1; //back1
	Gfx_Tex tex_back2; //back2
	Gfx_Tex tex_back3; //back3
	
	//Fire state
	Gfx_Tex tex_fire;
	u8 fire_frame, fire_tex_id;

	Animatable fire_animatable;

} Back_Gronch;

//Fire animation and rects
static const CharFrame fire_frame[] = {
	{0, {  0,  0,116, 74}, {  0,  0}}, //0 idle 1
	{0, {  0, 75,116, 74}, {  0,  0}}, //1 idle 2
	{0, {  0,150,116, 74}, {  0,  0}}, //2 idle 3
	{0, {  0,  0,  1,  1}, {  0,  0}}, //3 hide
};

static const Animation fire_anim[] = {
	{2, (const u8[]){0, 1, 2, 1, ASCR_CHGANI, 0}}, //Idle
	{1, (const u8[]){3, ASCR_BACK, 1}}, //Hide
};

//Fire functions
void Gronch_Fire_SetFrame(void *user, u8 frame)
{
	Back_Gronch *this = (Back_Gronch*)user;
	
	//Check if this is a new frame
	if (frame != this->fire_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &fire_frame[this->fire_frame = frame];
		if (cframe->tex != this->fire_tex_id)
			Gfx_LoadTex(&this->tex_fire, this->arc_fire_ptr[this->fire_tex_id = cframe->tex], 0);
	}
}

void Gronch_Fire_Draw(Back_Gronch *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &fire_frame[this->fire_frame];
    
    fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = { ox, oy, 86 << FIXED_SHIFT, 55 << FIXED_SHIFT};
	Debug_StageMoveDebug(&dst, 9, stage.camera.x, stage.camera.y);
	Stage_DrawTex(&this->tex_fire, &src, &dst, stage.camera.bzoom);
}

void Back_Gronch_DrawBG(StageBack *back)
{
	Back_Gronch *this = (Back_Gronch*)back;
	
	fixed_t fx, fy;
	
	//Animate and draw fire
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP && (stage.song_step == -29))
		Animatable_SetAnim(&this->fire_animatable, 0);
	if (stage.flag & STAGE_FLAG_JUST_STEP && (stage.song_step == 260))
		Animatable_SetAnim(&this->fire_animatable, 1);
	
	Animatable_Animate(&this->fire_animatable, (void*)this, Gronch_Fire_SetFrame);
	
	Gronch_Fire_Draw(this, FIXED_DEC(222,1) - fx, FIXED_DEC(108,1) - fy);
	
	//Draw back
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT back0_src = {  0,  0,255,255};
	RECT_FIXED back0_dst = {
		FIXED_DEC(0 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(281 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(281,1)
	};
	
	RECT back1_src = {  0,  0,197,255};
	RECT_FIXED back1_dst = {
		FIXED_DEC(280 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(0,1) - fy,
		FIXED_DEC(217 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(281,1)
	};
	
	RECT back2_src = {  0,  0,255,255};
	RECT_FIXED back2_dst = {
		FIXED_DEC(-1 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(13,1) - fy,
		FIXED_DEC(281 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(281,1)
	};
	
	RECT back3_src = {  0,  0,197,255};
	RECT_FIXED back3_dst = {
		FIXED_DEC(279 - screen.SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(13,1) - fy,
		FIXED_DEC(217 + screen.SCREEN_WIDEOADD,1),
		FIXED_DEC(281,1)
	};
	
	Debug_StageMoveDebug(&back0_dst, 5, fx, fy);
	Debug_StageMoveDebug(&back1_dst, 6, fx, fy);
	Debug_StageMoveDebug(&back2_dst, 7, fx, fy);
	Debug_StageMoveDebug(&back3_dst, 8, fx, fy);
	if (stage.song_step <= 260)
	{
		Stage_DrawTex(&this->tex_back0, &back0_src, &back0_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back1, &back1_src, &back1_dst, stage.camera.bzoom);
	}
	else if (stage.song_step >= 260)
	{
		Stage_DrawTex(&this->tex_back2, &back2_src, &back2_dst, stage.camera.bzoom);
		Stage_DrawTex(&this->tex_back3, &back3_src, &back3_dst, stage.camera.bzoom);
	}
}

void Back_Gronch_Free(StageBack *back)
{
	Back_Gronch *this = (Back_Gronch*)back;
	
	//Free fire archive
	Mem_Free(this->arc_fire);
	
	//Free structure
	Mem_Free(this);
}

StageBack *Back_Gronch_New(void)
{
	//Allocate background structure
	Back_Gronch *this = (Back_Gronch*)Mem_Alloc(sizeof(Back_Gronch));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = NULL;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_Gronch_DrawBG;
	this->back.free = Back_Gronch_Free;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\BG\\GRONCH.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_back2, Archive_Find(arc_back, "back2.tim"), 0);
	Gfx_LoadTex(&this->tex_back3, Archive_Find(arc_back, "back3.tim"), 0);
	Mem_Free(arc_back);
	
	//Load fire textures
	this->arc_fire = IO_Read("\\BG\\FIRE.ARC;1");
	this->arc_fire_ptr[0] = Archive_Find(this->arc_fire, "fire.tim");
	
	//Initialize fire state
	Animatable_Init(&this->fire_animatable, fire_anim);
	Animatable_SetAnim(&this->fire_animatable, 0);
	this->fire_frame = this->fire_tex_id = 0xFF; //Force art load
	
	return (StageBack*)this;
}
