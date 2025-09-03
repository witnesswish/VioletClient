#ifndef PROTOCOL_H
#define PROTOCOL_H

#if __linux__
#include <arpa/inet.h>
#elif __WIN32
#include <winsock2.h>
#include <windows.h>
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <vector>
#include <optional>
#include <cstdint>
#include <string.h>
#include <string>
#include <QSslSocket>


#pragma pack(push, 1)
struct VioletProtHeader {
    uint32_t magic;         // 魔数 0x43484154 ("CHAT")
    uint16_t version;       // 协议版本
    uint16_t type;  // 消息类型
    uint32_t length;   // 消息体长度
    uint32_t timestamp;     // 时间戳(可选)
    uint32_t checksum;      // 头部校验和(可选)
};
struct VioletProtNeck
{
    char command[12];     // 请求类型 0=注册 1=登录 2=私聊
    bool unlogin;         // 这个设计是为匿名用户，只有匿名用户，这个值为真
    char name[32];   // 用户名(固定长度)
    char email[32];
    char evt[5];
    char pass[64];   // 密码(设计为加密后，初始用明文)
    uint8_t encrypt;    // 加密类型 0=无 1=MD5 2=AES
    uint8_t os;         // 操作系统类型 0=Unknown 1=Windows 2=Linux 3=android...
    uint8_t mto;        // 发送的对象，发送给哪个用户，或者哪个群，使用id
    uint8_t mfrom;      // 数据发来的对象，是哪个用户，这个服务器收到消息的第一时间写入
    //uint8_t tmp;      // 后续有了再继续加
};
#pragma pack(pop)

/**
 * @brief The MessageTypes enum
 * 暂时不启用
 */
enum MessageTypes {
    MT_TEXT = 0,            // 文本消息
    MT_IMAGE = 1,           // 图片
    MT_FILE_START = 2,      // 文件传输开始
    MT_FILE_CHUNK = 3,      // 文件分块
    MT_FILE_END = 4,        // 文件传输结束
    MT_HEARTBEAT = 5,       // 心跳包
    MT_USER_ONLINE = 6,     // 用户上线通知
    MT_USER_OFFLINE = 7,    // 用户下线通知
    MT_PROTOCOL_UPGRADE = 8 // 协议升级请求
};

class Msg
{
public:
    VioletProtHeader header = {};
    VioletProtNeck neck = {};
    std::vector<char> content;
    std::vector<char> serialize() const;
    static std::optional<Msg> deserialize(const char *data, size_t length);
};

/**
 * @brief The SRHelper class
 * 该类是用于网络首发的工具函数
 * 由于转换比较麻烦，并且在外单独使用需要填充整合数据，所以有了这个类
 */

class SRHelper
{
public:
    SRHelper();
    void sendMsg(QSslSocket *sock, uint16_t msgType, const std::string &content);
    void sendMsg(QSslSocket *sock, VioletProtHeader header, const std::string &content);
    void sendMsg(QSslSocket *sock, VioletProtNeck neck, const std::string &content);
    void sendMsg(QSslSocket *sock, VioletProtHeader header, VioletProtNeck neck, const std::string &content);
    std::optional<Msg> recvMsg(QSslSocket *sock, ssize_t byteToRead);
};

#endif // PROTOCOL_H
