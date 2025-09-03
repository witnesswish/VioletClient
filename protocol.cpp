#include "protocol.h"

SRHelper::SRHelper(){}

void SRHelper::sendMsg(QSslSocket *sock, uint16_t msgType, const std::string &content)
{
    qintptr me = sock->socketDescriptor();
    Msg msg;
    msg.header.magic = htonl(0x43484154); // "CHAT"
    msg.header.version = htons(1);
    msg.header.type = htons(msgType);
    msg.header.length = htonl(content.size());
    msg.header.timestamp = htonl(static_cast<uint32_t>(time(nullptr)));
    msg.neck = {};
    msg.content.assign(content.begin(), content.end());
    auto packet = msg.serialize();
    sock->write(packet.data(), packet.size());
}

void SRHelper::sendMsg(QSslSocket *sock, VioletProtHeader header, const std::string &content) {
    Msg msg;
    msg.header = header;
    msg.neck = {};
    msg.content.assign(content.begin(), content.end());
    auto packet = msg.serialize();
    sock->write(packet.data(), packet.size());
}

void SRHelper::sendMsg(QSslSocket *sock, VioletProtNeck neck, const std::string &content) {
    Msg msg;
    msg.header.magic = htonl(0x43484154); // "CHAT"
    msg.header.version = htons(1);
    msg.header.type = htons(0);
    msg.header.length = htonl(content.size());
    msg.header.timestamp = htonl(static_cast<uint32_t>(time(nullptr)));
    msg.neck = neck;
    msg.content.assign(content.begin(), content.end());
    auto packet = msg.serialize();
    qDebug()<<"send " << packet.size() << " bytes to buffer";
    sock->write(packet.data(), packet.size());
}

void SRHelper::sendMsg(QSslSocket *sock,
                       VioletProtHeader header,
                       VioletProtNeck neck,
                       const std::string &content)
{
    Msg msg;
    msg.header = header;;
    msg.neck = neck;
    msg.content.assign(content.begin(), content.end());
    auto packet = msg.serialize();
    sock->write(packet.data(), packet.size());
}

std::optional<Msg> SRHelper::recvMsg(QSslSocket *sock, ssize_t byteToRead)
{
    qDebug()<<"into ssl recv";
    VioletProtHeader header;
    VioletProtNeck neck;
    qint64 len;
    qint64 totalRead = 0;
    if (sock->bytesAvailable() < sizeof(header))
    {
        qDebug()<< "sock bytesAvailable: " << sock->bytesAvailable();
        return std::nullopt;
    }
    qDebug()<< "protocol byte to read: " << byteToRead;
    if(byteToRead > 0)
    {
        qDebug()<< "into big table";
        Msg spByteRead;
        std::vector<char> recvBuffer(byteToRead);
        len = sock->read(recvBuffer.data(), recvBuffer.size());
        totalRead = len;
        if(len < 0)
        {
            qDebug()<< "read " << byteToRead << " failure, return nullopt";
            return std::nullopt;
        }
        else if(len == 0)
        {
            qDebug()<< "read 0, goint to kinck user out";
            Msg msg = {};
            msg.header.length = 0;
            msg.header.checksum = 0;
            return msg;
        }
        else if(len < byteToRead)
        {
            spByteRead.content.assign(recvBuffer.begin(), recvBuffer.begin()+totalRead);
            ssize_t tmp = byteToRead - totalRead;
            while(tmp > 0)
            {
                len = sock->read(recvBuffer.data(), tmp);
                if(len < 0)
                {
                    break;
                }
                else if(len == 0)
                {
                    qDebug()<< "read special byte complete after while loop len=0, total read this time: " << totalRead;
                    break;
                }
                else
                {
                    tmp = tmp - len;
                    totalRead += len;
                    spByteRead.content.insert(spByteRead.content.end(), recvBuffer.begin(), recvBuffer.begin()+len);
                }
            }
            qDebug()<< "read special byte complete after while loop, read: " << totalRead;
            spByteRead.header.checksum = totalRead;
            return spByteRead;
        }
        else
        {
            qDebug()<< "read special byte complete, read: " << totalRead;
            spByteRead.header.checksum = totalRead;
            spByteRead.content.assign(recvBuffer.begin(), recvBuffer.begin()+totalRead);
        }
        return spByteRead;
    }
    len = sock->peek(reinterpret_cast<char*>(&header), sizeof(header));
    //qDebug()<< "----" << len;
    if(len != sizeof(header))
    {
        qDebug() << "error len of peek";
        return std::nullopt;
    }
    uint32_t contentLen = ntohl(header.length);
    uint32_t totaLen = contentLen + sizeof(header) + sizeof(neck);
    std::vector<char> recvBuffer(totaLen);
    len = sock->read(recvBuffer.data(), recvBuffer.size());
    totalRead = len;
    qDebug()<<"recv len: " <<len;
    // if(len != static_cast<ssize_t>(recvBuffer.size()))
    // {
    //     return std::nullopt;
    // }
    qDebug()<< "actual recv: " << len << "bytes";
    return Msg::deserialize(recvBuffer.data(), totalRead);
}
std::vector<char> Msg::serialize() const
{
    std::vector<char> packet(sizeof(VioletProtHeader) + sizeof(VioletProtNeck) + content.size());
    memcpy(packet.data(), &header, sizeof(header));
    memcpy(packet.data()+sizeof(VioletProtHeader), &neck, sizeof(neck));
    if(!content.empty())
    {
        memcpy(packet.data()+sizeof(VioletProtHeader)+sizeof(VioletProtNeck), content.data(), content.size());
    }
    return packet;
}

std::optional<Msg> Msg::deserialize(const char *data, size_t length)
{
    qDebug()<< "protocol deserialize length: " << length;
    if(length < sizeof(VioletProtHeader))
        return std::nullopt;
    Msg msg;
    memcpy(&msg.header, data, sizeof(msg.header));
    memcpy(&msg.neck, data+sizeof(msg.header), sizeof(msg.neck));
    if (ntohl(msg.header.magic) != 0x43484154)
    {
        qDebug()<< "magic number error, return nullopt";
        return std::nullopt;
    }
    size_t body_len = length - sizeof(msg.header) - sizeof(msg.neck);
    qDebug()<< "protocol size of body_len" << body_len;
    if(body_len > 0)
    {
        msg.content.assign(data+sizeof(msg.header)+sizeof(msg.neck), data+sizeof(msg.header)+sizeof(msg.neck)+body_len);
        //qDebug()<< "content: " <<QByteArray::fromRawData(msg.content.data(), msg.content.size());
    }
    msg.header.checksum = length - sizeof(msg.header) - sizeof(msg.neck);
    return msg;
}
