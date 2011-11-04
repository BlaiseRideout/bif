#include <stdio.h>
#include <stdlib.h>

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned char alpha;
} pixdata;

typedef struct {
  pixdata *pixels;
  unsigned int width;
  unsigned int height;
} bitmap;

static FILE   *biffile;
static FILE   *pngfile;
static bitmap *image;

void errout(int status, char *str);
void cleanup(void);
void handle_args(int argc, char **argv);
void parse_bif(FILE *bif, bitmap *pixels);
void write_png(FILE *png, bitmap *pixels);

void errout(int status, char *str) {
  if(str)
    puts(str);
  exit(status);
}

void cleanup(void) {
  fclose(biffile);
  fclose(pngfile);
}

void handle_args(int argc, char **argv) {
  char c;
  if(argc != 3) {
    printf("Usage: %s [bif file (input)] [png file (output)]\n", argv[0]);
    exit(1);
  }

  if(!(biffile = fopen(argv[1], "r")))
    errout(1, "Cannot open source bif file");
  if((pngfile = fopen(argv[2], "r"))) {
    printf("Warning: this will overwrite %s. Continue(y/n)? ", argv[2]);
    c = getchar();
    if(c != 'y' && c != 'Y')
      exit(0);
   }
   if(pngfile)
     fclose(pngfile);
   if(!(pngfile = fopen(argv[2], "w")))
     errout(1, "Cannot open png file for writing");
}

void parse_bif(FILE *bif, bitmap *pixels) {
  
}

void write_png(FILE *png, bitmap *pixels) {
  fprintf(png, "%c%c%c%c%c%c%c%cIHDR", 137, 80, 78, 71, 13, 10, 26, 10);

  fprintf(png, "IEND");
}

int main(int argc, char **argv) {
  handle_args(argc, argv);
  parse_bif(biffile, image);
  write_png(pngfile, image);
  cleanup();

  return 0;
}
