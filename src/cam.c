#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gphoto2/gphoto2.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#include "list.h"
#include "main.h"
#include "cam.h"

#define RESIZE_TOOL "/usr/bin/vipsthumbnail"

LIST(image_list);

int getPort(GPContext * context, char ** cam, char ** port) {
	GPPortInfoList *list = NULL;
	CameraList *camlist;
	CameraAbilitiesList *abilitiesList;
	const char *name = NULL;
	const char *value = NULL;
	int count = 0;
	int result = 0;
	int i = 0;

	// Get List
	if (gp_port_info_list_new (&list) < GP_OK) {
		printf("Unable to list\n");
		gp_context_unref( context );
		return -2;
	}

	// Load list
	result = gp_port_info_list_load (list);
	if (result < 0) {
		printf("Unable to list\n");
		gp_port_info_list_free (list);
		gp_context_unref( context );
		return -2;
	}

	// Initialize return values
	*cam = NULL;
	*port = NULL;

	// Create new camlist
	gp_list_new (&camlist);

	// Create new abilities list
	gp_abilities_list_new(&abilitiesList);

	// Load abilities
	gp_abilities_list_load(abilitiesList, context);

	// List cameras
	gp_abilities_list_detect(abilitiesList, list, camlist, context);

	count = gp_list_count(camlist);

	printf("Found %d cams:\n", count);

	for(i=0; i<count; i++) {
		gp_list_get_name(camlist, i, &name);
		gp_list_get_value(camlist, i, &value);

		printf("\t%s on port %s\n", name, value);

		if( i == 0 ) {
			*cam = (char *) malloc(strlen(name) + 1);
			memcpy(*cam, name, strlen(name) + 1);

			*port = (char *) malloc(strlen(value) + 1);
			memcpy(*port, value, strlen(value) + 1);
		}
	}

	// Free up some memory
	gp_abilities_list_free(abilitiesList);
	gp_list_unref(camlist);
	gp_port_info_list_free (list);

	// Return an error if no camera has been found
	if( count == 0 ) {
		return -1;
	}

	// Return 0 on success
	return 0;
}

Camera * getCamera(char *port, GPContext *context) {
	Camera *pCamera = NULL;
	int result;
	GPPortInfoList *list = NULL;
	GPPortInfo info;

	if (gp_port_info_list_new (&list) < GP_OK) {
		printf("Unable to list\n");
		return NULL;
	}

	result = gp_port_info_list_load (list);
	if (result < 0) {
		printf("Unable to load list\n");
		gp_port_info_list_free (list);
		return NULL;
	}

	int portIndex = gp_port_info_list_lookup_path (list, port);
	if (portIndex < 0) {
		printf("Unable to lookup\n");
		gp_port_info_list_free (list);
		return NULL;
	}

	result = gp_port_info_list_get_info (list, portIndex, &info);
	if (result < 0) {
		printf("Unable to get info\n");
		gp_port_info_list_free (list);
		return NULL;
	}

	result = gp_camera_new(&pCamera);
	if (result < 0) {
		printf("Unable to new camera\n");
		gp_port_info_list_free (list);
		return NULL;
	}

	gp_camera_set_port_info(pCamera, info);

	gp_port_info_list_free (list);

	return pCamera;
}

int downloadCameraPhoto(Camera * camera, GPContext * context, const char * target, const char * source_dir, const char * source_file) {
	struct stat buffer;
	int i;
	int result;
	CameraFile *captureFile;

	i = stat(target, &buffer);
	if( i == 0 ) {
		return -1;
	}

	printf("Downloading %s/%s to %s\n", source_dir, source_file, target);

	result = gp_file_new(&captureFile);
	if (result != GP_OK) {
		printf("Unable to allocate CameraFile\n");
		return -2;
	}

	result = gp_camera_file_get(camera, source_dir, source_file, GP_FILE_TYPE_NORMAL, captureFile, context);
	if (result != GP_OK) {
		printf("Unable to get file: %d\n", result);
		gp_file_free(captureFile);
		return -2;
	}

	result = gp_file_save(captureFile, target);
	if (result != GP_OK) {
		printf("Unable to download image\n");
		gp_file_free(captureFile);
		return -2;
	}

	gp_file_free(captureFile);

	return 0;
}

int resizeCameraPhoto(char * source, char * target, int size) {
    char *args[9];
    int i = 0;
    char scaling[16];
    int pid = 0;
    int status;

    snprintf(scaling, 16, "%d", size);

    args[i++] = RESIZE_TOOL;
    args[i++] = source;
    args[i++] = "-s";
    args[i++] = scaling;
    args[i++] = "-o";
    args[i++] = target;
    args[i++] = NULL;

    pid = fork();
    if( pid == 0 ) {
    	execvp(RESIZE_TOOL, args);
    }
    waitpid(pid, &status, 0);

    return 0;
}

