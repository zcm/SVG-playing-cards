// Make playing cards

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <err.h>
#include <axl.h>
#include <iec18004.h>
#include <image.h>

#include <list.h>
#include <dict.h>

int debug = 0;
int poker = 0;
int bridge = 0;
int print = 0;
int w = 240,
    h = 336;
int ph = 70,
    vh = 32;
int ghost = 0;
int plain = 0;
int box = 0;
int toponly = 0;
int indexonly = 0;
int noleft = 0;
int noflip = 0;
int symmetric = 0;
int right = 0;
int ignis = 0;
int aspect = 0;
int reverse = 0;
int modern = 0;
int zero = 0;
int one = 0;
int eleven = 0;
int twelve = 0;
int thirteen = 0;
int court_border_under = 0;
int no_joker_pips = 0;
int force_joker_value = 0;
int noborder = 0;
int bleed = 0;
char *dpi_bleed = NULL;
int corner = 12;
int margin = 12;
int pipmargin = 5;
int index_width = 0;
int courtgrow = 0;
int courtstretch = 0;
int court_border_width = 0;
int court_pip_offset = 0;
int court_pip_offset_x = 0;
int court_pip_offset_y = 0;
int courtmargin = 2;
int topmargin = 0;
int backmargin = 0;
int pattern = -1;
int number = 0;
int fontsize = 20;
int nowidthonuse = 0;
int writeinline = 0;
int jokers = 2;
int blanks = 0;
int backs = 2;
int interleave = 0;
int doubleback = 0;
int grey = 0;
int fourcolour = 0;
int pipn = 1;
int valuen = 0;
char *extra = NULL;
char *extra_dir = "svg";
const char *back = "Diamond";
const char *ace = "Fancy";
const char *ace1 = "";
const char *ace2 = "";
const char *qr = NULL;
const char *fontfamily = NULL;
const char *fontweight = NULL;
const char *card = NULL;
char *red = "red";
char *green = "green";
char *blue = "blue";
char *black = "black";
const char *prefix = "";
const char *suffix = "";
const char *width = "2.5in";
const char *height = "3.5in";
const char *duplimate = NULL;
char *backcolour = NULL;
const char *backimage = NULL;
char *frontcolour = NULL;

const char *dir = NULL;
const char suits[] = "SHCD";
const char values[] = "0123456789TEJQKAWD";
char *colour[4];
const char *arrowcolours[] = { "red", "blue", "green", "purple", "orange" };

char *color_map_str = NULL;
dict *color_map;

#define PIPS 2
struct pip_s {
  const char *path;
  int width;
  int height;
} pip_path[PIPS][5];

#define VALUES 2
struct value_s {
  int stroke;
  const char *path;
} value_path[VALUES][18];

unsigned int duplimate_code[52];
int pipwidth(char suit, int ph);
int pipheight(char suit, int ph);

// Court card artwork - this is full path, for top half of court cards (SHCD
// then J-K within each), and joker (whole card)
#include "court.h"

#include "shared.c"

#define THO     1000
#define tho(v)  stho(alloca(20), v)
char *stho(char *t, int v) {   // Thousandths
  char *p = t + sprintf(t, "%d", v);

  p[1] = '\0';

  for (v = 0; v < 3 && p[-1] == '0'; ++v, --p) {
    p[-1] = '\0';
  }
  for (; v < 3; ++v, --p) {
    *p = p[-1];
  }

  *p = p[1] ? '.' : '\0';
  return t;
}

xml_t findid(xml_t e, const char *id) {
  xml_t s = NULL;

  while ((s = xml_element_next(e, s))) {
    if (!strcmp(xml_get(s, "@id") ? : "", id)) {
      break;
    }
  }

  return s;
}

static inline size_t get_pip_index(const char suit) {
  switch (suit) {
    // *INDENT-OFF*
    case 'S': return 0;
    case 'H': return 1;
    case 'C': return 2;
    case 'D': return 3;
    case 'J': return 4;
    // *INDENT-ON*
  }
  return -1;
}

static inline size_t get_color_index(const char suit, const char value) {
  switch (suit) {
    // *INDENT-OFF*
    case 'S': return 0;
    case 'H': return 1;
    case 'C': return 2;
    case 'D': return 3;
    case 'J': return (size_t) ('2' - value);
    // *INDENT-ON*
  }
  return -1;
}

xml_t adddefX(xml_t e, int bw, int bh, char suit, char value) {
  xml_t root = xml_tree_root(e);
  xml_t defs = xml_find(root, "defs");
  char id[4] = { 'X', suit, value };

  if (!defs || !findid(defs, id)) {
    if (!defs) {
      defs = xml_element_add_ns_after(root, NULL, "defs", root);
    }

    xml_t rect = xml_element_add(defs, "rect");
    xml_add(rect, "@id", id);
    xml_add(rect, "@width", tho(bw));
    xml_add(rect, "@height", tho(bh));
    xml_add(rect, "@x", tho(-bw / 2));
    xml_add(rect, "@y", tho(-bh / 2));
  }

  e = xml_element_add(e, "use");
  xml_addf(e, "@xlink:href", "#%s", id);

  return e;
}

xml_t adddefB(xml_t e, char value) {
  xml_t root = xml_tree_root(e);
  xml_t defs = xml_find(root, "defs");
  char id[3] = { 'B', value };

  if (!defs || !findid(defs, id)) {
    if (!defs) {
      defs = xml_element_add_ns_after(root, NULL, "defs", root);
    }

    xml_t pat = xml_element_add(defs, "pattern");
    xml_add(pat, "@id", id);
    xml_add(pat, "@width", tho(THO * pattern));
    xml_add(pat, "@height", tho(THO * pattern));

    if (!nowidthonuse) {
      xml_add(pat, "@width", tho(THO * pattern));
    }

    xml_add(pat, "@patternUnits", "userSpaceOnUse");

    if (!strcasecmp(back, "Illusion")) {
      int m = THO * pattern / 2,
          q = THO * pattern / 12;

      xml_t path = xml_element_add(pat, "path");
      xml_addf(path, "@d", "M0 0h%sv%sh%szM%s %sh%sv%sh%sz",
          tho(m), tho(m), tho(-m), tho(m), tho(m), tho(m), tho(m), tho(-m));
      xml_add(path, "@fill", dict_gets(color_map, colour[(value - '1') & 3]));

      int x, y;

      for (x = 0; x < 3; x++) {
        for (y = 0; y < 3; y++) {
          path = xml_element_add(pat, "path");
          xml_addf(path, "@d", "M%s %sl%s %sl%s %sl%s %sl%s %sl%s %sz",
              tho(x * m), tho(y * m - q * 2), tho(q), tho(q),
              tho(-q * 2), tho(q * 2), tho(q), tho(q), tho(q),
              tho(-q), tho(-q * 2), tho(-q * 2));
          xml_add(path, "@opacity", "0.75");
          xml_add(path, "@fill",
              dict_gets(color_map,
                (x + y) & 1 ? "white" : value == '2' ? black : red));

          path = xml_element_add(pat, "path");
          xml_addf(path, "@d", "M%s %sl%s %sl%s %sl%s %sl%s %sl%s %sz",
              tho(x * m + q * 2), tho(y * m), tho(-q), tho(q),
              tho(-q * 2), tho(-q * 2), tho(-q), tho(q), tho(q),
              tho(q), tho(q * 2), tho(-q * 2));
          xml_add(path, "@opacity", "0.75");
          xml_add(path, "@fill",
              dict_gets(color_map,
                (x + y) & 1 ? value == '2' ? black : red : "white"));
        }
      }
    } else {
      xml_t path = xml_element_add(pat, "path");
      xml_addf(path, "@d", "M%d 0L%d %dL%d %dL0 %dZ",
          pattern / 2, pattern, pattern / 2, pattern / 2, pattern, pattern / 2);
      xml_add(path, "@fill", dict_gets(color_map, colour[(value - '1') & 3]));
    }
  }

  e = xml_element_add(e, "rect");
  xml_addf(e, "@fill", "url(#%s)", id);

  return e;
}

xml_t addclippathZ(xml_t e, int bw, int bh, char suit, char value) {
  xml_t root = xml_tree_root(e);
  char id[4] = { suit, value, 'Z' };

  if (!findid(root, id)) {
    xml_t c = xml_element_add_ns_after(root, NULL, "clipPath", root);
    xml_add(c, "@id", id);
    adddefX(c, bw, bh, suit, value);
  }

  e = xml_element_add(e, "g");
  xml_addf(e, "@clip-path", "url(#%s)", id);

  return e;
}

xml_t addclippathQ(xml_t e, char suit) {  // Centre cut for symmetric
  xml_t root = xml_tree_root(e);
  char id[3] = { 'Q', suit };

  if (!findid(root, id)) {
    xml_t c = xml_element_add_ns_after(root, NULL, "clipPath", root);
    xml_add(c, "@id", id);
    xml_add(c, "@clipPathUnits", "objectBoundingBox");

    xml_t p = xml_element_add(c, "path");
    xml_add(p, "@d", "M0 -0.103v1.2h0.58l-0.18 -0.5l0.18 -0.2l-0.18 -0.5z");
  }

  xml_addf(e, "@clip-path", "url(#%s)", id);
  return e;
}

xml_t addsymbolM(xml_t e, char suit, char value) {
  const char *values = "A23456789TJQK";
  char *s = strchr(suits, suit);
  char *v = strchr(values, value);

  if (!s || !v) {
    return NULL;
  }

  xml_t root = xml_tree_root(e);
  char id[4] = { suit, value, 'M' };
  xml_t defs = xml_find(root, "defs");

  if (!defs || !findid(defs, id)) {
    if (!defs) {
      defs = xml_element_add_ns_after(root, NULL, "defs", root);
    }

    xml_t symbol = xml_element_add_ns_after(defs, NULL, "symbol", defs);
    xml_add(symbol, "@id", id);
    xml_add(symbol, "@viewBox", "0 0 137 1");  // 8 wide edges, 11 wide bars
    xml_add(symbol, "@preserveAspectRatio", "none");

    char *d;
    size_t len;
    FILE *path = open_memstream(&d, &len);
    unsigned int code =
      (duplimate_code[(s - suits) * 13 + (v - values)] << 1) + 0x1001;
    int n;

    for (n = 0; n < 13; n++) {
      if (code & (0x1000 >> n)) {
        int w = 1;

        while (code & (0x1000 >> (n + w))) {
          w++;
        }

        fprintf(path, "M%d 0h%dv1h%dZ",
            n * 11 - (n ? 3 : 0),
            (n && n < 12) ? w * 11 : 8,
            (n && n < 12) ? -w * 11 : -8);

        n += w - 1;
      }
    }

    fclose(path);

    xml_t x = xml_element_add(symbol, "path");
    xml_add(x, "@d", d);
  }

  e = xml_element_add(e, "use");
  xml_addf(e, "@xlink:href", "#%s", id);

  return e;
}

xml_t addsymbolsuit(xml_t e, char suit, char value, int *notfilledp) {
  int notfilled = 1;

  if (!ghost && (strcasecmp(ace, "Fancy") || suit != 'S' || value != 'A')) {
    notfilled = 0;
  }

  xml_t root = xml_tree_root(e);
  char id[4] = { 'S', suit, value };
  xml_t defs = xml_find(root, "defs");

  if (!defs || !findid(defs, id)) {
    if (!defs) {
      defs = xml_element_add_ns_after(root, NULL, "defs", root);
    }

    xml_t symbol = xml_element_add_ns_after(defs, NULL, "symbol", defs);
    xml_add(symbol, "@id", id);
    xml_add(symbol, "@viewBox", "-600 -600 1200 1200");
    xml_add(symbol, "@preserveAspectRatio", "xMinYMid");

    if (symmetric) {  // Fix bounding box for clippathQ
      xml_t rect = xml_element_add(symbol, "rect");
      xml_add(rect, "@x", "-500");
      xml_add(rect, "@y", "-500");
      xml_add(rect, "@width", "1000");
      xml_add(rect, "@height", "1000");
      xml_add(rect, "@opacity", "0");
    }

    xml_t path = xml_element_add(symbol, "path");
    xml_add(path, "@d", pip_path[pipn][get_pip_index(suit)].path);
    if (!notfilled) {
      xml_add(path, "@fill",
          dict_gets(color_map, colour[get_color_index(suit, value)]));
    }
  }

  e = xml_element_add(e, "use");
  xml_addf(e, "@xlink:href", "#%s", id);

  if (notfilledp) {
    *notfilledp = notfilled;
  }

  return e;
}

