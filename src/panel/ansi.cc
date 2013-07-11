#include "panel.h"

void ansi_setup(){
  for(int i=0; i<8; i++){
      for(int j=0; j<8; j++){
          init_pair(i*8+j+1, i, j);
      }
  }
}

void ansi_print(WINDOW * win, int y, int x, QString str){
  char * s = str.toUtf8().data();
  int i,k;
  char a;
  char c[3];

  wmove(win, y, x);

  while(*s != '\0'){
    if(*s == '\033'){
      s++;
      if(*s == '['){
        for(i=0;i<3;i++) c[i] = 0;
        k = 0;
        s++;

        while(*s != 'm' && *s >= '0' && *s <= '9' && k < 3){
          i = 0;
          while(*s >= '0' && *s <= '9' && i < 2){
            c[k] <<= 4;
            c[k] |= *s - 48;
            s++;
            i++;
          }
          k++;
          while(*s == ';') s++;
        }

        a = 0;
        for(;k>=0;k--){
          if(c[k] & 0x30) a += (c[k] & 0xF) << 3;
          else if(c[k] & 0x40) a += (c[k] & 0xF);
        }
        if(a == 0) wattron(win, COLOR(56));
        else wattron(win, COLOR(a));
      }
    } else {
      waddch(win, *s);
    }

    s++;
  }

  wattron(win, COLOR(56));
}
