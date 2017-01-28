#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;


class Settings
{
public:
    Settings() : goodInput(false) {}
    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};

    void write(FileStorage& fs) const                        //Write serialization for this class
    {
        fs << "{" << "BoardSize_Width"  << boardSize.width
                  << "BoardSize_Height" << boardSize.height
                  << "Square_Size"         << squareSize
                  << "Calibrate_Pattern" << patternToUse
                  << "Calibrate_NrOfFrameToUse" << nrFrames
                  << "Calibrate_FixAspectRatio" << aspectRatio
                  << "Calibrate_AssumeZeroTangentialDistortion" << calibZeroTangentDist
                  << "Calibrate_FixPrincipalPointAtTheCenter" << calibFixPrincipalPoint

                  << "Write_DetectedFeaturePoints" << bwritePoints
                  << "Write_extrinsicParameters"   << bwriteExtrinsics
                  << "Write_outputFileName"  << outputFileName

                  << "Show_UndistortedImage" << showUndistorsed

                  << "Input_FlipAroundHorizontalAxis" << flipVertical
                  << "Input_Delay" << delay
                  << "Input" << input
           << "}";
    }
    void read(const FileNode& node)                          //Read serialization for this class
    {
        node["BoardSize_Width" ] >> boardSize.width;
        node["BoardSize_Height"] >> boardSize.height;
        node["Calibrate_Pattern"] >> patternToUse;
        node["Square_Size"]  >> squareSize;
        node["Calibrate_NrOfFrameToUse"] >> nrFrames;
        node["Calibrate_FixAspectRatio"] >> aspectRatio;
        node["Write_DetectedFeaturePoints"] >> bwritePoints;
        node["Write_extrinsicParameters"] >> bwriteExtrinsics;
        node["Write_outputFileName"] >> outputFileName;
        node["Calibrate_AssumeZeroTangentialDistortion"] >> calibZeroTangentDist;
        node["Calibrate_FixPrincipalPointAtTheCenter"] >> calibFixPrincipalPoint;
        node["Input_FlipAroundHorizontalAxis"] >> flipVertical;
        node["Show_UndistortedImage"] >> showUndistorsed;
        node["Input"] >> input;
        node["Input_Delay"] >> delay;

    }
   //check
    void interprate()
    {
        goodInput = true;
        if (boardSize.width <= 0 || boardSize.height <= 0)
        {
            cerr << "Invalid Board size: " << boardSize.width << " " << boardSize.height << endl;
            goodInput = false;
        }
        if (squareSize <= 10e-6)
        {
            cerr << "Invalid square size " << squareSize << endl;
            goodInput = false;
        }
        if (nrFrames <= 0)
        {
            cerr << "Invalid number of frames " << nrFrames << endl;
            goodInput = false;
        }

        if (input.empty())      // Check for valid input
        {
            cerr<<"input emty"<<input <<endl;
            inputType = INVALID;
        }
        else
        {
            if (input[0] >= '0' && input[0] <= '9')
                        {
                            stringstream ss(input);
                            ss >> cameraID;
                           // inputType = CAMERA;
}
        }
        if (inputType == INVALID)
        {
            cerr << " Inexistent input: " << input;
            goodInput = false;
        }

        if (inputType == CAMERA)
                      inputCapture.open(cameraID);
        flag = 0;
        if(calibFixPrincipalPoint) flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(calibZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)            flag |= CV_CALIB_FIX_ASPECT_RATIO;


        //calibrationPattern = NOT_EXISTING;
        //if (!patternToUse.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
        //if (!patternToUse.compare("CIRCLES_GRID")) calibrationPattern = CIRCLES_GRID;
        //if (!patternToUse.compare("ASYMMETRIC_CIRCLES_GRID")) calibrationPattern = ASYMMETRIC_CIRCLES_GRID;

        cerr<<"Calibration Pattern in interprate func= "<<calibrationPattern<<endl;
        if (calibrationPattern == NOT_EXISTING)
            {
                cerr << " Inexistent camera calibration mode: " << patternToUse << endl;
                goodInput = false;
            }
        if (calibrationPattern == CHESSBOARD)
            {
                cerr << " I am a chessboard yeah! " << endl;
                goodInput = false;
            }


        atImageList = 0;

    }
    Mat nextImage()
    {
        Mat result;
        if( inputCapture.isOpened() )
        {
            Mat view0;
            inputCapture >> view0;
            view0.copyTo(result);
        }
        else if( atImageList < (int)imageList.size() )
            result = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);

        return result;
    }

    static bool readStringList( const string& filename, vector<string>& l )
    {
        l.clear();
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() )
            return false;
        FileNode n = fs.getFirstTopLevelNode();
        if( n.type() != FileNode::SEQ )
            return false;
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it )
            l.push_back((string)*it);
        return true;
    }
