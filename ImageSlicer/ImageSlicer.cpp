// ImageSlicer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <wtypes.h>

#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\imgproc.hpp>

using namespace cv;

Mat baseImage;
Point p1;
Rect* unconfirmedRect;
Point mouse;
Point mouseLast;
bool drawing;
bool rClick = false;
bool rMove = false;

Rect imageSection;

const Scalar UNCONFIRMED_COLOR = Scalar(255, 0, 0);
const Scalar CONFIRMED_COLOR = Scalar(0, 255, 0);
const Scalar GUIDE_COLOR = Scalar(0, 0, 255);

const float WINDOW_MARGIN_PERCENT = 0.05;
const int SCROLL_STEP = 5;
const float ZOOM_STEP_PERCENT = 0.05;
const float THICKNESS_FACTOR = 0.001;

int ZOOM_STEP_X;
int ZOOM_STEP_Y;

float x_zoom;
float y_zoom;

void printUsage() {
	std::cout << "Usage: ImageSlicer <image path> <export filename prefix> <export file type> <starting number>" << std::endl;
}

void printControls() {
	std::cout << "Controls:" << std::endl << "Click and drag to select a region to slice" << std::endl << "c - confirm your selection"
		<< std::endl << "d - delete your selection" << std::endl << "e - export all selections" << std::endl
		<< "g - toggle guide lines" << std::endl << "r - reset image" << std::endl << "q - quit" << std::endl;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata) {
	x *= x_zoom;
	y *= y_zoom;
	if (event == EVENT_LBUTTONDOWN) {
#ifdef _DEBUG
		std::cout << "Left button down (" << x << "," << y << ")" << std::endl;
		std::cout << "Image Section: " << imageSection << std::endl;
#endif
		p1 = Point(x, y);
		drawing = true;
	}
	else if (event == EVENT_LBUTTONUP) {
#ifdef _DEBUG
		std::cout << "Left button up (" << x << "," << y << ")" << std::endl;
		std::cout << "Image Section: " << imageSection << std::endl;
#endif
		unconfirmedRect = new Rect(p1, Point(x, y));
		drawing = false;
	}
	else if (event == EVENT_RBUTTONDOWN) {
#ifdef _DEBUG
		std::cout << "Right button down" << std::endl;
#endif
		rClick = true;
	}
	else if (event == EVENT_RBUTTONUP) {
#ifdef _DEBUG
		std::cout << "Right button up" << std::endl;
#endif
		rClick = false;
		rMove = false;
	}
	else if (event == EVENT_MOUSEMOVE) {
		if (rClick) {
			if (!rMove) {
				mouseLast = mouse;
				rMove = true;
			}

		}
		mouse = Point(x, y);
		if (drawing) {
			unconfirmedRect = new Rect(p1, mouse);
		}
	}
	else if (event == EVENT_MOUSEWHEEL) {
#ifdef _DEBUG
		std::cout << "Mouse Wheel: " << flags << std::endl;
#endif
		if (flags > 0) {//zoom in
#ifdef _DEBUG
			std::cout << "Zooming in" << std::endl;
#endif
			imageSection.width *= 1.0 - ZOOM_STEP_PERCENT;
			imageSection.height *= 1.0 - ZOOM_STEP_PERCENT;

			if ((imageSection.width <= 0) || (imageSection.height <= 0)) {
				imageSection.width = ZOOM_STEP_X;
				imageSection.height = ZOOM_STEP_Y;
			}
		}
		else if (flags < 0) {
#ifdef _DEBUG
			std::cout << "Zooming out" << std::endl;
#endif
			imageSection.width *= 1.0 + ZOOM_STEP_PERCENT;
			imageSection.height *= 1.0 + ZOOM_STEP_PERCENT;

			if (imageSection.width > baseImage.cols) {
				imageSection.height = (baseImage.cols - 1) * imageSection.height / imageSection.width;
				imageSection.width = baseImage.cols - 1;
				imageSection.x = 0;
			}
			else if ((imageSection.x + imageSection.width) >= baseImage.cols) {
				imageSection.x -= ((imageSection.x + imageSection.width) - baseImage.cols);
			}
			if (imageSection.height > baseImage.rows) {
				imageSection.width = (baseImage.rows - 1) * imageSection.width / imageSection.height;
				imageSection.height = baseImage.rows - 1;
				imageSection.y = 0;
			}
			else if ((imageSection.y + imageSection.height) >= baseImage.rows) {
				imageSection.y -= ((imageSection.y + imageSection.height) - baseImage.rows);
			}
		}
	}
}

// Get the horizontal and vertical screen sizes in pixel
Size GetDesktopResolution()
{
	RECT desktop;
	// Get a handle to the desktop window
	const HWND hDesktop = GetDesktopWindow();
	// Get the size of screen to the variable desktop
	GetWindowRect(hDesktop, &desktop);
	// The top left corner will have coordinates (0,0)
	// and the bottom right corner will have coordinates
	// (horizontal, vertical)
	return Size(desktop.right, desktop.bottom);
}

