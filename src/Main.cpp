#include "PhotoCamera.h"
#include <set>
#include <iostream>

#define DOCS_LOCATION_IMAGES	"/root/ramdisk/images"

bool _running = true;

typedef std::set<Photo> photoset;

int main()
{
	try {
		PhotoCamera cam;
		photoset photos;
		const std::string root_path("/");
		Photo last;

		std::string target(DOCS_LOCATION_IMAGES);

		while ( _running )
		{
			cam.list(root_path, photos);

			// skip if there are no files
			if (photos.size() == 0) continue;

			photoset::const_reverse_iterator it = photos.rbegin();
			const Photo &latest = (*it);

			if (last != latest) {
				// download file!
				std::cout << "download file: " << latest.path << "/" << latest.filename << std::endl;

				cam.download(latest, target + "/" + latest.filename);

				last = latest;
			}
		}
	} catch (const CameraException &e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}