public:
    Size boardSize;            // The size of the board -> Number of items by width and height
    Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
    float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;              // The number of frames to use from the input for calibration
    float aspectRatio;         // The aspect ratio
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;          // Flip the captured images around the horizontal axis
    string outputFileName;      // The name of the file where to write
    bool showUndistorsed;       // Show undistorted images after calibration
    string input;               // The input ->



    int cameraID;
    vector<string> imageList;
    int atImageList;
    VideoCapture inputCapture;
    InputType inputType;
    bool goodInput;
    int flag;
    string patternToUse;


};

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

class CameraCalibrate
{
public:

   Size imageSize_s;
   Mat cameraMatrix_s;
   Mat distCoeffs_s;

  CameraCalibrate() {}

  virtual ~CameraCalibrate() {}

  int calibrate();

  static void help();

  static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings());

  bool runCalibrationAndSave(Settings& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,
                             vector<vector<Point2f> > imagePoints );

  static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                           const vector<vector<Point2f> >& imagePoints,
                                           const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                           const Mat& cameraMatrix , const Mat& distCoeffs,
                                           vector<float>& perViewErrors);
  static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                                                                Settings::Pattern patternType);
  static bool runCalibration( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,vector<float>& reprojErrs,  double& totalAvgErr);

  void saveCameraParams( Settings& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,const vector<Mat>& rvecs, const vector<Mat>& tvecs,const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,double totalAvgErr );

  Settings::InputType get_inputType ()
  {
      return s.inputType;
  }

  void set_inputType (Settings::InputType ip)
  {
      s.inputType = ip;
  }

  Size get_boardSize ()
  {
      return s.boardSize;
  }

  void set_boardSize ( int w, int h)
  {
      //s.boardSize = bz;
      s.boardSize.width = w;
      s.boardSize.height = h;


  }
//
  Settings::Pattern get_calibrationPattern ()
  {
      return s.calibrationPattern;
  }

  void set_calibrationPattern ( Settings::Pattern p)
  {
      s.calibrationPattern = p;
  }
//
  float squareSize ()
  {
      return s.squareSize;
  }

  void set_squareSize (float sq)
  {
      s.squareSize = sq;
  }
//
  int get_nrFrames ()
  {
      return s.nrFrames;
  }

  void set_nrFrames (int nr)
  {
      s.nrFrames = nr;
  }
//
  bool get_showUndistorsed ()
  {
      return s.showUndistorsed;
  }

  void set_showUndistorsed (bool sh_un)
  {
      s.showUndistorsed = sh_un;
  }
//
  bool get_bwritePoints()
  {
      return s.bwritePoints;
  }

  void set_bwritePoints (bool wr_p)
  {
      s.bwritePoints = wr_p;
  }
//
  bool get_bwriteExtrinsics()
  {
      return s.bwriteExtrinsics;
  }

  void set_bwriteExtrinsics (bool wr_ext)
  {
      s.bwriteExtrinsics = wr_ext;
  }
//

  string get_input()
  {
      return s.input;
  }

  void set_input (string in)
  {
      s.input = in;
  }

  vector<string>* get_imageList()
  {
      return &s.imageList;
  }
  //
  bool get_aspectRatio()
  {
      return s.aspectRatio;
  }

  void set_aspectRatio (float ar)
  {
      s.aspectRatio = ar;
  }
//
  bool get_delay()
  {
      return s.delay;
  }

  void set_delay (bool d)
  {
      s.delay = d;
  }
//
  bool get_calibZeroTangetDist()
  {
      return s.calibZeroTangentDist;
  }

  void set_calibZeroTangentDist (bool cal_tan)
  {
      s.calibZeroTangentDist = cal_tan;
  }
//
  bool get_calibFixPrincipalPoint()
  {
      return s.calibFixPrincipalPoint;
  }

  void set_calibFixPrincipalPoint (bool cal_point)
  {
      s.calibFixPrincipalPoint= cal_point;
  }
  //
  bool get_flipVertical()
  {
      return s.flipVertical;
  }

  void set_flipVertical (bool flip)
  {
      s.flipVertical = flip;
  }

 string get_patternToUse()
  {
      return s.patternToUse;
  }

  void set_patternToUse (string patt)
  {
      s.patternToUse = patt;
  }


private:
  Settings s;



};
