/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

//thanks to spicyjpeg for helping me understand this code

#include "save.h"

#include <libmcrd.h>
#include "stage.h"
				  
	        //HAS to be BASCUS-scusid,somename
#define savetitle "bu00:BASCUS-01031loggo"
#define savename  "LoggoPSX"

static const u8 saveIconPalette[32] = 
{
  	0x00, 0x00, 0x00, 0x80, 0xE0, 0x95, 0x60, 0x84, 0xC0, 0x8C, 0x80, 0x91,
	0x32, 0xEF, 0xFF, 0xFF, 0xF0, 0xE6, 0x87, 0xBD, 0x6C, 0xDA, 0xEA, 0xD1,
	0x82, 0x91, 0x21, 0x95, 0x26, 0x80, 0xF7, 0x80
	
	
};

static const u8 saveIconImage[128] = 
{
 	0x00, 0x00, 0x10, 0x11, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x21, 0x22,
	0x22, 0x12, 0x01, 0x00, 0x00, 0x31, 0x22, 0x22, 0x22, 0x22, 0x12, 0x00,
	0x00, 0x23, 0x22, 0x22, 0x22, 0x22, 0x22, 0x01, 0x10, 0x22, 0x22, 0x22,
	0x22, 0x22, 0x22, 0x03, 0x10, 0x42, 0x11, 0x11, 0x11, 0x11, 0x21, 0x12,
	0x51, 0x12, 0x66, 0x77, 0x67, 0x66, 0x21, 0x12, 0x21, 0x81, 0x76, 0x77,
	0x66, 0x66, 0x16, 0x12, 0x21, 0x61, 0x66, 0x66, 0x66, 0x66, 0x16, 0x12,
	0x21, 0x91, 0x6A, 0x66, 0x66, 0xA6, 0x19, 0x12, 0x21, 0x12, 0xB1, 0x8B,
	0xA8, 0x9B, 0x21, 0x12, 0x21, 0x22, 0x32, 0x11, 0x11, 0x11, 0x22, 0x12,
	0x21, 0x12, 0x53, 0x22, 0x22, 0x35, 0x21, 0x12, 0x10, 0x22, 0x1C, 0x11,
	0x11, 0xC1, 0x22, 0x01, 0x10, 0xDD, 0xE1, 0xFF, 0xFF, 0x1E, 0xDD, 0x01,
	0x10, 0xDD, 0x44, 0x14, 0x41, 0x44, 0xDD, 0x01
};

static void toShiftJIS(u8 *buffer, const char *text)
{
    int pos = 0;
    for (u32 i = 0; i < strlen(text); i++) 
    {
        u8 c = text[i];
        if (c >= '0' && c <= '9') { buffer[pos++] = 0x82; buffer[pos++] = 0x4F + c - '0'; }
        else if (c >= 'A' && c <= 'Z') { buffer[pos++] = 0x82; buffer[pos++] = 0x60 + c - 'A'; }
        else if (c >= 'a' && c <= 'z') { buffer[pos++] = 0x82; buffer[pos++] = 0x81 + c - 'a'; }
        else if (c == '(') { buffer[pos++] = 0x81; buffer[pos++] = 0x69; }
        else if (c == ')') { buffer[pos++] = 0x81; buffer[pos++] = 0x6A; }
        else /* space */ { buffer[pos++] = 0x81; buffer[pos++] = 0x40; }
    }
}

static void initSaveFile(SaveFile *file, const char *name) 
{
	file->id = 0x4353;
 	file->iconDisplayFlag = 0x11;
 	file->iconBlockNum = 1;
  	toShiftJIS(file->title, name);
 	memcpy(file->iconPalette, saveIconPalette, 32);
 	memcpy(file->iconImage, saveIconImage, 128);
}

void defaultSettings()
{
	stage.prefs.ghost = 1;	
	stage.prefs.sfxmiss = 1;	
	stage.prefs.songtimer = 1;
	stage.prefs.flash = 1;

	for (int i = 0; i < StageId_Max; i++)
	{
		stage.prefs.savescore[i][0] = 0;
		stage.prefs.savescore[i][1] = 0;
		stage.prefs.savescore[i][2] = 0;
	}
}

boolean readSaveFile()
{
	int fd = open(savetitle, 0x0001);
	if (fd < 0) // file doesnt exist 
		return false;

	SaveFile file;
	if (read(fd, (void *) &file, sizeof(SaveFile)) == sizeof(SaveFile)) 
		printf("ok\n");
	else {
		printf("read error\n");
		return false;
	}
	memcpy((void *) &stage.prefs, (const void *) file.saveData, sizeof(stage.prefs));
	close(fd);
	return true;
}

void writeSaveFile()
{	
	int fd = open(savetitle, 0x0002);

	if (fd < 0) // if save doesnt exist make one
		fd =  open(savetitle, 0x0202 | (1 << 16));

	SaveFile file;
	initSaveFile(&file, savename);
  	memcpy((void *) file.saveData, (const void *) &stage.prefs, sizeof(stage.prefs));
	
	if (fd >= 0) {
	  	if (write(fd, (void *) &file, sizeof(SaveFile)) == sizeof(SaveFile)) 
	  		printf("ok\n");
	 	else 
	 		printf("write error\n");  // if save doesnt exist do a error
		close(fd);
	} 
	else 
		printf("open error %d\n", fd);  // failed to save
}
