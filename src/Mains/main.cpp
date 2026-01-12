#include <iostream>

#include "helpers.h"


int main() {
	Image img = loadImage("/catloaf.png");

	Image greyImg = img2grey(img);

	saveImage("/greyCatloaf.png", greyImg);

	greyImg = loadImage("/greyCatloaf.png");
	
	Image pixImg = {};
	pixImg.width = greyImg.width;
	pixImg.height = greyImg.height;
	pixImg.channels = greyImg.channels;
	pixImg.data = new unsigned char[static_cast<size_t>(pixImg.width) * pixImg.height * pixImg.channels];
	memset(pixImg.data, 0, static_cast<size_t>(pixImg.width) * pixImg.height * pixImg.channels);

	const int radius = 10;
	pixImg = pixelateImg(greyImg.data, pixImg.width, pixImg.height, pixImg.channels, radius);

	saveImage("/pixCatloaf.png", pixImg);

	const int size = (pixImg.width / radius) * (pixImg.height / radius);
	std::vector<char> output(size + 1);

	output = fillASCII(pixImg.data, pixImg.width, pixImg.height, pixImg.channels, radius);

	printASCII(output, pixImg.width, pixImg.height, radius);

	stbi_image_free(img.data);
	stbi_image_free(greyImg.data);
	delete[] pixImg.data;
	return 0;
}