#include "func.h"
#include "Configuration.h"
using namespace wdcpp;
#include "nlohmann/json.hpp"
#include "fifo_map.hpp"
using namespace nlohmann;
/* 以下为 nlohmann/json 库使用，保证插入顺序不变 */
template <class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = fifo_map<K, V, fifo_map_compare<K>, A>;
using my_json = basic_json<my_workaround_fifo_map>;
using Json = my_json;

//#include <ErrorCheck>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
using std::cin;
using std::cout;
using std::endl;
using std::istringstream;
using std::string;
using std::vector;

int netFd; // 全局网络套接字

void showMenu()
{
    printf("\n");
    printf("**** Welcome to the search engine system *******\n");
    printf("*                                              *\n");
    printf("*         1:    关键字查询                     *\n");
    printf("*         2:    网页查询                       *\n");
    printf("*         3：   说明                           *\n");
    printf("*         4:    退出                           *\n");
    printf("*                                              *\n");
    printf("************************************************\n");
    printf("\n");
}

/**
 *  分页显示网页查询结果
 */
void showWithPaging(Json &root)
{
    size_t pageNum = root["msg"].size();
    if (pageNum <= 1)
        cout << "[ 已找到" << pageNum << "页 ]" << endl;
    else
        cout << "[ 已找到" << pageNum << "页 ]" << endl;

    size_t restNum = pageNum; // 剩余页面数

    for (size_t idx = 0; idx < 5 && idx < pageNum; ++idx, --restNum)
    {
        auto &page = root["msg"][idx];
        cout << "[Title] " << page["title"] << endl;
        cout << "[url] " << page["url"] << endl;
        cout << "[Summary] " << page["summary"] << endl
             << endl;
    }

    while (restNum > 0)
    {
        // 提示还剩 5 篇
        if (restNum <= 1)
            cout << "[ 剩余 " << restNum << " 篇, 请输入'n'以显示剩余内容, 或者输入'c'以继续 ]" << endl;
        else
            cout << "[ 剩余 " << restNum << " 篇, 请输入'n'以显示剩余内容, 或者输入'c'以继续 ]" << endl;

        char opt;
        cin >> opt;
        if (opt == 'n')
        {
            size_t start = pageNum - restNum;
            for (size_t idx = start; idx < start + 5 && idx < pageNum; ++idx, --restNum)
            {
                auto &page = root["msg"][idx];
                cout << "[Title] " << page["title"] << endl;
                cout << "[url] " << page["url"] << endl;
                cout << "[Summary] " << page["summary"] << endl
                     << endl;
            }
        }
        else
            goto end;
    }

end:;
}

/**
 *  1. 将 buf 中的恰好 count 个字节写入到 SND 中
 *  2. 返回成功写入的字节数
 *  3. 出错返回 count + 1
 *  4. 未写完返回值将 < count
 */
size_t sendm(const void *buf, size_t count)
{

    int left = count;              // 待希尔的字节数
    const char *ptr = (char *)buf; // 下一个待写入字节的位置

    int ret = 0;

    while (left > 0)
    {
        do
        {
            ret = ::send(netFd, ptr, left, 0);
        } while (-1 == ret && errno == EINTR); // 中断事件直接忽略

        if (-1 == ret) // 出错
        {
            ::perror("send");
            return count - ret; // 返回 count + 1
        }
        else if (0 == ret) // peerFd 已断开
        {
            printf("send: server disconnected(连接服务器失败)\n");
            break;
        }
        else // 更新 ptr 和 left
        {
            ptr += ret;
            left -= ret;
        }
    }

    return count;
}

/**
 *  1. 从 RCV 中至少读取 count 个字节到 buf 中
 *  2. 返回成功读到的字节数
 *  3. 出错返回 count + 1
 *  4. 不保证字符串 buf 以 \0 结尾
 */
size_t recvm(void *buf, size_t count)
{
    int left = count;        // 待读取的字节数
    char *ptr = (char *)buf; // 下一个字节写入的位置

    int ret = 0;

    while (left > 0)
    {
        do
        {
            ret = ::recv(netFd, ptr, left, 0);
        } while (-1 == ret && errno == EINTR); // 中断事件直接忽略

        if (-1 == ret)
        {
            ::perror("recv");
            return count - ret; // 返回 count + 1
        }
        else if (0 == ret) // server 已断开
        {
            printf("recv: server disconnected(连接服务器失败)\n");
            break;
        }
        else // 更新 ptr 和 left
        {
            ptr += ret;
            left -= ret;
        }
    }

    return count;
}

