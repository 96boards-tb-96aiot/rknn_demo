#ifndef __SSD_H__
#define __SSD_H__

typedef struct _SSDRECT
{
    /**
     * The x coordinate of the upper-left corner of the rectangle.
     */
    int left;
    /**
     * The y coordinate of the upper-left corner of the rectangle.
     */
    int top;
    /**
     * The x coordinate of the lower-right corner of the rectangle.
     */
    int right;
    /**
     * The y coordinate of the lower-right corner of the rectangle.
     */
    int bottom;
} SSDRECT;

struct ssd_object {
  char name[256];
  int spoof_status;
  SSDRECT select;
};

struct ssd_group {
    int count;
    struct ssd_object objects[100];
};

int ssd_run(void *flag);
int ssd_post(void *flag);
int ssd_init(char *name);
int ssd_deinit();
float ssd_get_fps();
int ssd_play_sound(void *flag);
struct ssd_group* ssd_get_ssd_group();

// enroll = 0 for recognise mode
// enroll = 1 for enroll mode
extern int enroll;
extern int spoof_check;
extern char first_name[128];
extern char last_name[128];
extern char jpegPath[256];
extern int jpeg;

#endif
