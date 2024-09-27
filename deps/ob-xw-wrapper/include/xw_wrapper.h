#ifndef XW_WRAPPER_H_
#define XW_WRAPPER_H_

#include "spdlog/spdlog.h"
#include "xpack/json.h"

// nlohmann
#include <nlohmann/json.hpp>

#ifdef _WIN32
    #define XW_API __declspec(dllexport)
#else
    #define XW_API
#endif

enum XwStreamType : int
{
    DEPTH = 0,
    IR,
    COLOR
};

/**
 * @brief 渲染格式
 * 
 */
enum XwRenderFormat: int
{
    I420,
    GREY,
    TURBO,
    HISTOGRAM,
    RAINBOW,
    RAW,
    RGB8
};

/**
 * @brief 原始数据格式
 * 
 */
enum XwFrameFormat: int
{
    Y8,
    Y10,
    Y12,
    Y14,
    Y16,
    YUV420,
    YUV422,
    RGB888
};

/**
 * @brief 通信方式
 * 
 */
enum XwProtocl: int
{
    P_SHARED_MEMORY_CREATE,  // 共享内存 - 创建
    P_SHARED_MEMORY_OPEN,    // 共享内存 - 打开
    P_WEBSOCKET              // websocket
};

enum XwSide: int
{
    SERVER,
    CLIENT_SM,
    CLIENT_SOCKET
};

struct XwFrameInfo
{
    bool valid;      // 帧是否有效
    int width;       // 图像宽
    int height;      // 图像高
    int length;      // 图像长度
    int format;      // 图像格式
    float fd;        // fd
    float u0;        // u0
    float v0;        // v0
    bool dirty;      // dirty
    float scale;     // scale
    XPACK(O(valid, width, height, length, format, fd, u0, v0, dirty, scale));
};

class XW_API XwWrapper
{

public:
    XwWrapper(XwProtocl protocl = P_SHARED_MEMORY_CREATE, std::string name = "");
    ~XwWrapper();

    void send(int deviceIndex, XwStreamType type, XwFrameFormat frameFormat, XwRenderFormat renderFormat, char* data, int width, int height, int length, int timestamp, float fd = 0, float u0 = 0, float v0 = 0, float scale = 1);
    void read(int deviceInex, XwStreamType, char* dst, int length);
    void read(int chunkId, char* dst, int length);
    void read(int chunkId, char* dst, int start, int end);

    XwFrameInfo recv(int deviceIndex, XwStreamType type);
    XwFrameInfo recv(int chunkId);

    int getLargeDataLength(int offset);
    void writeLargeData(char* data, int length, int offset);
    void readLargeData(char* data, int length, int offset);


    /**
     * @brief 发布消息
     * 
     * @tparam T 
     * @param t 
     */
    template <class T>
    void publish(T t)
    {
        sendMessage(xpack::json::encode(t));
    }

    /**
     * @brief 发送消息
     * 
     * @param msg 
     */
    void sendMessage(std::string msg);

    /**
     * @brief 订阅自定义事件，订阅之后，即可接收该通道的数据
     * 
     * @param event    事件名
     * @param callback 数据回调函数
     */ 
    void subscribe(std::string event, std::function<void(nlohmann::json const&)> callback);

    /**
     * @brief 获取日志对象
     * 
     * @return std::shared_ptr<spdlog::logger> 
     */
    std::shared_ptr<spdlog::logger> logger();

private:
    XwProtocl protocl_;  

};

#endif