#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;


Mat Canny_Edge(Mat img, double edge1, double edge2, int apurtual)
{

	Mat gray, edge, draw;
	cvtColor(img, gray, CV_BGR2GRAY);
	Canny(gray, edge, edge1, edge2, apurtual);
	edge.convertTo(draw, CV_8U);

	return draw;
}