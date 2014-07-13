/*
 * PhotoCamera.cpp
 *
 *  Created on: 13.07.2014
 *      Author: morgenro
 */

#include "PhotoCamera.h"
#include <iostream>
#include <exception>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

Photo::Photo()
{
}

Photo::Photo(const std::string &p, const std::string &f)
 : path(p), filename(f)
{
}

Photo::~Photo()
{
}

bool Photo::operator!=(const Photo &other) const
{
	if (other.filename != filename) return true;
	return (other.path != path);
}

bool Photo::operator<(const Photo &other) const
{
	if (path < other.path) return true;
	if (path != other.path) return false;

	return filename < other.filename;
}

PhotoCamera::PhotoCamera()
 : _context(NULL)
{
	int ret = 0;

	// Get GPhoto context
	_context = gp_context_new();
	if( _context == NULL ) throw CameraException("Unable to generate context");

	// Find port of camera
	ret = initialize();
	if( ret < 0 ) {
		gp_context_unref( _context );
		throw CameraException("No Camera found");
	}

	std::cout << "Using '" << _camera << "' on port '" << _port << "'" << std::endl;
}

PhotoCamera::~PhotoCamera()
{
	// Deallocate resources
	gp_context_unref( _context );
}

Camera* PhotoCamera::open()
{
	Camera *pCamera = NULL;
	int result;
	GPPortInfoList *list = NULL;
	GPPortInfo info;

	if (gp_port_info_list_new (&list) < GP_OK) {
		throw CameraException("Unable to list");
	}

	result = gp_port_info_list_load (list);
	if (result < 0) {
		gp_port_info_list_free (list);
		throw CameraException("Unable to load list");
	}

	int portIndex = gp_port_info_list_lookup_path (list, _port.c_str());
	if (portIndex < 0) {
		gp_port_info_list_free (list);
		throw CameraException("Unable to lookup");
	}

	result = gp_port_info_list_get_info (list, portIndex, &info);
	if (result < 0) {
		gp_port_info_list_free (list);
		throw CameraException("Unable to get info");
	}

	result = gp_camera_new(&pCamera);
	if (result < 0) {
		gp_port_info_list_free (list);
		throw CameraException("Unable to new camera");
	}

	gp_camera_set_port_info(pCamera, info);
	gp_port_info_list_free (list);

	return pCamera;
}

void PhotoCamera::close(Camera *pCamera)
{
	// Close connection to camera
	gp_camera_exit(pCamera, _context);

	// Free some memory
	gp_camera_free(pCamera);
}

void PhotoCamera::list(const std::string &path, std::set<Photo> &photos)
{
	int result;

	Camera *pCamera = open();

	/***
	 * Begin of code for listing
	 */

	CameraList * filesList;
	CameraList * foldersList;
	int count = 0;
	int folderCount = 0;
	int i = 0;
	const char *name = NULL;

	gp_list_new(&filesList);
	result = gp_camera_folder_list_files(pCamera, path.c_str(), filesList, _context);

	if (result == GP_OK) {
		count = gp_list_count(filesList);
		for (i = 0; i < count; i++) {
			result = gp_list_get_name(filesList, i, &name);
			Photo p(path, std::string(name));
			photos.insert(p);
		}
	} else {
		std::cerr << "no list because " << result << std::endl;
	}
	gp_list_unref(filesList);

	// query all folders recursively
	std::string subpath;

	gp_list_new(&foldersList);
	result = gp_camera_folder_list_folders(pCamera, path.c_str(), foldersList, _context);
	if (result == 0) {
		folderCount = gp_list_count(foldersList);

		for (i = 0; i < folderCount; i++) {
			result = gp_list_get_name(foldersList, i, &name);
			if (path == "/") {
				subpath = path + std::string(name);
			} else {
				subpath = path + "/" + std::string(name);
			}

			if (result == 0) {
				list(subpath, photos);
			}
		}
	}

	gp_list_unref(foldersList);

	/**
	 * End of code for listing
	 */

	close(pCamera);
}

int PhotoCamera::initialize() {
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
		return -2;
	}

	// Load list
	result = gp_port_info_list_load (list);
	if (result < 0) {
		printf("Unable to list\n");
		gp_port_info_list_free (list);
		return -2;
	}

	// Initialize return values
	_camera = "";
	_port = "";

	// Create new camlist
	gp_list_new (&camlist);

	// Create new abilities list
	gp_abilities_list_new(&abilitiesList);

	// Load abilities
	gp_abilities_list_load(abilitiesList, _context);

	// List cameras
	gp_abilities_list_detect(abilitiesList, list, camlist, _context);

	count = gp_list_count(camlist);

	printf("Found %d cams:\n", count);

	for(i=0; i<count; i++) {
		gp_list_get_name(camlist, i, &name);
		gp_list_get_value(camlist, i, &value);

		printf("\t%s on port %s\n", name, value);

		if( i == 0 ) {
			_camera = std::string(name);
			_port = std::string(value);
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

void PhotoCamera::download(const Photo &photo, const std::string &target)
{
	struct stat buffer;
	int i;
	int result;
	CameraFile *captureFile;

	Camera *pCamera = open();

	i = stat(target.c_str(), &buffer);
	if( i == 0 ) {
		throw CameraException("file already exists");
	}

	printf("Downloading %s/%s to %s\n", photo.path.c_str(), photo.filename.c_str(), target.c_str());

	result = gp_file_new(&captureFile);
	if (result != GP_OK) {
		throw CameraException("Unable to allocate CameraFile");
	}

	result = gp_camera_file_get(pCamera, photo.path.c_str(), photo.filename.c_str(), GP_FILE_TYPE_NORMAL, captureFile, _context);
	if (result != GP_OK) {
		gp_file_free(captureFile);
		throw CameraException("Unable to get file");
	}

	result = gp_file_save(captureFile, target.c_str());
	if (result != GP_OK) {
		gp_file_free(captureFile);
		throw CameraException("Unable to download image");
	}

	gp_file_free(captureFile);

	close(pCamera);
}
