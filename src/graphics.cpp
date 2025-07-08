
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#include <stdarg.h>


#include <Arduino.h>
#include "global.h"
#include "graphics.h"
#include "EPD_3in52.h"
#include "epdpaint.h"
#include "main.h"

//#include "bitmap_abstract.h" // logo

int xp=0;
int yp=0;

int _width=320;
int _height=240;


//font structure
typedef struct
{
  char ht;
  char w[95];
  unsigned int pos[95];
  //char v[];
} fontstruct;

//const fontstruct cfont_basic=
//#include "cfont_basic.inc"

/*const fontstruct cfont_arial=
#include "fonts/carial.inc"

const fontstruct cfont_arialb=
#include "fonts/carialb.inc"

const fontstruct cfont_arial12=
#include "fonts/rle_arial12.inc"
const fontstruct cfont_arial12b=
#include "fonts/rle_arial12b.inc"


const fontstruct cfont_arial16=
#include "fonts/rle_arial16.inc"*/
//const fontstruct cfont_arial16b=
//#include "fonts/rle_arial16b.inc"
/*const fontstruct cfont_arial32=
#include "fonts/rle_arial32.inc"
const fontstruct cfont_georgia=
#include "fonts/cgeorgia.inc"
const fontstruct cfont_georgiab=
#include "fonts/cgeorgiab.inc"
const fontstruct cfont_georgia12=
#include "fonts/rle_georgia12.inc"
const fontstruct cfont_georgia12b=
#include "fonts/rle_georgia12b.inc"
const fontstruct cfont_georgia16=
#include "fonts/rle_georgia16.inc"
const fontstruct cfont_georgia16b=
#include "fonts/rle_georgia16b.inc"

const fontstruct cfont_lucida12=
#include "fonts/rle_lucida12.inc"
const fontstruct cfont_lucida14=
#include "fonts/rle_lucida14.inc"
//const fontstruct cfont_fixed14=
//#include "fonts/rle_cfixed14.inc"
const fontstruct cfont_courier16=
#include "fonts/ccourier16.inc"


const fontstruct cfont_basic=*/
#include "fonts/cbasic8.inc"
/*
//const fontstruct cfont_basic12=
#include "fonts/cbasic12.h"
//const fontstruct cfont_basic16=
#include "fonts/cbasic16.h"

const fontstruct cfont_swiss721md16=
#include "fonts/rle_swis721md_16.inc"
const fontstruct cfont_swiss721lt=
#include "fonts/rle_swis721lt.inc"*/


//const fontstruct cfont_lr14v=
#include "fonts/clr14v2.inc" 

#include "fonts/clrn14v2.inc" 


fontstruct *pcurrentfont=&cfont_lr14v;
char *pcurrentfontdata=(char*)&cfont_lr14vc[0];

unsigned char linespacing=2;
unsigned char align=0;
unsigned char valign=0;
unsigned char wordwrap=1;
unsigned char autofontsize=1;
int fontsize=16;
int cfont=0;
unsigned int colour=0xFFFFFF;
unsigned int backcolour=0;

unsigned char Trigger=0;
unsigned char IsClr=0;


//unsigned char remap_buf[(swidth*16)*(sheight*8)];

const uint16_t arrowup[]={
0b0000010000000000,
0b0000111000000000,
0b0001111100000000,
0b0011111110000000,
0b0111111111000000,
0b1111111111100000,
0b0011111110000000,
0b0011111110000000,
0b0011111110000000,
0b0011111110000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
};

const uint16_t arrowdown[]={
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0000000000000000,
0b0011111110000000,
0b0011111110000000,
0b0011111110000000,
0b0011111110000000,
0b1111111111100000,
0b0111111111000000,
0b0011111110000000,
0b0001111100000000,
0b0000111000000000,
0b0000010000000000,
};

