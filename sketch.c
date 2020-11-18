// Basic program skeleton for a Sketch File (.sk) Viewer
#include "displayfull.h"
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Allocate memory for a drawing state and initialise it
state *newState() {
  //TO DO
  state *s = malloc(sizeof(state));
  s->x = 0;
  s->y = 0;
  s->tx = 0;
  s->ty = 0;
  s->tool = LINE;
  s->start = 0;
  s->data = 0;
  s->end = 0;
  return s; // this is a placeholder only
}

// Release all memory associated with the drawing state
void freeState(state *s) {
  free(s);
  //TO DO
}

// Extract an opcode from a byte (two most significant bits).
int getOpcode(byte b) {
  //TO DO
  return  (b >> 6) & 3; // this is a placeholder only
}

// Extract an operand (-32..31) from the rightmost 6 bits of a byte.
int getOperand(byte b) {
  //TO DO
  char x;
  x = b & 0x3f;
  x = x << 2;
  x = x >> 2;
  return (int) x; 
}

// Execute the next byte of the command sequence.
void obey(display *d, state *s, byte op) {
  //TO DO
  int code = getOpcode(op);
  int operand = getOperand(op);
  if(code == DATA){
    s->data = s->data << 6;
    s->data = s->data | (((unsigned char) operand) & 0x3f);
  }
  else
    if(code == TOOL){
      if(operand == COLOUR)
      {
        colour(d, s->data);
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
          else
            if(operand == SHOW){
              show(d);
              s->data = 0;
            }
            else
              if(operand == PAUSE){
                pause(d, s->data);
                s->data = 0;
              }
              else
                if(operand == NEXTFRAME){
                  s->end = 1;
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
            line(d, s->x, s->y, s->tx, s->ty);
            s->data = 0;
          }
          if(s->tool == BLOCK){
            block(d, s->x, s->y, (s->tx - s->x), (s->ty - s->y));
            s->data = 0;
          }
          s->x = s->tx;
          s->y = s->ty;
        }
}

// Draw a frame of the sketch file. For basic and intermediate sketch files
// this means drawing the full sketch whenever this function is called.
// For advanced sketch files this means drawing the current frame whenever
// this function is called.
bool processSketch(display *d, void *data, const char pressedKey) {

    //TO DO: OPEN, PROCESS/DRAW A SKETCH FILE BYTE BY BYTE, THEN CLOSE IT
    //NOTE: CHECK DATA HAS BEEN INITIALISED... if (data == NULL) return (pressedKey == 27);
    //NOTE: TO GET ACCESS TO THE DRAWING STATE USE... state *s = (state*) data;
    //NOTE: TO GET THE FILENAME... char *filename = getName(d);
    //NOTE: DO NOT FORGET TO CALL show(d); AND TO RESET THE DRAWING STATE APART FROM
    //      THE 'START' FIELD AFTER CLOSING THE FILE
  if (data == NULL)
    return (pressedKey == 27);
  state *s = (state*) data;
  char *filename = getName(d);
  FILE *fin = fopen(filename, "rb");
  byte x;
  int count = 0, k = 0;
  x = fgetc(fin);
  bool ok = false;
  while(!feof(fin) && ok == 0){
    if(k == s->start){
      count++;
      obey(d, s, x);
      ok = s->end;
      x = fgetc(fin);
    }
    else{
      k++;
      x = fgetc(fin);
    }
  }
  show(d);
  fclose(fin);
  if(ok == 0)
    s->start = 0;
  else
    s->start = s->start + count;
  s->x = 0;
  s->y = 0;
  s->tx = 0;
  s->ty = 0;
  s->tool = LINE;
  s->data = 0;
  s->end = 0;
  return (pressedKey == 27);
}

// View a sketch file in a 200x200 pixel window given the filename
void view(char *filename) {
  display *d = newDisplay(filename, 200, 200);
  state *s = newState();
  run(d, s, processSketch);
  freeState(s);
  freeDisplay(d);
}

// Include a main function only if we are not testing (make sketch),
// otherwise use the main function of the test.c file (make test).
#ifndef TESTING
int main(int n, char *args[n]) {
  if (n != 2) { // return usage hint if not exactly one argument
    printf("Use ./sketch file\n");
    exit(1);
  } else view(args[1]); // otherwise view sketch file in argument
  return 0;
}
#endif
