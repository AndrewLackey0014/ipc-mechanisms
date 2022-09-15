#ifndef _SHMREQUESTCHANNEL_H_
#define _SHMREQUESTCHANNEL_H_

#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "RequestChannel.h"


class SHMQueue {
private:
    std::string name;
    int length;

    char* segment;
    sem_t* recv_done;
    sem_t* send_done;
    
public:
    SHMQueue(const std::string _name, int _length);
    ~SHMQueue();

    int shm_receive (void* msgbuf, int msgsize);
    int shm_send (void* msgbuf, int msgsize);
};

class SHMRequestChannel : public RequestChannel {
// TODO: declare derived components of SHMRequestChannel from RequestChannel
private:
    SHMQueue* rfd;
    SHMQueue* wfd;
public:
    SHMRequestChannel (const std::string _name, const Side _side, int _size);
    ~SHMRequestChannel ();
    int cread (void* msgbuf, int msgsize);
    int cwrite (void* msgbuf, int msgsize);
};

#endif
