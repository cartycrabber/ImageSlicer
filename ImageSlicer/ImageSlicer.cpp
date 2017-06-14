// ImageSlicer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>

#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>

using namespace cv;

Point p1;
Rect* unconfirmedRect;
Point mouse;
bool drawing;

const Scalar UNCONFIRMED_COLOR = Scalar(255, 0, 0);
const Scalar CONFIRMED_COLOR = Scalar(0, 255, 0);
const Scalar GUIDE_COLOR = Scalar(0, 0, 255);

void printUsage() {
	std::cout << "Usage: ImageSlicer <image path> <export filename prefix> <export file type>" << std::endl;
}

void printControls() {
	std::cout << "Controls:" << std::endl << "Click and drag to select a region to slice" << std::endl << "c - confirm your selection"
		<< std::endl << "d - delete your selection" << std::endl << "e - export all selections" << std::endl
		<< "g - toggle guide lines" << std::endl << "r - reset image" << std::endl << "q - quit" << std::endl;
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
		unconfirmedRect = new Rect(p1, Point(x, y));
		drawing = false;
	}
	else if (event == EVENT_MOUSEMOVE) {
		mouse = Point(x, y);
		if (drawing) {
			unconfirmedRect = new Rect(p1, mouse);
		}
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
	bool guides = false;

	printControls();

	while (run) {
		/***==-==-== Render Code ==-==-==***/
		//Reset frame to base image
		tempImage = baseImage.clone();
		
		//Render guides if necessary
		if (guides) {
#ifdef _DEBUG
			std::cout << "Drawing Guides" << std::endl;
#endif
			line(tempImage, Point(mouse.x, 0), Point(mouse.x, tempImage.cols), GUIDE_COLOR);
			line(tempImage, Point(0, mouse.y), Point(tempImage.rows, mouse.y), GUIDE_COLOR);
		}

		//Render an unconfirmed rectangle if necessary
		if (unconfirmedRect != nullptr) {
#ifdef _DEBUG
			std::cout << "Drawing unconfirmed rectangle" << std::endl;
#endif
			rectangle(tempImage, *unconfirmedRect, UNCONFIRMED_COLOR);
		}

		imshow("Image Slicer", tempImage);

		/***==-==-== Input Code ==-==-==***/
		//Get input
		int key = waitKey(1);
		//Handle Input
		if (key == -1) {//No input
			continue;
		}
		else if ((key == 'c') && !drawing && (unconfirmedRect != nullptr)) {//confirm rect
#ifdef _DEBUG
			std::cout << "Confirming rectangle" << std::endl;
#endif
			rectangle(baseImage, *unconfirmedRect, CONFIRMED_COLOR);
			//trim the green borders
			unconfirmedRect->x += 1;
			unconfirmedRect->y += 1;
			unconfirmedRect->width -= 2;
			unconfirmedRect->height -= 2;
			rects.push_back(*unconfirmedRect);

			unconfirmedRect = nullptr;
		}
		else if (key == 'd' && !drawing && (unconfirmedRect != nullptr)) {//delete rect
#ifdef _DEBUG
			std::cout << "Deleting rectangle" << std::endl;
#endif
			unconfirmedRect = nullptr;
		}
		else if (key == 'e' && !drawing) {//export all selections
			for (int i = 0; i < rects.size(); i++)
			{
				imwrite(exportPrefix + "_" + std::to_string(i) + "." + exportType, baseImage(rects[i]));
			}
			std::cout << "Finished exporting " << rects.size() << " images" << std::endl;
		}
		else if (key == 'q') {//quit
#ifdef _DEBUG
			std::cout << "Quitting" << std::endl;
#endif
			run = false;
		}
		else if (key == 'r') {//reset image
#ifdef _DEBUG
			std::cout << "Resetting Image" << std::endl;
#endif
			baseImage = imread(imagePath);
			unconfirmedRect = nullptr;
			rects.clear();
		}
		else if (key == 'g') {//guides
#ifdef _DEBUG
			std::cout << "Toggling Guides" << std::endl;
#endif
			guides = !guides;
		}
	}

	destroyAllWindows();

    return 0;
}