int main(int argc, char* argv[])
{
	if (argc != 5) {
		printUsage();
		return -1;
	}

	std::string imagePath(argv[1]);

	std::string exportPrefix(argv[2]);
	std::string exportType(argv[3]);
	int startingNumber = std::stoi(argv[4]);

	Size screenSize = GetDesktopResolution();
	//Trim some margin space
	screenSize.width *= 1 - WINDOW_MARGIN_PERCENT;
	screenSize.height *= 1 - WINDOW_MARGIN_PERCENT;

#ifdef _DEBUG
	std::cout << "Image Path: " << imagePath << std::endl;
	std::cout << "Export Prefix: " << exportPrefix << std::endl;
	std::cout << "Export Type: " << exportType << std::endl;
	std::cout << "Screen Width: " << screenSize.width << " Height: " << screenSize.height << std::endl;
#endif

	baseImage = imread(imagePath);
	Mat tempImage = baseImage.clone();

	if (baseImage.empty()) {
		std::cout << "Error loading image: " << imagePath << std::endl;
		printUsage();
		return -2;
	}

	if ((baseImage.cols > screenSize.width) || (baseImage.rows > screenSize.height)) {
#ifdef _DEBUG
		std::cout << "Image larger than screen" << std::endl;
#endif
		imageSection = Rect(Point(0,0), screenSize);
	}
	else {
		imageSection = Rect(Point(0,0), baseImage.size());
	}

	ZOOM_STEP_X = screenSize.width * ZOOM_STEP_PERCENT;
	ZOOM_STEP_Y = screenSize.height * ZOOM_STEP_PERCENT;
	x_zoom = 1.0;
	y_zoom = 1.0;

#ifdef _DEBUG
	std::cout << "Zoom Step X: " << ZOOM_STEP_X << std::endl;
	std::cout << "Zoom Step Y: " << ZOOM_STEP_Y << std::endl;
#endif

	namedWindow("Image Slicer", WINDOW_AUTOSIZE);

	setMouseCallback("Image Slicer", CallBackFunc, NULL);

	std::vector<Rect> rects;
	bool run = true;
	bool guides = false;

	printControls();

	while (run) {
		/***==-==-== Render Code ==-==-==***/
		//Reset frame to base image
		tempImage = baseImage.clone()(imageSection);
		
		//Render guides if necessary
		if (guides) {
#ifdef _DEBUG
			std::cout << "Drawing Guides " << tempImage.rows << std::endl;
#endif
			int thickness = max(tempImage.rows, tempImage.cols) * THICKNESS_FACTOR;
			line(tempImage, Point(mouse.x, 0), Point(mouse.x, tempImage.rows), GUIDE_COLOR, thickness);
			line(tempImage, Point(0, mouse.y), Point(tempImage.cols, mouse.y), GUIDE_COLOR, thickness);
		}

		//Render an unconfirmed rectangle if necessary
		if (unconfirmedRect != nullptr) {
#ifdef _DEBUG
			//std::cout << "Drawing unconfirmed rectangle" << std::endl;
#endif
			rectangle(tempImage, *unconfirmedRect, UNCONFIRMED_COLOR, max(tempImage.rows, tempImage.cols) * THICKNESS_FACTOR);
		}

		//resize if needed
		if ((tempImage.cols != screenSize.width) || (tempImage.rows != screenSize.height)) {
			x_zoom = (float)tempImage.cols / screenSize.width;
			y_zoom = (float)tempImage.rows / screenSize.height;
			resize(tempImage, tempImage, screenSize);
		}

		imshow("Image Slicer", tempImage);

		/***==-==-== Input Code ==-==-==***/
		//Get input
		int key = waitKey(1);
		//Handle Input
		if ((key == 'c') && !drawing && (unconfirmedRect != nullptr)) {//confirm rect
#ifdef _DEBUG
			std::cout << "Confirming rectangle" << std::endl;
#endif
			//Add the viewing offset to the rectangle
			unconfirmedRect->x += imageSection.x;
			unconfirmedRect->y += imageSection.y;

			int thickness = max(tempImage.rows, tempImage.cols) * THICKNESS_FACTOR;
			rectangle(baseImage, *unconfirmedRect, CONFIRMED_COLOR, thickness);

			//trim the green borders
			unconfirmedRect->x += thickness;
			unconfirmedRect->y += thickness;
			unconfirmedRect->width -= 2 * thickness;
			unconfirmedRect->height -= 2 * thickness;
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
				imwrite(exportPrefix + std::to_string(startingNumber + i) + "." + exportType, baseImage(rects[i]));
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
		else if (key == '6') {//right
#ifdef _DEBUG
			std::cout << "Scrolling Right" << std::endl;
#endif
			if ((imageSection.x + imageSection.width + SCROLL_STEP) <= baseImage.cols) {
				imageSection.x += SCROLL_STEP;
			}
		}
		else if (key == '8') {//up
#ifdef _DEBUG
			std::cout << "Scrolling Up" << std::endl;
#endif
			if (imageSection.y > 0) {
				imageSection.y -= SCROLL_STEP;
			}
		}
		else if (key == '4') {//left
#ifdef _DEBUG
			std::cout << "Scrolling Left" << std::endl;
#endif
			if (imageSection.x > 0) {
				imageSection.x -= SCROLL_STEP;
			}
		}
		else if (key == '2') {//down
#ifdef _DEBUG
			std::cout << "Scrolling Down" << std::endl;
#endif
			if ((imageSection.y + imageSection.height + SCROLL_STEP) <= baseImage.rows) {
				imageSection.y += SCROLL_STEP;
			}
		}
		if (rClick && rMove) {
#ifdef _DEBUG
			std::cout << "Right click and drag" << std::endl;
#endif
			int deltaX = mouse.x - mouseLast.x;
			int deltaY = mouse.y - mouseLast.y;
#ifdef _DEBUG
			std::cout << "Mouse Delta: " << mouse - mouseLast << std::endl;
#endif
			rMove = false;
			if (((imageSection.x + imageSection.width - deltaX) <= baseImage.cols)
				&& (imageSection.x - deltaX) >= 0) {
				imageSection.x -= deltaX;
			}

			if (((imageSection.y + imageSection.height - deltaY) <= baseImage.rows)
				&& (imageSection.y - deltaY) >= 0) {
				imageSection.y -= deltaY;
			}
		}
	}

	destroyAllWindows();

    return 0;
}

