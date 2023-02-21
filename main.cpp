#include"opencv2/opencv.hpp"

#include<iostream>

using namespace cv;
using namespace std;
int scale_Col = 0;
int contr_cnt = 0;

Mat colorConv(Mat input) {
    Mat mat = input.clone();
    int nRows = mat.rows;
    int nCols = mat.cols;
    int i, j;
    float sum = 0;
    float greater = 0.00;
    float difference = 0;
    for (i = 0; i < nRows; i++) {
        for (j = 0; j < nCols; j++) {
            sum = (((float) mat.at < Vec3b > (i, j)[0] / 255) * 0.140209042551032500) + (((float) mat.at < Vec3b > (i, j)[1] / 255) * 0.587043074451121360) + (((float) mat.at < Vec3b > (i, j)[2] / 255) * 0.298936021293775390);
            if ((((float) mat.at < Vec3b > (i, j)[0] / 255)) > (((float) mat.at < Vec3b > (i, j)[1] / 255)))
                greater = (((float) mat.at < Vec3b > (i, j)[0] / 255));
            else
                greater = (((float) mat.at < Vec3b > (i, j)[1] / 255));
            difference = sum - greater;
            if ((difference >= (0.033)) && (difference <= (0.13))) {
                mat.at < Vec3b > (i, j)[0] = 255;
                mat.at < Vec3b > (i, j)[1] = 255;
                mat.at < Vec3b > (i, j)[2] = 255;
            } else {
                mat.at < Vec3b > (i, j)[0] = 0;
                mat.at < Vec3b > (i, j)[1] = 0;
                mat.at < Vec3b > (i, j)[2] = 0;
            }
        }
    }
    return mat;
}

Mat scalingImage(Mat input) {
    Mat scaling, grey, thrshld;
    vector < vector < Point > > contours;
    vector < vector < Point > > contours_ns;
    scaling = input.clone();
    Mat scaled_dst = input.clone();
    int nRows = scaling.rows;
    scale_Col = nRows;
    int nCols = scaling.cols;
    int channels = scaling.channels();
    int k = (int) scaling.total();
    int m = nRows;
    int b = 0;
    Mat greyBlurred;
    vector < Vec4i > hierarchy;
    uchar * p = scaling.ptr < uchar > (0);
    while (m > 0) {
        b = k - nCols;
        for (int i = k; i > b; i--) {
            p[(i * channels) + 0] = 0;
            p[(i * channels) + 1] = 0;
            p[(i * channels) + 2] = 0;
        }
        Mat scaledImage = scaling.clone();
        cv::cvtColor(scaledImage, grey, cv::COLOR_BGR2GRAY);
        GaussianBlur(grey, greyBlurred, Size(3, 3), 0);
        int erosion_type = MORPH_RECT;
        Mat erosion_dst, dilet_dst;
        int erosion_size = 1;
        Mat element = getStructuringElement(erosion_type,
            Size(2 * erosion_size + 1, 2 * erosion_size + 1),
            Point(erosion_size, erosion_size));
        /// Apply the erosion operation
        erode(greyBlurred, erosion_dst, element);
        dilate(erosion_dst, dilet_dst, element);
        int j = 0;
        Canny(dilet_dst, thrshld, 100, 200, 3);
        findContours(thrshld, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
        for (int i = 0; i < contours.size(); i++) {
            if (contours[i].size() > 30) {
                j++;
                if (j > contr_cnt) {
                    contr_cnt = j;

                    scale_Col = m;
                    cout << "Contour count" << contr_cnt << endl;
                }
            }
        }
        k = b;
        m--;
    }
    cout << scale_Col << endl;
    uchar * q = scaled_dst.ptr < uchar > (0);
    m = nRows;
    int l = (int) scaled_dst.total();
    while (m >= scale_Col) {
        b = l - nCols;
        for (int i = l; i > b; i--) {
            q[(i * channels) + 0] = 0;
            q[(i * channels) + 1] = 0;
            q[(i * channels) + 2] = 0;
        }
        l = b;
        m--;
    }
    return scaled_dst;
}

Mat mySkinDetector(Mat cameraFeed) {

    Mat grey, thrshld, scale_dstn;
    double height = 0;
    double width = 0;
    Mat greyBlurred;
    Mat fina_image = cameraFeed.clone();
    vector < vector < Point > > contours;
    vector < Vec4i > hierarchy;
    vector < vector < Point > > contoursNew;
    Mat linearImage = colorConv(cameraFeed);
    Mat scaling = linearImage.clone();
    scale_dstn = scalingImage(scaling);
    cv::cvtColor(scale_dstn, grey, cv::COLOR_BGR2GRAY);
    GaussianBlur(grey, greyBlurred, Size(3, 3), 0);
    int erosion_type = MORPH_RECT;
    Mat erosion_dst, dilet_dst;
    int erosion_size = 1;
    Mat element = getStructuringElement(erosion_type,
        Size(2 * erosion_size + 1, 2 * erosion_size + 1),
        Point(erosion_size, erosion_size));

    // Apply the erosion operation
    erode(greyBlurred, erosion_dst, element);
    dilate(erosion_dst, dilet_dst, element);
    Canny(dilet_dst, thrshld, 100, 200, 3);
    findContours(thrshld, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));

    vector < RotatedRect > minRect(contours.size());
    for (size_t i = 0; i < contours.size(); i++)
        minRect[i] = minAreaRect(Mat(contours[i]));
    for (size_t i = 0; i < contours.size(); i++) {
        if (contours[i].size() > 180) { //we only want to keep big contour
            Scalar color = Scalar(0, 0, 0);
            height = minRect[i].size.height;
            width = minRect[i].size.width;
            Point2f rect_points[4];
            minRect[i].points(rect_points);
            //drawing rectangle
            if (height > width) {
                minRect[i].size.height = (float)(0.33) * minRect[i].size.height;
                minRect[i].center = (rect_points[1] + rect_points[2]) / 2 + (rect_points[0] - rect_points[1]) / 6;
            } else {
                minRect[i].size.width = (float)(0.40) * minRect[i].size.width;
                minRect[i].center = (rect_points[2] + rect_points[3]) / 2 + (rect_points[0] - rect_points[3]) / 6;
            }
            minRect[i].points(rect_points);
            for (int j = 0; j < 4; j++)
                line(fina_image, rect_points[j], rect_points[(j + 1) % 4], color, 2, 8);
        }
    }
    return fina_image;
}

int main() {
    /*VideoCapture capture;
     capture.open(0);
     capture.set(CV_CAP_PROP_FRAME_WIDTH,320);
     capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);*/
    Mat cameraFeed;
    Mat skinMat;
    cameraFeed = imread("hands_full.jpg");

    // while(1){

    // capture.read(cameraFeed);
    imshow("original", cameraFeed);
    skinMat = mySkinDetector(cameraFeed);
    imshow("Fingertip Detected", skinMat);
    waitKey(0);
    //}
    return 0;
}
