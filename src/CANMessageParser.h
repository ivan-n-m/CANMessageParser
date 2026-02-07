#ifndef CAN_MESSAGE_PARSER_H
#define CAN_MESSAGE_PARSER_H

#include <map>
#include <cstdint>
#include <vector>


constexpr uint8_t CAN_MAX_DATA = 8;
constexpr uint8_t FF_PAYLOAD = CAN_MAX_DATA - 2;
constexpr uint8_t CF_PAYLOAD = CAN_MAX_DATA - 1;

enum class FrameType : uint8_t {
    SF = 0x00,
    FF = 0x01,
    CF = 0x02,
    FC = 0x03,
    UNKNOWN = 0xFF
};

class CANMessageParser {
public:
    /**
     * @brief Processes a single CAN frame represented as a hexadecimal string.
     *
     * Parses the raw frame and processes it according to ISO-TP rules.
    */
    void proceedFrame(const std::string& frame);
protected:

    struct Message {
        Message() {
            reset();
        }
        void reset() {
            messageBuffer.clear();
            length = 0;
            received = 0;
            next = 0;
            ID = 0;
        }

        void appendData(const std::vector<uint8_t> &data, uint16_t length) {
            length = std::min<uint16_t>(length, data.size());
            messageBuffer.insert(messageBuffer.end(), data.begin(), data.begin() + length);
        }

        uint16_t ID;
        std::vector<uint8_t>  messageBuffer;   // message data
        uint16_t length;                       // total message length
        uint16_t received;                     // number of the message bytes already received
        uint8_t next;                          // expected sequence number for CF frame
    };

    // Active multi-frame message streams indexed by CAN ID
    std::map<uint16_t, Message> messagesStream;

    struct CanFrame {

        CanFrame() {
            reset();
        }

        void reset() {
            ID = 0;
            frameType = FrameType::UNKNOWN;
            pci = 0;
            payloadLength = 0;
            sequenceNumber = 0;
            fcStatus = 0;
            data.clear();
            payload.clear();
            fcBlockSize = 0;
            fcSTmin = 0;
        }
        uint16_t ID;
        FrameType frameType;

        uint8_t pci;                  // PCI byte
        uint16_t payloadLength;       // SF length or FF total length
        uint8_t sequenceNumber;       // CF only
        uint8_t fcStatus;             // FC only
        uint8_t fcBlockSize;          // FC only
        uint8_t fcSTmin;              // FC only

        std::vector<uint8_t> data;    // contain frame data bytes (0..8)
        std::vector<uint8_t> payload; // message data only 
    };

private:

    void processData(const CanFrame& frameData);

    CanFrame parseCanFrame(const std::string &strFrame);

    uint8_t hexByte(const std::string& s, size_t pos);

    std::string toHexString(const std::vector<uint8_t> &data);

    void proceedMessage(const std::vector<uint8_t>& message, uint16_t messageID);
    
    void printErrorMessage(uint16_t messageID, const std::string &message);

    bool validateCFMessageFrame(const CanFrame& frame);
};

#endif