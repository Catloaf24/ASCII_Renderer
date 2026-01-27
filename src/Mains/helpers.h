#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>

#include <Windows.h>
#include <debugapi.h>

struct Pixel {
	unsigned int r;
	unsigned int g;
	unsigned int b;
	unsigned int a;
};

struct Image {
	unsigned char* data;
	int width;
	int height;
	int channels;
};

int scaleInt(int a, int aMin, int aMax, int min, int max) {
	long double percentage = ((float)a - (float)aMin) / ((float)aMin - (float)aMax);
	return (percentage * (min - max) + min);
}

Image loadImage(char* path) {

	int width, height, channels;
	unsigned char* img = stbi_load(path, &width, &height, &channels, 0);
	if (img == NULL) {
		std::cerr << "Error loading image" << std::endl;
	}

	//delete[] actPath;

	return Image{ img, width, height, channels };
}

void saveImage(const char* path, Image img) {
	char* actPath = new char[strlen(RESOURCE_DIR) + strlen(path) + 1];
	strcpy(actPath, RESOURCE_DIR);
	strcat(actPath, path);
	stbi_write_png(actPath, img.width, img.height, img.channels, img.data, img.width * img.channels);
}

Pixel getRGBA(unsigned char* data, int xPos, int yPos, int w, int h, int c) {
	unsigned int bytePerPixel = c;
	unsigned char* pixelOffset = data + (xPos + w * yPos) * bytePerPixel;
	unsigned char r = pixelOffset[0];
	unsigned char g = pixelOffset[1];
	unsigned char b = pixelOffset[2];
	//unsigned char a = c >= 4 ? pixelOffset[3] : 0xff;
	unsigned char a;
	if (c >= 4) {
		a = pixelOffset[3];
	}
	else {
		a = 0xff;
	}

	return Pixel{ static_cast<unsigned int>(r), static_cast<unsigned int>(g), static_cast<unsigned int>(b), static_cast<unsigned int>(a) };
}

void writeRGBA(unsigned char* data, int xPos, int yPos, int w, int h, int c, Pixel p) {
	const size_t bytePerPixel = static_cast<size_t>(c);
	const size_t W = static_cast<size_t>(w);
	const size_t X = static_cast<size_t>(xPos);
	const size_t Y = static_cast<size_t>(yPos);

	const size_t offset = (X + W * Y) * bytePerPixel;
	const size_t total = W * static_cast<size_t>(h) * bytePerPixel;
	if (offset + bytePerPixel > total) {
		std::cerr << "writeRGBA: computed write range out of bounds (offset " << offset << " size " << bytePerPixel << " total " << total << ")\n";
		return;
	}

	unsigned char* pixelOffset = data + offset;

	pixelOffset[0] = static_cast<unsigned char>(p.r);
	if (bytePerPixel > 1) pixelOffset[1] = static_cast<unsigned char>(p.g);
	if (bytePerPixel > 2) pixelOffset[2] = static_cast<unsigned char>(p.b);
	if (c >= 4) {
		if (bytePerPixel > 3) pixelOffset[3] = static_cast<unsigned char>(p.a);
	}
}

Image img2grey(Image img) {
	unsigned char greyVal;

	size_t img_size = img.width * img.height * img.channels;
	int grey_channels = img.channels >= 4 ? 2 : 1;
	size_t grey_image_size = img.width * img.height * grey_channels;
	unsigned char* greyImg = new unsigned char[grey_image_size];

	if (greyImg == NULL) {
		std::cerr << "Error creating new Image" << std::endl;
	}

	for (int y = 0; y < img.height; y++) {
		for (int x = 0; x < img.width; x++) {
			Pixel p = getRGBA(img.data, x, y, img.width, img.height, img.channels);
			greyVal = (0.299 * p.r + 0.587 * p.g + 0.114 * p.b);
			greyImg[x * 2 + y * img.width * 2] = static_cast<unsigned char>(greyVal);
			if(grey_channels >= 2) greyImg[(x * 2 + y * img.width * 2) + 1] = static_cast<unsigned char>(p.a);
		}
	}

	return Image{ greyImg, img.width, img.height, grey_channels };
}

std::vector<Pixel> sampleRadius(unsigned char* data, int xPos, int yPos, int w, int h, int c, const int radius) {
	const int size = (2 * radius + 1) * (2 * radius + 1);
	std::vector<Pixel> pixels(size);

	int it = 0;
	for (int i = radius + 1; i >= 0; i--) {
		for(int j = radius + 1; j >= 0; j--) {
			int sampleX = xPos + i;
			int sampleY = yPos + j;
			if (sampleX < 0) sampleX = 0;
			if (sampleX >= w) sampleX = w - 1;
			if (sampleY < 0) sampleY = 0;
			if (sampleY >= h) sampleY = h - 1;
			pixels[it] = getRGBA(data, sampleX, sampleY, w, h, c);
			it++;
		}
	}

	return pixels;
}

