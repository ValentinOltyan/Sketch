#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
const char NMAX = 100;

typedef struct state{
  int tool, tx, ty, x, y;
  unsigned int colour, data;
}state;
enum { NONE = 0, LINE = 1,
       BLOCK = 2, COLOUR = 3, TARGETX = 4, TARGETY = 5,
       SHOW = 6, PAUSE = 7, NEXTFRAME = 8
     };
enum { DX = 0, DY = 1, TOOL = 2,
       DATA = 3
      };
void read(FILE *fin, int w, int h, unsigned char pg[][h]){
  for(int i = 0; i < w; i++)
    for(int j = 0; j < h; j++)
      pg[i][j] = fgetc(fin);
}

int compress(unsigned char *co, int w, int h, unsigned char pg[][h]){
  int k = 0;
  for(int i = 0; i < w; i++){
    co[k] = 1;
    co[k+1] = pg[i][0];
    k +=2;
    for(int j = 1; j < h; j++){
      if(pg[i][j] == co[k - 1])
        co[k - 2]++;
      else{
        co[k] = 1;
        co[k + 1] = pg[i][j];
        k += 2;
      }
    }
  }
  return k;
}

unsigned int rgba(unsigned char x){
  unsigned int rez = 0;
  rez = rez | x;
  rez = rez << 8;
  rez = rez | x;
  rez = rez << 8;
  rez = rez | x;
  rez = rez << 8;
  rez = rez | 0xff;
  return rez;
}

void showdata(FILE *fout, unsigned int rez){
  unsigned char a = 0xc0;
  a = a | rez >> 30;
  fprintf(fout, "%c", a);
  a = 0xc0;
  a = a | rez >> 24;
  fprintf(fout, "%c", a);
  a = 0xc0;
  a = a | rez >> 18;
  fprintf(fout, "%c", a);
  a = 0xc0;
  a = a | rez >> 12;
  fprintf(fout, "%c", a);
  a = 0xc0;
  a = a | rez >> 6;
  fprintf(fout, "%c", a);
  a = 0xc0;
  a = a | rez;
  fprintf(fout, "%c", a);
}

void setTx(FILE *fout, int countx){
  unsigned int x = countx;
  unsigned char a = 0xc0;
  a = a | x >> 6;
  fprintf(fout, "%c", a);
  a = 0xc0;
  a = a | x;
  fprintf(fout, "%c", a);
  fprintf(fout, "%c", 0x84);
  fprintf(fout, "%c", 0x40);
}

void nextLine(FILE *fout, int county){
  fprintf(fout, "%c", 0x80);
  unsigned int y = county;
  unsigned char a = 0xc0;
  a = a | y >> 6;
  fprintf(fout, "%c", a);
  a = 0xc0;
  a = a | y;
  fprintf(fout, "%c", a);
  fprintf(fout, "%c", 0x85);
  fprintf(fout, "%c", 0x84);
  fprintf(fout, "%c", 0x40);
  fprintf(fout,"%c", 0x81);
}

void write(FILE *fout, unsigned char *co, int k, int w, int h){
  fprintf(fout,"%c", 0x81);
  int countx = 0;
  int county = 0;
  for(int i = 1; i < k; i+=2){
    int a = co[i - 1];
    countx += a;
    unsigned int rez = rgba(co[i]);
    if(rez != 255)
    showdata(fout, rez);
    fprintf(fout, "%c", 0x83);
    setTx(fout, countx);
    if(countx >= w){
      county++;
      countx = 0;
      nextLine(fout, county);
    }
  }
}

void solve1(FILE *fin, FILE *fout){
  char a[NMAX];
  fgets(a, NMAX, fin);
  int w,h,s;
  char code[NMAX];
  sscanf(a,"%s %d %d %d", code, &w, &h, &s);
  unsigned char pg[w][h];
  read(fin, w, h, pg);
  unsigned char *co = malloc(sizeof(unsigned char) * w * h * 2 + 2);
  int k = compress(co, w, h, pg);
  write(fout, co, k, w, h);
  free(co);
}

int getOpcode(unsigned char b) {
  return  (b >> 6) & 3;
}

int getOperand(unsigned char b) {
  char x;
  x = b & 0x3f;
  x = x << 2;
  x = x >> 2;
  return (int) x;
}

