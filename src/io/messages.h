#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <variant>
#include <string>

namespace IO {

enum MessageName {
    PING_REQUEST,
    PING_RESPONSE,
};

struct PingRequest {
    bool operator==(const PingRequest&) const {return true;};
};
struct PingResponse {
    bool operator==(const PingResponse&) const {return true;};
};

typedef std::variant<PingRequest, PingResponse> Message;

struct Envelope {
    int message_id;
    MessageName name;
    int from;
    int to;
    Message content;
    bool operator==(const Envelope& other) const {
        return 
            message_id == other.message_id &&
            name == other.name &&
            from == other.from &&
            to == other.to &&
            content == other.content;
    }
};
} // namespace IO

#endif // _NETWORH_H_