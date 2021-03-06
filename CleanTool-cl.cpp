// Command-line Clean Tool
// by Steven Wirsz

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;
const char* window_name = "Image Cleaner";
Mat orig, bw, src, dst;

int bilateral_value = 0;
int blur_value = 0;
int cstretch_value = 0;
int cpstretch_value = 0;
int csmooth_value = 0;
int noise_value = 0;
int blackzero_value = 0;
int white_value = 255;

void bilateral( int, void* );
void blur( int, void* );
void cstretch( int, void* );
void cpstretch( int, void* );
void csmooth( int, void* );
void noise( int, void* );
void blackzero( int, void* );
void whitezero( int, void* );

int main( int, char** argv )
{
  src = imread( argv[1], 1 );
  orig = src;
  namedWindow( window_name, WINDOW_AUTOSIZE );
  createTrackbar( "Bilateral", window_name, &bilateral_value, 20, bilateral );
  createTrackbar( "Blur", window_name, &blur_value, 30, blur );
  createTrackbar( "C.Stretch1", window_name, &cpstretch_value, 100, cpstretch );
  createTrackbar( "C.Stretch2", window_name, &cpstretch_value, 100, cpstretch );
  createTrackbar( "C.Smooth", window_name, &csmooth_value, 100, csmooth );
  createTrackbar( "G.noise", window_name, &noise_value, 100, noise);
  createTrackbar( "Black->0", window_name, &blackzero_value, 255, blackzero );
  createTrackbar( "White->0", window_name, &white_value, 255, whitezero );


  noise( 0, 0 );
  waitKey(); // pause before clearing images
}

void bilateral( int, void* )
{
	bilateralFilter( src, dst, bilateral_value*2+1, bilateral_value*4+1, bilateral_value+1 );

	imshow( window_name, dst );
}

void blur( int, void* )
{
	GaussianBlur( src, dst, Size( blur_value*2+1, blur_value*2+1 ), 0, 0 );

	imshow( window_name, dst );
}

void cstretch( int, void* )
{
	double maxval[3] = {0, 0, 0};
	double minval[3] = {255, 255, 255}; // Stores min and max values
  	// Traverse through image, find minimum and maximum
	for( int y = 0; y < src.rows; y++ ) { 
		for( int x = 0; x < src.cols; x++ ) { 
			for( int c = 0; c < 3; c++ ) {
				if (minval[c]>saturate_cast<uchar>(src.at<Vec3b>(y,x)[c]))
					minval[c] = saturate_cast<uchar>(src.at<Vec3b>(y,x)[c]);
				if (maxval[c]<saturate_cast<uchar>(src.at<Vec3b>(y,x)[c]))
					maxval[c] = saturate_cast<uchar>(src.at<Vec3b>(y,x)[c]);
			}
		}
	}
    
  threshold( src, dst, 255, 0, 4 );
	for( int y = 0; y < src.rows; y++ ) { 
		for( int x = 0; x < src.cols; x++ ) { 
			for( int c = 0; c < 3; c++ ) {
		// Create new_image by shifting minimum contrast down to zero and multiplying to stretch maximum contrast to 255
				double shift = -(minval[c])*((double)cstretch_value/100);
				double scale = 1+((255/(maxval[c]-minval[c]))-1)*((double)cstretch_value/100);
				
				dst.at<Vec3b>(y,x)[c] = saturate_cast<uchar>(scale*( src.at<Vec3b>(y,x)[c] + shift) );
			}
		}
	}
	
	imshow( window_name, dst );
}

void cpstretch( int, void* )
{	

   cv::cvtColor( src, bw, cv::COLOR_BGR2GRAY ); // create greyscale image
   double maxval = 0;
   double minval = 255; // Stores minimum and maximum grayscale values

	// Traverse through image, find minimum and maximum
	for( int y = 0; y < bw.rows; y++ ) {
		for( int x = 0; x < bw.cols; x++ ) {
			if (minval>cv::saturate_cast<uchar>(bw.at<cv::Vec3b>(y,x)[0]))
				minval = cv::saturate_cast<uchar>(bw.at<cv::Vec3b>(y,x)[0]);
			if (maxval<cv::saturate_cast<uchar>(bw.at<cv::Vec3b>(y,x)[0]))
				maxval = cv::saturate_cast<uchar>(bw.at<cv::Vec3b>(y,x)[0]);
		}
	}

	for( int y = 0; y < src.rows; y++ ) {
		for( int x = 0; x < src.cols; x++ ) {
			for( int c = 0; c < 3; c++ ) {
		// Create new_image by shifting minimum contrast down to zero and multiplying to stretch maximum contrast to 255
				double shift = -(minval)*((double)cpstretch_value/100);
				double scale = 1+((255/(maxval-minval))-1)*((double)cpstretch_value/100);

				dst.at<Vec3b>(y,x)[c] = saturate_cast<uchar>(scale*( src.at<Vec3b>(y,x)[c] + shift) );
			}
		}
	}
	
	imshow( window_name, dst );
}

void csmooth( int, void* )
{
  threshold( src, dst, 255, 0, 4 );
	for( int y = 2; y < src.rows-2; y++ ) { 
		for( int x = 2; x < src.cols-2; x++ ) { 
			for( int c = 0; c < 3; c++ ) {
				// 75% of image contrast from original pixel
				dst.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( (src.at<Vec3b>(y,x)[c])*(1-(float)csmooth_value/100 ));
				// 25% of image contrast mean of surrounding 25 pixels (5x5)
				for ( int a = 0; a < 5; a++ ) {
					for ( int b = 0; b < 5; b++ ) {
						dst.at<Vec3b>(y,x)[c] += saturate_cast<uchar>( (src.at<Vec3b>(y-2+a,x-2+b)[c])/25*(float)csmooth_value/100);
					}
				}
			}
		}
	}
	imshow( window_name, dst );
}


void noise( int, void* )
{
	threshold( src, dst, 255, 0, 4 );
	dst = src.clone();
	Mat noisyI;
	noisyI.create (dst.rows,dst.cols,CV_32FC(1));
	noisyI.setTo (cv::Scalar::all (0));

	vector <Mat>  _channel;
	dst.convertTo (dst,CV_32FC(3),1.0,0);
	cv::split(dst,_channel);
	for(int i=0;i<dst.channels ();i++)
	{
		randn(noisyI, Scalar::all(0), Scalar::all((double)noise_value)/2);
		add(_channel[i],noisyI,_channel[i]);
	}

	cv::merge (_channel,dst);
	dst.convertTo (dst,CV_8UC(3),1.0,0);

	imshow( window_name, dst );
}

void whitezero( int, void* )
{
  threshold( src, dst, white_value, 0, 2 );
  imshow( window_name, dst );
}


void blackzero( int, void* )
{
  threshold( src, dst, blackzero_value, 0, 3 );
  imshow( window_name, dst );
}