void drawLine(int w, int h, unsigned char img[][h + 2], state *s){
  int m = (s->ty - s->y) / (s->tx - s->x);
  int a = (-1 * m) * s->x + s->y;
  int cnt = s->tx - s->x;
  if(cnt > 0)
    cnt = 1;
  else
    cnt = -1;
  for(int i = s->x ; i != s->tx; i+=cnt){
    img[((m * i) + a)][i] = (unsigned char)s->colour;
  }
  img[((m * s->tx) + a)][s->tx] = (unsigned char)s->colour;
}

void drawBlock(int w, int h, unsigned char img[][h + 2], state *s){
  int cntx ,cnty;
  cntx = s->tx - s->x;
  cnty = s->ty - s->y;
  if(cntx > 0)
    cntx = 1;
  else
    cntx = -1;
  if(cnty > 0)
    cnty = 1;
  else
    cnty = -1;
  for(int i = s->x; i != s->tx; i+=cntx)
    for(int j = s->y; j != s->ty; j+=cnty)
      img[i][j] = (unsigned char) s->colour;
  for(int i = 0; i != s->ty; i+=cnty)
    img[s->tx][i] = (unsigned char) s->colour;
  for(int i = 0; i != s->tx; i+=cntx)
    img[i][s->ty] = (unsigned char) s->colour;
  img[s->tx][s->ty] = (unsigned char) s->colour;
}

void conv(unsigned char ch, int w, int h, unsigned char img[][h + 2], state *s){
  int code = getOpcode(ch);
  int operand = getOperand(ch);
  if(code == DATA){
    s->data = s->data << 6;
    s->data = s->data | (((unsigned char) operand) & 0x3f);
  }
  else
    if(code == TOOL){
      if(operand == COLOUR)
      {
        s->colour = s->data >> 24;
        s->data = 0;
      }
      else
        if(operand == TARGETX){
          s->tx = s->data;
          s->data = 0;
        }
        else
          if(operand == TARGETY){
            s->ty = s->data;
            s->data = 0;
          }
          else{
            s->tool = operand;
          }
      }
      else
        if(code == DX){
          s->tx = s->tx + operand;
        }
        else
          if(code == DY){
            s->ty = s->ty + operand;
            if(s->tool == LINE){
              drawLine(w, h, img, s);
              s->colour = 0;
            }
            if(s->tool == BLOCK){
              drawBlock(w, h, img, s);
              s->colour = 0;
            }
            s->x = s->tx;
            s->y = s->ty;
          }
}

void createImg(FILE *fin, int w, int h, unsigned char img[][h + 2]){
  unsigned char ch = fgetc(fin);
  state *s = malloc(sizeof(state));
  *s = (state){1,0,0,0,0,0,0};
  while(!feof(fin)){
    conv(ch, w, h, img,s);
    ch = fgetc(fin);
  }
  free(s);
}

void solve2(FILE *fin, FILE *fout){
  int w = 200, h = 200;
  fprintf(fout, "P5 %d %d 255\n", w, h);
  unsigned char img[w + 2][h + 2];
  for(int i = 0; i <= w + 1; i++)
    for(int j = 0; j <= h + 1; j++)
      img[i][j] = 0;
  createImg(fin, w, h, img);
  for(int i = 0; i < w; i++){
    for(int j = 0; j < h ; j++)
      fprintf(fout, "%c", img[i][j]);
  }
}

void open(const char finame[]){
  FILE *fin = fopen(finame, "rb");
  char foname[NMAX], extension[10];
  sscanf(finame,"%[^.]", foname);
  strncpy(extension, (finame + strlen(foname)),5);
  extension[strlen(extension)] = 0;
  if(strcmp(extension, ".pgm") == 0){
    strcat(foname, ".sk");
    FILE *fout = fopen(foname, "wb");
    solve1(fin,fout);
  }
  else
    if(strcmp(extension, ".sk") == 0){
      strcat(foname, ".pgm");
      FILE *fout = fopen(foname, "wb");
      solve2(fin,fout);
    }
    else{
      printf("Invalid file extension\n");
      return;
    }
  printf("File %s has been written\n", foname);
}


int main(int n, char *args[n]){
  if(n != 2){
    printf("Use ./converter example.pgm");
  }
  else open(args[1]);
  return 0;
}
