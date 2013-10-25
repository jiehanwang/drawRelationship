// drawRelationship.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<string>
#include <opencv2\opencv.hpp>
#include<atlstr.h>
#include<iostream>
#include<fstream>
using namespace std;
using namespace cv;

#define maxClassNum 1000
#define LRB 3 
#define HOG_dimension 324//1764//720

int            classNum[LRB];  //The class number of left, right, both postures. 
float          postureC[LRB][maxClassNum][HOG_dimension];
float          postureMatrix[LRB][maxClassNum][maxClassNum];
vector<IplImage*> myImage[LRB]; 
IplImage* imageData[1000]; 


void readstr(FILE *f,char *string)
{
	do
	{
		fgets(string, HOG_dimension*10, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
	return;
}

void readstr2(FILE *f,char *string)
{
	do
	{
		fgets(string, maxClassNum*10, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
	return;
}

void readInPostureC(CString route, int lrb)
{
	FILE *filein;
	char oneline[HOG_dimension*10];
	int itemNumber;
	filein = fopen(route, "rt");

	readstr(filein,oneline);
	sscanf(oneline, "NUMBER %d\n", &itemNumber);
	classNum[lrb] = itemNumber;
	for (int loop = 0; loop < itemNumber; loop++)
	{
		readstr(filein,oneline);
		char* sp = oneline; 
		float num; 
		int read; 
		int dimensionIndex = 0;
		while( sscanf(sp, "%f %n", &num, &read)!=EOF )
		{ 
			postureC[lrb][loop][dimensionIndex++] = num;
			sp += read-1; 
		} 
	}
	fclose(filein);
}

void readInPostureMatrix(CString route, int lrb)
{
	FILE *filein;
	char oneline[maxClassNum*10];
	int itemNumber;
	filein = fopen(route, "rt");

	itemNumber = classNum[lrb];
	for (int loop = 0; loop < itemNumber; loop++)
	{
		readstr2(filein,oneline);
		char* sp = oneline; 
		float num; 
		int read; 
		int classIndex = 0;
		while( sscanf(sp, "%f %n", &num, &read)!=EOF )
		{ 
			postureMatrix[lrb][loop][classIndex++] = num;
			sp += read-1; 
		} 
	}
	fclose(filein);
	//delete[] oneline;
}

void imageStick(IplImage* backGround, IplImage* handImage, CvPoint Position)
{
	int width_hand = handImage->width;
	int height_hand = handImage->height;

	CvPoint iPosition;
	iPosition.y = Position.y - height_hand/2;
	iPosition.x = Position.x- width_hand/2;


	for (int j=iPosition.y; j<iPosition.y + height_hand; j++)
	{
		uchar* back_ptr = (uchar*)(backGround->imageData + j*backGround->widthStep);
		uchar* hand_ptr = (uchar*)(handImage->imageData + (j-iPosition.y)*handImage->widthStep);
		for (int i=iPosition.x; i<iPosition.x + width_hand; i++)
		{
			if (hand_ptr[3*(i-iPosition.x) + 0]>0 && hand_ptr[3*(i-iPosition.x) + 1]>0 && hand_ptr[3*(i-iPosition.x) + 2]
			&& back_ptr[3*i +0] <70 && back_ptr[3*i +1]<70 && back_ptr[3*i +2]<70)
			{
				back_ptr[3*i +0] = hand_ptr[3*(i-iPosition.x) + 0];
				back_ptr[3*i +1] = hand_ptr[3*(i-iPosition.x) + 1];
				back_ptr[3*i +2] = hand_ptr[3*(i-iPosition.x) + 2];
			}
			
		}
	}


}

int _tmain(int argc, _TCHAR* argv[])
{
	//////////////////////////////////////////////////////////////////////////
	cout<<"Reading data..."<<endl;
	readInPostureC("..\\input\\postureC_0.txt",0); //Left posture
	readInPostureC("..\\input\\postureC_1.txt",1); //Right posture
	readInPostureC("..\\input\\postureC_2.txt",2); //Both posture
	readInPostureMatrix("..\\input\\postureMatrix_0.txt",0);
	readInPostureMatrix("..\\input\\postureMatrix_1.txt",1);
	readInPostureMatrix("..\\input\\postureMatrix_2.txt",2);

	int imageCount = 0;
	for (int i=0; i<LRB; i++)
	{
		int iClassNum = classNum[i];
		for (int j=0; j<iClassNum; j++)
		{
			CString fileName;
			fileName.Format("..\\input\\%d\\mean_%d.jpg",i, j);
			imageData[imageCount] = cvLoadImage(fileName,1);
			myImage[i].push_back(imageData[imageCount]);
			imageCount++;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	int height_back = 700;
	int width_back = 700;
	int width_hand = myImage[0][0]->width;
	int height_hand = myImage[0][0]->height;

	IplImage* background = cvCreateImage(cvSize(height_back,width_back),8,3);
	for (int j=0; j<height_back; j++)
	{
		uchar* src_ptr = (uchar*)(background->imageData + j*background->widthStep);
		for (int i=0; i<width_back; i++)
		{
			src_ptr[3*i + 0]= 0;
			src_ptr[3*i + 1]= 0;
			src_ptr[3*i + 2]= 0;
		}
	}

	int lrb = 0;
	vector<CvPoint> Positions;
	for (int i=0; i<classNum[lrb]; i++)
	{

		int myRandX =  rand() % (width_back-width_hand/2 + 1 - width_hand/2) + width_hand/2;
		int myRandY =  rand() % (height_back-height_hand/2 + 1 - height_hand/2) + height_hand/2;
		CvPoint temp;
		temp.x = myRandX;
		temp.y = myRandY;
		Positions.push_back(temp);		
		if (i==0 || i==21)
		{
			continue;
		}
		imageStick(background, myImage[lrb][i],cvPoint(myRandX,myRandY));

	}
	

	for (int i=0; i<classNum[lrb]; i++)
	{
		if (i==0 || i==21)
		{
			continue;
		}
		for (int j=i+1; j<classNum[lrb]; j++)
		{
			if (postureMatrix[lrb][i][j]>0)
			{

				cvLine(background,Positions[i],Positions[j],cvScalar(225,225,225),1,8,0);
			}
		}
	}
	

// 	int center = 3;
// 	int x_center = 500;
// 	int y_center = 500;
// 	imageStick(background, myImage[lrb][center],cvPoint(x_center,y_center));
// 
// 	for (int i=0; i<classNum[lrb]; i++)
// 	{
// 		if (postureMatrix[lrb][center][i]>0)
// 		{
// 			int length = postureMatrix[lrb][center][i];
// 			CvPoint p1; 
// 			p1.x = x_center + length*100; 
// 			p1.y = y_center + length*100;
// 			imageStick(background, myImage[lrb][i],p1);
// 			cvLine(background,cvPoint(x_center-width_hand/2,y_center-height_hand/2),
// 				cvPoint(p1.x-width_hand/2,p1.y-height_hand/2),cvScalar(0,0,0),1,8,0);
// 		}
// 	}


	cvSaveImage("relationship.jpg",background);

	for (int i=0; i<1000; i++)
	{
		cvReleaseImage(&imageData[i]);
	}
	cout<<"Done!"<<endl;
	getchar();
	return 0;
}