void writeRadius(unsigned char* data, int xPos, int yPos, int w, int h, int c, int radius, Pixel p) {
	for (int i = radius + 1; i >= 0; i--) {
		for (int j = radius + 1; j >= 0; j--) {
			int sampleX = xPos + i;
			int sampleY = yPos + j;
			if (sampleX < 0) sampleX = 0;
			if (sampleX >= w) sampleX = w - 1;
			if (sampleY < 0) sampleY = 0;
			if (sampleY >= h) sampleY = h - 1;
			writeRGBA(data, sampleX, sampleY, w, h, c, p);
		}
	}
}

Image blurImg(unsigned char* data, int w, int h, int c, int r) {
	Image pixImg = {};
	pixImg.width = w;
	pixImg.height = h;
	pixImg.channels = c;
	pixImg.data = new unsigned char[static_cast<size_t>(pixImg.width) * pixImg.height * pixImg.channels];
	memset(pixImg.data, 0, static_cast<size_t>(pixImg.width)* pixImg.height* pixImg.channels);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			std::vector<Pixel> sampledPixels = sampleRadius(data, j, i, w, h, c, r);

			unsigned int ar = 0, ag = 0, ab = 0, aa = 0;
			for (Pixel ap : sampledPixels) {
				ar += ap.r;
				ag += ap.g;
				ab += ap.b;
				aa += ap.a;
			}
			ar /= sampledPixels.size();
			ag /= sampledPixels.size();
			ab /= sampledPixels.size();
			aa /= sampledPixels.size();

			/*
			ar = scaleInt(ar, 0, 80, 0, 255);
			ag = scaleInt(ar, 0, 80, 0, 255);
			ab = scaleInt(ar, 0, 80, 0, 255);
			*/

			writeRadius(pixImg.data, j, i, w, h, c, r, Pixel{ ar, ag, ab, aa });
		}
	}
	return pixImg;
}

Image pixelateImg(unsigned char* data, int w, int h, int c, int r) {
	Image pixImg = {};
	pixImg.width = w;
	pixImg.height = h;
	pixImg.channels = c;
	pixImg.data = new unsigned char[static_cast<size_t>(pixImg.width) * pixImg.height * pixImg.channels];
	memset(pixImg.data, 0, static_cast<size_t>(pixImg.width) * pixImg.height * pixImg.channels);

	for (int i = 0; i < h; i += r) {
		for (int j = 0; j < w; j += r) {
			std::vector<Pixel> sampledPixels = sampleRadius(data, j, i, w, h, c, r);

			unsigned int ar = 0, ag = 0, ab = 0, aa = 0;
			for (Pixel ap : sampledPixels) {
				ar += ap.r;
				ag += ap.g;
				ab += ap.b;
				aa += ap.a;
			}
			ar /= sampledPixels.size();
			ag /= sampledPixels.size();
			ab /= sampledPixels.size();
			aa /= sampledPixels.size();

			/*
			ar = scaleInt(ar, 0, 100, 0, 255);
			ag = scaleInt(ar, 0, 100, 0, 255);
			ab = scaleInt(ar, 0, 100, 0, 255);
			*/

			writeRadius(pixImg.data, j, i, w, h, c, r, Pixel{ ar, ag, ab, aa });
		}
	}
	return pixImg;
}

std::vector<char> fillASCII(unsigned char* data, int w, int h, int c, int r) {
	const int size = (w / r) * (h / r);
	std::vector<char> output(size + 1);

	char chars[4] = { '.', '=', '#', '%' };

	int i = 0;
	for (int y = r; y <= h; y += r) {
		for (int x = r; x <= w; x += r) {
			Pixel p = getRGBA(data, x, y, w, h, c);
			if (p.r > 70) output[i] = chars[3]; //vr light values
			else if (p.r > 50) output[i] = chars[2]; //light values
			else if (p.r > 25) output[i] = chars[1]; //medium values
			else if (p.r > 12) output[i] = chars[0]; //dark values
			else output[i] = chars[0];
			i++;
		}
	}
	return output;
}

void printASCII(std::vector<char> output, int w, int h, int r) {
	const int size = (w / r) * (h / r);
	std::cout << output[0];
	for (int i = 1; i < size; i++) {
		if (i % (w / r) == 0) {
			std::cout << "\n";
			std::cout << output[i];
		}
		else {
			std::cout << output[i];
		}
	}
}