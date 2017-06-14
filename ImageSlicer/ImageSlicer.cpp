// ImageSlicer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>

using namespace cv;

Point p1;
Point p2;
bool newRect;
bool drawing;

void printUsage() {
	std::cout << "Usage: ImageSlicer <image path> <export filename prefix> <export file type>" << std::endl;
}

void printControls() {
	std::cout << "Controls:" << std::endl << "Click and drag to select a region to slice" << std::endl << "c - confirm your selection"
		<< std::endl << "d - delete your selection" << std::endl << "e - export all selections" << std::endl;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata) {
	if (event == EVENT_LBUTTONDOWN) {
#ifdef _DEBUG
		std::cout << "Left button down" << std::endl;
#endif
		p1 = Point(x, y);
		drawing = true;
	}
	else if (event == EVENT_LBUTTONUP) {
#ifdef _DEBUG
		std::cout << "Left button up" << std::endl;
#endif
		p2 = Point(x, y);
		newRect = true;
		drawing = false;
	}
}

int main(int argc, char* argv[])
{
	if (argc != 4) {
		printUsage();
		return -1;
	}

	std::string imagePath(argv[1]);

	std::string exportPrefix(argv[2]);
	std::string exportType(argv[3]);

#ifdef _DEBUG
	std::cout << "Image Path: " << imagePath << std::endl;
	std::cout << "Export Prefix: " << exportPrefix << std::endl;
	std::cout << "Export Type: " << exportType << std::endl;
#endif


	Mat baseImage = imread(imagePath);
	Mat tempImage = baseImage.clone();

	if (baseImage.empty()) {
		std::cout << "Error loading image: " << imagePath << std::endl;
		printUsage();
		return -2;
	}

	namedWindow("Image Slicer");

	setMouseCallback("Image Slicer", CallBackFunc, NULL);

	imshow("Image Slicer", baseImage);

	std::vector<Rect> rects;
	bool run = true;
	bool confirmed = true;

	printControls();

	while (run) {
		if (newRect) {
			rectangle(tempImage, p1, p2, Scalar(255, 0, 0));
#ifdef _DEBUG
			std::cout << "Drew rectangle" << std::endl;
#endif
			newRect = false;
			confirmed = false;
			imshow("Image Slicer", tempImage);
		}
		else if (!confirmed && drawing) {
			tempImage = baseImage.clone();
			imshow("Image Slicer", baseImage);
		}
		int key = waitKey(1);
		if (key == -1) {
			continue;
		}
		else if (!confirmed) {
			if (key == 'c') {//confirm rect
#ifdef _DEBUG
				std::cout << "Confirmed rectangle" << std::endl;
#endif
				confirmed = true;
				rectangle(baseImage, p1, p2, Scalar(0, 255, 0));
				tempImage = baseImage.clone();
				imshow("Image Slicer", baseImage);
				//Add 1 to tl x and y to remove the green part
				Rect r = Rect(p1, p2);
				r.x += 1;
				r.y += 1;
				r.width -= 1;
				r.height -= 1;
				rects.push_back(r);
			}
			else if (key == 'd') {//delete rect
#ifdef _DEBUG
				std::cout << "Deleting rectangle" << std::endl;
#endif
				tempImage = baseImage.clone();
				imshow("Image Slicer", baseImage);
				confirmed = true;
			}
		}
		else if (key == 'e') {//export all selections
			for(int i = 0; i < rects.size(); i++)
			{
				imwrite(exportPrefix + "_" + std::to_string(i) + "." + exportType, baseImage(rects[i]));
			}
			std::cout << "Finished exporting all images" << std::endl;
		}
	}

    return 0;
}

