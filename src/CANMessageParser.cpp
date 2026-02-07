#include <iostream> 
#include "CANMessageParser.h"


/**
 * @brief Entry point for processing a CAN frame in hex string form.
 *
 * Parses the frame into an internal CanFrame structure and forwards it
 * to the appropriate handler based on ISO-TP frame type.
 *
 * @param frame Hexadecimal string representing a single CAN frame.
 */
void CANMessageParser::proceedFrame(const std::string& frame) {
    CanFrame frameData = parseCanFrame(frame);
    processData(frameData);
}

/**
 * @brief Parses a raw CAN frame string into a CanFrame structure.
 *
 * Extracts the CAN ID, data bytes, and interprets the first byte as PCI.
 * Determines frame type (SF, FF, CF, FC) and sets payload, sequence number,
 * or flow control fields accordingly.
 *
 * @param strFrame Raw CAN frame as a hexadecimal string.
 * @return CanFrame Parsed frame data.
 */
CANMessageParser::CanFrame CANMessageParser::parseCanFrame(const std::string& strFrame) {
    CanFrame frameData{};

    // message ID 
    frameData.ID = static_cast<uint16_t>(std::stoul(strFrame.substr(0, 3), nullptr, 16));

    // frame DATA  
    for (size_t i = 3; i + 1 < strFrame.size(); i += 2) {
        frameData.data.push_back(hexByte(strFrame, i));
        if (frameData.data.size() == 8) break;
    }

    // no data, returning empty framedata
    if (frameData.data.empty()) return frameData;

    frameData.pci = frameData.data[0];
    uint8_t pciType = frameData.pci >> 4;

    // process the frame according to its type
    switch (pciType) {    

        case 0x0: // (SF) Single Frame 
            frameData.frameType = FrameType::SF;
            frameData.payloadLength = frameData.pci & 0x0F;
            if(frameData.payloadLength > frameData.data.size() - 1) {
                printErrorMessage(frameData.ID, "Invalid frame data size!");
                frameData.frameType = FrameType::UNKNOWN;
                break;
            }
            frameData.payload.assign(frameData.data.begin() + 1, frameData.data.begin() + 1 + frameData.payloadLength);
            break;
    
        case 0x1: // (FF) First Frame (multi-frame start) 
            frameData.frameType = FrameType::FF;
            frameData.payloadLength = ((frameData.pci & 0x0F) << 8) | frameData.data[1];
            frameData.payload.assign(frameData.data.begin() + 2, frameData.data.end());
            break;

        case 0x2: // (CF) Consecutive Frame
            frameData.frameType = FrameType::CF;
            frameData.sequenceNumber = frameData.pci & 0x0F;
            frameData.payloadLength = frameData.data.size() - 1;
            frameData.payload.assign(frameData.data.begin() + 1, frameData.data.end());
            break;

        case 0x3: // (FC) Flow Control
            frameData.frameType = FrameType::FC;
            frameData.payload.assign(frameData.data.begin() + 1, frameData.data.end());
            if(frameData.payload.size() < 3) {
                frameData.frameType = FrameType::UNKNOWN;
                break;    
            }
            frameData.fcStatus =  (frameData.payload[0] >> 5) & 0x07;
            frameData.fcBlockSize = frameData.payload[1];
            frameData.fcSTmin = frameData.payload[2];
            break;

        default:
            frameData.frameType = FrameType::UNKNOWN;
            break;
    }

    return frameData;
}

/**
 * @brief Handles the parsed CanFrame based on its type.
 *
 * - SF: directly forwards payload
 * - FF: initializes a multi-frame message in messagesStream
 * - CF: appends payload to existing multi-frame message
 * - FC: not handled yet
 * - UNKNOWN: prints error
 *
 * @param frameData Parsed CAN frame.
 */
