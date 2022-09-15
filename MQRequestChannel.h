#ifndef _MQREQUESTCHANNEL_H_
#define _MQREQUESTCHANNEL_H_

#include "RequestChannel.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */

#include <mqueue.h>


class MQRequestChannel : public RequestChannel {
// TODO: declare derived components of MQRequestChannel from RequestChannel
private:
	mqd_t wfd;
    mqd_t rfd;
    int cap;
    std::string mqr_name1;
    std::string mqr_name2;
	
    mqd_t open_mqr (std::string _mqr_name, int _mode);
public:
    MQRequestChannel (const std::string _name, const Side _side, int buffer_capacity);
    ~MQRequestChannel ();
    int cread (void* msgbuf, int msgsize);
    int cwrite (void* msgbuf, int msgsize);
};

#endif
