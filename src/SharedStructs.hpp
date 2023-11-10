#include <stdint.h>
#include <vector>
#include <string>
#include <alpaca/alpaca.h>

namespace pimentel
{
    struct MessagePacket
    {
        enum class MessageType : uint8_t
        {
            AKNOWLEDGE = 0,

            _EOF = 255
        };

        MessageType m_msgType;
        std::vector<uint8_t> m_bytes;
    };

    struct Aknowledge
    {
        bool m_valid = false;
        std::string m_message = "";
    };

    template<typename T>
    std::vector<uint8_t> serializeMessage(const T& msg)
    {
        std::vector<uint8_t> bytes;
        alpaca::serialize(msg, bytes);
        return bytes;
    }

    template<typename T>
    MessagePacket::MessageType getMsgType(const T& msg);

    template<>
    MessagePacket::MessageType getMsgType(const Aknowledge&)
    {
        return MessagePacket::MessageType::AKNOWLEDGE;
    }

    template<typename T>
    MessagePacket generateMessagePacket(const T& msg)
    {
        MessagePacket res;

        res.m_bytes = serializeMessage(msg);

        res.m_msgType = getMsgType(msg);

        return res;
    }

    struct MessageVisitor
    {
        virtual void visit(const Aknowledge& akn) = 0;
        virtual void handleInvalidData(std::error_code ec) = 0;
    };

    template<typename T>
    void handle(const std::vector<uint8_t>& bytes, MessageVisitor& visitor)
    {
        std::error_code ec;
        auto object = alpaca::deserialize<T>(bytes, ec);
        if (ec)
        {
            visitor.handleInvalidData(ec);
            return;
        }

        visitor.visit(object);
    }

    void handleData(const std::vector<uint8_t>& packetBytes, MessageVisitor& visitor)
    {
        std::error_code ec;
        auto object = alpaca::deserialize<MessagePacket>(packetBytes, ec);
        if (ec)
        {
            visitor.handleInvalidData(ec);
            return;
        }

        using MessageType = MessagePacket::MessageType;
        switch (object.m_msgType)
        {
        case MessageType::AKNOWLEDGE:
            handle<Aknowledge>(object.m_bytes, visitor);
            break;

        default:
            break;
        }
    }
}