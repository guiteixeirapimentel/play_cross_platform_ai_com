#include <stdint.h>
#include <vector>
#include <string>


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

    struct AKNOWLEDGE
    {
        bool m_valid = false;
        std::string m_message = "";
    };
}