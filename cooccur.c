#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/mman.h>
#include <math.h>

#define true 1
#define false 0
#define NIL -1

struct Letter {
  char letter;
  int Next[256];
  int count;
  int cocount;
  int generation;
  int cogeneration;
};

struct Word {
  int pos;
  int len;
  int generation;
};

struct Letter *spide;
char *text;
int Root[256];
int p_spide = 0;

int get_p_spide(char letter) {
  int o = p_spide;
  int i;
  spide[p_spide].letter = letter;
  for (i = 0; i < 256; i++) spide[p_spide].Next[i] = NIL;
  spide[p_spide].cogeneration =
    spide[p_spide].generation = -1;
  spide[p_spide].count = 0;
  spide[p_spide++].cocount = 0;
  return o;
}


void putPair(struct Word *w1, struct Word *w2) {
  int i;
  int p;

  p = Root[text[w1->pos]]; 
  if (p == NIL) {
    p = Root[text[w1->pos]] = get_p_spide(text[w1->pos]);
  }
  for (i = 0; i < w1->len; i++) {
    if (i == w1->len - 1) {
      if (spide[p].generation < w1->generation) {
	spide[p].count++;
	spide[p].generation = w1->generation;
      }
    } else {
      if (spide[p].Next[text[w1->pos + i + 1]] == NIL) {
	spide[p].Next[text[w1->pos + i + 1]] = get_p_spide(text[w1->pos + i  + 1]);
      }
      p = spide[p].Next[text[w1->pos + i + 1]];
    }
  }

  if (spide[p].Next[text[w2->pos]] == NIL) {
    p = spide[p].Next[text[w2->pos]] = get_p_spide(text[w2->pos]);
  } else {
    p = spide[p].Next[text[w2->pos]];
  }
  
  for (i = 0; i < w2->len; i++) {
    if (i == w2->len - 1) {
      if (spide[p].cogeneration < w2->generation) {
	spide[p].cocount++;
	spide[p].cogeneration = w2->generation;
      }
    } else {
      if (spide[p].Next[text[w2->pos + i + 1]] == NIL) {
	spide[p].Next[text[w2->pos + i + 1]] = get_p_spide(text[w2->pos + i  + 1]);
      }
      p = spide[p].Next[text[w2->pos + i + 1]];
    }
  }

}


double checkPair(char *w1, char *w2) {
  double cocount, count;
  int i, p = Root[*w1];

  if (p == NIL) {
    return 0.0;
  }
  for (; *w1 != '\0'; w1++) {
    if (w1[1] == '\0') {
      count = (double)spide[p].count;
      break;
    }
    p = spide[p].Next[w1[1]];
    if (p == NIL) return 0.0;
  }

  p = spide[p].Next[*w2];
  if (p == NIL) return 0.0;
  
  for (; *w2 != '\0'; w2++) {
    if (w2[1] == '\0') {
      cocount = (double)spide[p].cocount;
      break;
    }
    p = spide[p].Next[w2[1]];
    if (p == NIL) return 0.0;
  }
  printf("Count:%.0lf  cocount:%.0lf\n", count, cocount);
  return cocount / count;
}

int main(int argc, char **argv) {

    int K;
    struct stat st;
    int fd;
    int i, k, generation;
    int word_begin = true;
    char word1[2048000];
    char word2[2048000];
    struct Word word[5] = {
      {-1, -1, -1},
      {-1, -1, -1},
      {-1, -1, -1},
      {-1, -1, -1},
      {-1, -1, -1}
    };
    
    if (argc != 3) {
	fprintf(stderr, "Usage: %s <filename> K\n", argv[0]);
	exit (0);
    }

    for (i = 0; i < 255; i++) Root[i] = NIL;

    stat(argv[1], &st);
    sscanf(argv[2], "%d\n", &K);

    spide = (struct Letter*)calloc(2 * K * st.st_size, sizeof(struct Letter));
    if (spide == NULL) {
	fprintf(stderr, "No memory for internal structure\n");
	exit(4);
    }
    fd = open(argv[1], 0);
    if (fd == -1) {
	fprintf(stderr, "Can not open %s file\n", argv[1]);
	exit(2);
    }
    
    text = (char*)mmap(NULL, (size_t)st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, (off_t)0 );
    if (text == MAP_FAILED) {
      fprintf(stderr, "Can not mmap input file %s into memory\n", argv[1]);
	exit(1);
    }

    p_spide = 0;
    word_begin = true;
    generation = 0;
    for(i = 0; i < st.st_size; i++) {
	if (isalpha(text[i])) {
	  text[i] = tolower(text[i]);
	  if (word_begin) {
	    word[4] = word[3];
	    word[3] = word[2];
	    word[2] = word[1];
	    word[1] = word[0];
	    word[0].pos = i; word[0].len = 1; word[0].generation = ++generation;
	  } else {
	    word[0].len++;
	  }
	  word_begin = false;
	} else {
	  if (word[0].pos != NIL)
	    for(k = 1; k <= K; k++) {
	      if (word[k].pos == NIL) continue;
	      putPair(&word[0], &word[k]);
	      putPair(&word[k], &word[0]);
	    }
	  word_begin = true;
	}
    }
    if (!word_begin) {
      if (word[0].pos != NIL)
	for(k = 1; k <= K; k++) {
	  if (word[k].pos == NIL) continue;
	  putPair(&word[0], &word[k]);
	  putPair(&word[k], &word[0]);
	}
    }

    if (-1 == munmap(text, st.st_size)) {
      fprintf(stderr, "Can not munmap input file %s\n", argv[1]);
      exit(5);
    }

    if (0 != close(fd)) {
      fprintf(stderr, "Can not close input file %s\n", argv[1]);
      exit(6);
    }
    
    while(!feof(stdin)) {
      if (2 == fscanf(stdin, "%s %s", word1, word2)) {
	printf("\nChecking pair %s %s\n", word1, word2);
  	if (Root[*word1] == NIL || Root[*word2] == NIL) {
	  printf("Relative frequency: 0.00\n");
	} else {
	  printf("Relative frequency: %.2f\n", round(100.0 * checkPair(word1, word2)) / 100.0);
	}
      }
    }

    return 0;
}
