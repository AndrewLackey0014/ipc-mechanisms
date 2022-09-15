#ifndef _REQUESTCHANNEL_H_
#define _REQUESTCHANNEL_H_

#include "common.h"


class RequestChannel {
public:
	enum Side {SERVER_SIDE, CLIENT_SIDE};

protected:
    std::string my_name;
    Side my_side;

public:
    RequestChannel (const std::string _name, const Side _side);
    virtual ~RequestChannel ();

    virtual int cread (void* msgbuf, int msgsize) = 0;
    virtual int cwrite (void* msgbuf, int msgsize) = 0;

    std::string name ();
};

#endif
