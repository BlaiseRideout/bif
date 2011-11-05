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
static char     *read_word(FILE *fp);
static pixel_t  *get_color(bifcolor *colors, char *name);
static void     write_bif(bif *bifstruct, FILE *biff);
static void     bitmap_to_bif(bif *bifstruct, bitmap_t *bitmap);
static int      get_png(bitmap_t *bitmap, FILE *png);

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

static void write_bif(bif *bifstruct, FILE *biff) {
  if(!(bifstruct && biff))
    errout(-1, "bif file not accessible or bif struct not allocated");
  unsigned int x, y;

  fprintf(biff, "%d %d\n[colors]\n", bifstruct->width, bifstruct->height);
  for(x = 0; x < bitmap->numcolors; ++x) {
    fprintf("%s %d %d %d %d\n", bifstruct->colors[i].name, bifstruct->colors[i].color.red,
                              bifstruct->colors[i].color.green, bifstruct->colors[i].color.blue,
                              bifstruct->colors[i].color.alpha);
  }
  fprintf(biff, "[/colors]\n[data]\n");

  for(y = 0; y < bitmap->height; ++y) {
    for(x = 0; x < bitmap->width; ++x) {
      fprintf(biff, "%s ", bitmap->data[y * bitmap->width + x]);
    }
    fprintf(biff, "\n");
  }

  fprintf(biff, "[/data]");
}

static void bif_to_bitmap(bif *bifstruct, bitmap_t *bitmap) {
  if(!(bitmap && bifstruct))
    errout(-1, "No bif data to convert from or image data uninitialized");
  unsigned int x, y;
  unsigned int i;

  bifstruct->width = bitmap->width;
  bifstruct->height = bitmap->height;

  bifstruct->data = malloc(sizeof(char *) * width * height);
  for(i = 0; i < width * height; ++i) {
    bifstruct->data[i] = calloc(sizeof(char), 256);
  }

}

static void read_png(bitmap_t *bitmap, FILE *png) {
  if(!(bitmap && png))
    errout(-1, "Error reading png");

  char header[8];
  png_structp png_ptr;
  png_infop info_ptr, end_info;
  png_bytep *row_pointers;
  unsigned int height;
  unsigned int width;
  unsigned short depth;
  unsigned int color_type;
  unsigned int filter_method;
  unsigned int compression_type;
  unsigned int interlace_type;

  fread(header, 1, 8, png);
  if(png_sig_cmp(header, 0, 8)
    errout(-1, "File not png");

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                   NULL, NULL, NULL);
  if(!png_ptr)
    errout(-1, "Error reading png");
  info_ptr = png_create_info_struct(png_ptr);
  if(!info_ptr)
    errout(-1, "Error reading png info");
  end_info = png_create_info_struct(png_ptr)
  if(!end_info)
    errout(-1, "Error reading png info");

  if(setjmp(png_jmpbuf(png_ptr)))
    errout(-1, "Error initializing longjmp");

  png_init_io(png_ptr, png);

  png_set_sig_bytes(png_ptr, 8);

  png_read_info(png_ptr, info_ptr);
  png_get_IHDR(png, ptr, info_ptr, &width, &height,
               &depth, &color_type, &interlace_type
               &compression_type, &filter_method);
  if(interlace_type == PNG_INTERLACE_ADAM7)
    errout(-1, "png file passed uses adam7 interlacing");

  if(color_type = PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY)
    png_set_add_alpha(png_ptr, 255, PNG_FILLER_AFTER);
  if(color_type = PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);

  bitmap->width = width;
  bitmap->height = height;

  row_pointers = calloc(sizeof(png_bytep), height);
  row_pointers = png_get_rows(png_ptr, info_ptr);

  png_read_image(png_ptr, row_pointers);

  png_read_end(png_ptr, end_info);
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
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
