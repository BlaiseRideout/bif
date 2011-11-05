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
  unsigned int numcolors;
  unsigned int width;
  unsigned int height;
} bif;

static char     verbose = 0;
static FILE     *biffile;
static FILE     *pngfile;
static bif      *gbif;
static bitmap_t *image;

static void     errout(int status, char *str);
static void     cleanup(void);
static void     handle_args(int argc, char **argv);
static void     init(void);
static void     write_bif(bif *bifstruct, FILE *biff);
static void     bitmap_to_bif(bif *bifstruct, bitmap_t *bitmap);
static void     read_png(bitmap_t *bitmap, FILE *png);

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

  if(!(pngfile = fopen(argv[1], "r")))
    errout(1, "Cannot open source png file");
  if((biffile = fopen(argv[2], "r"))) {
    printf("Warning: this will overwrite %s. Continue(y/n)? ", argv[2]);
    c = getchar();
    if(c != 'y' && c != 'Y')
      exit(0);
   }
   if(biffile)
     fclose(biffile);
   if(!(biffile = fopen(argv[2], "w")))
     errout(1, "Cannot open bif file for writing");
}

static void init(void) {
  image = malloc(sizeof(bitmap_t));
  if(!image)
    errout(-1, "Error allocating image data");
  gbif = malloc(sizeof(bif));
  if(!gbif)
    errout(-1, "Error allocating bif structure");
}

static void write_bif(bif *bifstruct, FILE *biff) {
  if(!(bifstruct && biff))
    errout(-1, "bif file not accessible or bif struct not allocated");
  unsigned int x, y;

  fprintf(biff, "%d %d\n[colors]\n", bifstruct->width, bifstruct->height);
  for(x = 0; x < bifstruct->numcolors; ++x) {
    fprintf(biff, "%s %d %d %d %d\n", bifstruct->colors[x].name, bifstruct->colors[x].color.red,
                              bifstruct->colors[x].color.green, bifstruct->colors[x].color.blue,
                              bifstruct->colors[x].color.alpha);
  }
  fprintf(biff, "[/colors]\n[data]\n");

  for(y = 0; y < bifstruct->height; ++y) {
    for(x = 0; x < bifstruct->width; ++x) {
      fprintf(biff, "%s ", bifstruct->data[y * bifstruct->width + x]);
    }
    fprintf(biff, "\n");
  }

  fprintf(biff, "[/data]\n");
}

static void bitmap_to_bif(bif *bifstruct, bitmap_t *bitmap) {
  if(!(bitmap && bifstruct))
    errout(-1, "No bif data to convert from or image data uninitialized");
  unsigned int i;

  bifstruct->width = bitmap->width;
  bifstruct->height = bitmap->height;

  bifstruct->data = malloc(sizeof(char *) * bifstruct->width * bifstruct->height);
  for(i = 0; i < bifstruct->width * bifstruct->height; ++i) {
    pixel_t *pixel = &bitmap->pixels[i];
    bifstruct->data[i] = calloc(sizeof(char), 256);
//    printf("Red: %d  Green: %d Blue: %d Alpha: %d\n", pixel->red, pixel->green, pixel->blue, pixel->alpha);
  }
}

static void read_png(bitmap_t *bitmap, FILE *png) {
  if(!(bitmap && png))
    errout(-1, "Error reading png");

  png_byte header[8];
  int width, height;
  png_byte color_type;
  png_byte bit_depth;

  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;

  unsigned int y, x;

  fread(header, 1, 8, png);
  if(png_sig_cmp(header, 0, 8))
    errout(-1, "File not a png file");

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr)
    errout(-1, "png_create_read_struct failed");

  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
    errout(-1, "png_create_info_struct failed");

  if(setjmp(png_jmpbuf(png_ptr)))
    errout(-1, "error initializing png");

  png_init_io(png_ptr, png);
  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);

  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  png_read_update_info(png_ptr, info_ptr);

  if(setjmp(png_jmpbuf(png_ptr)))
    errout(-1, "Error reading png");

  if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
    png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
  if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

  row_pointers = calloc(sizeof(png_bytep), height);
  for(y = 0; y < height; y++)
    row_pointers[y] = calloc(sizeof(png_byte), width);

  png_read_image(png_ptr, row_pointers);

  bitmap->width = width;
  bitmap->height = height;
  bitmap->pixels = calloc(sizeof(pixel_t), width * height);

  for(y = 0; y < height; y++) {
    png_bytep row;
    row = row_pointers[y];
    for(x = 0; x < width; x++) {
      pixel_t *pixel = &bitmap->pixels[bitmap->width * y + x];
      pixel->red = *row++;
      pixel->green = *row++;
      pixel->blue = *row++;
      pixel->alpha = *row++;
    }
  }
}

int main(int argc, char **argv) {
  handle_args(argc, argv);
  if(verbose) {
    puts("Initializing...");
    fflush(stdout);
  }
  init();
  if(verbose) {
    puts("Parsing png...");
    fflush(stdout);
  }
  read_png(image, pngfile);
  if(verbose) {
    puts("png parsed\nConverting to bif...");
    fflush(stdout);
  }
  bitmap_to_bif(gbif, image);
  if(verbose) {
    puts("Converted to bif\nWriting to file...");
    fflush(stdout);
  }
  write_bif(gbif, biffile);
  if(verbose) {
    puts("bif written");
    fflush(stdout);
  }
  cleanup();

  return 0;
}
