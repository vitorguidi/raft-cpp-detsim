#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <variant>
#include <string>

namespace IO {

enum MessageName {
    PING_REQUEST,
    PING_RESPONSE,
    APPEND_ENTRIES_REQUEST,
    APPEND_ENTRIES_RESPONSE,
    REQUEST_VOTE_REQUEST,
    REQUEST_VOTE_RESPONSE,
};

struct PingRequest {
    bool operator==(const PingRequest&) const {return true;};
};
struct PingResponse {
    bool operator==(const PingResponse&) const {return true;};
};

struct AppendEntriesRequest {
    int term, leader_id;
    bool operator==(const AppendEntriesRequest& other) const {
        return 
            term==other.term && 
            leader_id==other.leader_id;
    };
};

struct AppendEntriesResponse {
    int term;
    bool operator==(const AppendEntriesResponse& other) const {return term==other.term;};
};

struct RequestVoteRequest {
    int term, candidate_id;
    bool operator==(const RequestVoteRequest& other) const {
        return 
            term==other.term && 
            candidate_id == other.candidate_id;
    }
};

struct RequestVoteResponse {
    int term;
    bool vote_granted;
    bool operator==(const RequestVoteResponse& other) const {
        return 
            term==other.term && 
            vote_granted == other.vote_granted;
    }
};

typedef std::variant<
    PingRequest,
    PingResponse,
    AppendEntriesRequest,
    AppendEntriesResponse,
    RequestVoteRequest,
    RequestVoteResponse
> Message;

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