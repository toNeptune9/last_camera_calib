#include "cameracalibration.h"

void CameraCalibrate::help()
{
    cout <<  "This is a camera calibration sample." << endl
          <<  "Usage: calibration configurationFile"  << endl
           <<  "Near the sample file you'll find the configuration file, which has detailed help of "
               "how to edit it.  It may be any OpenCV supported file format XML/YAML." << endl;
}

void CameraCalibrate::read(const FileNode& node, Settings& x, const Settings& default_value )
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

int CameraCalibrate::calibrate()
{
    // help();

    try
    {

        s.interprate();

        if (!s.goodInput)
        {
            cout << "Invalid input detected. Application stopping. " << endl;
            return -1;
        }

        vector<vector<Point2f> > imagePoints;
        Mat cameraMatrix, distCoeffs;
        Size imageSize;
        int mode = s.inputType == Settings::IMAGE_LIST ? CAPTURING : DETECTION;
        clock_t prevTimestamp = 0;
        const Scalar RED(0,0,255), GREEN(0,255,0);
        const char ESC_KEY = 27;

        for(int i = 0;;++i)
        {
            cerr<<"i="<<i<<endl;
            cerr<<"boardSize width= "<<s.boardSize.width<<endl;
            cerr<<"boardSize height= "<<s.boardSize.height<<endl;
            cerr<<"SquareSize = "<<s.squareSize<<endl;
            cerr<<"Input Type (1= Camera) = "<<s.inputType<<endl;
            cerr<<"Input (should be 0 or 1)= "<<s.input<<endl;
            cerr<<"Camera ID ( should be 0 or 1) = "<<s.cameraID<<endl;
            cerr<<"number of Frames = "<<s.nrFrames<<endl;
            cerr<<"good Input= " << s.goodInput<<endl;
            cerr<<"Pattern type "<< s.patternToUse<<endl;
            cerr<<"Calibration Pattern= "<<s.calibrationPattern<<endl;
            cerr<<" mode = "<< mode<< endl;
            Mat view;
            bool blinkOutput = false;

            view = s.nextImage();

            //-----  If no more image, or got enough, then stop calibration and show result -------------
            if( mode == CAPTURING && imagePoints.size() >= (unsigned)s.nrFrames )
            {
                if( runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints))
                    mode = CALIBRATED;
                else
                    mode = DETECTION;
            }
            if(view.empty())          // If no more images then run calibration, save and stop loop.
            {
                if( imagePoints.size() > 0 )
                    runCalibrationAndSave(s, imageSize,  cameraMatrix, distCoeffs, imagePoints);
                break;
            }


            imageSize = view.size();  // Format input image.
            if( s.flipVertical )    flip( view, view, 0 );

            vector<Point2f> pointBuf;

            bool found;
            switch( s.calibrationPattern ) // Find feature points on the input format
            {
            case Settings::CHESSBOARD:
                found = findChessboardCorners( view, s.boardSize, pointBuf,
                                               CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);
                cerr<< "found = "<<found<<endl;
                break;
            case Settings::CIRCLES_GRID:
                found = findCirclesGrid( view, s.boardSize, pointBuf );
                break;
            case Settings::ASYMMETRIC_CIRCLES_GRID:
                found = findCirclesGrid( view, s.boardSize, pointBuf, CALIB_CB_ASYMMETRIC_GRID );
                break;
            default:
                found = false;
                break;
            }

            if ( found)                // If done with success,
            {
                // improve the found corners' coordinate accuracy for chessboard
                if( s.calibrationPattern == Settings::CHESSBOARD)
                {
                    Mat viewGray;
                    cvtColor(view, viewGray, COLOR_BGR2GRAY);
                    cornerSubPix( viewGray, pointBuf, Size(11,11),
                                  Size(-1,-1), TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
                }

                if( mode == CAPTURING &&  // For camera only take new samples after delay time
                        (!s.inputCapture.isOpened() || clock() - prevTimestamp > s.delay*1e-3*CLOCKS_PER_SEC) )
                {
                    imagePoints.push_back(pointBuf);
                    prevTimestamp = clock();
                    blinkOutput = s.inputCapture.isOpened();
                }

                // Draw the corners.
                drawChessboardCorners( view, s.boardSize, Mat(pointBuf), found );
            }

            //----------------------------- Output Text ------------------------------------------------
            string msg = (mode == CAPTURING) ? "100/100" :
                                               mode == CALIBRATED ? "Calibrated" : "Press 'g' to start";
            int baseLine = 0;
            Size textSize = getTextSize(msg, 1, 1, 1, &baseLine);
            Point textOrigin(view.cols - 2*textSize.width - 10, view.rows - 2*baseLine - 10);

            if( mode == CAPTURING )
            {
                if(s.showUndistorsed)
                    msg = format( "%d/%d Undist", (int)imagePoints.size(), s.nrFrames );
                else
                    msg = format( "%d/%d", (int)imagePoints.size(), s.nrFrames );
            }

            putText( view, msg, textOrigin, 1, 1, mode == CALIBRATED ?  GREEN : RED);

            if( blinkOutput )
                bitwise_not(view, view);

            //------------------------- Video capture  output  undistorted ------------------------------
            if( mode == CALIBRATED && s.showUndistorsed )
            {
                Mat temp = view.clone();
                undistort(temp, view, cameraMatrix, distCoeffs);
            }

            //------------------------------ Show image and check for input commands -------------------
            imshow("Image View", view);
            char key = (char)waitKey(s.inputCapture.isOpened() ? 50 : s.delay);

            if( key  == ESC_KEY )
                break;

            if( key == 'u' && mode == CALIBRATED )
                s.showUndistorsed = !s.showUndistorsed;

            if( s.inputCapture.isOpened() && key == 'g' )
            {
                mode = CAPTURING;
                imagePoints.clear();
            }
        }

        // -----------------------Show the undistorted image for the image list ------------------------
        if( s.inputType == Settings::IMAGE_LIST && s.showUndistorsed )
        {
            Mat view, rview, map1, map2;
            initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
                                    getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0),
                                    imageSize, CV_16SC2, map1, map2);

            for(int i = 0; i < (int)s.imageList.size(); i++ )
            {
                view = imread(s.imageList[i], 1);
                if(view.empty())
                    continue;
                remap(view, rview, map1, map2, INTER_LINEAR);
                imshow("Image View", rview);
                char c = (char)waitKey();
                if( c  == ESC_KEY || c == 'q' || c == 'Q' )
                    break;
            }
        }
    }
    catch ( cv::Exception& e )
    {
        const char* err_msg = e.what();
        std::cout << "exception caught: " << err_msg << std::endl;
    }
}


