#include <iostream>
#include <string>
#include <sstream>
#include <utility>
#include <map>
#include <vector>
#include <png++/png.hpp>

using namespace std;

struct color {
  public:
  int r, g, b, a;
  string name;

  color(string name, int r, int g, int b, int a) {
    this->name = name;
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }
};

struct bif {
  public:
  size_t              width;
  size_t              height;
  map<string, color*> colors;
  vector<color*>      data;

  bif(int width, int height) {
    this->width = width;
    this->height = height;
  }
  bif() {
    this->width = 0;
    this->height = 0;
  }
};

static void writebif(string, bif*);
static void readpng(string, bif*);
static void parseargs(int, char**);

static string biffile;
static string pngfile;
static bif *wbif;

static void writebif(string filename, bif *wbif) {
  if(wbif == NULL)
    return;

  ofstream bifstream (filename.c_str());
  if(!bifstream.is_open()) {
    cerr << "Cannot access file " << filename << endl;
    exit(-1);
  }

  bifstream << wbif->width << " " << wbif->height << endl;
  bifstream << "[colors]" << endl;
  for(map<string, color*>::iterator i = wbif->colors.begin(); i != wbif->colors.end(); ++i)
    bifstream << i->first << " " << i->second->r << " " << i->second->g << " " << i->second->b << " " << i->second->a << endl;
  bifstream << "[/colors]" << endl;

  bifstream << "[data]" << endl;
  string lastname;
  for(size_t y = 0; y < wbif->height; ++y)
    for(size_t x = 0; x < wbif->width; ++x) {
      if(wbif->data[y * wbif->height + x]->name == lastname)
        bifstream << "`";
      else {
        bifstream << wbif->data[y * wbif->height + x]->name;
        lastname = wbif->data[y * wbif->height + x]->name;
      }
      if(x == wbif->width - 1)
        bifstream << endl;
      else
        bifstream << " ";
    }
  bifstream << "[/data]" << endl;
}

static void readpng(string filename, bif* wbif) {
  png::image<png::rgba_pixel> img(filename);

  if(wbif == NULL)
    return;

  wbif->height = img.get_height();
  wbif->width = img.get_width();

  int lastr = -1, lastg = -1, lastb = -1, lasta = -1;
  string lastcolorname;
  for(size_t y = 0; y < wbif->height; ++y)
    for(size_t x = 0; x < wbif->width; ++x) {
      png::rgba_pixel pcolor = img[y][x];
      string colorname;
      if(pcolor.red == lastr && pcolor.green == lastg && pcolor.blue == lastb && pcolor.alpha == lasta)
        colorname = lastcolorname;
      else {
        lastr = pcolor.red;
        lastg = pcolor.green;
        lastb = pcolor.blue;
        lasta = pcolor.alpha;
        for(map<string, color*>::iterator i = wbif->colors.begin(); i != wbif->colors.end(); ++i)
          if(i->second->r == pcolor.red && i->second->g == pcolor.green && i->second->b == pcolor.blue && i->second->a == pcolor.alpha) {
            colorname = i->first;
            break;
          }
      }

      if(colorname.size() == 0) {
        stringstream colorstream;
        colorstream << "c";
        colorstream << wbif->colors.size();
        colorstream >> colorname;
        wbif->colors.insert(pair<string, color*>(colorname, new color(colorname, pcolor.red, pcolor.green, pcolor.blue, pcolor.alpha)));
      }
      wbif->data.push_back(wbif->colors[colorname]);
      lastcolorname = colorname;
    }
}

static void parseargs(int argc, char **argv) {
  if(argc != 3) {
    cerr << "Usage: " << argv[0] << " [png to convert] [bif file to write to]" << endl;
    exit(-1);   
  }
  pngfile = argv[1];
  biffile = argv[2];
}

int main(int argc, char **argv) {
  parseargs(argc, argv);
  wbif = new bif();
  readpng(pngfile, wbif);
  writebif(biffile, wbif);
  return 0;
}
