//find_color.h 내용

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

void detectHScolor(const cv::Mat&image, double minHue, double maxHue, double minSat, double maxSat, cv::Mat& mask) {
	cv::Mat hsv;
	cv::cvtColor(image, hsv, CV_BGR2HSV); //RGB > HSV색공간으로 변환

	std::vector<cv::Mat> channels;
	cv::split(hsv, channels);

	cv::Mat mask1;
	cv::threshold(channels[0], mask1, maxHue, 255, cv::THRESH_BINARY_INV);
	cv::Mat mask2;
	threshold(channels[0], mask2, minHue, 255, cv::THRESH_BINARY);
	cv::Mat hueMask;
	if (minHue < maxHue) hueMask = mask1 & mask2;
	else hueMask = mask1 | mask2;

	cv::threshold(channels[1], mask1, maxSat, 255, cv::THRESH_BINARY_INV);
	cv::threshold(channels[1], mask2, minSat, 255, cv::THRESH_BINARY);

	cv::Mat satMask;
	satMask = mask1 & mask2;

	mask = hueMask & satMask;
}

cv::Mat find_colors(cv::Mat _image, int _minHue01, int _maxHue01, int _minSat01, int _maxSat01, int _minHue02, int _maxHue02, int _minSat02, int _maxSat02)
{
	cv::Mat image = _image;
	cv::Mat mask;
	cv::Mat mask02;

	detectHScolor(image, _minHue01, _maxHue01, _minSat01, _maxSat01, mask);
	detectHScolor(image, _minHue02, _maxHue02, _minSat02, _maxSat02, mask02);


	cv::Mat detected(image.size(), CV_8UC3, cv::Scalar(0, 0, 0));
	image.copyTo(detected, mask);
	image.copyTo(detected, mask02);



	/* imwrite("C:/Users/TaeWon/Documents/Visual Studio 2017/OpenCVTest/color_mask.jpg", mask); */

	
	cv::waitKey(0);

	return detected;
};
