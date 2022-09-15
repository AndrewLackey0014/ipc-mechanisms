#include <sys/mman.h>

#include "SHMRequestChannel.h"

using namespace std;


SHMQueue::SHMQueue (const string _name, int _length) : name(_name), length(_length) {
    // TODO: implement SHMQueue constructor
    //mkfifo(_fifo_name.c_str(), S_IRUSR | S_IWUSR);
    //cout << "hello" << endl;
	int fd = shm_open(_name.c_str(), O_RDWR|O_CREAT, S_IRUSR | S_IWUSR);
    length = _length;
	if (fd < 0) {
		EXITONERROR(_name);
	}
	ftruncate(fd, _length);
    segment = (char*) mmap(NULL, _length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    //recv_done = sem_open((name+"1").c_str(), O_CREAT, S_IRUSR | S_IWUSR, 0);
    send_done = sem_open((name+"2").c_str(), O_CREAT,S_IRUSR | S_IWUSR, 0);
    recv_done = sem_open((name+"1").c_str(), O_CREAT, S_IRUSR | S_IWUSR, 1);
}

SHMQueue::~SHMQueue () {
    // TODO: implement SHMQueue destructor
    munmap(segment, length);
    // shm_unlink(name.c_str());
    sem_close(recv_done);
    sem_close(send_done);
    sem_unlink((name+"1").c_str());
    // sem_close(send_done);
    sem_unlink((name+"2").c_str());
    // munmap(segment, length);
    // shm_unlink(name.c_str());
}

int SHMQueue::shm_receive (void* msgbuf, int msgsize) {
    // TODO: implement shm_receive
    //cout << "shm recieve" << endl;
    // sem_wait(recv_done);
    sem_wait(send_done);
    memcpy(msgbuf, segment, msgsize);
    // sem_post(send_done);
    sem_post(recv_done);
    return msgsize;
}

int SHMQueue::shm_send (void* msgbuf, int msgsize) {
    // TODO: implement shm_send
    //cout << "shm send" << endl;
    // sem_wait(send_done);
    sem_wait(recv_done);
    memcpy(segment, msgbuf, msgsize);
    // sem_post(recv_done);
    sem_post(send_done);
    return msgsize;
}

// TODO: implement SHMRequestChannel constructor/destructor and functions
SHMRequestChannel::SHMRequestChannel (const std::string _name, const Side _side, int _size) : RequestChannel(_name, _side){
    //cout << "shmr constructor" << endl;
    string shm_name1 = "/shm_" + my_name + "1";
	string shm_name2 = "/shm_" + my_name + "2";
	if (my_side == CLIENT_SIDE) {
		rfd = new SHMQueue(shm_name1, _size);
		wfd = new SHMQueue(shm_name2, _size);
	}
	else {
		wfd = new SHMQueue(shm_name1, _size);
		rfd = new SHMQueue(shm_name2, _size);
	}
}
SHMRequestChannel::~SHMRequestChannel (){
    delete rfd;
    delete wfd;
}
int SHMRequestChannel::cread (void* msgbuf, int msgsize){
    return rfd->shm_receive(msgbuf, msgsize);
}
int SHMRequestChannel::cwrite (void* msgbuf, int msgsize){
    return wfd->shm_send(msgbuf,msgsize);
}