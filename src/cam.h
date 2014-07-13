#ifndef __CAM_H__
#define __CAM_H__

struct image {
	struct image * next;
	char * name;
};

int mutex;

struct image * getImages();
int camLoop(char * target_full, char * target_thumbs, int size);

#endif /* __CAM_H__ */