xml_t addsymbolvalue(xml_t e, char suit, char value) {
  xml_t root = xml_tree_root(e);
  char id[4] = { 'V', suit, value };
  xml_t defs = xml_find(root, "defs");

  if (!defs || !findid(defs, id)) {
    if (!defs) {
      defs = xml_element_add_ns_after(root, NULL, "defs", root);
    }

    char *v = strchr(values, value);

    xml_t symbol = xml_element_add_ns_after(defs, NULL, "symbol", defs);
    xml_add(symbol, "@id", id);
    xml_add(symbol, "@viewBox", "-500 -500 1000 1000");
    xml_add(symbol, "@preserveAspectRatio", "xMinYMid");

    xml_t path = xml_element_add(symbol, "path");
    xml_add(path, "@d", value_path[valuen][v - values].path);
    size_t color_index = get_color_index(suit, value);
    xml_add(path, "@stroke",
        dict_gets(color_map, ghost ? black : colour[color_index]));
    if (grey && (color_index & 2)) {
      xml_add(path, "@opacity", "0.5");
    }
    xml_addf(path, "@stroke-width", "%d",
        index_width ?: value_path[valuen][v - values].stroke);
    xml_add(path, "@stroke-linecap", "square");
    xml_add(path, "@stroke-miterlimit", "1.5");
    xml_add(path, "@fill", "none");
  }

  e = xml_element_add(e, "use");
  xml_addf(e, "@xlink:href", "#%s", id);

  return e;
}

xml_t addsymbolAA(xml_t e) {
  xml_t root = xml_tree_root(e);
  char *id = "AA";
  xml_t defs = xml_find(root, "defs");

  if (!defs || !findid(defs, id)) {
    if (!defs) {
      defs = xml_element_add_ns_after(root, NULL, "defs", root);
    }

    xml_t symbol = xml_element_add_ns_after(defs, NULL, "symbol", defs);
    xml_add(symbol, "@id", id);
    xml_add(symbol, "@viewBox", "-505 -505 1010 1010");
    xml_add(symbol, "@preserveAspectRatio", "xMinYMid");
    // xml_add (symbol, "+circle@r=600@fill=white", NULL);
    xml_add(symbol, "+circle@r=505", NULL);

    xml_t path = xml_element_add(symbol, "path");
    xml_add(path, "@fill", dict_gets(color_map, "white"));
    xml_add(path, "@d",
        "M495 0A495 495 0 0 1 -495 0A495 495 0 0 1 495 0"
        "M460 0A460 460 0 0 0 -460 0A460 460 0 0 0 460 0"
        "M465 -5A225 225 0 0 1 15 -5A225 225 0 0 1 465 -5"
        "M390 -5A150 150 0 0 0 90 -5A150 150 0 0 0 390 -5"
        "M-15 -5A225 225 0 0 1 -465 -5A225 225 0 0 1 -15 -5"
        "M-90 -5A150 150 0 0 0 -390 -5A150 150 0 0 0 -90 -5"
        "M15 -220l75 0l0 405l-75 35Z"
        "M-15 -220l0 440l-75 -35l0 -405Z");
  }

  e = xml_element_add(e, "use");
  xml_addf(e, "@xlink:href", "#%s", id);

  return e;
}

xml_t addsymbolFB(xml_t e) {
  xml_t root = xml_tree_root(e);
  char *id = "FB";
  xml_t defs = xml_find(root, "defs");

  if (!defs || !findid(defs, id)) {
    if (!defs) {
      defs = xml_element_add_ns_after(root, NULL, "defs", root);
    }

    xml_t symbol = xml_element_add_ns_after(defs, NULL, "symbol", defs);
    xml_add(symbol, "@id", id);
    xml_add(symbol, "@viewBox", "0 0 150 120");
    xml_add(symbol, "@preserveAspectRatio", "xMinYMid");

    xml_t path = xml_element_add(symbol, "path");
    xml_add(path, "@d",
        "M 0,120 "
        "L 0,36.5 "
        "A 36.5,36.5,0,0,1,36.5,0 "
        "L 113.5,0 "
        "A 36.5,36.5,0,0,1,141.42848,60 "
        "A 36.5,36.5,0,0,1,113.5,120 "
        "L 50,120,50,96,113.5,96 "
        "A 12.5,12.5,0,0,0,113.5,71 "
        "L 50,71,50,49,113.5,49 "
        "A 12.5,12.5,0,0,0,113.5,24 "
        "L 36.5,24 "
        "A 12.5,12.5,0,0,0,24,36.5 "
        "L 24,120 z");
  }

  e = xml_element_add(e, "use");
  xml_addf(e, "@xlink:href", "#%s", id);

  return e;
}

void makebox(int *bwp, int *bhp, char suit, char value) {
  int bw = THO * w - THO * margin    * 2 - THO * vh * 8 / 5;  // Box width
  int bh = THO * h - THO * topmargin * 2 - THO * vh * 8 / 5;  // Box height

  if (strchr("JQK", value)) {
    bw += THO * courtgrow;  // extra width
    bh += THO * courtstretch;  // extra height
  }

  if (suit == 'J'
      || (noleft && !right && (*ace1 || *ace2)
          && !strcasecmp(ace, "Goodall") && suit == 'S' && value == 'A')
      || (!strcasecmp(back, "Goodall") && suit == 'B')) {
    // Yes, use back margin, as this is bigger
    bw = THO * w - THO * (backmargin ? : margin) * 2;
    bh = THO * h - THO * (backmargin ? : topmargin) * 2;
  }

  // Aspect fix
  if (aspect || suit == 'J'
      || ((*ace1 || *ace2) && !strcasecmp(ace, "Goodall")
          && suit == 'S' && value == 'A')
      || (!strcasecmp(back, "Goodall") && suit == 'B')) {
    if (bh > bw * 2000 / 1300) {
      bh = bw * 2000 / 1300;
    } else {
      bw = bh * 1300 / 2000;
    }
  }

  *bwp = bw;
  *bhp = bh;
}

void parse_unit_multiplier(char *unit, intmax_t *out) {
  if (unit[0] == 'i' && unit[1] == 'n') {
    *out = 96;
  }
}

intmax_t units2tho(const char *in) {
  intmax_t multiplier = 1;
  char *work = alloca(sizeof (char) * (strlen(in) + 3));
  char *startwork = work;
  strcpy(work, in);

  while (*work >= '0' && *work <= '9') {
    work++;
  }

  int remaining = 3;

  if (*work == '.') {
    char *ahead = work + 1;
    while (*ahead >= '0' && *ahead <= '9' && remaining > 0) {
      remaining--;
      *work++ = *ahead++;
    }
    // I guess we should round properly here but truncating is SO much easier
  }

  while (remaining--) {
    parse_unit_multiplier(work, &multiplier);
    *work++ = '0';
  }

  *work = '\0';

  while (multiplier == 1 && ++*work) {
    parse_unit_multiplier(work, &multiplier);
  }

  char *endptr;

  return multiplier * strtoimax(startwork, &endptr, 10);
}

void compute_dpi_bleed(
    const char *w, const char *h, int *pxmod, char *out_w, char *out_h) {
  if (!dpi_bleed) {
    *pxmod = 0;
    return;
  }

  char *dpi, *bleedpx = strdupa(dpi_bleed);

  if ((dpi = strchr(bleedpx, '@'))) {
    *dpi++ = '\0';
  } else {
    dpi = "96";
  }

  size_t w_len = strlen(w),
         h_len = strlen(h);

  if (w_len < 3 || h_len < 3) {
    errx(1, "invalid width/height units: %s x %s", w, h);
  }

  char units[3] = { w[w_len - 2], w[w_len - 1] };
  intmax_t i_w = units2tho(w),
           i_h = units2tho(h),
           i_px = units2tho(bleedpx),
           i_dpi = units2tho(dpi);

  i_px = i_px * 96000 / i_dpi;

  if (!strcmp(units, "in")) {
    strcpy(out_w, tho((i_w + i_px * 2) / 96));
    strcpy(out_h, tho((i_h + i_px * 2) / 96));
  } else if (!strcmp(units, "px")) {
    strcpy(out_w, tho(i_w + i_px * 2));
    strcpy(out_h, tho(i_h + i_px * 2));
  } else {
    errx(1, "unimplemented units: %s", units);
  }

  strcpy(out_w + strlen(out_w), units);
  strcpy(out_h + strlen(out_h), units);

  *pxmod = (int) i_px;
}

xml_t makeroot(char suit, char value) {
  xml_t root = xml_tree_new("svg");
  xml_element_set_namespace(
      root, xml_namespace(root, NULL, "http://www.w3.org/2000/svg"));
  xml_namespace(root, "^xlink", "http://www.w3.org/1999/xlink");
  root->tree->encoding = NULL;

  int pxmod;
  char dpi_bleed_w[20] = { '\0' }, dpi_bleed_h[20] = { '\0' };

  compute_dpi_bleed(width, height, &pxmod, dpi_bleed_w, dpi_bleed_h);

  xml_add(root, "@width", *dpi_bleed_w ? dpi_bleed_w : width);
  xml_add(root, "@height", *dpi_bleed_h ? dpi_bleed_h : height);
  xml_addf(root, "@viewBox", "%s %s %s %s",
      tho(-THO * w / 2 - THO * bleed - pxmod),
      tho(-THO * h / 2 - THO * bleed - pxmod),
      tho(THO * w + THO * bleed * 2 + pxmod * 2),
      tho(THO * h + THO * bleed * 2 + pxmod * 2));
  xml_add(root, "@class", "card");

  // Stretch to required size both ways
  xml_add(root, "@preserveAspectRatio", "none");

  if (value && suit) {
    xml_addf(root, "@face", "%c%c", value, suit);
  }

  return root;
}

void makebackground(xml_t root, char suit, char value) {
  const char *background = "white";

  if (suit == 'B' && !strcasecmp(back, "Goodall")) {
    background = (value == '1' ? "#0fc" : "#f80");
  }
  if (suit == 'B' && !strcasecmp(back, "FireBrick")) {
    background = (value == '1' ? "#bd1220" : "white");
  }
  if (suit == 'B' && backcolour) {
    background = backcolour;
  }
  if (suit != 'B' && frontcolour) {
    background = frontcolour;
  }
  if (suit == 'J' && indexonly
      && value >= '1' && value <= '1' + sizeof (colour) / sizeof (*colour)) {
    background = colour[value - '1'];  // Solid colour jokers
  }

  if (bleed) {  // Background colour (with bleed)
    xml_t rect = xml_element_add(root, "rect");
    xml_add(rect, "@x", tho(-THO * w));
    xml_add(rect, "@y", tho(-THO * h));
    xml_add(rect, "@width", tho(THO * w * 2));
    xml_add(rect, "@height", tho(THO * h * 2));
    xml_add(rect, "@fill", background);
  }

  if (!noborder) {  // Outline of card
    xml_t border = xml_element_add(root, "rect");
    xml_add(border, "@width", tho(THO * w - THO));
    xml_add(border, "@height", tho(THO * h - THO));
    xml_add(border, "@x", tho(-THO * w / 2 + THO / 2));
    xml_add(border, "@y", tho(-THO * h / 2 + THO / 2));
    xml_add(border, "@rx", tho(THO * corner));
    xml_add(border, "@ry", tho(THO * corner));
    xml_add(border, "@fill", background);
    xml_add(border, "@stroke", "black");
  }
}

typedef struct layer {
  char *color;
  char *path;
} layer_t;

typedef struct excard {
  char *name;
  char value;
  char suit;
  list *layers;
  int flip;  // 0 forces flip, 1 forces no flip; 2 is default behavior
  int rightpip;
} excard;

