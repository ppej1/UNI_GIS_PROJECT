#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

#include "find_circle.h"
#include "find_color.h"
#include "center_point.h"
#include "resize.h"
#include "cannyEdge.h"
#include "find_polygon.h"
using namespace cv;

int main() {
	
	
	/////////////////////////////////���ڵ� //////////////////////////
	////csv ���� ����////
	FILE *out;
	out = fopen("output.csv", "w");                     //������ ���� ��� ������Ʈ �Ӽ� - ��ó���� - ��ó���� ���ǿ�"_CRT_SECURE_NO_WARNINGS"�� �־��ּ���.
	fprintf(out, "Y,X,pixY,pixX,Name,No,PtColor\n");
	
	
	
	//// �̹��� �ҷ����� ////													
	cv::Mat image_R = cv::imread("b_gggme.jpg"); //RGB�̹����� �о���δ�.
	double RcXa = 13762;
	double RcYa = 7237;
	/*
	/////�̹��� �������� ////
	Mat image_R = image_resize(image_0);
	imwrite("resize_image.jpg", image_R);              //0.33��� ����� �̹��� ����
			   ////�ܰ��� ����� ĳ�Ͽ���////
	Mat image_R_canny = Canny_Edge(image_R,170,150,3);
	imwrite("image_R_canny.jpg", image_R_canny);             //�������� �� �̹��� ĳ�Ͽ��� - �ε� �ܰ� �����
	*/
	//// ���� �� ���� ////
	Mat detected_pavement= find_colors(image_R, 12, 38, 27, 255, 100, 130, 30, 255);  //�÷� ���� (�Է� �̹��� , int _minHue01, int _maxHue01, int _minSat01, int _maxSat01, int _minHue02, int _maxHue02, int _minSat02, int _maxSat02) �� 2���� �������� ���� ���Ұ��� �� �ڸ� ���������� ����

	imwrite("pavements.jpg", detected_pavement);  //������ �̹��� ����
				////���� �� ĳ�Ͽ���////
	Mat pavements_canny = Canny_Edge(detected_pavement,30,40,3);
	imwrite("pavements_canny.jpg", pavements_canny);        //������ ĳ�Ͽ��� 
	

	//// ����� ����  /////
	Mat preprocessing_image = preprocessing(image_R,130, CV_THRESH_BINARY_INV);  //��ó���� ���� �ڵ� (�̹�����, �Ӱ谪)

	cv::Mat image_bolad = image_R.clone();                                                  //�������� �������ϸ� ������ �Ѽյ����� ���� �ʼ�
	Mat bolad_circle = find_circles(preprocessing_image, image_bolad, 70, 120, 0, 255, 0, 20);				//find_circle(Mat th_img, Mat _image, int min, int max, int B, int G, int R, int H)

	imwrite("find_bolad.jpg", bolad_circle);   //������ ����� ����

	//// ����� ����� �� ���� ����  ////

	Mat detected_bolad = find_colors(bolad_circle, 48, 62, 200, 255, 48, 62, 200, 255);  //�÷� ���� (�Է� �̹��� , int _minHue01, int _maxHue01, int _minSat01, int _maxSat01, int _minHue02, int _maxHue02, int _minSat02, int _maxSat02) �� 2���� �������� ���� ���Ұ��� �� �ڸ� ���������� ����
	imwrite("bolad_circle.jpg", detected_bolad);        //����� ���� ������ ����   -����� ���Ͷ���¡��
	
	///// ����� ����� ���� ����  /////

	Mat bolad_point = find_points(detected_bolad, out, "bolad",16,16,RcXa,RcYa);   //�߽��� ����  (�̹�����,csv��, csv�� ǥ���� ��ֹ� �� )


	//// ��Ȧ ����  /////
	Mat preprocessing_image01 = preprocessing(detected_pavement, 130, CV_THRESH_BINARY);  //��ó���� ���� �ڵ� (�̹�����, �Ӱ谪)

	cv::Mat image_manholl = detected_pavement.clone();                                                  //�������� �������ϸ� ������ �Ѽյ����� ���� �ʼ�
	Mat manholl_circle = find_circles(preprocessing_image01, image_manholl, 145, 150, 0, 255, 0, 20);				//find_circle(Mat th_img, Mat _image, int min, int max, int B, int G, int R, int H)

	imwrite("find_manholl.jpg", manholl_circle);   //������ ����� ����


	   //// ����� ��Ȧ �� ���� ����  ////

	Mat detected_manholl = find_colors(manholl_circle, 48, 62, 200, 255, 48, 62, 200, 255);  //�÷� ���� (�Է� �̹��� , int _minHue01, int _maxHue01, int _minSat01, int _maxSat01, int _minHue02, int _maxHue02, int _minSat02, int _maxSat02) �� 2���� �������� ���� ���Ұ��� �� �ڸ� ���������� ����
	imwrite("manholl_circle.jpg", detected_manholl);        //����� ���� ������ ����   -����� ���Ͷ���¡��
		///// �����  ��Ȧ ���� ����  /////

	Mat manhall_point = find_points(detected_manholl, out, "manholl", 16, 35, RcXa, RcYa);   //�߽��� ����  (�̹�����,csv��, csv�� ǥ���� ��ֹ� �� )



	//////////csv ���� �ݱ�/////////////////////
	 fclose(out);  // csv �ҷ��°��� �ݵ�� �ݾ���� �Ѵ�.
	
	
				   
	 /*
	////////////////////////////////////////////  �װ����� ���� - ����: ȭ��Ʈ - ���� - ���� :�׸� - XYpoint/////////////////////////////////

	FILE *skypointXY;
	skypointXY = fopen("SkyPointXY.csv", "w");                     //������ ���� ��� ������Ʈ �Ӽ� - ��ó���� - ��ó���� ���ǿ�"_CRT_SECURE_NO_WARNINGS"�� �־��ּ���.
	fprintf(skypointXY, "Y,X,pixY,pixX,Name,No,PtColor\n");

	Mat detected_sky = find_colors(image_R, 90, 130, 0, 30, 90, 130, 0, 30);  //�÷� ���� (�Է� �̹��� , int _minHue01, int _maxHue01, int _minSat01, int _maxSat01, int _minHue02, int _maxHue02, int _minSat02, int _maxSat02) �� 2���� �������� ���� ���Ұ��� �� �ڸ� ���������� ����   0, 360, 0, 255, 0, 360, 0, 255

	Mat skypoint = find_polygons(detected_sky);
	 imwrite("sky_point.jpg", skypoint);
	Mat sky_poly = find_colors(skypoint, 48, 62, 200, 255, 48, 62, 200, 255);  //�÷� ���� (�Է� �̹��� , int _minHue01, int _maxHue01, int _minSat01, int _maxSat01, int _minHue02, int _maxHue02, int _minSat02, int _maxSat02) �� 2���� �������� ���� ���Ұ��� �� �ڸ� ���������� ����
	// imwrite("sky_poly.jpg", sky_poly);
	Mat sky_point = find_points(sky_poly, skypointXY, "sky", 5, 40);   //�߽��� ����  (�̹�����,csv��, csv�� ǥ���� ��ֹ� �� , �𼭸��� �ּ�,�ִ밪)
		imwrite("sky.jpg", detected_sky);  //������ �̹��� ����
	fclose(skypointXY);  // csv �ҷ��°��� �ݵ�� �ݾ���� �Ѵ�.
	

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

		*/		  


}	