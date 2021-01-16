#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

using namespace cv;

#include <iostream>
using namespace std;

// opencv 색상 정의
#define CVX_RED      CV_RGB(255,0,0)
#define CVX_ORANGE   CV_RGB(255,165,0)
#define CVX_YELLOW   CV_RGB(255,255,0)
#define CVX_GREEN   CV_RGB(0,255,0)
#define CVX_BLUE   CV_RGB(0,0,255)
#define CVX_PINK   CV_RGB(255,0,255)
#define CVX_BLICK   CV_RGB(0,0,0)
#define CVX_WHITE   CV_RGB(255,255,255)


double GetDistance2D(CvPoint p1, CvPoint p2)
{
	return sqrt(pow((float)p1.x - p2.x, 2) + pow((float)p1.y - p2.y, 2));
}
void GetMidpoint(CvPoint p1, CvPoint p2, CvPoint *p3)
{
	p3->x = (p1.x + p2.x) / 2.0;
	p3->y = (p1.y + p2.y) / 2.0;
}

cv::Mat find_points(cv::Mat _image, FILE *out, string Point_name, int p_min, int p_max , double RcXa, double RcYa)
{
	cv::Mat imgHSV = _image;
	IplImage copy;
	copy = imgHSV;

	char c;

	IplImage* Img_rgb = &copy;
	IplImage* Img_Gray = cvCreateImage(cvSize(Img_rgb->width, Img_rgb->height), IPL_DEPTH_8U, 1);
	cvCvtColor(Img_rgb, Img_Gray, CV_RGB2GRAY);

	//csv 파일 생성

	

	// 외곽선 추적 및 근사화 변수 초기화
	CvMemStorage*   m_storage = cvCreateMemStorage(0);      // 배열 자료(점의 좌표가 들어간다)
	CvMemStorage*   m_storage2 = cvCreateMemStorage(0);      // 배열 자료(점의 좌표가 들어간다)
	CvSeq*         m_seq = 0;                        // 경계 계수를 저장할 변수
	CvSeq*         m_approxDP_seq = 0;
	CvSeq*         m_dominant_points = 0;               // 특징점 찾기 위한 변수

														// ConvexHull 변수 초기화   
	int counterConvexHull;
	CvSeq* hull = 0;
	CvSeq* ptseq;
	CvSeq* defect = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);

	// moment 변수 선언
	CvMoments moments;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// (1) 외곽선 추적
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cvFindContours(Img_Gray,
		m_storage,
		&m_seq,
		sizeof(CvContour),
		CV_RETR_EXTERNAL,
		CV_CHAIN_APPROX_NONE,
		cvPoint(0, 0));
	//cvDrawContours(Img_rgb, m_seq, CVX_RED, CVX_RED, 1,1,8 );               // (Test) 외곽선 추적이 잘되었는지 테스트




	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// (2) 외곽선 근사화
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (m_seq != 0)
	{
		m_approxDP_seq = cvApproxPoly(m_seq,
			sizeof(CvContour),
			m_storage,
			CV_POLY_APPROX_DP,
			3,
			1);
		//cvDrawContours(Img_rgb, m_approxDP_seq, CVX_RED, CVX_RED, 1,1,8 );   // (Test) 외곽선 근사화가 잘되었는지 테스트

		int testcount = 0;                                          // 각 엣지 리스트별로 색상 다르게 출력되는지 확인위한 변수
		for (CvSeq* c = m_approxDP_seq; c != NULL; c = c->h_next)         // 엣지의 링크드리스트를 순회하면서 각 엣지들에대해서 출력한다.
		{
			ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);

			if ((c->total >= p_min) && (c->total <= p_max))                                     // 외곽선을 이루는 점의 갯수가 이것보다 미만이면 잡음이라고 판단
			{
				for (int i = 0; i < c->total; ++i)
				{
					CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);            // CvSeq로부터좌표를얻어낸다.
					CvPoint temp;
					temp.x = p->x;
					temp.y = p->y;

					cvSeqPush(ptseq, &temp);                           // 컨백스헐을 구하기위해 좌표 저장

																	   //// (Test) 링크드리스트를 순회하면서 점을 잘찍나 테스트
																	   //if( testcount == 0 )
																	   //   cvCircle(Img_rgb, temp, 2, CVX_RED, CV_FILLED);   
																	   //if( testcount == 1 )
																	   //   cvCircle(Img_rgb, temp, 2, CVX_GREEN, CV_FILLED);   
																	   //if( testcount == 2 )
																	   //   cvCircle(Img_rgb, temp, 2, CVX_YELLOW, CV_FILLED);   
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// (3) ConvexHull
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				hull = cvConvexHull2(ptseq, m_storage2, CV_CLOCKWISE, 0);
				counterConvexHull = hull->total;

				// 컨벡스헐 공간에서 깊이가가장 큰 부분 구하기 위해 변수 선언(p1,p2 의 중간이 mid)
				CvPoint p1, p2, mid;
				double dist;

				defect = cvConvexityDefects(ptseq, hull, m_storage2);
				CvConvexityDefect *Item;

				for (; defect; defect = defect->h_next)
				{
					int nomdef = defect->total;
					Item = (CvConvexityDefect*)malloc(sizeof(CvConvexityDefect)*nomdef);
					cvCvtSeqToArray(defect, Item, CV_WHOLE_SEQ);

					for (int i = 0; i < nomdef; i++)
					{
						p1.x = Item[i].start->x;
						p1.y = Item[i].start->y;
						p2.x = Item[i].end->x;
						p2.y = Item[i].end->y;
						dist = GetDistance2D(p1, p2);
						GetMidpoint(p1, p2, &mid);

						cvCircle(Img_rgb, *Item[i].start, 2, CVX_RED, CV_FILLED);            // 컨벡스 결함으로 구한 점(외곽선)그리기
						cvCircle(Img_rgb, *Item[i].depth_point, 2, CVX_GREEN, CV_FILLED);      // 컨벡스 결함으로 구한 점(뎁스깊이가 깊은것)그리기
						cvLine(Img_rgb, p1, p2, CVX_RED, 1, 8, 0);                           // 컨벡스헐 외곽선 그리기
						cvLine(Img_rgb, mid, *Item[i].depth_point, CVX_GREEN, 1, 8, 0);         // 컨벡스헐 외곽선과 컨벡스결함으로 구한 점과 연결
					}
					free(Item);
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// (4) cvMoment 로 중심점 계산
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				cvMoments(c, &moments);

				double M;
				int x_order, y_order;
				double cX, cY, m00;
				double RcX, RcY, zX1, zY1, pX1, pY1, zX2, zY2, pX2, pY2, sX, sY, RcX0, RcXA, RcY0, RcYA;
				zX1 = 211792.508, zY1 = 478006.485;
				zX2 = 211795.166, zY2 = 478003.46;  //항공지표 좌표값

				pX1 = 3455, pY1 = 5411;
				pX2 = 4284, pY2 = 6380;    //항공지표 픽셀값  가로/세로 


				RcX0 = 0;
				RcY0 = 0;
				RcXA = RcXa;
				RcYA = RcYa;   // 이미지 크기 XY


				sX = (zX2 - zX1) / (pX2 - pX1);
				sY = (zY2 - zY1) / (pY2 - pY1);  //1픽셀당 좌표 값

				cvCircle(Img_rgb, cvPoint(cvRound(0), cvRound(0)), 5, CVX_RED, CV_FILLED);
				cvCircle(Img_rgb, cvPoint(cvRound(1152), cvRound(648)), 5, CVX_RED, CV_FILLED);
				for (y_order = 0; y_order <= 3; y_order++)
				{
					for (x_order = 0; x_order <= 3; x_order++)
					{
						if (x_order + y_order > 3)
							continue;

						M = cvGetSpatialMoment(&moments, x_order, y_order);

						if (x_order == 0 && y_order == 0)
							m00 = M;
						else if (x_order == 1 && y_order == 0)
							cX = M;
						else if (x_order == 0 && y_order == 1)
							cY = M;
					}
				}


				cX /= m00;
				cY /= m00;

				string color;
				if (testcount % 3 == 0) {
					cvCircle(Img_rgb, cvPoint(cvRound(cX), cvRound(cY)), 5, CVX_WHITE, CV_FILLED);   // 중심점 찍어주기
					color = "white";
				}
				else if (testcount % 3 == 1) {
					cvCircle(Img_rgb, cvPoint(cvRound(cX), cvRound(cY)), 5, CVX_RED, CV_FILLED);   // 중심점 찍어주기
					color = "red";
				}
				else {
					cvCircle(Img_rgb, cvPoint(cvRound(cX), cvRound(cY)), 5, CVX_BLUE, CV_FILLED);   // 중심점 찍어주기
					color = "blue";
				}


				printf("point No:%d\n", testcount);
				printf("X = %d Y= %d \n ", cvRound(cX), cvRound(cY));  // 중간점의 픽셀 
				RcX = cvRound(cX);
				RcY = cvRound(cY);
				printf("RcX = %.0f , RcY= %.0f \n ", RcX, RcY);  //중간점 더블형
				printf("SX = %.3f , SY= %.3f \n ", sX, sY);    //1픽셀당 좌표변화량
				printf("CX1 = %.3f , CY1= %.3f \n ", sX*(RcX - pX1), sY*(RcY - pY1));  //항공좌표점 기준 좌표값 변화량 
				printf("zX0 = %.3f , zY0= %.3f \n\n\n ", zX1 + (sX*(RcX0 - pX1)), zY1 + (sY*(RcY0 - pY1)));  //중간점 좌표값
				printf("zXA = %.3f , zYA= %.3f \n\n\n ", zX1 + (sX*(RcXA - pX1)), zY1 + (sY*(RcYA - pY1)));  //중간점 좌표값
				printf("zX = %.3f , zY= %.3f \n\n\n ", zX1 + (sX*(RcX - pX1)), zY1 + (sY*(RcY - pY1)));  //중간점 좌표값

				fprintf(out, "%.3f,%.3f,%.0f,%.0f,%s,%d ,%s \n", zY1 + (sY*(RcY - pY1)), zX1 + (sX*(RcX - pX1)), RcY, RcX, Point_name, testcount, color);  //좌표 입력  좌표 Y,X ,Pix_Y,Pix,X , 이름

				testcount++;
			}
		}


	}

	cvReleaseMemStorage(&m_storage);
	cvReleaseMemStorage(&m_storage2);


	cvNamedWindow("test", WINDOW_NORMAL);
	cvShowImage("test", Img_rgb);
	
	Mat imgOriginal;

	imgOriginal = cvarrToMat(Img_rgb);
	


	imwrite("circle_center.jpg", imgOriginal);
	c = cvWaitKey(0);

	return imgOriginal;
}