void writecard(xml_t root, char suit, char value, excard *extra_card) {  // Write out
  if (writeinline) {
    if (card) {
      printf("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    }

    xml_element_write(stdout, root, 1, 1);
  } else {
    char *filename;

    if (extra_card) {
      if (!asprintf(&filename, "%s%s%s.svg", prefix, extra_card->name, suffix)) {
        errx(1, "Malloc");
      }
    } else if (number) {
      if (!asprintf(&filename, "%s%03d%s.svg", prefix, number++, suffix)) {
        errx(1, "Malloc");
      }
    } else {
      if (!asprintf(&filename, "%s%c%c%s.svg", prefix, value, suit, suffix)) {
        errx(1, "Malloc");
      }
    }

    FILE *f = f = fopen(filename, "w");

    if (!f) {
      err(1, "Cannot write %s", filename);
    }

    free(filename);

    fprintf(f,
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");

    xml_element_write(f, root, 1, 1);
    fclose(f);
  }

  xml_tree_delete(root);
}

list *svg2layers(char *filename) {
  xml_t g = NULL;
  xml_t cardxml = xml_tree_read_file(filename);

  if (!cardxml) {
    return NULL;
  }

  list *layers = list_new();

  while ((g = xml_element_next_by_name(cardxml, g, "g"))) {
    char *color, *color_label = strdupa(xml_get(g, "@inkscape:label"));

    if (color_label) {
      color = strdupa(color_label);

      for (int i = 0; color[i]; i++) {
        color[i] = tolower(color[i]);
      }

      if ((color = (char *) dict_gets(color_map, color))) {
        layer_t *layer = malloc(sizeof (layer_t));
        layer->color = color;
        layer->path = strdup(svg_merge_group_paths(g, color_label, filename));

        list_unshift(layers, layer);
      }
    }
  }

  return layers;
}

void destroy_layer(layer_t *layer) {
  free(layer->path);
  free(layer);
}

excard *load_excard(char *name, char suit, char value) {
  char *filename;

  if (!asprintf(&filename, "%s/%s.svg", extra_dir, name)) {
    errx(1, "malloc");
  }

  excard *card = malloc(sizeof (excard));

  card->name = name;
  card->suit = suit;
  card->value = value;
  card->layers = svg2layers(filename);
  card->flip = 2;
  card->rightpip = 0;

  free(filename);

  return card;
}

void destroy_excard(excard *card) {
  void *layer;

  while ((layer = list_pop(card->layers))) {
    destroy_layer(layer);
  }

  list_del(card->layers);
  free(card);
}

int makecourt(xml_t root, char suit, char value, excard *extra_card) {
  char *s = strchr(suits, suit);
  char *v = strchr(values, value);
  int bw, bh;

  if (value == 'W' || value == 'D') {
    return 0;
  }

  size_t color_index = get_color_index(suit, value);

  makebox(&bw, &bh, suit, value);

  int layer = 0;

  // Court/Joker
  if ((!plain && !indexonly && (strchr("JQK", value) || suit == 'J'))
      || ((*ace1 || *ace2)
          && !strcasecmp(ace, "Goodall")
          && suit == 'S' && value == 'A')
      || (!strcasecmp(back, "Goodall") && suit == 'B')) {
    int n = 12;  // Joker (whole card)

    if (ignis) {
      n++;  // Ignis alternative for joker
    }
    if (s && v) {
      n = (s - suits) * 3 + (v - strchr(values, 'J'));  // Court card
    }
    if (!strcasecmp(ace, "Goodall") && suit == 'S' && value == 'A') {
      n = 14;  // Goodall Ace of spades
    }
    if (!strcasecmp(back, "Goodall") && suit == 'B') {
      n = 15;
    }

    xml_t symbol = NULL;  // So we know top layer for any suit symbols

    void col_path(
        const char *path[15], const char *path_data, const char *col) {
      xml_t defs = xml_find(root, "defs");

      if (!defs) {
        defs = xml_element_add_ns_after(root, NULL, "defs", root);
      }

      symbol = xml_element_add_ns_after(defs, NULL, "symbol", defs);
      xml_addf(symbol, "@id", "%c%c%d", suit, value, ++layer);
      xml_add(symbol, "@viewBox", "0 0 1300 2000");

      // Stretch to required size both ways
      xml_add(symbol, "@preserveAspectRatio", "none");

      int swap = 0;

      if (!plain && !indexonly && n < 12) {
        if (modern && ((value == 'Q' && suit != 'H')
                       || (value == 'J' && suit != 'S'))) {
          swap = 1;
        }
        if (reverse) {
          swap = (!swap);
        }
        if (swap) {
          symbol = xml_add(
              symbol, "g@transform", "scale(-1,1)translate(-1300,0)");
        }
      }

      xml_t p = xml_element_add(symbol, "path");

      if (path == Stroke_path || path == Thin_path) {
        xml_add(p, "@stroke", col);
        xml_add(p, "@stroke-linecap", "round");
        xml_add(p, "@stroke-linejoin", "round");
        xml_add(p, "@stroke-width", path == Thin_path ? "3" : "6");
        xml_add(p, "@fill", "none");
      } else {
        xml_add(p, "@fill", col);
      }

      if (path == Stroke_path && suit == 'H' && value == 'K') {
        xml_addf(p, "@d", "%s%s", path[n],
            swap
            ? "M965.00982,185l25,30 -30,25"
              "m49.09178,-25h-19.09178"
              "m34.99998,25 -34.99998,-55v55"
            : "M1020,185l-25,30 30,25"
              "M975.9082,215H995"
              "m-35,25 35,-55v55");
      } else {
        xml_add(p, "@d", path_data);
      }
    }

    void col(const char *path[15], const char *col) {
      if (!path[n] || !*path[n]) {
        return;
      }

      col_path(path, path[n], col);
    }

    if (extra_card) {
      int do_extra_card_color(void *each_layer, size_t _unused_index) {
        col_path(NULL,
            ((layer_t *) each_layer)->path,
            ((layer_t *) each_layer)->color);
        return 0;
      }

      list_foreach(extra_card->layers, &do_extra_card_color);
    } else if (n > 13) {
      col(Black_path, dict_gets(color_map, "black"));
    } else if (ghost || (suit == 'J' && value == '2')) {
      col(Blue_path, dict_gets(color_map, "black"));
      if (ghost && suit == 'J' && value == '1') {
        col(Red_path, dict_gets(color_map, "red"));
      }
      col(Black_path, dict_gets(color_map, "black"));
      col(Stroke_path, dict_gets(color_map, "black"));
      col(Thin_path, dict_gets(color_map, "black"));
    } else {
      col(Gold_path, dict_gets(color_map, "gold"));
      col(Red_path, dict_gets(color_map, "red"));
      col(Blue_path, dict_gets(color_map, "blue"));
      col(Black_path, dict_gets(color_map, "black"));
      col(Stroke_path, dict_gets(color_map, "stroke"));
      col(Thin_path, dict_gets(color_map, "thin"));
    }

    if (symbol && s && v && !extra_card) {
      int p = 0;

      while (p < sizeof (pips[0]) / sizeof (*pips[0]) && pips[n][p].s) {
        int notfilled = 0;

        // Outline for black on black, or white background for grey
        if (ghost && !(color_index & 1)) {
          xml_t x = addsymbolsuit(symbol, suit, value, &notfilled);
          xml_addf(x, "@height", "%d", pips[n][p].s);
          if (!nowidthonuse) {
            xml_addf(x, "@width", "%d", pips[n][p].s);
          }
          xml_addf(x, "@transform",
              "translate(%d,%d)scale(1,%s)rotate(%d)translate(%d,%d)",
              /* translate(x,y) */ pips[n][p].x, 2000 - pips[n][p].y,
              /* scale(1,v)     */ tho(THO * 20ULL * bw / 13ULL / bh),
              /* rotate(r)      */ pips[n][p].r,
              /* translate(x,y) */ -pips[n][p].s / 2, -pips[n][p].s / 2);
          xml_add(x, "@fill", "none");
          xml_add(x, "@stroke", ghost ? "white" : "none");
          if (ghost) {
            xml_add(x, "@stroke-width", tho(2 * THO * 6 * 1200 / pips[n][p].s));
            xml_add(x, "@stroke-linejoin", "round");
            xml_add(x, "@stroke-linecap", "round");
          }
        }

        xml_t x = addsymbolsuit(symbol, suit, value, &notfilled);
        xml_addf(x, "@height", "%d", pips[n][p].s);
        if (!nowidthonuse) {
          xml_addf(x, "@width", "%d", pips[n][p].s);
        }
        xml_addf(x, "@transform",
            "translate(%d,%d)scale(1,%s)rotate(%d)translate(%d,%d)",
            /* translate(x,y) */ pips[n][p].x, 2000 - pips[n][p].y,
            /* scale(1,v)     */ tho(THO * 20ULL * bw / 13ULL / bh),
            /* rotate(r)      */ pips[n][p].r,
            /* translate(x,y) */ -pips[n][p].s / 2, -pips[n][p].s / 2);
        if (notfilled) {
          xml_add(x, "@fill",
              dict_gets(color_map,
                (ghost && !color_index % 2) ? "black" : colour[color_index]));
        }
        if (pips[n][p].border) {
          xml_add(x, "@stroke", dict_gets(color_map, ghost ? black : "stroke"));
          xml_add(x, "@stroke-width", tho(THO * 6 * 1200 / pips[n][p].s));
          xml_add(x, "@stroke-linejoin", "round");
          xml_add(x, "@stroke-linecap", "round");
        }

        p++;
      }
    }
  }

  return layer;
}

void makearrows(xml_t root, int bw, int bh, int n) {
  srand(n * 1000);
  int x, y;

  for (y = -bh / 2 + THO * pattern / 2; y < 0; y += THO * pattern) {
    for (x = -bw / 2 + THO * pattern / 2;
         x <= bw / 2 - THO * pattern / 2;
         x += THO * pattern) {
      int r = (rand() % 12) * 30;
      int q = rand() % (sizeof (arrowcolours) / sizeof (*arrowcolours));

      xml_t p = xml_element_add(root, "path");
      xml_addf(p, "@transform", "translate(%s,%s)rotate(%d)",
          tho(x), tho(y), r);
      xml_addf(p, "@d", "M0 %sL%s %sh%sz",
          tho(-THO * pattern / 3),
          tho(THO * pattern / 4),
          tho(THO * pattern * 433 / 1000),
          tho(-THO * pattern / 2));
      xml_add(p, "@fill", arrowcolours[q]);

      p = xml_element_add(root, "path");
      xml_addf(p, "@transform", "translate(%s,%s)rotate(%d)",
          tho(-x), tho(-y),
          (r + 180) % 360);
      xml_addf(p, "@d", "M0 %sL%s %sh%sz",
          tho(-THO * pattern / 3),
          tho(THO * pattern / 4),
          tho(THO * pattern * 433 / 1000),
          tho(-THO * pattern / 2));
      xml_add(p, "@fill", arrowcolours[q]);
    }
  }
}

void makemaze(xml_t root, int bw, int bh, int n) {
  srand(n * 1000);

  int D = 1 * THO;  // Dividing line

  // Units W/H
  int W = (bw - D * 2) / (THO * pattern),
      H = (bh - D * 2) / (THO * pattern);

  // Odd size so centre is valid
  W |= 1;
  H |= 1;

  int U = bw / W;  // Unit spacing

  if (bh / H < U) {
    U = bh / H;
  }

  // New width and height
  bw = U * W + D * 2;
  bh = U * H + D * 2;

  xml_t rect = xml_element_add(root, "rect");
  xml_add(rect, "@width", tho(bw));
  xml_add(rect, "@height", tho(bh));
  xml_add(rect, "@x", tho(-bw / 2));
  xml_add(rect, "@y", tho(-bh / 2));
  xml_add(rect, "@fill", dict_gets(color_map, colour[n & 3]));

  char *d = NULL;
  size_t len = 0;
  FILE *path = open_memstream(&d, &len);
  const int dx[] = { -1, 0, 1, 0 };
  const int dy[] = { 0, 1, 0, -1 };
  char m[W][H];  // Maze used

  memset(m, 0, W * H);

  // x/y positions for back track
  int lx[W * H],
      ly[W * H];

  memset(lx, 0, sizeof (int) * W * H);
  memset(ly, 0, sizeof (int) * W * H);

  int l = 0;  // Level for back track
  int x = W / 2,
      y = H / 2;

  lx[l] = x;
  ly[l] = y;
  m[x][y] = 1;
  l++;

  int move = 0;

  while (1) {
    int d = 0, n = 0;

    for (d = 0; d < 4; d++) {
      if (x + dx[d] >= 0
          && x + dx[d] < W
          && y + dy[d] >= 0
          && y + dy[d] < H
          && !m[x + dx[d]][y + dy[d]]) {
        n++;
      }
    }

    // fprintf (stderr, "x=%d y=%d n=%d l=%d\n", x, y, n, l);
    if (!n) {  // back track
      if (!l) {
        break;
      }

      l--;

      x = lx[l];
      y = ly[l];
      move = 0;

      continue;
    }

    n = (rand() % n);

    for (d = 0; d < 4; d++) {
      if (x + dx[d] >= 0 && x + dx[d] < W
          && y + dy[d] >= 0 && y + dy[d] < H
          && !m[x + dx[d]][y + dy[d]] && !n--) {
        break;
      }
    }

    if (!move) {
      fprintf(path, "M%d,%d", x - W / 2, y - H / 2);
    }

    move = 1;

    if (dx[d]) {
      fprintf(path, "h%d", dx[d]);
    }
    if (dy[d]) {
      fprintf(path, "v%d", dy[d]);
    }

    x += dx[d];
    y += dy[d];
    lx[l] = x;
    ly[l] = y;
    m[x][y] = 1;
    m[W - 1 - x][H - 1 - y] = 1;  // Flip side

    l++;
  }

  fclose(path);

  xml_t addpath(int a) {
    xml_t p = xml_element_add(root, "path");
    xml_add(p, "@d", d);
    if (a) {
      xml_addf(p, "@transform", "scale(%s)rotate(%d)", tho(U), a);
    } else {
      xml_addf(p, "@transform", "scale(%s)", tho(U), a);
    }
    xml_add(p, "@fill", "none");
    xml_add(p, "@stroke", dict_gets(color_map, backcolour ? : "white"));
    xml_add(p, "@stroke-width", tho(THO - THO * D * 2 / U));
    xml_add(p, "@stroke-linecap", "square");
    xml_add(p, "@stroke-linejoin", "miter");

    return p;
  }

  addpath(0);
  addpath(180);

  if (d) {
    free(d);
  }
}

void makemarked(xml_t root, int bw, int bh, char suit, char value) {
  const char *values = "A23456789TJQK";
  char *s = suit ? strchr(suits, suit) : NULL;
  char *v = value ? strchr(values, value) : NULL;
  int mx = bw / 2 - THO * pattern / 2,
      my = bh / 2 - THO * pattern / 2;
  int x, y, w = 2, h = 3, n;
  int sx[4], sy[4], vx[4], vy[4];  // Suit and value position

  for (n = 0; n < 4; n++) {
    sx[n] = (rand() % w);
    sy[n] = (rand() % h);

    do{
      vx[n] = (rand() % w);
      vy[n] = (rand() % h);
    }while (vx[n] == sx[n] && vy[n] == sy[n]);

    if (n == 0 || n == 2) {
      sx[n] = -mx + THO * pattern * sx[n];
      vx[n] = -mx + THO * pattern * vx[n];
    } else {
      sx[n] = mx - THO * pattern * sx[n];
      vx[n] = mx - THO * pattern * vx[n];
    }

    if (n == 0 || n == 1) {
      sy[n] = -my + THO * pattern * sy[n];
      vy[n] = -my + THO * pattern * vy[n];
    } else {
      sy[n] = my - THO * pattern * sy[n];
      vy[n] = my - THO * pattern * vy[n];
    }
  }

  for (y = -my; y <= my; y += THO * pattern) {
    for (x = -mx; x <= mx; x += THO * pattern) {
      int r = (rand() % 12) * 30;
      int q = rand() % (sizeof (arrowcolours) / sizeof (*arrowcolours));

      // Corner
      if (
        // *INDENT-OFF*
             (!noleft && x < -mx + THO * pattern * w
                      && y < -my + THO * pattern * h)
          ||   (right && x >  mx - THO * pattern * w
                      && y < -my + THO * pattern * h)
          ||   (right && x < -mx + THO * pattern * w
                      && y >  my - THO * pattern * h)
          || (!noleft && x >  mx - THO * pattern * w
                      && y >  my - THO * pattern * h)
        // *INDENT-ON*
          ) {
        for (n = 0; n < 4 && (sx[n] != x || sy[n] != y); n++) {}

        if (n < 4 && s && v) {
          // Suit
          q = 0;  // red
          r = (s - suits) * 90;  // suit
        } else {
          for (n = 0; n < 4 && (vx[n] != x || vy[n] != y); n++) {}

          if (n < 4 && v && value != 'A') {  // value
            q = 1;  // blue
            r = 30 + 30 * (v - values);  // clock face
          } else {  // other
            while ((!q && !(r % 90)) || q == 1) {
              if (!q && !(r % 90)) {
                // Not allowed to be red on N/S/E/W as that means suit
                r = (rand() % 12) * 30;
              }
              if (q == 1) {
                // Not allowed blue as value
                q = rand() % (sizeof (arrowcolours) / sizeof (*arrowcolours));
              }
            }
          }
        }
      }

      if (y > 0) {
        // other end of card
        r = (r + 180) % 360;
      }

      xml_t p = xml_element_add(root, "path");
      xml_addf(p, "@transform", "translate(%s,%s)rotate(%d)",
          tho(x), tho(y), r);
      xml_addf(p, "@d", "M0 %sL%s %sh%sz",
          tho(-THO * pattern / 3),
          tho(THO * pattern / 4),
          tho(THO * pattern * 433 / 1000),
          tho(-THO * pattern / 2));
      xml_add(p, "@fill", arrowcolours[q]);
    }
  }
}

void makeback(int n, char suit, char value) {
  int bw, bh;

  makebox(&bw, &bh, 'B', '1' + n);
  xml_t root = makeroot('B', '1' + n);
  makebackground(root, 'B', '1' + n);

  if (strcasecmp(back, "Plain")) {
    if (!strcasecmp(back, "FireBrick")) {
      xml_t logo = addsymbolFB(root);
      xml_add(logo, "@height", tho(THO * h / 3));
      if (!nowidthonuse) {
        xml_add(logo, "@width", tho(THO * h / 3));
      }
      xml_add(logo, "@x", tho(-THO * h / 5));
      xml_add(logo, "@y", tho(-THO * h / 3 - THO * h / 12));
      xml_add(logo, "@fill",
          dict_gets(color_map, n ? ghost ? black : "firebrick" : "white"));
      logo = addsymbolFB(root);
      xml_add(logo, "@transform", "rotate(180)");
      xml_add(logo, "@height", tho(THO * h / 3));
      if (!nowidthonuse) {
        xml_add(logo, "@width", tho(THO * h / 3));
      }
      xml_add(logo, "@x", tho(-THO * h / 5));
      xml_add(logo, "@y", tho(-THO * h / 3 - THO * h / 12));
      xml_add(logo, "@fill",
          dict_gets(color_map, n ? ghost ? black : "firebrick" : "white"));
    } else if (!strcasecmp(back, "Goodall")) {
      makecourt(root, 'B', '1' + n, NULL);
      xml_t ace = xml_element_add(root, "use");
      xml_add(ace, "@height", tho(bh));
      xml_add(ace, "@width", tho(bw));
      xml_add(ace, "@x", tho(-bw / 2));
      xml_add(ace, "@y", tho(-bh / 2));
      xml_addf(ace, "@xlink:href", "#%c%c%d", 'B', '1' + n, 1);
    } else {
      int bw = (THO * w - THO * (backmargin ? : margin) * 2)
               / (THO * pattern * 2) * (THO * pattern * 2);
      int bh = (THO * h - THO * (backmargin ? : topmargin) * 2)
               / (THO * pattern * 2) * (THO * pattern * 2);

      if (!strcasecmp(back, "Marked")) {
        makemarked(root, bw, bh, suit, value);
      } else if (!strcasecmp(back, "Maze")) {
        makemaze(root, bw, bh, n);
      } else if (!strcasecmp(back, "Arrows") || !strcasecmp(back, "AA") || !strcasecmp(back, "AA2")) {
        makearrows(root, bw, bh, n);
      } else {  // Other pattern based (Diamond, Illusion)
        xml_t border = adddefB(root, '1' + n);
        xml_add(border, "@width", tho(bw));
        xml_add(border, "@height", tho(bh));
        xml_add(border, "@x", tho(-bw / 2));
        xml_add(border, "@y", tho(-bh / 2));
        xml_add(border, "@rx", tho(THO * corner));
        xml_add(border, "@ry", tho(THO * corner));
      }

      if (!strcasecmp(back, "AA")) {
        int h = bw / (THO * pattern * 2) * (THO * pattern * 2);

        if (h > bw - THO * pattern * 2) {
          h = bw - THO * pattern * 2;
        }

        xml_t logo = addsymbolAA(root);
        xml_add(logo, "@height", tho(h));
        if (!nowidthonuse) {
          xml_add(logo, "@width", tho(h));
        }
        xml_add(logo, "@x", tho(-h / 2));
        xml_add(logo, "@y", tho(-h / 2));
        xml_add(logo, "@fill",
            dict_gets(color_map, n ? red : ghost ? black : "darkblue"));
      } else if (!strcasecmp(back, "AA2")) {
        int h = bh / 2 / (THO * pattern * 2) * (THO * pattern * 2);

        if (h > bh / 2 - THO * pattern * 2) {
          h = bh / 2 - THO * pattern * 2;
        }

        int w = bw / 2 / (THO * pattern * 2) * (THO * pattern * 2);

        if (w > bw / 2 - THO * pattern * 2) {
          w = bw / 2 - THO * pattern * 2;
        }

        void logo(xml_t g) {
          xml_t logo = addsymbolAA(g);
          xml_t t;

          xml_add(logo, "@height", tho(w));
          if (!nowidthonuse) {
            xml_add(logo, "@width", tho(w));
          }
          xml_add(logo, "@x", tho(-h));
          xml_add(logo, "@y", tho(-w));
          xml_add(logo, "@fill",
              dict_gets(color_map, n ? red : ghost ? black : "darkblue"));
          xml_add(logo, "@transform", "rotate(-90)");

          void txt(void) {
            t = xml_add(g, "+text", "www.aa.net.uk");
            xml_add(t, "@transform", "rotate(90)");
            xml_add(t, "@x", tho(-h + w + w / 4));
            xml_add(t, "@y", tho(-w / 2 + w / 10));
            xml_add(t, "@fill",
                dict_gets(color_map, n ? red : ghost ? black : "darkblue"));
            xml_add(t, "@font-family", "Ebisu");
            xml_add(t, "@font-size", tho(w / 3));
            xml_add(t, "@font-weight", "300");
          }

          txt();
          xml_add(t, "@stroke", dict_gets(color_map, "white"));
          xml_addf(t, "@stroke-width", "%d", pattern);
          txt();
        }

        logo(root);
        xml_t g = xml_element_add(root, "g");
        xml_add(g, "@transform", "rotate(180)");
        logo(g);
      }
    }
  }

  if (backimage) {
    xml_t image = xml_tree_read_file(backimage);

    if (!image) {
      err(1, "Cannot open %s", backimage);
    }

    xml_t g = xml_element_add(root, "g");
    xml_t e;

    while ((e = xml_element_next(image, NULL))) {
      xml_element_attach(g, e);
    }

    xml_tree_delete(image);
  }

  writecard(root, 'B', '1' + n, NULL);
}

void makecard(char suit, char value, excard *extra_card) {
  char *s = strchr(suits, suit);
  char *v = strchr(values, value);
  int bw, bh;

  size_t color_index = get_color_index(suit, value);

  makebox(&bw, &bh, suit, value);
  xml_t root = makeroot(suit, value);

  int layer = makecourt(root, suit, value, extra_card);

  makebackground(root, suit, value);

  void court(int flip, int ghost) {  // Court cards
    xml_t g = root;
    // if (flip) g = addclippathZ (root, bw, bh, suit, value);
    int n;

    for (n = 1; n <= layer; n++) {
      xml_t court = xml_element_add(g, "use");
      xml_add(court, "@width", tho(bw));
      xml_add(court, "@height", tho(bh));
      xml_add(court, "@x", tho(-bw / 2));
      xml_add(court, "@y", tho(-bh / 2));
      xml_addf(court, "@xlink:href", "#%c%c%d", suit, value, n);
      if ((flip && (!extra_card || extra_card->flip))
          || (extra_card && extra_card->flip == 1)) {
        court = xml_element_add(g, "use");
        xml_add(court, "@transform", "rotate(180)");
        xml_add(court, "@width", tho(bw));
        xml_add(court, "@height", tho(bh));
        xml_add(court, "@x", tho(-bw / 2));
        xml_add(court, "@y", tho(-bh / 2));
        xml_addf(court, "@xlink:href", "#%c%c%d", suit, value, n);
      }
    }
  }

  void court_border() {
    xml_t box = adddefX(root, bw, bh, suit, value);
    xml_add(box, "@stroke", dict_gets(color_map, ghost ? black : "stroke"));
    if (court_border_width) {
      char cbw[12];
      sprintf(cbw, "%d", court_border_width);
      xml_add(box, "@stroke-width", cbw);
    }
    xml_add(box, "@fill", "none");
  }

  // Box (underlay for court card artwork) when using --court-border-under
  if (court_border_under && strchr("JQK", value) && !plain && !indexonly) {
    court_border();
  }

  if ((s && v) || (suit == 'J' && !no_joker_pips)) {  // Pips
    if (box
        && (indexonly || plain || !strchr("JQK", value))
        && ((!*ace1 && !*ace2)
            || !strcasecmp(ace, "Goodall")
            || (!one && suit != 'S')
            || value != 'A' || indexonly || plain)) {  // Box (background)
      xml_t box = adddefX(root, bw, bh, suit, value);
      if (!nowidthonuse) {
        xml_add(box, "@width", tho(bw));
        xml_add(box, "@height", tho(bh));
      }
      xml_add(box, "@stroke", dict_gets(color_map, ghost ? black : "lightslateblue"));
      xml_add(box, "@fill", ghost ? "none" : dict_gets(color_map, "darkivory"));
    } else if (frontcolour && strchr("JQK", value) && !plain && !indexonly) {
      // Court background white
      xml_t box = adddefX(root, bw, bh, suit, value);
      if (!nowidthonuse) {
        xml_add(box, "@width", tho(bw));
        xml_add(box, "@height", tho(bh));
      }
      xml_add(box, "@stroke", "none");
      xml_add(box, "@fill", dict_gets(color_map, "white"));
    }

    // Pip positioning (outside of pips)
    int px = THO * w / 2 - THO * margin - THO * vh * 2 / 3;
    int py = THO * h / 2 - THO * topmargin - THO * vh * 2 / 3;

    // Limit too close to edge of card
    if (px > THO * w / 2 - THO * ph / 3) {
      px = THO * w / 2 - THO * ph / 3;
    }
    if (py > THO * h / 2 - THO * ph / 3) {
      py = THO * h / 2 - THO * ph / 3;
    }

    // Fit in box
    if (box) {
      if (px > bw / 2) {
        px = bw / 2;
      }
      if (py > bh / 2) {
        py = bh / 2;
      }

      // Position to edge of box with margin
      px -= pipwidth(suit, THO * ph / 2) + THO * pipmargin;
      py -= pipheight(suit, THO * ph / 2) + THO * pipmargin;
    } else {
      // Position to edge but align with other cards
      px -= THO * ph * 5 / 12 + THO * pipmargin;
      py -= pipheight(suit, THO * ph / 2) + THO * pipmargin;
    }

    xml_t g = root;

    xml_t pip(int x, int y, int h) {
      int notfilled = 0;
      xml_t p = addsymbolsuit(g, suit, value, &notfilled);

      x -= h / 2;
      y -= h / 2;

      if (notfilled && !ghost) {
        xml_add(p, "@fill", dict_gets(color_map, colour[color_index]));
      }
      xml_add(p, "@height", tho(h));
      if (!nowidthonuse) {
        xml_add(p, "@width", tho(h));
      }
      xml_add(p, "@x", tho(x));
      xml_add(p, "@y", tho(y));
      if (grey && (color_index & 2)) {
        xml_add(p, "@opacity", "0.5");
      }
      if (noflip && symmetric && h == THO * ph) {
        addclippathQ(p, suit);
      }

      return p;
    }

    void side2(int y) {  // half the pips
      if (indexonly || suit == 'J') {
        return;
      }

      // Face
      if (strchr("456789TEWD", value)) {  // Top row
        pip(-px, y, THO * ph);
        pip(px, y, THO * ph);
      }
      if (strchr("9TEWD", value)) {  // Second row
        pip(-px, y / 3, THO * ph);
        pip(px, y / 3, THO * ph);
      }
      if (strchr("23", value)) {
        pip(0, y, THO * ph);  // Top left
      }
      if (strchr("8", value)) {
        pip(0, (!pipn) ? y * 4 / 5 : y / 2, THO * ph);  // Middle for 8
      }
      if (strchr("TE", value)) {
        pip(0, (!pipn) ? y * 4 / 5 : y * 2 / 3, THO * ph);  // Middle for 10/11
      }
      if (value == 'W') {
        pip(0, y, THO * ph);
        pip(0, y / 3, THO * ph);
      } else if (value == 'D') {
        pip(0, y * 6 / 5, THO * ph);
        // pip (0, y, THO * ph);
        pip(0, y * 3 / 5, THO * ph);
      }

      if (symmetric && strchr("68", value)) {
        // Left centre line
        if (noflip && y > 0) {
          pip(px, 0, THO * ph);
        } else {
          pip(-px, 0, THO * ph);
        }
      }
      if (symmetric && strchr("7", value)) {
        // Left centre line
        if (noflip && y > 0) {
          pip(px, y / 3, THO * ph);
        } else {
          pip(-px, y / 3, THO * ph);
        }
      }

      if (symmetric
          && (!noflip || y > 0)
          && ((!one && suit != 'S') || value != 'A'
              || !strcasecmp(ace, "Plain"))
          && strchr("A13579ED", value)) {
        addclippathQ(pip(0, 0, THO * ph), suit);
      }
    }

    void side(int pips, int indices) {  // Symmetric pips
      if (duplimate && s && v && !strchr("EWD10", value)) {
        // 8.8mm for 13 bars, but end bars are 8/11 of others,
        // 13.1mm from right edge, 5mm height
        xml_t x = addsymbolM(g, suit, value);

        // 5mm ish plus a bit over edge of bleed
        xml_add(x, "@height", tho(THO * (bleed + 20 + (bleed ? 2 : 0))));
        xml_add(x, "@width", "8.8mm");

        // 33 is 8.8mm, 50 is 13mm
        xml_add(x, "@x", tho(THO * w / 2 - THO * (33 + 50)));
        xml_add(x, "@y", tho(-THO * (h / 2 + bleed + (bleed ? 2 : 0))));
      }

      if (indices && (!noleft || right)) {  // corners
        int value_left_x =
            -THO * w / 2 + THO * margin - THO * vh / 5;
        int value_left_y =
            -THO * h / 2 + THO * (topmargin > corner ? topmargin : corner);
        int value_right_x =
            THO * w / 2 - THO * margin + THO * vh / 5 - THO * vh;
        int value_right_y =
            -THO * h / 2 + THO * (topmargin > corner ? topmargin : corner);

        // Value
        if (!noleft && (suit != 'J' || force_joker_value)) {  // Corner
          xml_t x = addsymbolvalue(g, suit, value);
          xml_add(x, "@height", tho(THO * vh));
          if (!nowidthonuse) {
            xml_add(x, "@width", tho(THO * vh));
          }
          xml_add(x, "@x", tho(value_left_x));
          xml_add(x, "@y", tho(value_left_y));
        }

        if (right && (suit != 'J' || force_joker_value)) {
          xml_t x = addsymbolvalue(g, suit, value);
          xml_add(x, "@height", tho(THO * vh));
          if (!nowidthonuse) {
            xml_add(x, "@width", tho(THO * vh));
          }
          xml_add(x, "@x", tho(value_right_x));
          xml_add(x, "@y", tho(value_right_y));
        }

        // Same width as value
        int ph2 = THO * vh * 65 / 100 * THO / pipwidth('C', THO);
        int pip_left_x =
          -THO * w / 2 + THO * margin - THO * vh / 5 + THO * vh / 2;
        int pip_left_y =
            -THO * h / 2 + THO * vh
            + THO * (topmargin > corner ? topmargin : corner)
            + THO * pipmargin + ph2 / 2;
        int pip_right_x =
            THO * w / 2 - THO * margin + THO * vh / 5 + THO * vh / 2 - THO * vh;
        int pip_right_y =
            -THO * h / 2 + THO * vh
            + THO * (topmargin > corner ? topmargin : corner)
            + THO * pipmargin + ph2 / 2;

        if (suit == 'J' && !force_joker_value) {
          ph2 = THO * vh * 3 / 2;
          pip_left_x += THO * vh / 5;
          pip_left_y = value_left_y + ph2 / 3;
          pip_right_x -= THO * vh / 5;
          pip_right_y = value_right_y + ph2 / 3;
        }

        // pip
        if (!noleft) {
          if (ghost && colour[color_index] != black) {
            xml_t p2 = pip(pip_left_x, pip_left_y, ph2);

            xml_add(p2, "@stroke", dict_gets(color_map, black));
            xml_add(p2, "@stroke-width", "100");
            xml_add(p2, "@stroke-linejoin", "round");
            xml_add(p2, "@stroke-linecap", "round");
          }

          xml_t p2 = pip(pip_left_x, pip_left_y, ph2);

          if (ghost) {
            xml_add(p2, "@fill", dict_gets(color_map, colour[color_index]));
          }
        }

        if (right) {
          if (ghost && colour[color_index] != black) {
            xml_t p2 = pip(pip_right_x, pip_right_y, ph2);

            xml_add(p2, "@stroke", dict_gets(color_map, black));
            xml_add(p2, "@stroke-width", "100");
            xml_add(p2, "@stroke-linejoin", "round");
            xml_add(p2, "@stroke-linecap", "round");
          }

          xml_t p2 = pip(pip_right_x, pip_right_y, ph2);

          if (ghost) {
            xml_add(p2, "@fill", dict_gets(color_map, colour[color_index]));
          }
        }
      }

      if (indexonly) {
        return;
      }

      if (pips) {
        side2(-py);
      }

      // PIP on court card
      if (strchr("JQK", value) && layer) {
        // int x = bw / 2 - bw * 14 / 100;

        // Size and position that should fit
        int y = bh / 2 - bh * 10 / 100,
            sx = bw * (!pipn ? 32 : 35) / 100;

        if (bh < bw * 20 / 13) {
          sx = (unsigned long long) s
               * (unsigned long long) bh * 13ULL
               / (unsigned long long) bw / 20ULL;
        }

        sx -= THO * courtmargin;

        int x = bw / 2 - pipwidth(suit, sx / 2) - THO * courtmargin;

        // int p = (old ? bw * 2 / 7 : bw / 3);      // size
        int rightpip = extra_card ? extra_card->rightpip
                                  : ((value == 'Q' && suit != 'H')
                                      || (value == 'J' && suit != 'S'));

        if (modern) {
          rightpip = 0;
        }
        if (reverse) {
          rightpip = (!rightpip);
        }
        if (!rightpip) {
          x = -x;
        }

        x += THO * court_pip_offset_x;
        y -= THO * court_pip_offset_y;

        xml_t p = pip(x, -y, sx);
        if (ghost && !color_index % 2) {
          xml_add(p, "@fill", dict_gets(color_map, "black"));
        }
      }
    }

    // One side
    if (strchr("JQK", value) && !indexonly) {  // Court
      if (!layer) {
        xml_t x = addsymbolvalue(g, suit, value);
        xml_add(x, "@height", tho(bw));
        if (!nowidthonuse) {
          xml_add(x, "@width", tho(bw));
        }
        xml_add(x, "@x", tho(-bw / 2));
        xml_add(x, "@y", tho(-bw / 2));
      } else {
        court(1, ghost);
      }
    }

    if (value == 'A' && !indexonly) {
      if (suit == 'S' && !strcasecmp(ace, "Goodall") && layer) {
        xml_t ace = xml_element_add(g, "use");
        xml_add(ace, "@height", tho(bh));
        xml_add(ace, "@width", tho(bw));
        xml_add(ace, "@x", tho(-bw / 2));
        xml_add(ace, "@y", tho(-bh / 2));
        xml_addf(ace, "@xlink:href", "#%c%c%d", suit, value, 1);
      } else if ((!strcasecmp(ace, "Large") || !strcasecmp(ace, "Fancy"))
                 && (one || suit == 'S')) {
        if (!strcasecmp(ace, "Fancy") && suit == 'S') {
          xml_t x = pip(0, 0, bw);
          xml_add(x, "@stroke", dict_gets(color_map, colour[0]));
          xml_add(x, "@stroke-width", "100");
          xml_add(x, "@stroke-dasharray", "100,100");
          xml_add(x, "@stroke-linecap", "round");
          x = pip(0, 0, bw);
          xml_add(x, "@stroke", dict_gets(color_map, frontcolour ? : "white"));
          xml_add(x, "@stroke-width", "50");
          x = pip(0, 0, bw);
          xml_add(x, "@fill", dict_gets(color_map, colour[0]));
        } else {
          pip(0, 0, bw);        // Simple big Ace
        }

        if (suit == 'S' && qr) {  // QR on ace
          int S = 0;
          unsigned char *grid = qr_encode(
              strlen(qr), qr, 0, QR_ECL_L, 0, 0, &S, 0, 0, 0, 0);

          if (grid) {
            int x, y;
            Image *i;

            i = ImageNew(S, S, 2);
            i->Colour[0] = 0xFFFFFF;
            i->Colour[1] = 0;

            for (y = 0; y < S; y++) {
              for (x = 0; x < S; x++) {
                if (grid[(S - 1 - y) * S + x] & 1) {
                  ImagePixel(i, x, S - y - 1) = 1;
                }
              }
            }

            char *d;
            size_t len;
            FILE *path = open_memstream(&d, &len);

            ImageSVGPath(i, path, 1);

            fclose(path);
            ImageFree(i);
            free(grid);

            xml_t q = xml_element_add(root, "path");
            xml_addf(q, "@transform",
                "translate(0,-10)rotate(45)scale(%s)translate(%d,%d)",
                tho(bw * 3 / S / 10),
                -S / 2, -S / 2);
            xml_add(q, "@fill", dict_gets(color_map, frontcolour ? : "white"));
            xml_add(q, "@stroke", "none");
            xml_add(q, "@d", d);

            free(d);
          }
        }
      } else if (!symmetric) {
        // Normal (Plain)
        pip(0, 0, THO * ph);
      }

      // Ace of spades
      if ((one || suit == 'S')
          && (*ace1 || *ace2)
          && strcasecmp(ace, "Goodall")) {
        void addtext(xml_t e) {
          xml_t x;

          if (*ace1) {
            x = xml_add(e, "+text", ace1);
            xml_addf(x, "@font-size", "%d", fontsize);
            if (fontfamily) {
              xml_add(x, "@font-family", fontfamily);
            }
            if (fontweight) {
              xml_add(x, "@font-weight", fontweight);
            }
            xml_add(x, "@fill", dict_gets(color_map, black));
            xml_add(x, "@text-anchor", "middle");
            xml_add(x, "@y", tho(bh / 2 - THO * fontsize * 3 / 2));
          }

          if (*ace2) {
            x = xml_add(e, "+text", ace2);
            xml_addf(x, "@font-size", "%d", fontsize);
            if (fontfamily) {
              xml_add(x, "@font-family", fontfamily);
            }
            if (fontweight) {
              xml_add(x, "@font-weight", fontweight);
            }
            xml_add(x, "@fill", dict_gets(color_map, black));
            xml_add(x, "@text-anchor", "middle");
            xml_add(x, "@y", tho(bh / 2 - THO * fontsize / 2));
          }

          if (duplimate) {
            xml_t x = xml_addf(root, "+text",
                "Barcode patented by Jannersten, licence no. %s", duplimate);
            xml_addf(x, "@font-size", "6");
            xml_add(x, "@font-family", "Bariol");
            xml_add(x, "@text-anchor", "middle");
            xml_add(x, "@fill", dict_gets(color_map, black));
            xml_add(x, "@y", tho(bh / 2));
          }
        }

        if (suit == 'S') {
          addtext(root);
        }

        if (symmetric && !strcasecmp(ace, "Plain")) {
          xml_t g = xml_element_add(root, "g");
          xml_add(g, "@transform", "rotate(180)");
          addtext(g);
        }
      }
    }

    side(1, 1);

    if (!symmetric && !indexonly && suit != 'J') {
      if (suit == 'C' && value == '9' && !noflip) {
        pip(0, -THO * ph / 10, THO * ph);
      } else if (strchr("1359ED", value)) {
        pip(0, 0, THO * ph);
      }
      if (strchr("678", value)) {
        pip(-px, 0, THO * ph);
        pip(px, 0, THO * ph);
      }
      if (value == '7') {
        // Middle for 7
        pip(0, !pipn ? -py * 4 / 5 : -py / 2, THO * ph);
      }
    }

    if (noflip && !strchr("JQK", value)) {
      side2(py);
    }

    // Flip of side 1
    if (!noflip || strchr("JQK", value)
        || (!toponly && (!noleft || right))
        || (symmetric && noflip)) {
      g = xml_element_add(root, "g");
      xml_add(g, "@transform", "rotate(180)");

      side(!noflip, !toponly);

      if (noflip && symmetric && !strchr("JQK", value)) {
        side(1, 1);
        side2(py);
      }
    }

    // Box (overlay for court card artwork)
    if (!court_border_under && strchr("JQK", value) && !plain && !indexonly) {
      court_border();
    }
  }

  // Joker
  if (suit == 'J' && !indexonly) {
    if (plain) {
      xml_t x = xml_add(root, "+text", "Joker");
      xml_add(x, "@font-size", "80");
      if (fontfamily) {
        xml_add(x, "@font-family", fontfamily);
      }
      if (fontweight) {
        xml_add(x, "@font-weight", fontweight);
      }
      xml_add(x, "@text-anchor", "middle");
      xml_add(x, "@transform", "rotate(60)");
      xml_add(x, "@y", "20");
      if (value == '2') {
        xml_add(x, "@fill", dict_gets(color_map, red));
        if (ghost) {
          xml_add(x, "@stroke", dict_gets(color_map, black));
        }
      }
    } else {
      court(0, value == '2');
    }
  }

  if (suit == 'Z') {
    xml_add(root, "g@id", "artwork");
  }

  writecard(root, suit, value, extra_card);
}

int main(int argc, const char *argv[]) {
  {                             // POPT
    poptContext optCon;         // context for parsing command-line options
    const struct poptOption optionsTable[] = {
      { "dir", 'd', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &dir, 0, "Directory", "path" },
      { "ghost", 0, POPT_ARG_NONE, &ghost, 0, "Ghost" },
      { "red", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &red, 0, "Colour of red" },
      { "black", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &black, 0, "Colour of black" },
      { "green", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &green, 0, "Colour of green" },
      { "blue", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &blue, 0, "Colour of blue" },
      { "four-colour", 0, POPT_ARG_NONE, &fourcolour, 0, "4 colour deck" },
      { "plain", 0, POPT_ARG_NONE, &plain, 0, "Plain court cards" },
      { "box", 0, POPT_ARG_NONE, &box, 0, "Box on all cards" },
      { "pip", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &pipn, 0, "Pip style", "N" },
      { "value", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &valuen, 0, "Value style", "N" },
      { "no-flip", 0, POPT_ARG_NONE, &noflip, 0, "Old style - pips all the same way up" },
      { "symmetric", 0, POPT_ARG_NONE, &symmetric, 0, "Symmetric pips" },
      { "bleed", 0, POPT_ARG_INT, &bleed, 0, "Bleed area", "pixels" },
      { "dpi-bleed", 0, POPT_ARG_STRING, &dpi_bleed, 0, "DPI-based bleed area", "pixels@dpi" },
      { "corner", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &corner, 0, "Corner radius", "pixels" },
      { "margin", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &margin, 0, "Margin", "pixels" },
      { "margin-top", 0, POPT_ARG_INT, &topmargin, 0, "Margin at top", "pixels" },
      { "margin-back", 0, POPT_ARG_INT, &backmargin, 0, "Back margin", "pixels" },
      { "margin-pip", 0, POPT_ARG_INT, &pipmargin, 0, "Pip margin", "pixels" },
      { "index-width", 0, POPT_ARG_INT, &index_width, 0, "Stroke width of card index symbols", "pixels" },
      { "court-grow", 0, POPT_ARG_INT, &courtgrow, 0, "Extra width on court cards", "pixels" },
      { "court-stretch", 0, POPT_ARG_INT, &courtstretch, 0, "Extra height on court cards", "pixels" },
      { "court-border-width", 0, POPT_ARG_INT, &court_border_width, 0, "Width of border box for court cards", "pixels" },
      { "court-pip-offset", 0, POPT_ARG_INT, &court_pip_offset, 0, "Extra distance for pips from borders on court cards", "pixels" },
      { "court-pip-offset-x", 0, POPT_ARG_INT, &court_pip_offset_x, 0, "Extra distance for pips from borders on court cards", "pixels" },
      { "court-pip-offset-y", 0, POPT_ARG_INT, &court_pip_offset_y, 0, "Extra distance for pips from borders on court cards", "pixels" },
      { "no-left", 0, POPT_ARG_NONE, &noleft, 0, "No indices on left" },
      { "right", 0, POPT_ARG_NONE, &right, 0, "Indices on right" },
      { "top-only", 0, POPT_ARG_NONE, &toponly, 0, "Indices only on top of card" },
      { "index-only", 0, POPT_ARG_NONE, &indexonly, 0, "Indices only (i.e. no pips or court images)" },
      { "zero", 0, POPT_ARG_NONE, &zero, 0, "Include a zero" },
      { "one", 0, POPT_ARG_NONE, &one, 0, "Include a one" },
      { "eleven", 0, POPT_ARG_NONE, &eleven, 0, "Include an eleven" },
      { "twelve", 0, POPT_ARG_NONE, &twelve, 0, "Include a twelve" },
      { "thirteen", 0, POPT_ARG_NONE, &thirteen, 0, "Include a thirteen" },
      { "court-border-under", 0, POPT_ARG_NONE, &court_border_under, 0, "Position the border for court cards under the art instead of on top" },
      { "no-joker-pips", 0, POPT_ARG_NONE, &no_joker_pips, 0, "Disable pips on joker cards" },
      { "force-joker-value", 0, POPT_ARG_NONE, &force_joker_value, 0, "Force a value to be printed on joker cards (default: 1)" },
      { "extra", 0, POPT_ARG_STRING, &extra, 0, "Extra cards to include (comma-separated)" },
      { "extra-dir", 0, POPT_ARG_STRING, &extra_dir, 0, "SVG source directory for extra cards" },
      { "color-map", 0, POPT_ARG_STRING, &color_map_str, 0, "Mapping of layer colors to output colors" },
      { "ignis", 0, POPT_ARG_NONE, &ignis, 0, "Ignis Jokers" },
      { "modern", 0, POPT_ARG_NONE, &modern, 0, "Modern facing of court cards" },
      { "reverse", 0, POPT_ARG_NONE, &reverse, 0, "Reverse facing of court cards" },
      { "no-border", 0, POPT_ARG_NONE, &noborder, 0, "No border (e.g. for printing on cards)" },
      { "width", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &width, 0, "Width (inc any bleed)", "units/mm/in/etc" },
      { "height", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &height, 0, "Height (inc any bleed)", "units/mm/in/etc" },
      { "w", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &w, 0, "Width (not including bleed)", "pixels" },
      { "h", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &h, 0, "Height (not including bleed)", "pixels" },
      { "ph", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &ph, 0, "Pip height", "pixels" },
      { "vh", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &vh, 0, "Value height", "pixels" },
      { "font-family", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &fontfamily, 0, "Font family for text on ace", "font" },
      { "font-weight", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &fontweight, 0, "Font weight for text on ace", "NNN" },
      { "font-size", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &fontsize, 0, "Size of text on ace", "pixels" },
      { "prefix", 0, POPT_ARG_STRING, &prefix, 0, "Filename prefix", "text" },
      { "number", 0, POPT_ARG_INT, &number, 0, "Filenames using number", "start" },
      { "suffix", 0, POPT_ARG_STRING, &suffix, 0, "Filename suffix", "text" },
      { "duplimate", 0, POPT_ARG_STRING, &duplimate, 0, "Barcode", "licence no" },
      { "aspect", 0, POPT_ARG_NONE, &aspect, 0, "Fix aspect ratio of court cards" },
      { "no-width-on-use", 0, POPT_ARG_NONE, &nowidthonuse, 0, "No width attribute on use objects" },
      { "card", 0, POPT_ARG_STRING, &card, 0, "One card", "[value][suit]" },
      { "inline", 0, POPT_ARG_NONE, &writeinline, 0, "Write to stdout" },
      { "jokers", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &jokers, 0, "Jokers", "N" },
      { "blanks", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &blanks, 0, "Blank padding cards", "N" },
      { "backs", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &backs, 0, "Backs", "N" },
      { "back", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &back, 0, "Back pattern", "Diamond/Arrows/Marked/Goodall/AA/FireBrick/Plain" },
      { "back-colour", 0, POPT_ARG_STRING, &backcolour, 0, "Back background colour", "svg-colour" },
      { "back-image", 0, POPT_ARG_STRING, &backimage, 0, "Back overlay image", "svg-filename" },
      { "front-colour", 0, POPT_ARG_STRING, &frontcolour, 0, "Front background colour", "svg-colour" },
      { "ace", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &ace, 0, "Ace of Spades", "Fancy/Large/Plain/Goodall" },
      { "ace1", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &ace1, 0, "Line 1 on ace", "text" },
      { "ace2", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &ace2, 0, "Line 2 on ace", "text" },
      { "qr", 0, POPT_ARG_STRING, &qr, 0, "QR on ace", "text" },
      { "pattern", 0, POPT_ARG_INT, &pattern, 0, "Pattern size", "pixels" },
      { "double-back", 0, POPT_ARG_INT, &doubleback, 0, "Additional double back cards (implied --interleave)", "N" },
      { "interleave", 0, POPT_ARG_NONE, &interleave, 0, "Interleave backs (implies --number=1)" },
      { "grey", 0, POPT_ARG_NONE, &grey, 0, "Grey Clubs/Diamonds" },
      { "poker", 0, POPT_ARG_NONE, &poker, 0, "Poker pre-sets" },
      { "bridge", 0, POPT_ARG_NONE, &bridge, 0, "Bridge pre-sets" },
      { "print", 0, POPT_ARG_NONE, &print, 0, "Print pre-sets" },
      { "debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug" },
      POPT_AUTOHELP {}
    };
    optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
    // poptSetOtherOptionHelp (optCon, "");
    int c;

    if ((c = poptGetNextOpt(optCon)) < -1) {
      errx(1, "%s: %s\n",
          poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));
    }

    if (poptPeekArg(optCon) || (!dir && !writeinline)) {
      poptPrintUsage(optCon, stderr, 0);
      return -1;
    }

    poptFreeContext(optCon);
  }

  if (court_pip_offset) {
    if (court_pip_offset_x || court_pip_offset_y) {
      errx(1, "can't specify both --court-pip-offset and --court-pip-offset-[xy]");
    }

    court_pip_offset_x = court_pip_offset;
    court_pip_offset_y = court_pip_offset;
  }

  // Presets
  if (poker && bridge) {
    errx(1, "Make you mind up");
  }
  if (print && !poker && !bridge) {
    poker = 1;  // default print
  }
  if (poker) {
    width = (print ? "69mm" : "63mm");
    height = (print ? "94mm" : "88mm");
    ph = 64;
  }
  if (bridge) {
    width = (print ? "62mm" : "56mm");
    height = (print ? "93mm" : "87mm");
    ph = 62;
    w = 212;
    h = 329;
    fontsize = 14;
    corner = 19;
    courtgrow = 8;
  }
  if (poker || bridge) {
    margin = 14;
    pipmargin = 3;
    backmargin = 16;
    if (print) {
      noborder = 1;
      bleed = 12;
      if (!*prefix) {
        prefix = "print";
      }
      if (!*suffix) {
        suffix = "card";
      }
      interleave = 1;
    }
  }
  // Sanity checks
  if (pipn < 0 || pipn >= PIPS) {
    pipn = 1;
  }
  if (valuen < 0 || valuen >= VALUES) {
    valuen = 1;
  }
  if (noleft && !right) {
    vh = 0;
  }
  if (!topmargin) {
    topmargin = margin;
  }
  if (!strcasecmp(back, "Marked")) {
    interleave = 1;
  }
  if (interleave && !number) {
    number = 1;
  }
  if (pattern < 0) {  // Default
    if (!strcasecmp(back, "Illusion")) {
      pattern = 20;
    } else if (!strcasecmp(back, "Marked")) {
      pattern = 10;
    } else {
      pattern = 6;
    }
  }
  if (*ace1 == '$') {
    ace1 = getenv(ace1 + 1);
  }
  if (*ace2 == '$') {
    ace2 = getenv(ace2 + 1);
  }
  if (duplimate && *duplimate == '$') {
    duplimate = getenv(duplimate + 1);
  }
  if (duplimate && !*duplimate) {
    duplimate = NULL;
  }
  if (qr && *qr == '$') {
    qr = getenv(qr + 1);
  }
  if (qr && !*qr) {
    duplimate = NULL;
  }
  colour[0] = black;
  colour[1] = red;
  if (fourcolour) {
    colour[2] = green;
    colour[3] = blue;
  } else {
    colour[2] = black;
    colour[3] = red;
  }

  color_map = dict_new(64);
  dict_puts(color_map, "gold", "#FC4");
  dict_puts(color_map, "red", "red");
  dict_puts(color_map, "blue", "#44F");
  dict_puts(color_map, "green", "green");
  dict_puts(color_map, "purple", "purple");
  dict_puts(color_map, "orange", "orange");
  dict_puts(color_map, "white", "white");
  dict_puts(color_map, "black", "black");
  dict_puts(color_map, "darkblue", "#008");
  dict_puts(color_map, "firebrick", "#bd1220");
  dict_puts(color_map, "lightslateblue", "#88f");
  dict_puts(color_map, "darkivory", "#FFC");
  dict_puts(color_map, "stroke", "#44F");
  dict_puts(color_map, "thin", "#44F");

  if (color_map_str) {
    char *color_from = strtok_r(color_map_str, ",", &color_map_str);
    char *color_to = NULL;

    while (color_from) {
      if (!(color_to = strchr(color_from, '='))) {
        errx(1, "invalid color mapping");
      }

      char *modifier = color_to - 1;

      *color_to++ = '\0';

      dict_removes(color_map, color_from);

      if (*modifier != ':') {
        dict_puts(color_map, color_from, color_to);
      } else {
        char *color_ref = dict_gets(color_map, color_to);

        if (!color_ref) {
          errx(1, "invalid color reference '%s'", color_to);
        }

        *modifier = '\0';

        dict_puts(color_map, color_from, color_ref);
      }

      color_from = strtok_r(NULL, ",", &color_map_str);
    }
  }

  list *extras = NULL;

  if (extra) {
    char *token = strtok_r(extra, ",", &extra);
    char *split = NULL;

    extras = list_new();

    while (token) {
      char value = 0, suit = 0;

      if ((split = strchr(token, ':'))) {
        *split++ = '\0';
        if (*split) {
          value = *split++;
          if (*split) {
            suit = *split;
          }
        }
      }

      excard *extra_card = load_excard(token, suit, value);

      if ((split = strchr(split, ':'))) {
        while (*(++split)) {
          switch (*split) {
            case 'N':
              extra_card->flip = 0;
              break;
            case 'F':
              extra_card->flip = 1;
              break;
            case 'R':
              extra_card->rightpip = 1;
              break;
          }
        }
      }

      list_unshift(extras, extra_card);

      token = strtok_r(NULL, ",", &extra);
    }
  }

  // Let's make some cards
  srand(0);
  if (dir && chdir(dir)) {
    err(1, "Cannot change to %s", dir);
  }
  if (card && strlen(card) == 2) {
    if (card[1] == 'B') {
      makeback(card[0] - '1', 0, 0);
    } else {
      makecard(card[1], card[0], NULL);
    }
  } else {
    int s, v, d, decks = 1;

    if (interleave && !backs) {
      backs = 1;
    }
    if (interleave) {
      decks = backs;
    }

    void startcard(void) {
      if (writeinline) {
        printf("<div style='display:inline-block'>");
      }
    }

    void endcard(void) {
      if (writeinline) {
        printf("</div> ");
      }
    }

    for (d = 0; d < decks; d++) {
      void docardextra(char s, char v, excard *extra) {
        startcard();
        makecard(s, v, extra);
        if (interleave) {
          makeback(d, s, v);
        }
        endcard();
      }

      void docard(char s, char v) {
        docardextra(s, v, NULL);
      }

      for (v = 0; v < blanks && v < 9; v++) {
        docard('Z', '1' + v);
      }

      for (s = 0; suits[s]; s++) {
        if (zero) {
          docard(suits[s], '0');
        }
        if (!one && strcasecmp(ace, "None")) {
          docard(suits[s], 'A');
        }

        for (v = 0; values[v]; v++) {
          // *INDENT-OFF*
          switch (values[v]) {
            case 'A':
            case '0': continue;
            case '1': if (!one) continue; break;
            case 'E': if (!eleven) continue; break;
            case 'W': if (!twelve) continue; break;
            case 'D': if (!thirteen) continue; break;
          }
          // *INDENT-ON*

          docard(suits[s], values[v]);
        }

        if (one && strcasecmp(ace, "None")) {
          docard(suits[s], 'A');
        }
      }

      for (v = 0; v < jokers && v < 9; v++) {
        docard('J', '1' + v);
      }

      if (extras) {
        int do_extras(void *extra_card_ptr, size_t _unused_index) {
          excard *extra_card = (excard *) extra_card_ptr;
          docardextra(extra_card->suit, extra_card->value, extra_card);
          return 0;
        }

        list_foreach(extras, &do_extras);
      }

      if (interleave && doubleback) {
        if (doubleback == backs) {
          for (v = 0; v < doubleback; v++) {
            startcard();
            makeback(d, 0, 0);
            makeback(v, 0, 0);
            endcard();
          }
        } else {
          for (v = 0; v < doubleback; v++) {
            startcard();
            makeback(d, 0, 0);
            makeback(d, 0, 0);
            endcard();
          }
        }
      }
    }

    if (!interleave) {
      for (v = 0; v < backs + doubleback && v < 9; v++) {
        startcard();
        makeback(v, 0, 0);
        endcard();
      }
    }
  }

  if (extras) {
    void *item_ptr;

    while ((item_ptr = list_pop(extras))) {
      destroy_excard(item_ptr);
    }

    list_del(extras);
  }

  dict_del(color_map);

  return 0;
}

int pipwidth(char suit, int ph) {
  size_t pip_index = get_pip_index(suit);

  if (pip_index == -1) {
    return 0;
  }

  return pip_path[pipn][pip_index].width * ph / 1200;
}

int pipheight(char suit, int ph) {
  size_t pip_index = get_pip_index(suit);

  if (pip_index == -1) {
    return 0;
  }

  return pip_path[pipn][pip_index].height * ph / 1200;
}

// Data

// Fill, target 800 wide (-400 to 400) and 1000 high (-500 to 500)
struct pip_s pip_path[PIPS][5] = {
  // *INDENT-OFF*
  {  // Old
   {"M0 -500C350 -250 460 -100 460 100C460 300 260 340 210 340C110 340 55 285 100 300L130 500L-130 500L-100 300C-55 285 -110 340 -210 340C-260 340 -460 300 -460 100C-460 -100 -350 -250 0 -500Z", 920, 1000},
   {"M0 -300A230 230 0 0 1 460 -150C400 0 200 300 0 500C-200 300 -400 0 -460 -150A230 230 0 0 1 0 -300Z", 943, 967},
   {"M-100 500L100 500L100 340A260 260 0 1 0 200 -150A230 230 0 1 0 -200 -150A260 260 0 1 0 -100 340Z", 959, 994},
   {"M-400 0L0 -500L400 0L 0 500Z", 800, 1000 },
   {"M 0,-400.75731 A 400.75735,400.75735 0 0 0 -400.75731,0 400.75735,400.75735 0 0 0 0,400.75731 400.75735,400.75735 0 0 0 400.75731,0 400.75735,400.75735 0 0 0 0,-400.75731 Z m 0,75.142 A 325.61535,325.61535 0 0 1 325.61531,0 325.61535,325.61535 0 0 1 191.34793,263.45293 325.61535,325.61535 0 0 1 0,325.61531 325.61535,325.61535 0 0 1 -191.37921,263.43532 L -0.027377,130.26569 191.34793,263.45293 123.83364,40.263575 309.62612,-100.61199 76.521565,-105.36318 -0.027377,-325.61335 l -76.550911,220.25017 -233.102632,4.75119 185.79056,140.875565 -67.50452,223.160005 A 325.61535,325.61535 0 0 1 -325.61531,0 a 325.61535,325.61535 0 0 1 15.9344,-100.61199 325.61535,325.61535 0 0 1 309.653533,-225.00136 325.61535,325.61535 0 0 1 0.02651,-0.002 z", 800, 1000 }
  },
  {  // New
   {"M0 -500C100 -250 355 -100 355 185A150 150 0 0 1 55 185A10 10 0 0 0 35 185C35 385 85 400 130 500L-130 500C-85 400 -35 385 -35 185A10 10 0 0 0 -55 185A150 150 0 0 1 -355 185C-355 -100 -100 -250 0 -500Z", 710, 1000},
   {"M0 -300C0 -400 100 -500 200 -500C300 -500 400 -400 400 -250C400 0 0 400 0 500C0 400 -400 0 -400 -250C-400 -400 -300 -500 -200 -500C-100 -500 0 -400 -0 -300Z", 800, 1000},
   {"M30 150C35 385 85 400 130 500L-130 500C-85 400 -35 385 -30 150A10 10 0 0 0 -50 150A210 210 0 1 1 -124 -51A10 10 0 0 0 -110 -65A230 230 0 1 1 110 -65A10 10 0 0 0 124 -51A210 210 0 1 1 50 150A10 10 0 0 0 30 150Z", 933, 997},
   {"M-400 0C-350 0 0 -450 0 -500C0 -450 350 0 400 0C350 0 0 450 0 500C0 450 -350 0 -400 0Z", 800, 1000 },
   {"M 0,-400.75731 A 400.75735,400.75735 0 0 0 -400.75731,0 400.75735,400.75735 0 0 0 0,400.75731 400.75735,400.75735 0 0 0 400.75731,0 400.75735,400.75735 0 0 0 0,-400.75731 Z m 0,75.142 A 325.61535,325.61535 0 0 1 325.61531,0 325.61535,325.61535 0 0 1 191.34793,263.45293 325.61535,325.61535 0 0 1 0,325.61531 325.61535,325.61535 0 0 1 -191.37921,263.43532 L -0.027377,130.26569 191.34793,263.45293 123.83364,40.263575 309.62612,-100.61199 76.521565,-105.36318 -0.027377,-325.61335 l -76.550911,220.25017 -233.102632,4.75119 185.79056,140.875565 -67.50452,223.160005 A 325.61535,325.61535 0 0 1 -325.61531,0 a 325.61535,325.61535 0 0 1 15.9344,-100.61199 325.61535,325.61535 0 0 1 309.653533,-225.00136 325.61535,325.61535 0 0 1 0.02651,-0.002 z", 800, 1000 }
  }
  // *INDENT-ON*
};

// Stroke 80, target -285 to 285, and -460 to 460 making around 610 by 1000
struct value_s value_path[VALUES][18] = {
  // *INDENT-OFF*
  {  // Standard
   /* 0 */ {80, "M-175 0L-175 -285A175 175 0 0 1 175 -285L175 285A175 175 0 0 1 -175 285Z"},
   /* 1 */ {80, "M0 430L0 -430"},
   /* 2 */ {80, "M-225 -225C-245 -265 -200 -460 0 -460C 200 -460 225 -325 225 -225C225 -25 -225 160 -225 460L225 460L225 300"},
   /* 3 */ {80, "M-250 -320L-250 -460L200 -460L-110 -80C-100 -90 -50 -120 0 -120C200 -120 250 0 250 150C250 350 170 460 -30 460C-230 460 -260 300 -260 300"},
   /* 4 */ {80, "M50 460L250 460M150 460L150 -460L-300 175L-300 200L270 200"},
   /* 5 */ {80, "M170 -460L-175 -460L-210 -115C-210 -115 -200 -200 0 -200C100 -200 255 -80 255 120C255 320 180 460 -20 460C-220 460 -255 285 -255 285"},
   /* 6 */ {80, "M-250 100A250 250 0 0 1 250 100L250 210A250 250 0 0 1 -250 210L-250 -210A250 250 0 0 1 0 -460C150 -460 180 -400 200 -375"},
   /* 7 */ {80, "M-265 -320L-265 -460L265 -460C135 -200 -90 100 -90 460"},
   /* 8 */ {80, "M-1 -50A205 205 0 1 1 1 -50L-1 -50A255 255 0 1 0 1 -50Z"},
   /* 9 */ {80, "M250 -100A250 250 0 0 1 -250 -100L-250 -210A250 250 0 0 1 250 -210L250 210A250 250 0 0 1 0 460C-150 460 -180 400 -200 375"},
   /* T */ {80, "M-260 460L-260 -460M-50 0L-50 -310A150 150 0 0 1 250 -310L250 310A150 150 0 0 1 -50 310Z"},
   /* E */ {80, "M-180 460L-180 -460M180 460L180 -460"},
   /* J */ {80, "M50 -460L250 -460M150 -460L150 250A100 100 0 0 1 -250 250L-250 220"},
   /* Q */ {80, "M-260 100C40 100 -40 460 260 460M-175 0L-175 -285A175 175 0 0 1 175 -285L175 285A175 175 0 0 1 -175 285Z"},
   /* K */ {80, "M-285 -460L-85 -460M-185 -460L-185 460M-285 460L-85 460M85 -460L285 -460M185 -440L-170 155M85 460L285 460M185 440L-10 -70"},
   /* A */ {80, "M-270 460L-110 460M-200 450L0 -460L200 450M110 460L270 460M-120 130L120 130"},
   // The second digit for numbers 12+ should be clamped from -50 to 250
   /* W */ {80, "M-260 460V -460M-93 -225C-110 -265 -71 -460 100 -460C 271 -460 293 -325 293 -225C293 -25 -93 160 -93 460L293 460L293 300"},
   /* D */ {80, "M-260 460V -460M-114 -320L-114 -460L271 -460L6 -80C14 -90 57 -120 100 -120C271 -120 314 0 314 150C314 350 246 460 74 460C-97 460 -123 300 -123 300"},
  },
  {
   /* 0 */ {160, "M-160 -420v720M0 -420v60M160 -420v720M0 240v200"},
   /* 1 */ {160, "M-160 240v60M0 240v170M160 240v60M-160 -420v60"},
   /* 2 */ {160, "M-160 240v60M0 240v170M160 240v60M-160 -420v60M0 -420v60"},
   /* 3 */ {160, "M-160 240v60M0 240v170M160 240v60M-160 -420v60M0 -420v60M160 -420v60"},
   /* 4 */ {160, "M-160 240v60M0 240v170M160 240v60M-160 -420v280M0 -420v60M160 -420v60"},
   /* 5 */ {160, "M-160 240v60M0 240v170M160 240v60M-160 -420v280M0 -420v280M160 -420v60"},
   /* 6 */ {160, "M-160 240v60M0 240v170M160 240v60M-160 -420v280M0 -420v280M160 -420v280"},
   /* 7 */ {160, "M-160 -420v720M0 240v170M160 240v60M0 -420v280M160 -420v280"},
   /* 8 */ {160, "M-160 -420v720M0 -420v830M160 240v60M160 -420v280"},
   /* 9 */ {160, "M-160 -420v720M0 -420v830M160 -420v720"},
   /* T */ {160, "M-160 -420v60M160 -420v60M-160 20v280M0 20v60M160 20v280"},
   /* E */ {160, "M-160 -420v720M0 20v60M160 -420v60M160 20v280"},
   /* J */ {160, "M-160 -420v280M0 -420v60M160 -420v280M-160 240v60M0 240v60M160 240v60"},
   /* Q */ {160, "M-160 -200v60M0 -420v500M160 -200v60M-160 240v60M160 240v60"},
   /* K */ {160, "M-160 -420v280M0 -420v720M-160 240v60"},
   /* A */ {160, "M-160 -420v60M0 -420v60M160 -420v720M-160 20v60M0 20v60"},
   /* W */ {160, "M-160 -420v720M0 20v60M160 -420v60M160 20v280"},
   /* D */ {160, "M-160 -420v720M0 20v60M160 -420v60M160 20v280"},
  }
  // *INDENT-ON*
};

unsigned int duplimate_code[] = {
  0x2A4,  // SA 010 1010 0100
  0x164,  // S2 001 0110 0100
  0x264,  // S3 010 0110 0100
  0x12C,  // S4 001 0010 1100
  0x134,  // S5 001 0011 0100
  0x334,  // S6 011 0011 0100
  0x194,  // S7 001 1001 0100
  0x294,  // S8 010 1001 0100
  0x154,  // S9 001 0101 0100
  0x32C,  // ST 011 0010 1100
  0x1AC,  // SJ 001 1010 1100
  0x2D4,  // SQ 010 1101 0100
  0x254,  // SK 010 0101 0100
  0x124,  // HA 001 0010 0100
  0x0DA,  // H2 000 1101 1010
  0x11A,  // H3 001 0001 1010
  0x14A,  // H4 001 0100 1010
  0x24A,  // H5 010 0100 1010
  0x0CA,  // H6 000 1100 1010
  0x2CA,  // H7 010 1100 1010
  0x31A,  // H8 011 0001 1010
  0x19A,  // H9 001 1001 1010
  0x15A,  // HT 001 0101 1010
  0x25A,  // HJ 010 0101 1010
  0x29A,  // HQ 010 1001 1010
  0x2DA,  // HK 010 1101 1010
  0x1A4,  // CA 001 1010 0100
  0x24C,  // C2 010 0100 1100
  0x14C,  // C3 001 0100 1100
  0x2AC,  // C4 010 1010 1100
  0x0CC,  // C5 000 1100 1100
  0x326,  // C6 011 0010 0110
  0x1A6,  // C7 001 1010 0110
  0x166,  // C8 001 0110 0110
  0x266,  // C9 010 0110 0110
  0x332,  // CT 011 0011 0010
  0x2C6,  // CJ 010 1100 0110
  0x2CC,  // CQ 010 1100 1100
  0x1B2,  // CK 001 1010 0010
  0x324,  // DA 011 0010 0100
  0x126,  // D2 001 0010 0110
  0x2A6,  // D3 010 1010 0110
  0x132,  // D4 001 0011 0010
  0x2B2,  // D5 010 1011 0010
  0x152,  // D6 001 0101 0010
  0x252,  // D7 010 0101 0010
  0x0D2,  // D8 000 1101 0010
  0x2D2,  // D9 010 1101 0010
  0x12A,  // DT 001 0010 1010
  0x32A,  // DJ 011 0010 1010
  0x1AA,  // DQ 001 1010 1010
  0x2AA,  // DK 010 1010 1010
};
