#include "three_step_phase_shift.h"
#include <cstdio>

#define _USE_MATH_DEFINES
#include <math.h>

ThreeStepPhaseShift::ThreeStepPhaseShift(
	string imgPath1
	, string imgPath2
	, string imgPath3
	, string imgPath4
	, int k_shift
) :

	imgPhase1( imread(imgPath1, CV_LOAD_IMAGE_COLOR)),
	imgPhase2 (imread(imgPath2, CV_LOAD_IMAGE_COLOR)),
	imgPhase3 (imread(imgPath3, CV_LOAD_IMAGE_COLOR)),
	imgPhase4( imread(imgPath4, CV_LOAD_IMAGE_COLOR)),
	mask(0),
	process(0)

{
	n_shift = k_shift;

	width = imgPhase1.cols;
	height = imgPhase1.rows;

	Rect crop_region(0, 0, width*0.5, height);
	// specifies the region of interest in Rectangle form
	imgPhase1 = imgPhase1(crop_region);
	imgPhase2 = imgPhase2(crop_region);
	imgPhase3 = imgPhase3(crop_region);
	imgPhase4 = imgPhase4(crop_region);


	width = imgPhase1.cols;
	height = imgPhase1.rows;
	if (width != imgPhase2.cols ||
		width != imgPhase3.cols ||
		height != imgPhase2.rows ||
		height != imgPhase3.rows ) {
		throw "invalid arguments: input images must have same dimension!";
	}

	int size = width * height;
	imgColor = imread(imgPath1, IMREAD_COLOR);
	imgColor = imgColor(crop_region);

	imgPhase1Gray = imread(imgPath1, IMREAD_GRAYSCALE);
	imgPhase1Gray = imgPhase1Gray(crop_region);

	imgWrappedPhase = imread(imgPath1, IMREAD_GRAYSCALE);
	imgWrappedPhase = imgWrappedPhase(crop_region);
	imgWrappedPhase.convertTo(imgWrappedPhase, CV_32F);

	imgUnwrappedPhase = imread(imgPath1, IMREAD_GRAYSCALE);
	imgUnwrappedPhase = imgUnwrappedPhase(crop_region);
	imgUnwrappedPhase.convertTo(imgUnwrappedPhase, CV_32F);

	imgdepth = imread(imgPath1, IMREAD_GRAYSCALE);
	imgdepth = imgdepth(crop_region);
	//imgdepth.convertTo(imgdepth, CV_32F);
	mask = new bool[size];
	process = new bool[size];
	quality = new float[size];
	range = new float[size];
	//depth = new float[size];

	// initilize matrices
	noiseThreshold = 0.1;
	zscale = 130;
	zskew = 24;

	// init step width for color and single channel images
	step = width;
	cout << "width: " << width << "\nheight: " << height << endl;
	cout << "size " << size << endl;
}

// dtor
ThreeStepPhaseShift::~ThreeStepPhaseShift() {
	imgPhase1.release();
	imgPhase2.release();
	imgPhase3.release();
	imgPhase4.release();

	imgColor.release();
	imgWrappedPhase.release();
	delete[] mask;
	delete[] process;
	delete[] quality;
	//delete[] depth;
	imgdepth.release();
}

