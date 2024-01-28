#ifndef __ANSI_COLOR_H__
#define __ANSI_COLOR_H__

////////////// Most Useful ///////////

// Reset all ANSI SGR configurations
#define _RST			"\e[m"

#define _BOLD			"\e[1m"
#define _UNBOLD			"\e[22m"
#define _DIM			"\e[2m"

#define _ITALIC			"\e[3m"
#define _NOITALIC		"\e[23m"

#define _UNDERL			"\e[4m"
#define _NOUNDERL		"\e[24m"

#define _STRIKE			"\e[9m"
#define _NOSTRIKE		"\e[29m"

#define _BLK			"\e[30m"
#define _RED			"\e[31m"
#define _GRN			"\e[32m"
#define _YEL			"\e[33m"
#define _BLU			"\e[34m"
#define _MAGNTA			"\e[35m"
#define _CYAN			"\e[36m"
#define _WHITE			"\e[37m"

// Reset the text Foreground color
#define _DFCOLOR		"\e[39m"

#define _BLUB			_BLU _BOLD
#define _REDB			_RED _BOLD
#define _GRNB			_GRN _BOLD
#define _YELB			_YEL _BOLD
#define _CYANB			_CYAN _BOLD


///////////// Less useful ////////////////

#define _BLK_BKG		"\e[40m"
#define _RED_BKG		"\e[41m"
#define _GRN_BKG		"\e[42m"
#define _YEL_BKG		"\e[43m"
#define _BLU_BKG		"\e[44m"
#define _MAGNTA_BKG		"\e[45m"
#define _CYAN_BKG		"\e[46m"
#define _WHITE_BKG		"\e[47m"

// Reset the text background color
#define _DBCOLOR		"\e[49m"


#define _SGR_SHOWBLK	"\e[5m"
#define _SGR_RAPIDBLK	"\e[6m"
#define _SGR_NOBLK		"\e[25m"

#define _SGR_INVERT		"\e[7m"
#define _SGR_NOINVERT	"\e[27m"

#define _SAVE_CURSOR "\e[s"
#define _REST_CURSOR "\e[u"


#endif