void CANMessageParser::processData(const CanFrame& frameData) {
    uint16_t remaining;
    uint8_t copyLen;
    switch(frameData.frameType) {
        
        case FrameType::SF:
            if(messagesStream.find(frameData.ID) != messagesStream.end()) {
                messagesStream.erase(frameData.ID);
                printErrorMessage(frameData.ID, "Message reset!");
                break;
            }
            proceedMessage(frameData.payload, frameData.ID);
            break;
           
        case FrameType::FF:
                messagesStream[frameData.ID].ID = frameData.ID;
                messagesStream[frameData.ID].length = frameData.payloadLength;
                copyLen = std::min<uint16_t>(FF_PAYLOAD, frameData.payloadLength);
                messagesStream[frameData.ID].received = copyLen;
                messagesStream[frameData.ID].next = 1;
                messagesStream[frameData.ID].appendData(frameData.payload, copyLen);
                break;
        
        case FrameType::CF:
            if(!validateCFMessageFrame(frameData)) break;
                
            remaining = messagesStream[frameData.ID].length - messagesStream[frameData.ID].received;
            copyLen = std::min<uint16_t>(CF_PAYLOAD, remaining);
            messagesStream[frameData.ID].appendData(frameData.payload, copyLen);
            messagesStream[frameData.ID].received += copyLen;
            messagesStream[frameData.ID].next = (messagesStream[frameData.ID].next + 1) & 0x0F;

            if (messagesStream[frameData.ID].received >= messagesStream[frameData.ID].length) {
                proceedMessage(messagesStream[frameData.ID].messageBuffer, frameData.ID);
                messagesStream.erase(frameData.ID);
            }
            break;

        case FrameType::FC:
            // not handled yet.
            break;

        case FrameType::UNKNOWN:
            printErrorMessage(0, "Unknown frame type");
            break;
    }
}

/**
 * @brief Converts 2 hex characters at position `pos` into a uint8_t.
 *
 * @param s Input string containing hexadecimal digits
 * @param pos Starting index of the byte in the string
 * @return uint8_t Parsed byte
 */
uint8_t CANMessageParser::hexByte(const std::string& s, size_t pos) {
    return static_cast<uint8_t>(std::stoul(s.substr(pos, 2), nullptr, 16));
}

/**
 * @brief Converts a vector of bytes to an uppercase hexadecimal string.
 *
 * @param data Vector of bytes
 * @return std::string Hexadecimal string
 */
std::string CANMessageParser::toHexString(const std::vector<uint8_t> &data) {
    static const char hex[] = "0123456789ABCDEF";

    std::string rv;
    rv.reserve(data.size() * 2);

    for (uint8_t b : data) {
        rv.push_back(hex[b >> 4]);
        rv.push_back(hex[b & 0x0F]);
    }

    return rv;
}

/**
 * @brief Prints a CAN message to stdout.
 *
 * @param message Payload bytes
 * @param messageID CAN ID
 */
void CANMessageParser::proceedMessage(const std::vector<uint8_t>& message, uint16_t messageID) {
    std::cout << std::uppercase << std::hex << messageID << ": " << toHexString(message) << std::dec << std::endl;
}

/**
 * @brief Prints an error message to stderr with the CAN ID.
 *
 * @param messageID CAN ID
 * @param message Error description
 */
void CANMessageParser::printErrorMessage(uint16_t messageID, const std::string &message) {
    std::cerr << "ERROR Message: 0x" << std::uppercase << std::hex << messageID  << std::dec << " " << message << std::endl;
}

/**
 * @brief Validates a Consecutive Frame (CF) against the expected multi-frame stream.
 *
 * Checks that the CAN ID exists in the active messagesStream and that the sequence number
 * matches the expected value.
 *
 * @param frame Parsed CF frame
 * @return true If the frame is valid
 * @return false Otherwise
 */
bool CANMessageParser::validateCFMessageFrame(const CanFrame& frame) {    
    if (messagesStream.find(frame.ID) == messagesStream.end()) {
        printErrorMessage(frame.ID, "Invalid CF Frame!");
        return false;
    }
        
    if(frame.sequenceNumber != messagesStream[frame.ID].next) {               
        printErrorMessage(frame.ID, "invalid CF frame sequence");
        return false;
    }

    return true;
}