const uint16_t exclaim[]={
0b0000001111000000,
0b0000001111000000,
0b0000011111100000,
0b0000011111100000,
0b0000111001110000,
0b0000111001110000,
0b0001111001111000,
0b0001111001111000,
0b0011111001111100,
0b0011111001111100,
0b0111111001111110,
0b0111111111111110,
0b1111111001111111,
0b1111111001111111,
0b1111111111111111,
0b1111111111111111,
};

const uint16_t info[]={
0b0000001111000000,
0b0001111111111000,
0b0011111001111100,
0b0111111001111110,
0b0111111111111110,
0b0111110001111110,
0b1111110001111111,
0b1111111001111111,
0b1111111001111111,
0b1111111001111111,
0b0111111001111110,
0b0111111001111110,
0b0111110000111110,
0b0011110000111100,
0b0001111111111000,
0b0000001111000000,
};

const uint16_t dmx[]={
0b0000001111000000,
0b0001111111111000,
0b0011111001111100,
0b0111111001111110,
0b0111001111001110,
0b0111001111001110,
0b1111111111111111,
0b1100111111110011,
0b1100111111110011,
0b1111111111111111,
0b0111111111111110,
0b0111111111111110,
0b0111111111111110,
0b0011111111111100,
0b0001111111111000,
0b0000001111000000,
};

const uint16_t rj45[]={
0b0000000000000000,
0b1111111111111111,
0b1111111111111111,
0b1111110000111111,
0b1111110000111111,
0b1111110000111111,
0b1100000000000011,
0b1100000000000011,
0b1100000000000011,
0b1100000000000011,
0b1100101010101011,
0b1100101010101011,
0b1111111111111111,
0b1111111111111111,
0b0000000000000000,
0b0000000000000000,
};

const uint16_t artnet[]={
0b0000000000000000,
0b0111001111111111,
0b1101101101101100,
0b1101101101101100,
0b1111101111001100,
0b1101101101101100,
0b1101101101101100,
0b0000000000001100,
0b1100110111101100,
0b1100110110001100,
0b1110110110001100,
0b1111110111101100,
0b1101110110001100,
0b1100110110001100,
0b1100110111101100,
0b0000000000000000,
};

const uint16_t tick[]={
0b0000000000000000,
0b0000000000000000,
0b0000000000001110,
0b0000000000001110,
0b0000000000011100,
0b0000000000011100,
0b0000000000111000,
0b0111000000111000,
0b0111000001110000,
0b0011100001110000,
0b0011100011100000,
0b0001110011100000,
0b0001111111000000,
0b0000111111000000,
0b0000011110000000,
0b0000000000000000,
};

const uint16_t cross[]={
0b0000000000000000,
0b0000000000000000,
0b0011100000011100,
0b0011110000111100,
0b0001111001111000,
0b0000111111110000,
0b0000011111100000,
0b0000001111000000,
0b0000011111100000,
0b0000111111110000,
0b0001111001111000,
0b0011110000111100,
0b0111100000011110,
0b0111000000001110,
0b0000000000000000,
0b0000000000000000,
};

const uint16_t wifi[]={
0b0000001111000000,
0b0000111111110000,
0b0011100000011100,
0b0110001111000110,
0b1100111111110011,
0b0011100000011100,
0b0110001111000110,
0b0000111111110000,
0b0011100000011100,
0b0000001111000000,
0b0000111111110000,
0b0001100000011000,
0b0000000110000000,
0b0000001111000000,
0b0000001111000000,
0b0000000110000000,
0b0000000000000000,
};

void drawPixel(int x, int y, int col)
{
  epd.Draw
}


