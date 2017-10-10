// FLIR_file_SDK.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
string createIRImage(UInt16	*frame, UInt16 width, UInt16 height, const wchar_t* fname);
vector <Mat> DetectBlobs(string filePath, VideoWriter outPutVideo);

bool RecFilePath(const fs::path& path) {
	string str=path.string();
	wstring wstemp;
	if ((str.find("Rec") != string::npos) && (str.find(".ats") != string::npos))
	{
		return 1;
	}
	else {
		return 0;
	}
		
}
//const wchar_t* fname = L"C:/Users/RDCRLGRH/Documents/Snow Friction/IR data/170515_tribometer/Rec-000421.ats"; //File to open


int main()
{
	//string rootdir = "G:/IR/6_1_snowTribometer/";
	string rootdir = "G:/testing/";

	string temppath;
	for (auto& p : fs::recursive_directory_iterator(rootdir)) {
		//Recpath = RecFilePath(p);
		if (RecFilePath(p)) {
			temppath = p.path().string();
			cout << temppath << endl;
			wstring wstemp(temppath.begin(), temppath.end());
			const wchar_t* fname = wstemp.c_str();
		
			wstring ws(fname);
			string rootpath(ws.begin(), ws.end());
			size_t n = rootpath.find_last_of('/');
			string filename = rootpath.substr(n + 1);
			filename.resize(filename.size() - 4);
			rootpath.resize(n + 1);

			VideoWriter outputVideo;
			outputVideo.open(rootpath + filename + "outVid.wmv", CV_FOURCC('W', 'M', 'V', '2'), 10, Size(640, 512), true);
			ofstream outputFile;

			outputFile.open(rootpath + filename + "outData.csv");
			CImagerFile mFile; // file object


		//const wchar_t* fname = L"C:/Users/RDCRLGRH/Documents/Snow Friction/IR data/170512_tribometer/Rec-000410.ats"; //File to open
			if (!mFile.Open(fileSystem(), fname, NULL))
				return -1;

			UInt16	mImageWidth, mImageHeight;
			UInt32	mFrameNumber;
			UInt32	mNumFrames;
			EUnit	mUnit;
			ETempType	mTempType;
			EDataType dataType;

			mImageWidth = mFile.width(); //Get File width
			mImageHeight = mFile.height();//Get File Height
			mNumFrames = mFile.numFrames(); // Get number of frames in file.
			cout << "Number of Frame:" << mNumFrames << endl;
			mFrameNumber = 0; //Frame counter


			dataType = mFile.dataType(); //Data storage type UINT16 
			mUnit = mFile.baseUnit(); //Base unit of image
			mTempType = mFile.baseTempType(); // base units for temperature

			mUnit = unitTemperatureFactory; // Factory setting for temperature calc
			mTempType = TT_Celsius; // Celsius 
			mFile.SetUnit(mUnit, mTempType); //set unit and temp type

			// print some file information
			/*
			printf("Image Size: %ux%u, Num Frames: %u\n", mImageWidth,
				mImageHeight, mNumFrames);
			*/

			//-------------------Begin loop for each FRAME ---------------------//
			//for (mFrameNumber = 1; mFrameNumber < 2; mFrameNumber++) {
			for (mFrameNumber; mFrameNumber < mNumFrames; mFrameNumber++) {
				if (!mFile.GetFrame(mFrameNumber))
					printf("could not get frame");

				string ImageFilePath;


				UInt16	*frame = mFile.adjusted()->uint16();
				UInt16	width = mFile.width(), height = mFile.height(); //width and height used for creating bit map
				ImageFilePath = createIRImage(frame, width, height, fname);

				vector <Mat> FlatTopPixels = DetectBlobs(ImageFilePath, outputVideo);
				//cout << "Size of Vector: " << FlatTopPixels.size() << endl;
				//waitKey(0); // Wait for a keystroke in the window

				//for(int contour = 0; contour <FlatTopPixels.size())
				//cout << "Zero#" << 0 << ": " << nonZeroCoordinates.at<Point>(0).x << ", " << nonZeroCoordinates.at<Point>(0).y << endl;
				int ix, iy;
				vector <double> AverageTemp(FlatTopPixels.size(), 1);

				for (int i = 0; i < FlatTopPixels.size(); i++) {
					Mat contour = FlatTopPixels[i];
					double temp = 0;
					for (int j = 0; j < contour.total(); j++)
					{
						ix = contour.at<Point>(j).x;
						iy = contour.at<Point>(j).y;
						temp = (double)mFile.final()->getAt(iy*mImageWidth + ix) + temp;

					}
					AverageTemp[i] = temp / contour.total();
				}
				double TotalAvgTemp = 0;
				int TotalArea = 0;
				for (int i = 0; i < AverageTemp.size(); i++) {
					//cout << "Contour: " << i << " Avg. Temp: " << AverageTemp[i] << " Area: " << FlatTopPixels[i].total() << endl;
					TotalAvgTemp = AverageTemp[i] + TotalAvgTemp;
					TotalArea = FlatTopPixels[i].total() + TotalArea;
				}

				TotalAvgTemp = TotalAvgTemp / AverageTemp.size();
				//cout << "Frame Number: "<< mFrameNumber << " FlatTop Avg Temp: " << TotalAvgTemp << "Total Area: " << TotalArea << endl;
				if (mFrameNumber % 100 == 0) {
					cout << mFrameNumber << endl;
				}
				outputFile << mFrameNumber << "," << FlatTopPixels.size() << "," << TotalAvgTemp << "," << TotalArea << "," << endl;
				//double temp = (double)mFile.final()->getAt(iy*mImageWidth + ix);
				//cout << "Frame Number: "<< mFrameNumber<<" (" << ix << "," << iy << ") " << temp << endl;


			}

			// close file
			mFile.Close();
			outputFile.close();
			outputVideo.release();
		}
	}
	return 0;
}


