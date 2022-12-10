#define XA_LENGTH(x) (((u64)(x) * 75) / 100 * IO_SECT_SIZE) //Centiseconds to sectors in bytes (w)

typedef struct
{
	XA_File file;
	u32 length;
} XA_TrackDef;

static const XA_TrackDef xa_tracks[] = {
	//MENU.XA
	{XA_Menu, XA_LENGTH(7822)}, //XA_GettinFreaky
	{XA_Menu, XA_LENGTH(3837)},  //XA_GameOver
	//WEEK1A.XA
	{XA_Week1A, XA_LENGTH(14057)}, //XA_Christmas
	{XA_Week1A, XA_LENGTH(8657)}, //XA_ChrismasHard
	//WEEK1B.XA
	{XA_Week1B, XA_LENGTH(13521)}, //XA_Spookpostor
	{XA_Week1B, XA_LENGTH(10017)}, //XA_Grinch
};

static const char *xa_paths[] = {
	"\\MUSIC\\MENU.XA;1",   //XA_Menu
	"\\MUSIC\\WEEK1A.XA;1", //XA_Week1A
	"\\MUSIC\\WEEK1B.XA;1", //XA_Week1B
	NULL,
};

typedef struct
{
	const char *name;
	boolean vocal;
} XA_Mp3;

static const XA_Mp3 xa_mp3s[] = {
	//MENU.XA
	{"freaky", false},   //XA_GettinFreaky
	{"gameover", false}, //XA_GameOver
	//WEEK1A.XA
	{"christmas", true}, //XA_Christmas
	{"christmashard", true},   //XA_ChrismasHard
	//WEEK1B.XA
	{"spookpostor", true}, //XA_Spookpostor
	{"grinch", true}, //XA_Grinch
	
	{NULL, false}
};