//----------------------------------------------------------------------------
//!  set font
//----------------------------------------------------------------------------
void setfont(char size, char style)
{
    /*if (style==0)
    {
      if (size==8) pcurrentfont=&cfont_arial;
      if (size==12) pcurrentfont=&cfont_arial12;
      if (size==16) pcurrentfont=&cfont_arial16;
      if (size==32) pcurrentfont=&cfont_arial32;
    }*/
    
    /*if (style==1)
    {
      //if (size==8) pcurrentfont=&cfont_arialb;
      //if (size==12) pcurrentfont=&cfont_arial12b;
      if (size==16) {pcurrentfont=&cfont_arial16b; pcurrentfontdata=(char*)cfont_arial16bc;}
    }*/
    
    /*
    if (style==2)
    {
      if (size==8) pcurrentfont=&cfont_georgia;
      if (size==12) pcurrentfont=&cfont_georgia12;
      if (size==16) pcurrentfont=&cfont_georgia16;
    }*/
    /*if (style==3)
    {
      if (size==8) pcurrentfont=&cfont_georgiab;
      if (size==12) pcurrentfont=&cfont_georgia12b;
      if (size==16) pcurrentfont=&cfont_georgia16b;
    }*/
    /*if (style==4)
    {
      if (size==8) pcurrentfont=&cfont_arialn;
      if (size==12) pcurrentfont=&cfont_arialn12;
      if (size==16) pcurrentfont=&cfont_arialn16;
    }
    */
    if (style==5)
    {
      //if (size==8) pcurrentfont=&cfont_arialnb;
      //if (size==13) pcurrentfont=&cfont_lr13;
      if (size==14) {pcurrentfont=&cfont_lr14v; pcurrentfontdata=(char*)cfont_lr14vc;}
      //if (size==16) pcurrentfont=&cfont_arialn16;
    }
    
    if (style==6)
    {
      //if (size==8) pcurrentfont=&cfont_impact8;
      //if (size==12) pcurrentfont=&cfont_lucida12;
      if (size==14) {pcurrentfont=&cfont_lrn14v; pcurrentfontdata=(char*)cfont_lrn14vc;}
      //if (size==16) pcurrentfont=&cfont_courier16;
    }
    
    if (style==7)
    {
      if (size==8) {pcurrentfont=&cfont_basic; pcurrentfontdata=(char*)cfont_basicc;}
      //if (size==12) pcurrentfont=&cfont_basic12;
      //if (size==16) pcurrentfont=&cfont_basic16;
    }
    
    /*if (style==8)
    {
      if (size==16) pcurrentfont=&cfont_swiss721md16;
      if (size==32) pcurrentfont=&cfont_swiss721lt;
    }*/


    cfont=style;
}



//----------------------------------------------------------------------------
//!  string height of multiline string
//----------------------------------------------------------------------------
unsigned int stringheight(char *c)
{
    unsigned int ht=pcurrentfont->ht; //start of one line high
    while (*c!=0)
    {
       if (*c==10) ht=ht+(pcurrentfont->ht)+linespacing;
       c++;
    }
    return ht;
}

//----------------------------------------------------------------------------
//!  string width to next cr or end
//----------------------------------------------------------------------------
unsigned int linewidth(char *c)
{
    int wid=0;
    while ((*c!=0)&&(*c!=13))
    {
       if (*c>=32) wid=wid+(pcurrentfont->w[(*c)-32])+1;
       c++;
    }
    return wid;
}

//----------------------------------------------------------------------------
//!  set pixel
//----------------------------------------------------------------------------
void opset(unsigned int x, unsigned int y, unsigned char c)
{
   if ((x<0)||(x>=_width)) return;
   if ((y<0)||(y>=_height)) return;
   //x=(_width)-1-x;    //correct for being upside down
   //y=_height*8-1-y;
   drawPixel(x,y,c);
   IsClr=0; //flag screen not clr

}

//----------------------------------------------------------------------------
//!  clearscreen
//----------------------------------------------------------------------------
void opclrscr(void)
{
   if (IsClr) return; //save time if already clr
   fillScreen(backcolour);
   IsClr=1;
   xp=0;yp=0;
}

