#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

void
dump_buf(ptr, l, ni)
     unsigned char      *ptr;
     int                 l;
     long                ni;
{
  int                 n;
  unsigned char       s[81];
  unsigned char       t[81];

  *s = '\0';
  for (n = 0; n < l; n++) {
    if ((n % 16) != 0) {
      if (((n % 2) == 0) && (n != 0))
        strcat((char *) s, " ");
      if (((n % 8) == 0) && (n != 0))
        strcat((char *) s, " ");
    } else {
      *t = '\0';
      memset(t, 0, sizeof (t));
      sprintf((char *) s, "%.6X : ", n + ni);
    }
    sprintf((char *) s, "%s%.2X", s, *(ptr + n));
    if ((*(ptr + n) >= 0x20) && (*(ptr + n) <= 0x7F))
      t[n % 16] = *(ptr + n);
    else
      t[n % 16] = '.';

    if ((((n + 1) % 16) == 0) && (n != 0)) {
      t[16] = '\0';
      printf("%-49s - %s\n", s, t);
    }
  }
  if (((n % 16) != 0) && (n != 0)) {
    printf("%-49s - %s\n", s, t);
  }
}

#define buf_size        0x1000

int
main(argc, argv)
     int                 argc;
     char              **argv;
{
  int                 fin;
  char               *buf;
  int                 len;
  long                ni = 0;

  if ((buf = (char *) malloc(buf_size)) == NULL) {
    printf("Erreur pendant allocation de buffer\n");
    return 1;
  }

  while (--argc > 0) {
#if 0
    printf("i = %3d, %s\n", argc, argv[argc]);
#endif
    fin = open(argv[argc], O_RDONLY);
    if (fin != -1) {
      while ((len = read(fin, buf, buf_size)) > 0) {
        dump_buf(buf, len, ni);
        ni += len;
      }
      close(fin);
    } else
      printf("   erreur pendant ouverture de fichier\n");
  }
  free(buf);

  return 0;
}
