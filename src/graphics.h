
//alignment values
#define ALIGN_LEFT 0
#define ALIGN_RIGHT 1
#define ALIGN_CENTRE 2

#define VALIGN_TOP 0
#define VALIGN_BOTTOM 1
#define VALIGN_CENTRE 2

#define WIDTH 360
#define HEIGHT 240

extern unsigned int colour;
extern unsigned int backcolour;
extern unsigned char linespacing;
extern unsigned char align;
extern unsigned char valign;
extern unsigned char wordwrap;
extern unsigned char autofontsize;
extern int fontsize;
extern unsigned char Trigger;
extern volatile uint8_t KeyHit, LastKey, KeyState, KdState, PrevKey, OpMode, KeyDn, DispBlanked;
extern uint32_t KdTime, KeyDel, DispBlankTime;
extern int _width;
extern int _height;

void OutputRemap(void);
void setfont(char size, char style);
unsigned int stringheight(char *c);
unsigned int linewidth(char *c);
void opclrscr(void);
void UserRectClr(void);
void disp_putc_big(char c, char mag);
void putstr(char *c);
void putstr_align(char *c);
void setxy(int x, int y);
void LogoDisplay(void);
void BitmapDisplay(uint8_t n, uint32_t txp, uint32_t typ);
void SymbolDisplay(int px, int py, char v);

void disp_menuclear(void);
void disp_putmenuline(char y, const char *s, char style);
void disp_putmenupreview(const char *s, char style);
void disp_putmenuip(char y, char sel, uint32_t ip);
void disp_menu_scrollbar(char typ);
void disp_info(char typ, char *s);
