#include<iostream>
#include<opencv2\opencv.hpp>

using namespace std;
using namespace cv;

Mat image_resize(Mat img)
{

	Mat src = img;
	//imshow("before", src);
	//cout << src.rows << " " << src.cols << endl;
	int re_rows, re_cols;
	re_rows = src.rows*0.33;
	re_cols = src.cols*0.33;
	Mat resize_image;
	resize(src, resize_image, Size(re_cols, re_rows), 0, 0, CV_INTER_LINEAR);
	//imshow("after", dst);
	
	//cout << resize_image.rows << " " << resize_image.cols << endl;
	waitKey();
	return resize_image;
}