string createIRImage(UInt16	*frame,UInt16 width, UInt16 height,const wchar_t* fname) {
												
															//create color pallete to assign bit map
	SRGB	mPalette[256];
	for (int i = 0; i < 256; ++i)
		mPalette[i].r = mPalette[i].g = mPalette[i].b = (BYTE)i;

	//Create blank bitmap
	CBitmap mBitmap;
	mBitmap.CreateDIB(width, height);
	
	//Fill in bitmap based on the color pallete and value of image.

	UInt32	n = width * height;
	UInt32	hist[65536], i;


	for (i = 0; i < 65536; ++i)
		hist[i] = 0;

	for (i = 0; i < n; ++i)
		++hist[frame[i]];

	int			nMax = (int)(n*0.0003);
	UInt16		minValue, maxValue;

	n = 0;
	for (i = 0; i < 65535; ++i)
	{
		n += hist[i];

		if ((int)n > nMax)
		{
			minValue = (WORD)(i + 1);
			break;
		}
	}

	n = 0;
	for (i = 65535; i > 0; --i)
	{
		n += hist[i];

		if ((int)n > nMax)
		{
			maxValue = (WORD)(i - 1);
			break;
		}
	}

	//	simple linear auto scale
	double		scale = 255.0 / (maxValue - minValue);
	int			x, y;

	BYTE		*pBits = mBitmap.GetBits();
	int			pitch = mBitmap.Pitch;

	for (i = 0, y = 0; y < height; ++y)
	{
		for (x = 0; x < width; ++x, ++i)
		{
			BYTE	index = (BYTE)((min(max(minValue, frame[i]), maxValue) - minValue) * scale);
			pBits[(height - (y + 1)) * pitch + x * 3 + 0] = mPalette[index].b;
			pBits[(height - (y + 1)) * pitch + x * 3 + 1] = mPalette[index].g;
			pBits[(height - (y + 1)) * pitch + x * 3 + 2] = mPalette[index].r;
		}
	}


	mBitmap.SetBits();
	
	wstring ws(fname);
	string rootpath(ws.begin(), ws.end());
	size_t loc = rootpath.find_last_of('/');
	string filename = rootpath.substr(loc + 1);
	filename.resize(filename.size() - 4);
	rootpath.resize(loc + 1);

	string temppath = rootpath + filename + "temp.bmp";
	wstring wstemp(temppath.begin(), temppath.end());
	const wchar_t* outputBit = wstemp.c_str();
	//const wchar_t* outputBit = L"C:/Users/RDCRLGRH/Documents/temp.bmp";
	mBitmap.Save(outputBit);
	//wstring ws(outputBit);
	//string str(ws.begin(), ws.end());
	return temppath;
}

