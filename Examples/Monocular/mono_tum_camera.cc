#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<System.h>

using namespace std;

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        cerr << endl << "Usage: ./mono_tum path_to_vocabulary path_to_settings" << endl;
        return 1;
    }

    // Open camera using OpenCV
    cv::VideoCapture cap(0); // 0 is the ID for the default camera
    if(!cap.isOpened())
    {
        cerr << "Failed to open the camera." << endl;
        return 1;
    }

    // Retrieve camera parameters from settings file
    cv::FileStorage fsSettings(argv[2], cv::FileStorage::READ);
    if(!fsSettings.isOpened())
    {
        cerr << "Failed to open settings file at: " << argv[2] << endl;
        return 1;
    }
    float fps = fsSettings["Camera.fps"];
    float imageScale = 1.0f;
    if(fsSettings["Camera.image_scale"].isReal())
        imageScale = (float)fsSettings["Camera.image_scale"];

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SLAM(argv[1], argv[2], ORB_SLAM3::System::MONOCULAR, true);

    cout << endl << "-------" << endl;
    cout << "Start processing camera input ..." << endl;

    // Vector for tracking time statistics
    vector<float> vTimesTrack;

    cv::Mat frame; // Add a variable for the grayscale frame
    double t_resize = 0.f;
    double t_track = 0.f;

    while(true)
    {
        // Capture frame from camera
        cap >> frame;
        if(frame.empty())
        {
            cerr << "Failed to capture frame from camera." << endl;
            break;
        }

        // Resize the image if necessary
        if(imageScale != 1.f)
        {
            int width = frame.cols * imageScale;
            int height = frame.rows * imageScale;
            cv::resize(frame, frame, cv::Size(width, height));
        }

        // Get current timestamp
        double tframe = cv::getTickCount() / cv::getTickFrequency();

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the grayscale image to the SLAM system
        SLAM.TrackMonocular(frame, tframe);

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

        double ttrack = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();
        vTimesTrack.push_back(ttrack);

        // Display the grayscale frame
        cv::imshow("ORB-SLAM3: Camera Input (Grayscale)", grayFrame);

        // Wait for the next frame
        if(cv::waitKey(1000.0 / fps) == 27) // Press 'ESC' to exit
            break;
    }

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(), vTimesTrack.end());
    float totaltime = 0;
    for(size_t i = 0; i < vTimesTrack.size(); i++)
    {
        totaltime += vTimesTrack[i];
    }
    cout << "-------" << endl << endl;
    cout << "Median tracking time: " << vTimesTrack[vTimesTrack.size()/2] << endl;
    cout << "Mean tracking time: " << totaltime / vTimesTrack.size() << endl;

    // Save camera trajectory
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");

    return 0;
}
