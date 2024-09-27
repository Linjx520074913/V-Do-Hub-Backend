#include <format>
#include <string>
#include <iostream>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include "xw_wrapper.h"

#include <chrono>
#include <thread>

using namespace std;
namespace fs = std::filesystem;

XwWrapper wrapper(P_SHARED_MEMORY_OPEN, "V-DoHub");

struct BaseMessage{
    string event;
    string uuid;
    int value;
    XPACK(O(event, uuid, value));
};

string SetCurrentWorkingDir(string exec_path)
{
#ifdef _WIN32
    string dir = exec_path.substr(0, exec_path.find_last_of("\\"));

    char working_directory[MAX_PATH + 1];
    GetCurrentDirectoryA(sizeof(working_directory), working_directory); // **** win32 specific ****

    SetCurrentDirectoryA(dir.c_str());

    GetCurrentDirectoryA(sizeof(working_directory), working_directory); // **** win32 specific ****
    wrapper.logger()->warn("working directory : {0}", working_directory);

    return dir;
#elif __linux__

#elif __APPLE__
    string dir = exec_path.substr(0, exec_path.find_last_of("/"));
    const int MAXPATH=250;
    char working_directory[MAXPATH];

    getcwd(working_directory, MAXPATH);

    int res = chdir(dir.c_str());
    if (-1 == res) {
        std::cout << "Something went wrong! errno " << errno << ": ";
        std::cout << strerror(errno) << std::endl;
    }
    
    getcwd(working_directory, MAXPATH);
    wrapper.logger()->warn("working directory : {0}", working_directory);
#endif
}

cv::VideoCapture* capture = nullptr;  

cv::Mat g_TmpFrame;
bool g_SaveFrame = false;
string g_FilePath = "";
  
bool OpenCamera() {  
    // 在这里打开相机  
    capture = new cv::VideoCapture(0);  
    if (!capture->isOpened()) {  
        std::cerr << "Failed to open camera." << std::endl;  
        delete capture;  
        capture = nullptr;  
        return false;
    }else{
        thread t([=]{
            cv::Mat frame;
            while(capture && capture->isOpened()){
                // 捕获视频帧
                *capture >> frame;
                if (frame.empty()) {
                    std::cerr << "Failed to capture frame." << std::endl;
                    break;
                }

                if(g_SaveFrame){
                    g_TmpFrame = frame.clone();
                    g_SaveFrame = false;
                    cout << "@@@@@@@@@ save frame " << endl;
                    cv::imwrite(g_FilePath, g_TmpFrame);
                }

                cv::Mat rgb;
        
                // 转换BGR到RGB
                cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);

                BaseMessage msg;
                msg.event = "update-image";
                wrapper.send(0, COLOR, RGB888, RAINBOW, (char*)rgb.data, rgb.cols, rgb.rows, rgb.cols * rgb.rows * 3, 0);
                wrapper.publish(msg);

                // 按下 ESC 键退出循环
                if (cv::waitKey(30) == 27) {
                    break;
                }
            }
        });
        t.detach();
        return true;
    }
}  
  
void CloseCamera() {  
    // 在这里关闭相机  
    if (capture != nullptr) {  
        capture->release();  
        delete capture;  
        capture = nullptr;  
    }  
}

int main(int argc, char* argv[])
{
    string dir = SetCurrentWorkingDir(argv[0]);

    wrapper.subscribe("open-camera", [](nlohmann::json const& data){
        wrapper.logger()->error("######### open-camera ############");
        bool res = OpenCamera();
        BaseMessage msg;
        msg.event = "open-camera";
        msg.value = res;
        msg.uuid = data["uuid"]; 
        wrapper.publish(msg);
    });
    wrapper.subscribe("close-camera", [](nlohmann::json const& data){
        wrapper.logger()->error("######### close-camera ############");
        CloseCamera();
    });
    wrapper.subscribe("take-photo", [](nlohmann::json const& data){
        cout << "######### take-photo" << endl;
        g_FilePath = data["data"]["filePath"];
        g_SaveFrame = true;
    });
    wrapper.subscribe("get-photo", [](nlohmann::json const& data){
         cout << "######### get-photo" << endl;
        cv::Mat rgb;
        
        // 转换BGR到RGB
        cv::cvtColor(g_TmpFrame, rgb, cv::COLOR_BGR2RGB);

        BaseMessage msg;
        msg.event = "get-photo";
        msg.uuid = data["uuid"];
        wrapper.send(1, COLOR, RGB888, RAINBOW, (char*)rgb.data, rgb.cols, rgb.rows, rgb.cols * rgb.rows * 3, 0);
        wrapper.publish(msg);
    });

    while(true){
        std::this_thread::sleep_for(chrono::seconds(1));
    }

    return 1;
}