void svg_compact_path(char *d) {
  char *i = d,
       *o = d;

  while (*i) {
    if ((isalpha(*i) && isspace(i[1]))
        || (isdigit(*i) && isspace(i[1]) && isalpha(i[2]))) {
      *o++ = *i++;
      i++;
    } else {
      *o++ = *i++;
    }
  }

  *o = 0;
}

char *svg_merge_group_paths(xml_t g, const char *layer, const char *card) {
  char *d;
  size_t len;
  FILE *path = open_memstream(&d, &len);
  xml_t p = NULL;

  while ((p = xml_element_next_by_name(g, p, "path"))) {
    if (xml_get(p, "@transform")) {
      errx(1, "Transform found on path in %s in %s", layer, card);
    }

    d = xml_get(p, "@d");

    if (d) {
      if (*d == 'm' && d[1] == ' ') {
        // Special case for inkscape relative start
        char *q = strchr(d + 2, ' ');

        if (q && !isalpha(q[1])) {
          *q = 'l';
        }

        *d = 'M';
      }

      svg_compact_path(d);
      fprintf(path, "%s", d);
    }
  }

  fclose(path);

  return d;
}