/**
 *  发送关键词
 */
void sendKey(string &key)
{
    Json root;
    root["msgID"] = 1;
    root["msg"] = key;
    string msg = root.dump(4);
#ifdef __DEBUG__
    printf("\t(File:%s, Func:%s(), Line:%d)\n", __FILE__, __FUNCTION__, __LINE__);
    cout << msg << endl;
#endif

    const size_t length = msg.size();
    sendm(&length, sizeof(size_t)); // 发送车头
    sendm(msg.c_str(), length);     // 发送车厢
}

/**
 *  发送查询词
 */
void sendQuery(string &query)
{
    Json root;
    root["msgID"] = 2;
    root["msg"] = query;
    string msg = root.dump(4);

#ifdef __DEBUG__
    printf("\t(File:%s, Func:%s(), Line:%d)\n", __FILE__, __FUNCTION__, __LINE__);
    cout << msg << endl;
#endif

    const size_t length = msg.size();
    sendm(&length, sizeof(size_t)); // 发送车头
    sendm(msg.c_str(), length);     // 发送车厢
}

void recvKeys()
{

    size_t length = 0;
    recvm(&length, sizeof(size_t)); // 接收车头
    char buf[length + 1] = {0};
    recvm(buf, length); // 接收车厢

    string msg(buf);
#ifdef __DEBUG__
    printf("\t(File:%s, Func:%s(), Line:%d)\n", __FILE__, __FUNCTION__, __LINE__);
    cout << msg << endl;
#endif
    Json root = json::parse(msg); // 解析
    if (100 == root["msgID"])
    {
        cout << "Response from server: " << endl;
        for (auto &key : root["msg"])
        {
            cout << key << endl;
        }
    }
    else if (404 == root["msgID"])
    {
        cout << "Response from server: " << endl;
        cout << root["msg"] << endl;
    }
    else
    {
        cout << "Something Error! System close!" << endl;
        close(netFd);
        exit(EXIT_FAILURE);
    }
}

void recvWebPages()
{
    size_t length = 0;
    recvm(&length, sizeof(size_t)); // 接收车头
    char buf[length + 1] = {0};
    recvm(buf, length); // 接收车厢

    string msg(buf);
#ifdef __DEBUG__
    printf("\t(File:%s, Func:%s(), Line:%d)\n", __FILE__, __FUNCTION__, __LINE__);
    cout << msg << endl;
#endif
    Json root = json::parse(msg); // 解析
    if (200 == root["msgID"])
    {
        cout << "Response from server: " << endl;
        showWithPaging(root);
    }
    else if (404 == root["msgID"])
    {
        cout << "Response from server: " << endl;
        cout << root["msg"] << endl;
    }
    else
    {
        cout << "Something Error! System close!" << endl;
        close(netFd);
        exit(EXIT_FAILURE);
    }
}

void explain(){

    printf("说明:\n");
    printf("1.选择1则进入关键字查询模式，会在库中搜索并推荐相同或者相似的五个关键字。\n");
    printf("2.选择2则进入网页查询模式，会检索网页库输出所查询相关的文章与网站。      \n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    string ip = Configuration::getInstance()->getConfigMap()["ip"];
    string port = Configuration::getInstance()->getConfigMap()["port"];

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(atoi(port.c_str()));

    netFd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(netFd, -1, "socket");

    int ret = connect(netFd, (struct sockaddr *)&addr, sizeof(addr));
    ERROR_CHECK(ret, -1, "connect");

    while (1)
    {
        showMenu();
        cout << "请选择: " << endl;

        int opt;
        cin >> opt;

        string msg;

        switch (opt)
        {
        case 1:
            cout << "请输入一个关键字: " << endl;
            cin >> msg;
            sendKey(msg);
            recvKeys();
            break;
        case 2:
            cout << "请输入一个问题: " << endl;
            cin >> msg;
            sendQuery(msg);
            recvWebPages();
            break;
        case 3:
            explain();
            break;
        case 4:
            close(netFd);
            exit(EXIT_SUCCESS);
        default:
            cout << "没有这个选择！请重新选择。" << endl;
            //close(netFd);
            //exit(EXIT_FAILURE);
            break;
        }
    }

    close(netFd);

    return 0;
}
