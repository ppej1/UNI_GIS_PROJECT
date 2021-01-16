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

// opencv ���� ����
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

	//csv ���� ����

	

	// �ܰ��� ���� �� �ٻ�ȭ ���� �ʱ�ȭ
	CvMemStorage*   m_storage = cvCreateMemStorage(0);      // �迭 �ڷ�(���� ��ǥ�� ����)
	CvMemStorage*   m_storage2 = cvCreateMemStorage(0);      // �迭 �ڷ�(���� ��ǥ�� ����)
	CvSeq*         m_seq = 0;                        // ��� ����� ������ ����
	CvSeq*         m_approxDP_seq = 0;
	CvSeq*         m_dominant_points = 0;               // Ư¡�� ã�� ���� ����

														// ConvexHull ���� �ʱ�ȭ   
	int counterConvexHull;
	CvSeq* hull = 0;
	CvSeq* ptseq;
	CvSeq* defect = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);

	// moment ���� ����
	CvMoments moments;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// (1) �ܰ��� ����
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cvFindContours(Img_Gray,
		m_storage,
		&m_seq,
		sizeof(CvContour),
		CV_RETR_EXTERNAL,
		CV_CHAIN_APPROX_NONE,
		cvPoint(0, 0));
	//cvDrawContours(Img_rgb, m_seq, CVX_RED, CVX_RED, 1,1,8 );               // (Test) �ܰ��� ������ �ߵǾ����� �׽�Ʈ




	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// (2) �ܰ��� �ٻ�ȭ
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (m_seq != 0)
	{
		m_approxDP_seq = cvApproxPoly(m_seq,
			sizeof(CvContour),
			m_storage,
			CV_POLY_APPROX_DP,
			3,
			1);
		//cvDrawContours(Img_rgb, m_approxDP_seq, CVX_RED, CVX_RED, 1,1,8 );   // (Test) �ܰ��� �ٻ�ȭ�� �ߵǾ����� �׽�Ʈ

		int testcount = 0;                                          // �� ���� ����Ʈ���� ���� �ٸ��� ��µǴ��� Ȯ������ ����
		for (CvSeq* c = m_approxDP_seq; c != NULL; c = c->h_next)         // ������ ��ũ�帮��Ʈ�� ��ȸ�ϸ鼭 �� �����鿡���ؼ� ����Ѵ�.
		{
			ptseq = cvCreateSeq(CV_SEQ_KIND_GENERIC | CV_32SC2, sizeof(CvContour), sizeof(CvPoint), m_storage);

			if ((c->total >= p_min) && (c->total <= p_max))                                     // �ܰ����� �̷�� ���� ������ �̰ͺ��� �̸��̸� �����̶�� �Ǵ�
			{
				for (int i = 0; i < c->total; ++i)
				{
					CvPoint* p = CV_GET_SEQ_ELEM(CvPoint, c, i);            // CvSeq�κ�����ǥ������.
					CvPoint temp;
					temp.x = p->x;
					temp.y = p->y;

					cvSeqPush(ptseq, &temp);                           // ���齺���� ���ϱ����� ��ǥ ����

																	   //// (Test) ��ũ�帮��Ʈ�� ��ȸ�ϸ鼭 ���� ���ﳪ �׽�Ʈ
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

				// �������� �������� ���̰����� ū �κ� ���ϱ� ���� ���� ����(p1,p2 �� �߰��� mid)
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

						cvCircle(Img_rgb, *Item[i].start, 2, CVX_RED, CV_FILLED);            // ������ �������� ���� ��(�ܰ���)�׸���
						cvCircle(Img_rgb, *Item[i].depth_point, 2, CVX_GREEN, CV_FILLED);      // ������ �������� ���� ��(�������̰� ������)�׸���
						cvLine(Img_rgb, p1, p2, CVX_RED, 1, 8, 0);                           // �������� �ܰ��� �׸���
						cvLine(Img_rgb, mid, *Item[i].depth_point, CVX_GREEN, 1, 8, 0);         // �������� �ܰ����� �������������� ���� ���� ����
					}
					free(Item);
				}

				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				// (4) cvMoment �� �߽��� ���
				//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				cvMoments(c, &moments);

				double M;
				int x_order, y_order;
				double cX, cY, m00;
				double RcX, RcY, zX1, zY1, pX1, pY1, zX2, zY2, pX2, pY2, sX, sY, RcX0, RcXA, RcY0, RcYA;
				zX1 = 211792.508, zY1 = 478006.485;
				zX2 = 211795.166, zY2 = 478003.46;  //�װ���ǥ ��ǥ��

				pX1 = 3455, pY1 = 5411;
				pX2 = 4284, pY2 = 6380;    //�װ���ǥ �ȼ���  ����/���� 


				RcX0 = 0;
				RcY0 = 0;
				RcXA = RcXa;
				RcYA = RcYa;   // �̹��� ũ�� XY


				sX = (zX2 - zX1) / (pX2 - pX1);
				sY = (zY2 - zY1) / (pY2 - pY1);  //1�ȼ��� ��ǥ ��

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
					cvCircle(Img_rgb, cvPoint(cvRound(cX), cvRound(cY)), 5, CVX_WHITE, CV_FILLED);   // �߽��� ����ֱ�
					color = "white";
				}
				else if (testcount % 3 == 1) {
					cvCircle(Img_rgb, cvPoint(cvRound(cX), cvRound(cY)), 5, CVX_RED, CV_FILLED);   // �߽��� ����ֱ�
					color = "red";
				}
				else {
					cvCircle(Img_rgb, cvPoint(cvRound(cX), cvRound(cY)), 5, CVX_BLUE, CV_FILLED);   // �߽��� ����ֱ�
					color = "blue";
				}


				printf("point No:%d\n", testcount);
				printf("X = %d Y= %d \n ", cvRound(cX), cvRound(cY));  // �߰����� �ȼ� 
				RcX = cvRound(cX);
				RcY = cvRound(cY);
				printf("RcX = %.0f , RcY= %.0f \n ", RcX, RcY);  //�߰��� ������
				printf("SX = %.3f , SY= %.3f \n ", sX, sY);    //1�ȼ��� ��ǥ��ȭ��
				printf("CX1 = %.3f , CY1= %.3f \n ", sX*(RcX - pX1), sY*(RcY - pY1));  //�װ���ǥ�� ���� ��ǥ�� ��ȭ�� 
				printf("zX0 = %.3f , zY0= %.3f \n\n\n ", zX1 + (sX*(RcX0 - pX1)), zY1 + (sY*(RcY0 - pY1)));  //�߰��� ��ǥ��
				printf("zXA = %.3f , zYA= %.3f \n\n\n ", zX1 + (sX*(RcXA - pX1)), zY1 + (sY*(RcYA - pY1)));  //�߰��� ��ǥ��
				printf("zX = %.3f , zY= %.3f \n\n\n ", zX1 + (sX*(RcX - pX1)), zY1 + (sY*(RcY - pY1)));  //�߰��� ��ǥ��

				fprintf(out, "%.3f,%.3f,%.0f,%.0f,%s,%d ,%s \n", zY1 + (sY*(RcY - pY1)), zX1 + (sX*(RcX - pX1)), RcY, RcX, Point_name, testcount, color);  //��ǥ �Է�  ��ǥ Y,X ,Pix_Y,Pix,X , �̸�

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