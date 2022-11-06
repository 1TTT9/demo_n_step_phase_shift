#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <math.h>
#include <deque>
#include <queue>

#ifndef THREESTEPPHASESHIFT_H 
#define THREESTEPPHASESHIFT_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
using namespace std;

struct UnwrapPath {
	int x;
	int y;
	float phi;    // last phase 
	float q;      // phase quality 

	UnwrapPath(int x, int y, float phi) :
		x(x), y(y), phi(phi)
	{}

	UnwrapPath(int x, int y, float phi, float q) :
		x(x), y(y), phi(phi), q(q)
	{}

	bool operator<(const UnwrapPath & path) const { return q < path.q; }

};

class ThreeStepPhaseShift {

public:

	ThreeStepPhaseShift(
		string imgPath1,
		string imgPath2,
		string imgPath3,
		string imgPath4,
		int k_shift
	);

	~ThreeStepPhaseShift();

	void phaseDecode();
	void phaseUnwrap();
	void computeDepth();

	void compute() {
		phaseDecode();
		phaseUnwrap();
		computeDepth();
	}

	void  setZscale(float _zscale) { zscale = _zscale; }
	void  setZskew(float _zskew) { zskew = _zskew; }
	void  setNoiseThreshold(float _threshold) { noiseThreshold = _threshold; }
	float getZscale() { return zscale; }
	float getZskew() { return zskew; }
	float getNoiseThreshold() { return noiseThreshold; }
	//float* getDepth() { return depth; }
	Mat getDepthImg() { return imgdepth; }
	bool* getMask() { return mask; }

	Mat getWrappedPhase() { return imgWrappedPhase; };
	Mat getUnwrappedPhase() { return imgUnwrappedPhase; };
	Mat getColorImage() { return imgColor; };

	Mat imgPhase1Gray;

protected:

	// unwrap at x,y
	void phaseUnwrap(int x, int y, float phi);
	void phaseUnwrap(int x, int y, float phi, float dist);

	void computeQuality();

	// inline helper functions
	float max_phase(float v1, float v2, float v3) {
		float max;
		max = v1 > v2 ? v1 : v2;
		max = max > v3 ? max : v3;
		return max;
	}

	float min_phase(float v1, float v2, float v3) {
		float min = v1 < v2 ? v1 : v2;
		min = min < v3 ? min : v3;
		return min;
	}

	/* use mean as luminance of an rgb triple */
	float luminance(uchar *color) { return (color[0] + color[1] + color[2]) / (3.f * 255); }

	void copy_channels(uchar *dest, uchar *src) {
		for (int i = 0; i < 3; i++)
			*(dest + i) = *(src + i);
	}

	float sqdist(float v1, float v2) {
		float d = v1 - v2;
		return 1 - d * d;
	}

private:

	Mat imgPhase1;
	Mat imgPhase2;
	Mat imgPhase3;
	Mat imgPhase4;

	Mat imgColor;  // reconstructed color image 
	Mat imgWrappedPhase;
	Mat imgUnwrappedPhase;

	// some helper matrices to track phase quality and
	// processing state (each from the same dimension as the input image)
	bool  *mask;
	bool  *process;
	float *quality;
	float *range;
	//float *depth;
	Mat imgdepth;
	float noiseThreshold;
	float zscale;
	float zskew;

	int width;
	int height;
	int step;   // for single channel images

	int n_shift;

	//deque<UnwrapPath> procQueue;
	priority_queue<UnwrapPath> processHeap;
};

#endif