double CameraCalibrate::computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                                   const vector<vector<Point2f> >& imagePoints,
                                                   const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                                   const Mat& cameraMatrix , const Mat& distCoeffs,
                                                   vector<float>& perViewErrors)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); ++i )
    {
        projectPoints( Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix,
                       distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), CV_L2);

        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

void CameraCalibrate::calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                               Settings::Pattern patternType )
{
    corners.clear();

    switch(patternType)
    {
    case Settings::CHESSBOARD:
    case Settings::CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; ++i )
            for( int j = 0; j < boardSize.width; ++j )
                corners.push_back(Point3f(float( j*squareSize ), float( i*squareSize ), 0));
        break;

    case Settings::ASYMMETRIC_CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
            for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float((2*j + i % 2)*squareSize), float(i*squareSize), 0));
        break;
    default:
        break;
    }
}

bool CameraCalibrate::runCalibration( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                      vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                                      vector<float>& reprojErrs,  double& totalAvgErr)
{

    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( s.flag & CV_CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = 1.0;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcBoardCornerPositions(s.boardSize, s.squareSize, objectPoints[0], s.calibrationPattern);

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    //Find intrinsic and extrinsic camera parameters
    double rms = calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix,
                                 distCoeffs, rvecs, tvecs, s.flag|CV_CALIB_FIX_K4|CV_CALIB_FIX_K5);

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                                            rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}

// Print camera parameters
void CameraCalibrate::saveCameraParams( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                        const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                        const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
                                        double totalAvgErr )
{

    CameraCalibrate::cameraMatrix_s = cameraMatrix;
    CameraCalibrate::distCoeffs_s = distCoeffs;
    CameraCalibrate::imageSize_s.width = imageSize.width;
    CameraCalibrate::imageSize_s.height = imageSize.height;
}

bool CameraCalibrate::runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints )
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;

    bool ok = runCalibration(s,imageSize, cameraMatrix, distCoeffs, imagePoints, rvecs, tvecs,
                             reprojErrs, totalAvgErr);
    cout << (ok ? "Calibration succeeded" : "Calibration failed")
         << ". avg re projection error = "  << totalAvgErr ;

    if( ok )
        saveCameraParams( s, imageSize, cameraMatrix, distCoeffs, rvecs ,tvecs, reprojErrs,
                          imagePoints, totalAvgErr);
    return ok;
}