//----------------------------------------------------------------------------
//!  putch
//----------------------------------------------------------------------------
void opputc(char c)
{
   int txp=xp, typ=yp;
   int i,j,cc;
   char *p,pv;
   unsigned char rc=0,rv;
   char cr=(colour>>16)&0xFF,cg=(colour>>8)&0xFF,cb=colour&0xFF;
   char br=(backcolour>>16)&0xFF,bg=(backcolour>>8)&0xFF,bb=backcolour&0xFF;

   //check for special chars
   if (c==180) c=39; //strange '

   
   p=(char *)(pcurrentfontdata+pcurrentfont->pos[c-32]); //this gives a pointer to the character bitmap data
   if (c>=32)
   {
     for (j=0; j<pcurrentfont->ht; j++)
     {
        for (i=0; i<(pcurrentfont->w[c-32]); i++)
        {
           if (rc==0) //not in a runlength encode
           {
             pv=*p;     
             p++;
             if (pv==1) 
             {
              rv=*p; p++; rc=*p-1; p++; //run length encoding start. if pix value is 1 then next 2 bytes are value:count
              pv=rv;
             }
           }
           else //currently in a runlength encode
           {
             pv=rv; //just keep repeating the same value
             rc--;
           }
           //cc=color565(((pv*cr)>>8)+(((255-pv)*br)>>8),((pv*cg)>>8)+(((255-pv)*bg)>>8),((pv*cb)>>8)+(((255-pv)*bb)>>8));
           cc=pv;
           drawPixel(xp+i,yp+j,cc);
        }
        drawPixel(xp+i,yp+j,backcolour); //1pix gap to right of char
     }
     xp+=pcurrentfont->w[c-32]+1;
     if (xp>=_width) {xp=0; yp+=pcurrentfont->ht+linespacing;}
   }
   IsClr=0; //flag screen not clr
}


void disp_putc_big(char c, char mag)
{
   int txp=xp, typ=yp;
   int i,j,cc,m,my;
   char *p,pv;
   unsigned char rc=0,rv;
   char cr=(colour>>16)&0xFF,cg=(colour>>8)&0xFF,cb=colour&0xFF;
   char br=(backcolour>>16)&0xFF,bg=(backcolour>>8)&0xFF,bb=backcolour&0xFF;

   //check for special chars
   if (c==180) c=39; //strange '

   
   p=(char *)(pcurrentfontdata+pcurrentfont->pos[c-32]); //this gives a pointer to the character bitmap data
   if ((c>=32)&&(c<128))
   {
     for (j=0; j<pcurrentfont->ht; j++)
     {
        for (i=0; i<(pcurrentfont->w[c-32]); i++)
        {
           if (rc==0) //not in a runlength encode
           {
             pv=*p;     
             p++;
             if (pv==1) 
             {
              rv=*p; p++; rc=*p-1; p++; //run length encoding start. if pix value is 1 then next 2 bytes are value:count
              pv=rv;
             }
           }
           else //currently in a runlength encode
           {
             pv=rv; //just keep repeating the same value
             rc--;
           }
           //cc=color565(((pv*cr)>>8)+(((255-pv)*br)>>8),((pv*cg)>>8)+(((255-pv)*bg)>>8),((pv*cb)>>8)+(((255-pv)*bb)>>8));
           cc=pv;

           for (m=0; m<mag;m++) {for (my=0; my<mag; my++) DrawPixel(xp+i*mag+m,yp+j*mag+my,cc);}
        }
        for (m=0; m<mag;m++) {for (my=0; my<mag; my++) DrawPixel(xp+i*mag+m,yp+j*mag+my,color565(br,bg,bb));}; //1pix gap to right of char
     }
     xp+=mag*pcurrentfont->w[c-32]+mag;
     if (xp>=_width) {xp=0; yp+=pcurrentfont->ht+linespacing;}
   }
   IsClr=0; //flag screen not clr
}