struct image * getImages() {
	while( mutex == 1 ) {
		usleep(50);
	}

	return list_head(image_list);
}

int getCameraPhotosInFolder(Camera * camera, GPContext * context, const char * target_full, const char * target_thumbs, const char * source, int size) {
	int result = 0;
	int count = 0;
	int folderCount = 0;
	int childCount = 0;
	int i = 0;
	int h = 0;
	const char *name = NULL;
	char name_lower[128];
	char path[128];
	char path_thumb[128];
	CameraList * filesList;
	CameraList * foldersList;
	struct image * itr = NULL;
	int found = 0;

	gp_list_new(&filesList);
	result = gp_camera_folder_list_files(camera, source, filesList, context);

	if (result == GP_OK) {
		count = gp_list_count(filesList);
		for (i = 0; i < count; i++) {
			result = gp_list_get_name(filesList, i, &name);
			found = 0;

			// convert filename to lowercase
			memset(name_lower, 0, 128);
			for (h=0; h<strlen(name); h++) {
				name_lower[h] = tolower(name[h]);
			}	

			// only download jpegs, skip the rest
			char * ptr = NULL;
			int offset = strlen(name) - 3;
			ptr = &name_lower[offset];
			if( strncmp(ptr, "jpg", 3) != 0 ) {
				continue;
			}

			// Check list if the image is in there
			for(itr = list_head(image_list); itr != NULL; itr = list_item_next(itr)) {
				if( strcasecmp(itr->name, name) == 0 ) {
					found = 1;
					break;
				}
			}

			if( found == 0 ) {
				mutex = 1;

				itr = malloc(sizeof(struct image));
				if( itr == NULL ) {
					break;
				}

				itr->name = malloc(strlen(name) + 1);
				if( itr->name == NULL ) {
					break;
				}

				memcpy(itr->name, name, strlen(name) + 1);
				list_push(image_list, itr);
				itr = NULL;

				mutex = 0;
			}

			//snprintf(path, 128, "%s/%s", target_full, name)
;			snprintf(path, 128, "%s/img.jpg", target_full);
			result = downloadCameraPhoto(camera, context, path, source, name);

			if( result == 0 ) {
				snprintf(path_thumb, 128, "%s/%s", target_thumbs, name);
				// resizeCameraPhoto(path, path_thumb, size);
			}

		}
	} else {
		printf("no list because %d\n", result);
	}
	gp_list_unref(filesList);

	gp_list_new(&foldersList);
	result = gp_camera_folder_list_folders(camera, source, foldersList, context);
	if (result == 0) {
		folderCount = gp_list_count(foldersList);

		for (i = 0; i < folderCount; i++) {
			result = gp_list_get_name(foldersList, i, &name);
			if (strcmp(source, "/") == 0) {
				snprintf(path, 128, "%s%s", source, name);
			} else {
				snprintf(path, 128, "%s/%s", source, name);
			}

			if (result == 0) {
				childCount = getCameraPhotosInFolder(camera, context, target_full, target_thumbs, path, size);
				if (childCount > 0) {
					count += childCount;
				}
			}

		}
	}

	gp_list_unref(foldersList);

	return count;
}

int camLoop(char * target_full, char * target_thumbs, int size) {
	char * camera_string = NULL;
	char * port_string = NULL;
	GPContext * context = NULL;
	int ret = 0;
	Camera * camera = NULL;
	int initial = 1;
	mutex = 0;

	// Get GPhoto context
	context = gp_context_new();
	if( context == NULL ) {
		printf("Unable to generate context\n");
		gp_context_unref( context );
		exit(EXIT_FAILURE);
	}

	// Find port of camera
	ret = getPort(context, &camera_string, &port_string);
	if( ret < 0 ) {
		printf("No Camera found\n");
		gp_context_unref( context );
		exit(EXIT_FAILURE);
	}
	printf("Using '%s' on port '%s'\n", camera_string, port_string);

	// Initialize image list
	list_init(image_list);

	while( RUNNING ) {
		// Get GPhoto Camera handle
		camera = getCamera(port_string, context);
		if( camera == NULL ) {
			printf("Unable to obtain camera handle\n");
			gp_context_unref( context );
			free(camera_string);
			free(port_string);
			exit(EXIT_FAILURE);
		}

		// Download all images from camera
		getCameraPhotosInFolder(camera, context, target_full, target_thumbs, "/", size);

		// Close connection to camera
		gp_camera_exit(camera, context);

		// Free some memory
		gp_camera_free(camera);

		if( initial ) {
			printf("Initial scan done\n");
			initial = 0;
		}

		// Wait 50ms for the next turn
		usleep(50 * 1000);

		printf(".");
		fflush(stdout);
	}

	// Deallocate resources
	gp_context_unref( context );
	free(camera_string);
	free(port_string);

	return 0;
}
