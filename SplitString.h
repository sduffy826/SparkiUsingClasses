#ifndef __SPLITSTRING_H
#define __SPLITSTRING_H

// This method takes in a char* and counts how many parms there are and what the length of the max
// parameter is
void parmCountAndLength(const char* str_data, size_t& num_params, size_t& length) {
  size_t t_max    = 0;
  const char* src = str_data;
  const char* p   = nullptr;
  char  ch;
  while ( (ch = *src++) ) {
    t_max++;  
    if ( ',' == ch ) {
      p = src;
      num_params++;
      if ( t_max > length ) {
        length = t_max;
        t_max  = 0;
      }
    }
  }
  
  const size_t count_remaining = strlen(p);
  if ( count_remaining ) {
    num_params++;
    if ( count_remaining > length ) {
      length = count_remaining;
    }
  }
}

// This routine extracts data from src and populates the array at dst
const char* extractToken(char* dst, const char* src, char const sep = ',') {
  while ( *src ) {
    if ( sep == *src ) {
      *dst = '\0';
      return ++src;
    }
    *dst++ = *src++;
  }
  *dst = '\0';
  return NULL;
}
#endif