//----------------------------------------------------------------------------
//!  word wrap the string
//----------------------------------------------------------------------------
void wrap(char *c, char *out)
{
    int wid=0;
    int len;
    char *lastsp=c;
    char *plinestart;
    while (*c!=0)
    {
        wid=0;
        lastsp=0;
        plinestart=c;
        while ((*c!=0)&&(*c!=13)&&(wid<((_width*16)+1)))
        {
           if (*c>=32) wid=wid+(pcurrentfont->w[(*c)-32])+1;
           if (*c==32) lastsp=c; //remember the pos of most recent space
           c++;
        }
        if (wid>(_width*16)) //line ran over
        {
           if (lastsp!=0) //there was a space on the line
           {
              len=lastsp-plinestart;
              memcpy(out,plinestart,len);
              out+=len;
              *out++=13;
              *out++=10;
              c=lastsp+1;
           }
           else //there wasn't a space, split the word with a -
           {
              len=c-plinestart-2;
              memcpy(out,plinestart,len);
              out+=len;
              *out++='-';
              *out++=13;
              *out++=10;
              c-=2;
           }
        }
        else //line fits, copy the line
        {
            len=c-plinestart;
            memcpy(out,plinestart,len+1);
            out+=len+1;
            if (*c==0) *out=0;
            if (*c==13) 
            {
              c++;
              *out++=10;
              if (*c==10) c++;
            }
        }

    }
    *out++=0; //term null
}

//----------------------------------------------------------------------------
//!  putstr
//----------------------------------------------------------------------------
void putstr(char *c)
{
    char wrapbuf[256];
    if (wordwrap!=0)
    {
      wrap(c,wrapbuf);
      c=wrapbuf;
    }
    while (*c!=0)
    {
       if (*c==13) //carriage return
       {
          c++;
          if (align==ALIGN_CENTRE)
            xp=(_width/2)-(linewidth(c)/2);
          else if (align==ALIGN_RIGHT)
            xp=_width-linewidth(c);
          else
            xp=0; 
       }
       else if (*c==10) 
       {
          yp=yp+pcurrentfont->ht+linespacing; 
          c++;

       }
       else opputc(*c++);
    }
}

//----------------------------------------------------------------------------
//!  putstr with alignment
//----------------------------------------------------------------------------
void putstr_align(char *c)
{
    char wrapbuf[256];
    if (wordwrap!=0)
    {
      if (autofontsize!=0)
      {
        fontsize=16;
        setfont(fontsize,cfont);
      }
      wrap(c,wrapbuf);
      if (autofontsize!=0)
      {
        if (stringheight(wrapbuf)>_height*8)
        {
          fontsize=12;
          setfont(fontsize,cfont);
          wrap(c,wrapbuf);
        }
        if (stringheight(wrapbuf)>_height*8)
        {
          fontsize=8;
          setfont(fontsize,cfont);
          wrap(c,wrapbuf);
        }
        if (stringheight(wrapbuf)>_height*8)
        {
          fontsize=8; cfont=4; //switch to ArialN 8 if it still doesn't fit, as this is the smallest font
          setfont(fontsize,cfont);
          wrap(c,wrapbuf);
        }
      }
      c=wrapbuf;
    }

    if (align==ALIGN_CENTRE)
        xp=(_width/2)-(linewidth(c)/2);
    else if (align==ALIGN_RIGHT)
        xp=_width-linewidth(c);
    else
        xp=0; 

    if (valign==VALIGN_CENTRE)
        yp=(_height/2)-(stringheight(c)/2);
    else if (align==VALIGN_BOTTOM)
        yp=_height-stringheight(c);
    //else
    //    yp=0; 

    while (*c!=0)
    {
       if (*c==13) //carriage return
       {
          c++;
          if (align==ALIGN_CENTRE)
            xp=(_width/2)-(linewidth(c)/2);
          else if (align==ALIGN_RIGHT)
            xp=_width-linewidth(c);
          else
            xp=0; 
       }
       else if (*c==10) 
       {
          yp=yp+pcurrentfont->ht+linespacing; 
          c++;

       }
       else opputc(*c++);
    }
}


//----------------------------------------------------------------------------
//!  setxy
//----------------------------------------------------------------------------
void setxy(int x, int y)
{
    xp=x;
    yp=y;
}

//---------------------------------------------------------
//! display logo
//---------------------------------------------------------
void LogoDisplay(void)
{
  int x,y,txp,typ;
  uint16_t *dp;

  dp=(uint16_t *)&logobw[0];
  txp=0;
  typ=25; //ystart pos

  for (y=0; y<44; y++) //bitmap height
  {
    for (x=0; x<160; x++) //bitmap must be full width of screen 160px
    {    
      drawPixel(x+txp,y+typ,*dp++);
    }
  }

}

