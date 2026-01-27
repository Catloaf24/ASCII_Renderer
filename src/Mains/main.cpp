#include <iostream>

#include "helpers.h"


int main(int argc, char *argv[]) {
	char* actPath;
	if (IsDebuggerPresent()) {
		const char* path = "/awesomeface.png";
		actPath = new char[strlen(RESOURCE_DIR) + strlen(path) + 1];
		strcpy(actPath, RESOURCE_DIR);
		strcat(actPath, path);
	}
	else {
		actPath = argv[1];
	}

	Image originalImg = loadImage(actPath);

	Image greyImg = img2grey(originalImg);
	
	Image pixImg = {};
	pixImg.width = greyImg.width;
	pixImg.height = greyImg.height;
	pixImg.channels = greyImg.channels;
	pixImg.data = new unsigned char[static_cast<size_t>(pixImg.width) * pixImg.height * pixImg.channels];
	memset(pixImg.data, 0, static_cast<size_t>(pixImg.width) * pixImg.height * pixImg.channels);

	const int radius = 10;
	pixImg = pixelateImg(greyImg.data, pixImg.width, pixImg.height, pixImg.channels, radius);

	const int size = (pixImg.width / radius) * (pixImg.height / radius);
	std::vector<char> output(size + 1);

	output = fillASCII(pixImg.data, pixImg.width, pixImg.height, pixImg.channels, radius);

	printASCII(output, pixImg.width, pixImg.height, radius);

	stbi_image_free(originalImg.data);
	stbi_image_free(greyImg.data);
	//stbi_image_free(pixImg.data);
	delete[] pixImg.data;
	return 0;
}