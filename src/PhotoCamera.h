/*
 * PhotoCamera.h
 *
 *  Created on: 13.07.2014
 *      Author: morgenro
 */

#include <set>
#include <string>

#include <gphoto2/gphoto2.h>

#ifndef PHOTOCAMERA_H_
#define PHOTOCAMERA_H_

class CameraException : public std::exception
{
public:
	CameraException() throw()
	{};

	CameraException(const exception&) throw()
	{};

	virtual ~CameraException() throw()
	{};

	/**
	 * Get the explaining reason as string value.
	 * @return The reason as string value.
	 */
	virtual const char* what() const throw()
	{
			return _what.c_str();
	}

	/**
	 * constructor with attached string value as reason.
	 * @param what The detailed reason for this exception.
	 */
	CameraException(std::string what) throw()
	{
			_what = what;
	};

protected:
	std::string _what;
};

class Photo {
public:
	Photo();
	Photo(const std::string &path, const std::string &filename);
	virtual ~Photo();

	bool operator<(const Photo &other) const;
	bool operator!=(const Photo &other) const;

	std::string path;
	std::string filename;
};

class PhotoCamera {
public:
	PhotoCamera();
	virtual ~PhotoCamera();

	void list(const std::string &path, std::set<Photo> &photos);
	void download(const Photo &photo, const std::string &target);

private:
	int initialize();

	Camera* open();
	void close(Camera *camera);

	GPContext *_context;

	std::string _camera;
	std::string _port;
};

#endif /* PHOTOCAMERA_H_ */
