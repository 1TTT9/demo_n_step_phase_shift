// demoApp.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

// resize
#include "opencv2/imgproc/imgproc.hpp"

#include "three_step_phase_shift.h"

//***C++11 Style:***
#include <chrono>

#ifdef _WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif


using namespace std;
using namespace cv;


float printMinMax(Mat img) {

	int w = img.cols;
	int h = img.rows;
	int step = w;

	int i;
	float min, max;
	min = 1e6;
	max = 1e-6;

	for (i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			float val = (float)img.at<float>(i, j); 
			if (val < min) min = val;
			if (val > max) max = val;
		}
	}

	cout << "min: " << min << "\nmax: " << max << endl;
	return max;

}

void scale(Mat img) {

	int w = img.cols;
	int h = img.rows;
	int step = w;

	int i;
	float min, max;
	min = 1e6;
	max = 1e-6;

	for (i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			float val = (float)img.at<float>(i, j); //ptr[i*step + j * 4];
			if (val < min) min = val;
			if (val > max) max = val;
		}

	}

	cout << "min: " << min << "\nmax: " << max << endl;
	img.convertTo(img, CV_32FC1,  1. / (max - min), -min / (max - min));
}

string get_current_dir() {
	char buff[FILENAME_MAX]; //create string buffer to hold path
	GetCurrentDir(buff, FILENAME_MAX);
	string current_working_dir(buff);
	return current_working_dir;
}

inline bool exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}


int main(int argc, char* argv[])
{
	cout << "Start to generate depth image via n-step phase shifting algorithm\n";


	cout << "current dir:" << get_current_dir() << endl;

	cout << "input dir: " << argv[1] << endl;

	int sample_id = 1;
	int k_step = 4;

	string inputDir(argv[1]);
	string imgPath1 = inputDir + "/" + "0.bmp";
	string imgPath2 = inputDir + "/" + "1.bmp";
	string imgPath3 = inputDir + "/" + "2.bmp";
	string imgPath4 = inputDir + "/" + "3.bmp";


	if (!exists(imgPath1)) {
		cout << "File " << imgPath1 << " does not exist?" << endl;

		return 0;
	}

	Mat image = imread(imgPath1, CV_LOAD_IMAGE_COLOR);

	/*  [ref](https://stackoverflow.com/questions/14189492/opencv-unable-to-write-images-using-imwrite)
	     unable to write image with Opencv 3.1.0 and debug mode, win 64 version.
		 instead, use filestorage to save Mat file.
	 */ 
	//image.convertTo(image, CV_32F);
	//imwrite("d:\\out.jpg", image);

	chrono::steady_clock::time_point begin = chrono::steady_clock::now();

	cout << "============================================================" << endl;
	ThreeStepPhaseShift decoder(imgPath1, imgPath2, imgPath3, imgPath4, k_step);

	cout << "------------------------------------------------------------" << endl;
	decoder.compute();
	cout << "------------------------------------------------------------" << endl;

	Mat wrappedPhase = decoder.getWrappedPhase();
	scale(wrappedPhase);

	Mat unwrappedPhase = decoder.getUnwrappedPhase();
	scale(unwrappedPhase);

	Mat imgDepth = decoder.getDepthImg();
	Mat colorImage = decoder.getColorImage();
	cout << "============================================================" << endl;
	chrono::steady_clock::time_point end = chrono::steady_clock::now();
	cout << "Time difference (sec) = " << (std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) / 1000000.0 << endl;
	cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[micro s]" << endl;
	cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[nano s]" << endl;

	//Display
	namedWindow("Display getColorImage", CV_WINDOW_NORMAL);
	resizeWindow("Display getColorImage", colorImage.cols*0.3, colorImage.rows*0.3);
	imshow("Display getColorImage", colorImage);

	namedWindow("Display wrappedPhase", CV_WINDOW_NORMAL);
	resizeWindow("Display wrappedPhase", wrappedPhase.cols*0.3, wrappedPhase.rows*0.3);
	imshow("Display wrappedPhase", wrappedPhase);

	namedWindow("Display unwrappedPhase", CV_WINDOW_NORMAL);
	resizeWindow("Display unwrappedPhase", unwrappedPhase.cols*0.3, unwrappedPhase.rows*0.3);
	imshow("Display unwrappedPhase", unwrappedPhase);

	namedWindow("Display imgDepth", CV_WINDOW_NORMAL);
	//resizeWindow("Display imgDepth", imgDepth.cols*0.3, imgDepth.rows*0.3);
	imshow("Display imgDepth", imgDepth);

	string outFilename = "matDepth_" + to_string(k_step) + ".mat";
	cv::FileStorage file(outFilename, cv::FileStorage::WRITE);
	file << "depth" << decoder.getDepthImgF();



	if (!image.data) {
		cout << "Can not load the image!" << std::endl;
		return -1;
	}
	waitKey(0);
	return 0;
}