//---------------------------------------------------------
//! display symbol
//---------------------------------------------------------
void SymbolDisplay(int px, int py, char v)
{
   const uint16_t *p;
   int x,y;
   char d;
   uint16_t fg;

   fg=color565((colour>>16)&0xFF,(colour>>8)&0xFF,colour&0xFF);

   if (v==0) p=arrowup;
   if (v==1) p=arrowdown;
   if (v==2) p=exclaim;
   if (v==3) p=info;
   if (v==4) p=dmx;
   if (v==5) p=rj45;
   if (v==6) p=artnet;
   if (v==7) p=wifi;
   if (v==0xFE) p=tick;
   if (v==0xFF) p=cross;

  for (y=0; y<16; y++) //bitmap height
  {
    for (x=0; x<16; x++) //bitmap width
    {    
      if ((*p)&(0x8000>>x)) drawPixel(x+px,y+py,fg); //else drawPixel(x+px,y+py,0);
    }
    p++;
  }


}

//---------------------------------------------------------
//! display bitmap symbol
//---------------------------------------------------------
void BitmapDisplay(uint8_t n, uint32_t txp, uint32_t typ)
{
  int w,h,x,y;
  uint16_t *dp=0;

  /*if (n==1) {dp=(uint16_t *)&bitmap_menu[0]; w=30; h=26;}
  else if (n==2) {dp=(uint16_t *)&bitmap_left[0]; w=19; h=30;}
  else if (n==3) {dp=(uint16_t *)&bitmap_right[0]; w=19; h=30;}
  else {dp=(uint16_t *)&bitmap_pwr[0]; w=30; h=32;}*/

  for (y=0; y<h; y++) //bitmap height
  {
    for (x=0; x<w; x++) //bitmap must be full width of screen 160px
    {    
      drawPixel(x+txp,y+typ,*dp++);
    }
  }

}


//---------------------------------------------------------
//! menu display routines
//---------------------------------------------------------

#define MENUBG 0x374B53
#define MENULINEBG 0x8BA6CA
#define MENUACTBG 0xF3EC84
#define MENUFGLIGHT 0xFFFFFF
#define MENUFGDARK 0x000000

void disp_menuclear(void)
{
    fillRect(0,0,160,128,color565((MENUBG>>16)&0xFF,(MENUBG>>8)&0xFF,MENUBG&0xFF));
    setxy(0,0);
}

void disp_putmenuline(char y, const char *s, char style)
{
   unsigned int bg, fg, oldbg=backcolour, oldfg=colour,w;
   unsigned char old_align=align;
   if (style&0x80) w=145; else w=160;
   style&=0x7F;
   if (style==0) {bg=MENUBG; fg=MENUFGLIGHT; align=ALIGN_CENTRE;}
   else if (style==1) {bg=MENULINEBG; fg=MENUFGLIGHT; align=ALIGN_LEFT;}
   else if (style==2) {bg=MENUACTBG; fg=MENUFGDARK; align=ALIGN_LEFT;}
   fillRect(0,y*20,w,19,color565((bg>>16)&0xFF,(bg>>8)&0xFF,bg&0xFF));
   setxy(2,y*20+4);
   backcolour=bg; colour=fg;
   putstr_align((char *)s);
   backcolour=oldbg; colour=oldfg;
}

void disp_putmenupreview(const char *s, char style)
{
   unsigned int bg, fg, oldbg=backcolour, oldfg=colour,w;
   unsigned char old_align=align;
   setfont(8,7);
   if (style&0x80) w=145; else w=160;
   style&=0x7F;
   if (style==0) {bg=MENUBG; fg=MENUFGLIGHT; align=ALIGN_CENTRE;}
   else if (style==1) {bg=MENULINEBG; fg=MENUFGLIGHT; align=ALIGN_LEFT;}
   else if (style==2) {bg=MENUACTBG; fg=MENUFGDARK; align=ALIGN_LEFT;}
   backcolour=bg; colour=fg;
   putstr((char *)s);
   backcolour=oldbg; colour=oldfg;
   setfont(14,5);
}

