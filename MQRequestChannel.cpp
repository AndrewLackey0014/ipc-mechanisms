#include <mqueue.h>

#include "MQRequestChannel.h"

using namespace std;


// TODO: implement MQRequestChannel constructor/destructor and functions
MQRequestChannel::MQRequestChannel (const string _name, const Side _side, int buffer_capacity) : RequestChannel(_name, _side) {
	mqr_name1 = "/mq_" + my_name + "1";
	mqr_name2 = "/mq_" + my_name + "2";
    cap = buffer_capacity;
	if (my_side == CLIENT_SIDE) {
		rfd = open_mqr(mqr_name1, O_RDONLY|O_CREAT);
		wfd = open_mqr(mqr_name2, O_WRONLY|O_CREAT);
	}
	else {
		wfd = open_mqr(mqr_name1, O_WRONLY|O_CREAT);
		rfd = open_mqr(mqr_name2, O_RDONLY|O_CREAT);
	}
}

MQRequestChannel::~MQRequestChannel(){
    mq_close(rfd);
	mq_unlink(mqr_name1.c_str());
	mq_close(wfd);
	mq_unlink(mqr_name2.c_str());
}

mqd_t MQRequestChannel::open_mqr (string _mqr_name, int _mode) {
	//mkfifo(_mqr_name.c_str(), S_IRUSR | S_IWUSR);
    mq_attr mqattr {0, 1, cap, 0}; //mq_flags, mq_maxmsg, mq_msgsize, mq_curmsg
	mqd_t fd = mq_open(_mqr_name.c_str(), _mode, S_IRUSR|S_IWUSR, &mqattr);
	if (fd < 0) {
		EXITONERROR(_mqr_name);
	}
	return fd;
}

int MQRequestChannel::cread (void* msgbuf, int msgsize) {
	return mq_receive(rfd, (char*) msgbuf, 8192, NULL); 
}

int MQRequestChannel::cwrite (void* msgbuf, int msgsize) {
	return mq_send(wfd, (char*) msgbuf, msgsize, 0);
}