void ThreeStepPhaseShift::phaseDecode()
{
	// Some initializing and optimization( only the sqrt thing )
	float sqrt3 = sqrt(3);
	//int step = imgPhase1->widthStep;

	int ii, iic;

	float phi1;
	float phi2;
	float phi3;
	float phi4;

	float phiSum;
	float phiRange;
	float phiMax;
	float phiMin;

	float noise;
	float twoPi = M_PI * 2;
	int stepc = imgPhase1.step1();//imgPhase1->widthStep;

	float theta;

	if (false)
	{
		cout << "phaseDecode()" << endl;
		cout <<"\t" << "step " << step << endl;
		cout << "\t" << "imgPhase1 " << imgPhase1.size() << ", " << imgPhase1.at<Vec3b>(0, 0) << endl;
		cout << "\t" << "imgPhase1Gray " << imgPhase1Gray.size() << ", "<< imgPhase1Gray.at<uchar>(0, 0) << endl;
		//imgWrappedPhase
		cout << "\t" << "imgWrappedPhase " << imgWrappedPhase.size() << ", " << imgWrappedPhase.at<float>(0, 0) << endl;
		//imgColor
		cout << "\t" << "imgColor " << imgColor.size() << ", " << imgColor.at<Vec3b>(0, 0) << endl;
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			ii = i * step + j;
			iic = i * stepc + j * 3;
			
			// get intensity of each (rgb) phi image
			phi1 = imgPhase1.at<Vec3b>(i , j)[0]; 
			phi2 = imgPhase2.at<Vec3b>(i,  j)[0]; 
			phi3 = imgPhase3.at<Vec3b>(i , j)[0]; 
			if (n_shift == 4) {
				phi4 = imgPhase4.at<Vec3b>(i, j)[0]; 
			}

			phiSum = phi1 + phi2 + phi3;
			phiMax = max_phase(phi1, phi2, phi3);
			phiMin = min_phase(phi1, phi2, phi3);
			if (n_shift == 4) {
				if (phi4 > phiMax)
					phiMax = phi4;
				if (phi4 < phiMin)
					phiMin = phi4;
			}

			phiRange = phiMax - phiMin;

			// compute phase quality, try to filter background pixel
			// i.e. where the phase range is too low
			noise = phiRange / phiSum;
			mask[ii] = (noise < noiseThreshold);
			process[ii] = !mask[ii];
			range[ii] = phiRange;

			if (!mask[ii])
			{
				theta = (float)atan2(sqrt3 * (phi1 - phi3), 2 * phi2 - phi1 - phi3) / twoPi;

				if (n_shift == 4)
				{
					theta = -1* (phi2 - phi4) / (phi1 - phi3);
				}
				imgWrappedPhase.at<float>(i, j) = theta;
			}
			else
			{
				// noise part
				imgWrappedPhase.at<float>(i, j) = 0.f;
			}

			// user lightest pixel of all phase images as color
			if (true)
			{
				imgColor.at<Vec3b>(i, j)[0] = phiMax;
				imgColor.at<Vec3b>(i, j)[1] = phiMax;
				imgColor.at<Vec3b>(i, j)[2] = phiMax;
			}
			else if (!mask[ii])
			{
				imgColor.at<Vec3b>(i, j)[0] = phiMax;
				imgColor.at<Vec3b>(i, j)[1] = phiMax;
				imgColor.at<Vec3b>(i, j)[2] = phiMax;
			}
			else {
				imgColor.at<Vec3b>(i, j)[0] = 255;
				imgColor.at<Vec3b>(i, j)[1] = 255;
				imgColor.at<Vec3b>(i, j)[2] = 255;
			}
		}
	}
	computeQuality();
}

void ThreeStepPhaseShift::computeDepth() {

	for (int i = 0; i < height; i++) {
		float planephase = 0.5 - (i - (height / 2)) / zskew;
		for (int j = 0; j < width; j++) {
			int ii = i * step + j;
			if (!mask[ii]) {
				imgdepth.at<uchar>(i, j) = ((float)imgUnwrappedPhase.at <float > (i, j) - planephase) * zscale;
			}
			else {
				imgdepth.at<uchar>(i, j) = 255.f;
				//depth[ii] = 0.f;
			}
		}
	}

	cout << "Computed zmatrix" << endl;
}


void ThreeStepPhaseShift::phaseUnwrap(int x, int y, float phi, float q) {

	if (process[y*step + x]) {

		float frac = phi - (int)phi;     // discontinue unwrapped phase
		float diff = (float)imgUnwrappedPhase.at<float>(y, x) - frac;
		q += quality[y*step + x];         // add current quality
		if (diff > 0.5) {
			diff--;
		}
		if (diff < -0.5) {
			diff++;
		}
		processHeap.push(UnwrapPath(x, y, phi + diff, q));
	}
}

void ThreeStepPhaseShift::phaseUnwrap()
{
	int startX = width / 2;
	int startY = height / 2;

	imgWrappedPhase.copyTo(imgUnwrappedPhase);

	UnwrapPath path = UnwrapPath(startX, startY, (float)imgUnwrappedPhase.at<float>(startY, startX), 0);
	//procQueue.push_back(p);
	processHeap.push(path);

	while (!processHeap.empty()) {
		UnwrapPath current = processHeap.top();
		processHeap.pop();
		int x = current.x;
		int y = current.y;
		float q = current.q;

		if (process[y*step + x]) {
			imgUnwrappedPhase.at<float>(y, x) = current.phi;
			process[y*width + x] = false;

			// follow path in each direction
			if (y > 0) {
				phaseUnwrap(x, y - 1, current.phi, q);
			}
			if (y < height - 1) {
				phaseUnwrap(x, y + 1, current.phi, q);
			}
			if (x > 0) {
				phaseUnwrap(x - 1, y, current.phi, q);
			}
			if (x < width - 1) {
				phaseUnwrap(x + 1, y, current.phi, q);
			}
		}
	}
}

void ThreeStepPhaseShift::computeQuality() {

	for (int i = 1; i < height - 1; i++) {
		for (int j = 1; j < width - 1; j++) {
			int ii = i * step + j;
			float phi = (float)imgWrappedPhase.at<float>(i , j); //ptrPhase[ii];
			quality[ii] = sqdist(phi, (float)imgWrappedPhase.at<float>(i , j+1)) +
				sqdist(phi, imgWrappedPhase.at<float>(i , j-1)) +
				sqdist(phi, (float)imgWrappedPhase.at<float>(i , j+1)) +
				sqdist(phi, (float)imgWrappedPhase.at<float>(i , j-1));
			quality[ii] /= range[ii];
		}
	}
}
