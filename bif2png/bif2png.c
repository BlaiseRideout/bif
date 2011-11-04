#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
  unsigned short alpha;
} pixel_t;

typedef struct {
  pixel_t *pixels;
  size_t  width;
  size_t  height;
} bitmap_t;

typedef struct {
  pixel_t color;
  char name[256];
} bifcolor;

typedef struct {
  bifcolor *colors;
  char **data;
  unsigned int width;
  unsigned int height;
} bif;

static char     verbose = 0;
static FILE     *biffile;
static FILE     *pngfile;
static bif      *gbif;
static bitmap_t *image;
static char     eofstring[2] = { EOF, '\0' };
static pixel_t  nocolor = { 0, 0, 0, 0 };

static void     errout(int status, char *str);
static void     cleanup(void);
static void     handle_args(int argc, char **argv);
static void     init(void);
static char     *read_word(FILE *fp);
static pixel_t  *get_color(bifcolor *colors, char *name);
static void     parse_bif(bif *bifstruct, FILE *biff);
static void     bif_to_bitmap(bif *bifstruct, bitmap_t *bitmap);
static int      write_png(bitmap_t *bitmap, FILE *png);

static void errout(int status, char *str) {
  if(str)
    puts(str);
  fflush(stdout);
  exit(status);
}

static void cleanup(void) {
  fclose(biffile);
  fclose(pngfile);
}

