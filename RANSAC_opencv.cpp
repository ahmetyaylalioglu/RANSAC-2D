// RANSAC_opencv.cpp 
// Ahmet Yaylalioglu


#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <random>
#include <cstdint>
#include <unordered_set>
#include <vector>

//hold inliers and outliers which by created this function
std::vector<cv::Point> totalPoints;

double Slope(int x0, int y0, int x1, int y1)
{
	return (y1 - y0) / (x1 - x0);
}

void createLine(cv::Mat& img, cv::Point p1, cv::Point p2, cv::Scalar color, int lineWidth)
{
	double slope = Slope(p1.x, p1.y, p2.x, p2.y);

	cv::Point a(0, 0);
	cv::Point b(img.cols, img.rows);

	a.y  = p1.y - (p1.x - a.x) * slope;
	b.y  = p2.y - (p2.x - b.x) * slope;


	cv::line(img, a, b, color, lineWidth, cv::LINE_AA, 0);
 
}

std::vector<cv::Point> createData(int inlierNum,int outlierNum,int imageSize) //for creating useful data for my RANSAC implementation
{
	
	//create inlier data for testing purpose
	std::random_device Seed_Device;
	std::mt19937 RNG = std::mt19937(Seed_Device());
	std::uniform_int_distribution<int> UniDist(0, imageSize - 1);
	int Perturb = 10;
	std::normal_distribution<double> PerturbDist(0, Perturb);

	for (int i = 0; i < inlierNum; i++) 
	{
	    int Diag = UniDist(RNG);
		cv::Point p(floor(Diag + PerturbDist(RNG)), floor(Diag + PerturbDist(RNG)));
		totalPoints.push_back(p);
	}

	//create outlier data
	for (int i = 0; i < outlierNum; i++)
	{
		double rx = 500 * (((double)rand() / (RAND_MAX)) - 0.5);
		double ry = 500 * (((double)rand() / (RAND_MAX)) - 0.5);
		cv::Point p;
		p.x = 10 * rx;
		p.y = 10 * ry;
		if (p.x < 0)
			p.x = -1 * p.x;
		if (p.y < 0)
			p.y = -1 * p.y;
		totalPoints.push_back(p);
	}

	return totalPoints;
}

std::unordered_set<int> executeRANSAC(std::vector<cv::Point> data, float distanceTolerance, int maxIteration)
{
	std::unordered_set<int> bestInliers;
	for (int i = 0; i < maxIteration; i++)
	{
		std::unordered_set<int> inliers;

		//for line we need 2 points, select points randomly
		while (inliers.size() < 2)
		{
			inliers.insert(rand() % (totalPoints.size()));
		}
		
		float x1, y1, x2, y2;

		auto iterator = inliers.begin();
		x1 = totalPoints[*iterator].x;
		y1 = totalPoints[*iterator].y;

		iterator++;

		x2 = totalPoints[*iterator].x;
		y2 = totalPoints[*iterator].y;

		float a = (y1 - y2);
		float b = (x2 - x1);
		float c = (x1 * y2) - (x2 * y1);

		for (int index = 0; index < totalPoints.size();index++)
		{
			if (inliers.count(index) > 0)
				continue;

			cv::Point point = totalPoints[index];
			float x3 = point.x;
			float y3 = point.y;
			float distance = fabs(a*x3 + b * y3 + c) / sqrt(a*a + b*b);

			if (distance <= distanceTolerance)
			{
				inliers.insert(index);
			}

		}

		if (inliers.size() > bestInliers.size())
		{
			bestInliers = inliers;
		}
	}

	return bestInliers;
    
}

int main()
{
	int size = 1000;
	int nPoints = 100;
	cv::Mat screen(size, size, CV_8UC3);
	screen.setTo(255);
	std::vector<cv::Point> data = createData(400, 2500,size);
	std::unordered_set<int> inliersSet = executeRANSAC(data, 7.0, 20);
	for (int i = 0; i < data.size(); i++)
	{
		if (inliersSet.count(i))
		{
			cv::circle(screen, data[i], 6, cv::Scalar(0, 255, 0), cv::FILLED, cv::LINE_8);
			
		}
		else
		{
			cv::circle(screen, data[i], 4, cv::Scalar(0, 0, 255), cv::FILLED, cv::LINE_8);
		}
		
		
	}

	cv::imshow("RANSAC", screen);
	cv::waitKey(0);
	cv::destroyAllWindows();
	return 0;
}