void disp_putmenuip(char y, char sel, uint32_t ip)
{
   unsigned int oldbg=backcolour, oldfg=colour,w;
   char buf[8];
   backcolour=MENULINEBG; colour=MENUFGLIGHT; 
   fillRect(0,y*20,160,19,color565((backcolour>>16)&0xFF,(backcolour>>8)&0xFF,backcolour&0xFF)); //draw full back box
   if (sel==0) {backcolour=MENUACTBG; colour=MENUFGDARK; fillRect(1,y*20,25,19,color565((backcolour>>16)&0xFF,(backcolour>>8)&0xFF,backcolour&0xFF));}
   sprintf(buf,"%03d",ip&0xFF);
   setxy(2,y*20+4);putstr(buf);
   backcolour=MENULINEBG; colour=MENUFGLIGHT;   
   setxy(27,y*20+4);putstr(".");
   if (sel==1) {backcolour=MENUACTBG; colour=MENUFGDARK; fillRect(30,y*20,25,19,color565((backcolour>>16)&0xFF,(backcolour>>8)&0xFF,backcolour&0xFF));}
   sprintf(buf,"%03d",(ip>>8)&0xFF);
   setxy(31,y*20+4);putstr(buf);
   backcolour=MENULINEBG; colour=MENUFGLIGHT;   
   setxy(56,y*20+4);putstr(".");
   if (sel==2) {backcolour=MENUACTBG; colour=MENUFGDARK; fillRect(59,y*20,25,19,color565((backcolour>>16)&0xFF,(backcolour>>8)&0xFF,backcolour&0xFF));}
   sprintf(buf,"%03d",(ip>>16)&0xFF);
   setxy(60,y*20+4);putstr(buf);
   backcolour=MENULINEBG; colour=MENUFGLIGHT;   
   setxy(87,y*20+4);putstr(".");
   if (sel==3) {backcolour=MENUACTBG; colour=MENUFGDARK; fillRect(90,y*20,25,19,color565((backcolour>>16)&0xFF,(backcolour>>8)&0xFF,backcolour&0xFF));}
   sprintf(buf,"%03d",(ip>>24)&0xFF);
   setxy(91,y*20+4);putstr(buf);
   
   backcolour=oldbg; colour=oldfg;
}


void disp_menu_scrollbar(char typ)
{
   unsigned int oldfg=colour;
   drawFastVLine(145, 20, 108, color565((MENUBG>>16)&0xFF,(MENUBG>>8)&0xFF,MENUBG&0xFF)); //separator line
   drawFastVLine(146, 20, 108, color565((MENUBG>>16)&0xFF,(MENUBG>>8)&0xFF,MENUBG&0xFF)); //separator line
   fillRect(147,20,16,99,color565((MENULINEBG>>16)&0xFF,(MENULINEBG>>8)&0xFF,MENULINEBG&0xFF));
   colour=MENUFGLIGHT;
   if (typ&1) SymbolDisplay(148,23,0); //up
   if (typ&2) SymbolDisplay(148,102,1); //down
   colour=oldfg;
}

void disp_info(char typ, char *s)
{
   unsigned int bg, fg, oldbg=backcolour, oldfg=colour,w;
   
   while (popuptimer>0) delay(100); //if currently displaying a msg wait for it to finish

   bg=MENULINEBG;
   fillRect(4,48,152,32,color565((bg>>16)&0xFF,(bg>>8)&0xFF,bg&0xFF));
   
   backcolour=bg; colour=fg;
   colour=MENUFGLIGHT;
   SymbolDisplay(8,56,typ); //exclaim
   setxy(28,58); //***check for \n in string and position correctly
   putstr(s);
   backcolour=oldbg; colour=oldfg;
   popuptimer=2000;
}

