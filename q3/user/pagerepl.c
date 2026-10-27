#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXREF 64
#define MAXFRAMES 10

int reference[MAXREF];
int nref = 0;

/* Required reference string from Assignment 9. */
int default_ref[] = {
  7, 0, 1, 2, 0, 3, 0, 4, 2, 3,
  0, 3, 2, 1, 2, 0, 1, 7, 0, 1
};

/* Display the current contents of the frames. */
void
printframes(int frames[], int nframes)
{
  int i;

  printf("[ ");
  for (i = 0; i < nframes; i++) {
    if (frames[i] == -1)
      printf("- ");
    else
      printf("%d ", frames[i]);
  }
  printf("]");
}

/*
 * FIFO: replace the page that entered memory first.
 */
int
fifo(int refs[], int count, int nframes, int verbose)
{
  int frames[MAXFRAMES];
  int next = 0;
  int faults = 0;
  int i, j, hit;

  for (i = 0; i < nframes; i++)
    frames[i] = -1;

  if (verbose)
    printf("\n--- FIFO: %d frames ---\n", nframes);

  for (i = 0; i < count; i++) {
    hit = 0;

    for (j = 0; j < nframes; j++) {
      if (frames[j] == refs[i]) {
        hit = 1;
        break;
      }
    }

    if (!hit) {
      frames[next] = refs[i];
      next = (next + 1) % nframes;
      faults++;
    }

    if (verbose) {
      printf("Reference %d: ", refs[i]);
      printframes(frames, nframes);
      if (hit)
        printf(" HIT\n");
      else
        printf(" FAULT\n");
    }
  }

  if (verbose)
    printf("FIFO page faults = %d, hits = %d\n",
           faults, count - faults);

  return faults;
}

/*
 * LRU: replace the page that has not been used
 * for the longest time.
 */
int
lru(int refs[], int count, int nframes, int verbose)
{
  int frames[MAXFRAMES];
  int lastused[MAXFRAMES];
  int faults = 0;
  int i, j, victim, hit;

  for (i = 0; i < nframes; i++) {
    frames[i] = -1;
    lastused[i] = -1;
  }

  if (verbose)
    printf("\n--- LRU: %d frames ---\n", nframes);

  for (i = 0; i < count; i++) {
    hit = 0;
    victim = -1;

    /* Check whether the page is already in a frame. */
    for (j = 0; j < nframes; j++) {
      if (frames[j] == refs[i]) {
        hit = 1;
        lastused[j] = i;
        break;
      }
    }

    if (!hit) {
      faults++;

      /* Prefer an empty frame if one is available. */
      for (j = 0; j < nframes; j++) {
        if (frames[j] == -1) {
          victim = j;
          break;
        }
      }

      /* Otherwise, select the least recently used page. */
      if (victim == -1) {
        victim = 0;

        for (j = 1; j < nframes; j++) {
          if (lastused[j] < lastused[victim])
            victim = j;
        }
      }

      frames[victim] = refs[i];
      lastused[victim] = i;
    }

    if (verbose) {
      printf("Reference %d: ", refs[i]);
      printframes(frames, nframes);
      if (hit)
        printf(" HIT\n");
      else
        printf(" FAULT\n");
    }
  }

  if (verbose)
    printf("LRU page faults = %d, hits = %d\n",
           faults, count - faults);

  return faults;
}

/* Convert a command-line argument to a nonnegative integer. */
int
parseint(char *s, int *valid)
{
  int value = 0;
  int i = 0;

  if (s[0] == '\0') {
    *valid = 0;
    return 0;
  }

  while (s[i] != '\0') {
    if (s[i] < '0' || s[i] > '9') {
      *valid = 0;
      return 0;
    }

    value = value * 10 + (s[i] - '0');
    i++;
  }

  return value;
}

int
main(int argc, char *argv[])
{
  int i, valid;
  int frames;
  int fifo_faults, lru_faults;
  int fifo_hits, lru_hits;

  /*
   * Optional input format:
   * pagerepl <number_of_frames> <reference1> <reference2> ...
   */
  if (argc > 1) {
    if (argc < 3 || argc - 2 > MAXREF) {
      printf("Usage: pagerepl frames ref1 ref2 ...\n");
      exit(1);
    }

    valid = 1;
    frames = parseint(argv[1], &valid);

    if (!valid || frames < 1 || frames > MAXFRAMES) {
      printf("Frames must be between 1 and %d.\n", MAXFRAMES);
      exit(1);
    }

    nref = argc - 2;

    for (i = 0; i < nref; i++) {
      valid = 1;
      reference[i] = parseint(argv[i + 2], &valid);

      if (!valid) {
        printf("Invalid page reference: %s\n", argv[i + 2]);
        exit(1);
      }
    }

    printf("Reference string contains %d pages.\n", nref);

    fifo_faults = fifo(reference, nref, frames, 1);
    lru_faults = lru(reference, nref, frames, 1);

    printf("\n--- Summary for %d frames ---\n", frames);
    printf("FIFO faults: %d, hit ratio: %d%%\n",
           fifo_faults,
           ((nref - fifo_faults) * 100) / nref);
    printf("LRU faults:  %d, hit ratio: %d%%\n",
           lru_faults,
           ((nref - lru_faults) * 100) / nref);

    exit(0);
  }

  /*
   * Default assignment test: use the specified reference string.
   * Show every reference for three frames, then compare 3, 4, 5.
   */
  nref = sizeof(default_ref) / sizeof(default_ref[0]);

  for (i = 0; i < nref; i++)
    reference[i] = default_ref[i];

  printf("Assignment 9 - Question 3\n");
  printf("Reference string: ");

  for (i = 0; i < nref; i++)
    printf("%d ", reference[i]);

  printf("\nNumber of references: %d\n", nref);

  fifo(reference, nref, 3, 1);
  lru(reference, nref, 3, 1);

  printf("\n\n--- Comparison for 3, 4 and 5 frames ---\n");
  printf("Algorithm Frames Faults Hits HitRatio(%%)\n");

  for (frames = 3; frames <= 5; frames++) {
    fifo_faults = fifo(reference, nref, frames, 0);
    fifo_hits = nref - fifo_faults;

    printf("FIFO      %d       %d      %d    %d%%\n",
           frames, fifo_faults, fifo_hits,
           (fifo_hits * 100) / nref);

    lru_faults = lru(reference, nref, frames, 0);
    lru_hits = nref - lru_faults;

    printf("LRU       %d       %d      %d    %d%%\n",
           frames, lru_faults, lru_hits,
           (lru_hits * 100) / nref);
  }

  exit(0);
}