static void handle_args(int argc, char **argv) {
  char c;
  if((argc != 4 && argc != 3) || (argc == 4 && strcmp(argv[3], "-v"))) {
    printf("Usage: %s [bif file (input)] [png file (output)] [optional: -v for verbose]\n", argv[0]);
    exit(1);
  }

  if(argc == 4)
    verbose = 1;

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

static void init(void) {
  image = malloc(sizeof(bitmap_t));
  if(!image)
    errout(-1, "Error allocating image data");
  gbif = malloc(sizeof(bif));
  if(!gbif)
    errout(-1, "Error allocating bif structure");
}

static char *read_word(FILE *fp) {
  char *word;
  char c;
  unsigned int length = 1;
  unsigned int i;

  word = malloc(length * sizeof(char));

  while((c = fgetc(fp)) != EOF)
    if(c != ' ' && c != '\n' && c != '\t')
      break;
  word[length - 1] = c;

  for(i = 0; (c = fgetc(fp)) != EOF; i++) {
    if(c != ' ' && c != '\n' && c != '\t') {
      word = realloc(word, ++length * sizeof(char));
      word[length - 1] = c;
    }
    else
      break;
  }
  if(c == EOF) {
    word[length - 1] = c;
  }
  word = realloc(word, ++length * sizeof(char));
  word[length - 1] = '\0';
  return word;
}

static pixel_t *get_color(bifcolor *colors, char *name) {
  unsigned int i;
  for(i = 0; ; ++i) {
    if(!strcmp(colors[i].name, name))
      return &colors[i].color;
  }
}

static void parse_bif(bif *bifstruct, FILE *biff) {
  char curword[256];
  char lastcolor[256];
  char contained;
  unsigned int width, height;
  unsigned int length = 0;
  unsigned int i;
  unsigned int curpix = 0;

  if(!strcmp(strcpy(curword, read_word(biff)), ""))
    errout(-1, "No width given");
  if(!(width = atoi(curword)))
    errout(-1, "Invalid width");
  if(!strcmp(strcpy(curword, read_word(biff)), ""))
    errout(-1, "No height given");
  if(!(height = atoi(curword)))
    errout(-1, "Invalid height");

  while(strcmp(strcpy(curword, read_word(biff)), "[colors]"))
    if(!strcmp(curword, eofstring))
      errout(-1, "No color block found");

  while(1) {
    bifstruct->colors = realloc(bifstruct->colors, sizeof(bifcolor) * ++length);

    strcpy(curword, read_word(biff));
    if(!strcmp(curword, "[/colors]"))
      break;
    if(!strcmp(curword, eofstring))
      errout(-1, "Colors block should end with [/colors]");
    strcpy(bifstruct->colors[length - 1].name, curword);

    strcpy(curword, read_word(biff));
    if(!(strcmp(curword, eofstring) && strcmp(curword, "[/colors]")))
      errout(-1, "Invalid color red value");
    bifstruct->colors[length - 1].color.red = atoi(curword);

    strcpy(curword, read_word(biff));
    if(!(strcmp(curword, eofstring) && strcmp(curword, "[/colors]")))
      errout(-1, "Invalid color green value");
    bifstruct->colors[length - 1].color.green = atoi(curword);

    strcpy(curword, read_word(biff));
    if(!(strcmp(curword, eofstring) && strcmp(curword, "[/colors]")))
      errout(-1, "Invalid color blue value");
    bifstruct->colors[length - 1].color.blue = atoi(curword);

    strcpy(curword, read_word(biff));
    if(!(strcmp(curword, eofstring) && strcmp(curword, "[/colors]")))
      errout(-1, "Invalid color alpha value");
    bifstruct->colors[length - 1].color.alpha = atoi(curword);
  }

  bifstruct->data = malloc(sizeof(char *) * width * height);
  for(i = 0; i < width * height; ++i) {
    bifstruct->data[i] = calloc(sizeof(char), 256);
  }

  while(strcmp(strcpy(curword, read_word(biff)), "[data]"))
    if(!strcmp(curword, eofstring))
      errout(-1, "No data found");

  while(1) {
    strcpy(curword, read_word(biff));
    if(!strcmp(curword, "[/data]"))
      break;
    if(!strcmp(curword, eofstring))
      errout(-1, "Data block should end with [/data]");

    if(!strcmp(curword, "`")) {
      if(lastcolor[0] != '\0') {
        strcpy(bifstruct->data[curpix], lastcolor);
        continue;
      }
      else
        errout(-1, "Repeat operator used before any other color");
    }
    contained = 0;
    for(i = 0; i < length; ++i)
      if(!strcmp(bifstruct->colors[i].name, curword))
        contained = 1;
    if(!contained)
      errout(-1, "Used undefined color");
    strcpy(bifstruct->data[curpix], curword);
    strcpy(lastcolor, curword);
    if(++curpix > width * height)
      break;
  }

  bifstruct->width = width;
  bifstruct->height = height;
}

static void bif_to_bitmap(bif *bifstruct, bitmap_t *bitmap) {
  if(!(bitmap && bifstruct))
    errout(-1, "No bif data to convert to or image data uninitialized");
  unsigned int x, y;

  bitmap->height = bifstruct->height;
  bitmap->width = bifstruct->width;

  bitmap->pixels = malloc(sizeof(pixel_t) * bitmap->height * bitmap->width);

  for (y = 0; y < bitmap->height; ++y) {
    for (x = 0; x < bitmap->width; ++x) {
      if(bifstruct->data[bitmap->width * y + x][0] == '\0')
        bitmap->pixels[bitmap->width * y + x] = nocolor;
      else
        bitmap->pixels[bitmap->width * y + x] = *get_color(bifstruct->colors, bifstruct->data[bitmap->width * y + x]);
    }
  }
}

static int write_png(bitmap_t *bitmap, FILE *png) {
  if(!(bitmap && png))
    return -1;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  png_byte ** row_pointers = NULL;
  int pixel_size = 4;
  int depth = 8;
  unsigned int x, y;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
      return -1;
  }

  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr) {
    return -1;
  }

  if (setjmp (png_jmpbuf (png_ptr))) {
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return -1;
  }

  png_set_IHDR (png_ptr,
                info_ptr,
                bitmap->width,
                bitmap->height,
                depth,
                PNG_COLOR_TYPE_RGBA,
                PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);

  row_pointers = png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
  for (y = 0; y < bitmap->height; ++y) {
    png_byte *row = png_malloc (png_ptr, sizeof (unsigned short) * bitmap->width * pixel_size);
    row_pointers[y] = row;
    for (x = 0; x < bitmap->width; ++x) {
      pixel_t *pixel = bitmap->pixels + bitmap->width * y + x;
      *row++ = pixel->red;
      *row++ = pixel->green;
      *row++ = pixel->blue;
      *row++ = pixel->alpha;
    }
  }

  png_init_io(png_ptr, png);
  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  for (y = 0; y < bitmap->height; ++y) {
      png_free(png_ptr, row_pointers[y]);
  }
  png_free(png_ptr, row_pointers);

  return 0;
}

int main(int argc, char **argv) {
  handle_args(argc, argv);
  if(verbose) {
    puts("Initializing...");
    fflush(stdout);
  }
  init();
  if(verbose) {
    puts("Parsing bif...");
    fflush(stdout);
  }
  parse_bif(gbif, biffile);
  if(verbose) {
    puts("bif parsed\nConverting to png...");
    fflush(stdout);
  }
  bif_to_bitmap(gbif, image);
  if(verbose) {
    puts("Converted to png\nWriting to file...");
    fflush(stdout);
  }
  if(write_png(image, pngfile))
    errout(-1, "Problem writing png file");
  if(verbose) {
    puts("png written");
    fflush(stdout);
  }
  cleanup();

  return 0;
}
