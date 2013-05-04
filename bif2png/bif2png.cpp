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

  color(int r, int g, int b, int a) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->a = a;
  }
};

struct bif {
  public:
  unsigned int        width;
  unsigned int        height;
  map<string, color*> colors;
  vector<color*>      data;

  bif(int width, int height) {
    this->width = width;
    this->height = height;
  }
};

static void   readbif(string);
static void   writepng(string);
static void   parseargs(int, char**);
static string tolower(string);

static string biffile;
static string pngfile;
static bif *rbif;

static void readbif(string filename) {
  ifstream bifstream (filename.c_str());
  if(!bifstream.is_open()) {
    cerr << "Cannot access file " << filename << endl;
    exit(-1);
  }

  string tmp;
  unsigned int width, height;

  if(!(bifstream >> width) || !(bifstream >> height)) {
    cerr << "File " << filename << " not a bif file" << endl;
    exit(-1);
  }

  rbif = new bif(width, height);

  if(!(bifstream >> tmp) || tolower(tmp) != "[colors]") {
    cerr << "File " << filename << " missing [colors] block" << endl;
    exit(-1);
  }

  for(getline(bifstream, tmp); tolower(tmp) != "[/colors]" && bifstream.good(); getline(bifstream, tmp)) {
    stringstream linestream;
    linestream << tmp;
    vector<string> linearr;

    while(linestream.good()) {
      linestream >> tmp;
      linearr.push_back(tmp);
    }

    if(linearr.size() != 5) {
      if(linearr[0].size() > 0)
        cerr << "Color " << linearr[0] << " is not valid, attempting to continue without it" << endl;
      continue;
    }

    if(rbif->colors.count(linearr[0]) == 1) {
      cerr << "Color " << linearr[0] << " defined twice. Ignoring all but first definition" << endl;
      continue;
    }

    rbif->colors.insert(pair<string, color*>(linearr[0], new color(stoi(linearr[1]), stoi(linearr[2]), 
                                          stoi(linearr[3]), stoi(linearr[4]))));
  }

  if(tmp != "[/colors]") {
    cerr << "Reached end of file " << filename << " before [/colors]" << endl;
    exit(-1);
  }

  if(!(bifstream >> tmp) || tolower(tmp) != "[data]") {
    cerr << "File " << filename << " missing [data] block" << endl;
    exit(-1);
  }

  while((bifstream >> tmp) && tmp != "[/data]" && rbif->data.size() < rbif->width * rbif->height) {
    if(rbif->colors.count(tmp) == 0) {
      cerr << "Used undefined color " << tmp << endl;
      exit(-1);
    }

    rbif->data.push_back(rbif->colors[tmp]);
  }

  if(tmp != "[/data]") {
    cerr << "Reached end of file " << filename << " before [/data]" << endl;
  }

  while(rbif->data.size() < rbif->width * rbif->height)
    rbif->data.push_back(new color(0, 0, 0, 0));

}

static void writepng(string filename) {
  png::image<png::rgba_pixel> img(rbif->width, rbif->height);

  for(size_t y = 0; y < img.get_height(); ++y)
    for(size_t x = 0; x < img.get_width(); ++x) {
      color *c = rbif->data[y * rbif->width + x];

      img[y][x] = png::rgba_pixel(c->r, c->g, c->b, c->a);
    }

  img.write(filename);
}

static void parseargs(int argc, char **argv) {
  if(argc != 3) {
    cerr << "Usage: " << argv[0] << " [bif to convert] [png file to write to]" << endl;
    exit(-1);   
  }
  biffile = argv[1];
  pngfile = argv[2];
}

static string tolower(string str) {
  for(unsigned int i = 0; i < str.length(); ++i)
    str[i] = tolower(str[i]);
  return str;
}

int main(int argc, char **argv) {
  parseargs(argc, argv);
  readbif(biffile);
  writepng(pngfile);
  return 0;
}