vector <Mat> DetectBlobs(string filePath,VideoWriter outPutVideo)
{

	RNG rng(12345);
	Mat image;
	image = imread(filePath, IMREAD_GRAYSCALE); // Read the file
	//imwrite("C:/Users/RDCRLGRH/Documents/Snow Friction/ResultsShow/Original.png",image);

	
	//filter
	Mat imgFiltered = Mat::zeros(image.size(), CV_8UC1);
	bilateralFilter(image, imgFiltered, 6, 75, 75);
	

	//gamma correction
	int gamma = 6;
	Mat imgGammaCorrect = Mat::zeros(imgFiltered.size(), CV_8UC1);
	int nRows = imgFiltered.rows;
	int nCols = imgFiltered.cols;

	int i, j;
	uchar* p;
	for (i = 0; i < nRows; i++) {
		for (j = 0; j < nCols; j++) {
			imgGammaCorrect.at<uchar>(i, j) = pow((imgFiltered.at<uchar>(i, j) / 255), gamma) * 255;
		}
	}

	//contour detection
	Ptr<MSER> detector = MSER::create();
	vector<vector<cv::Point>> mserContours;
	vector<cv::Rect>mserBbox;
	detector->detectRegions(imgGammaCorrect, mserContours, mserBbox);

	Mat MSERRegions = Mat::zeros(imgGammaCorrect.size(), CV_8UC1);
	for (vector<cv::Point> v : mserContours) {
		for (cv::Point p : v) {
			MSERRegions.at<uchar>(p.y, p.x) = 255;
		}
	}
	
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(MSERRegions, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));

	//int border = 1;
	//copyMakeBorder(image, image, border, border,
		//border, border, BORDER_CONSTANT);
		


	findContours(MSERRegions, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	Mat drawing = Mat::zeros(MSERRegions.size(), CV_8UC3);
	Mat drawingOverlay = image.clone();
	for (size_t i = 0; i< contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	}

	

	Mat adrawingOverlay = Mat::zeros(MSERRegions.size(), CV_8UC1);



	for (int i = 0; i < contours.size(); i++) {
		Scalar color = Scalar(255, 255, 255);
		drawContours(drawingOverlay, contours, (int)i, color, -1, 8, hierarchy, 0, Point());
		imshow("drawContours", drawingOverlay);
	}
	
	vector <Mat> FlatTopPixels(contours.size());
	double area = 0;
	
	for (int i = 0; i < contours.size(); i++) {

		Scalar color = Scalar(255, 255, 255);
		area = contourArea(contours[i]) + area;


		Mat cimag = Mat::zeros(MSERRegions.size(), CV_8UC1);

		drawContours(cimag, contours, (int)i, color, -1, 8, hierarchy, 0, Point());
		Mat  nonZeroCoordinates;
		findNonZero(cimag, nonZeroCoordinates);
		FlatTopPixels[i] = nonZeroCoordinates;
		for (int j = 0; j < nonZeroCoordinates.total(); j++)
		{
			int x = nonZeroCoordinates.at<Point>(j).x;
			int y = nonZeroCoordinates.at<Point>(j).y;
		}

	}
	//end temp
	
	cvtColor(drawingOverlay, drawingOverlay, CV_GRAY2BGR);
	for (int i = 0; i < FlatTopPixels.size(); i++) {
		Mat contour = FlatTopPixels[i];
		for (int j = 0; j < contour.total(); j++)
		{
			int x = contour.at<Point>(0).x;
			int y = contour.at<Point>(0).y;
			//putText(approx_drawingOverlay, to_string(i), Point(x, y), 1, 1, Scalar(0,0,255));

		}
	}

	

	//namedWindow("Approx OverlayCountours", WINDOW_AUTOSIZE);
	//imshow("Approx OverlayCountours", approx_drawingOverlay);
	//imwrite("C:/Users/RDCRLGRH/Documents/Snow Friction/ResultsShow/Result.png", approx_drawingOverlay);
	outPutVideo.write(drawingOverlay);
	//test
	return FlatTopPixels;
}


