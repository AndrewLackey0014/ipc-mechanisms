// your PA1 client code here
#include <sys/wait.h>
#include "common.h"
#include "RequestChannel.h"
#include "FIFORequestChannel.h"
#include "MQRequestChannel.h"
#include "SHMRequestChannel.h"

using namespace std;


int main (int argc, char *argv[]) {
	clock_t start, stop;
	start = clock(); //my timing stuff

	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	bool new_channel = false;
    int number_channels = 1; //number of channels to open in addition to the control channel
	char* new_buffer;
	int buffer_size = MAX_MESSAGE; //default 256
    string ipc_method = "f"; //fifo

	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:i:c:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'c':
                number_channels = atoi (optarg);
				new_channel = true;
				break;
			case 'm':
				buffer_size = atoi (optarg);
				new_buffer = optarg;
				break;
            case 'i':
                ipc_method = optarg;
                break;
		}
	}

	//runs server as child of client
	pid_t pid = fork();
	if(!pid){
        if(buffer_size != 256 && ipc_method != "f"){//change both buffer and ipc
		    char* args[] = {(char*) string("./server").c_str(), (char*) string("-m").c_str(), new_buffer, (char*) string("-i").c_str(), (char*) ipc_method.c_str(), NULL};
		    execvp(args[0], args);
		    perror("exec failing");
        }else if(buffer_size == 256 && ipc_method != "f"){//change ipc
		    char* args[] = {(char*) string("./server").c_str(), (char*) string("-i").c_str(), (char*) ipc_method.c_str(), NULL};
		    execvp(args[0], args);
		    perror("exec failing");
        }else if(buffer_size != 256 && ipc_method == "f"){//change buffer
            char* args[] = {(char*) string("./server").c_str(), (char*) string("-m").c_str(), new_buffer, NULL};
		    execvp(args[0], args);
		    perror("exec failing");
        }else{//change none
            char* args[] = {(char*) string("./server").c_str(), NULL};
		    execvp(args[0], args);
		    perror("exec failing");
        }
	}


	string channel_name = "control";
    RequestChannel* control;
    vector<RequestChannel*> vec_chan;
    RequestChannel* chan;
    if (ipc_method == "f"){
        control = new FIFORequestChannel(channel_name, RequestChannel::CLIENT_SIDE);
    }else if(ipc_method == "q"){
        control = new MQRequestChannel(channel_name, RequestChannel::CLIENT_SIDE, buffer_size);
    }else if(ipc_method == "s"){
		control = new SHMRequestChannel(channel_name, RequestChannel::CLIENT_SIDE, buffer_size);
    }
    //switch channels if needed
    if(new_channel){// modify for pa4 to accept and int of number of channels to create
        for(int i = 0; i < number_channels; i++){
            MESSAGE_TYPE new_chan = NEWCHANNEL_MSG;
            control->cwrite(&new_chan, sizeof(MESSAGE_TYPE));
            char * buf_newc = new char[buffer_size];
            control->cread(buf_newc, sizeof(string));
            channel_name = buf_newc;
            delete[] buf_newc;
            if(ipc_method == "f"){
                chan = new FIFORequestChannel(channel_name.c_str(), RequestChannel::CLIENT_SIDE);
            }else if(ipc_method == "q"){
                chan = new MQRequestChannel(channel_name.c_str(), RequestChannel::CLIENT_SIDE, buffer_size);
            }else if(ipc_method == "s"){
				chan = new SHMRequestChannel(channel_name.c_str(), RequestChannel::CLIENT_SIDE, buffer_size);
            }
            //cout << "control:" << control << endl;
            //cout << "chan:" << chan << endl;
            vec_chan.push_back(chan);
        }
    }
	char buf[MAX_MESSAGE]; // 256
	//we are reading 1000 lines of data
	if(p != -1 && t == -1 && e == -1 && filename == ""){
        cout << "are we here" << endl;
        for(int j = 0; j < number_channels; j++){//modified
            int entry_number = 0;
            string location;
            if(new_channel){
                location = "received/x" + to_string(j+1) + ".csv";
            }else{
                location = "received/x1.csv";
            }
            ofstream x1File;
            x1File.open(location);
            for(double i = 0; i < 4; i += .004){
                entry_number += 1;
                datamsg ecg1(p, i, 1);
                memcpy(buf, &ecg1, sizeof(datamsg));
                if(new_channel){
                    vec_chan.at(j)->cwrite(buf, sizeof(datamsg)); // question
                }else{
                    control->cwrite(buf, sizeof(datamsg));
                }
                double reply1;
                if (new_channel){
                    vec_chan.at(j)->cread(&reply1, sizeof(double)); //answer
                }else{
                    control->cread(&reply1, sizeof(double));
                }
                datamsg ecg2(p, i, 2);
                memcpy(buf, &ecg2, sizeof(datamsg));
                if (new_channel){
                    vec_chan.at(j)->cwrite(buf, sizeof(datamsg)); // question
                }else{
                    control->cwrite(buf, sizeof(datamsg));
                }
                double reply2;
                if(new_channel){
                    vec_chan.at(j)->cread(&reply2, sizeof(double)); //answer
                }else{
                    control->cread(&reply2, sizeof(double));
                }
                if(entry_number == 1000){
                    x1File << i << "," <<  reply1 << "," << reply2;
                }else{
                    x1File << i << "," << reply1 << "," << reply2 << "\n";
                }
            }
            x1File.close();
        }
	}
	//we are reading a single data point
	else if(p != -1 && t != -1 && e != -1 && filename == ""){//modified
        datamsg x(p, t, e);
        memcpy(buf, &x, sizeof(datamsg));
        control->cwrite(buf, sizeof(datamsg)); // question
        double reply;
        control->cread(&reply, sizeof(double)); //answer
        cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
    }
	//we are transfering a file
	else if(p == -1 && t == -1 && e == -1 && filename != ""){

		//cout << "file buffer size :" << buffer_size << endl;
		filemsg fm(0, 0);
		string fname = filename; //added filename

		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		control->cwrite(buf2, len);  // I want the file length;
		__int64_t size; //size of file
		control->cread(&size, sizeof(__int64_t));
		int iterations = ceil(double(size)/buffer_size); //calculates above iterations

		filemsg* fm2 = (filemsg*) buf2;
		fm2->offset = 0;//starting offset

		if(iterations == 1){ //does it take 1 cycle
			fm2->length = size;

		}else{ //doesnt take 1 cycle
			fm2->length = buffer_size;
		}

		__int64_t last_size = size - buffer_size * (iterations - 1); //final cycle size
		if(new_channel){
			vec_chan.at(0)->cwrite(buf2, len);
		}else{
			control->cwrite(buf2, len);
		}
		char* buf3 = new char[buffer_size];
		if(new_channel){
			vec_chan.at(0)->cread(buf3, buffer_size);
		}else{
			control->cread(buf3, buffer_size);
		}
		FILE* fp = fopen(("received/"+fname).c_str(), "wb");
		fwrite(buf3, 1, fm2->length, fp);
		int j = 1;
		for(int i = 1; i < iterations; i++){

			if(j == number_channels && new_channel){
				j = 0;
			}
			if(i == iterations-1){ //last cycle
				//cout << iterations << " " << i << " " << last_size << " Jump" << endl;
				fm2->length = last_size;
				delete[] buf3;
				buf3 = new char[last_size];
				fm2->offset += buffer_size;
				if(new_channel){
					vec_chan.at(j)->cwrite(buf2, len);
					vec_chan.at(j)->cread(buf3, buffer_size);
				}else{
					control->cwrite(buf2, len);
					control->cread(buf3, buffer_size);
				}
				fwrite(buf3, 1, fm2->length, fp);

			}else{ //every other cycle
				fm2->offset += buffer_size;
				if(new_channel){
					vec_chan.at(j)->cwrite(buf2, len);
					vec_chan.at(j)->cread(buf3, buffer_size);
				}else{
					control->cwrite(buf2, len);
					control->cread(buf3, buffer_size);
				}
				fwrite(buf3, 1, fm2->length, fp);
			}
			j++;
		}
		fclose(fp);
		delete[] buf3;
		delete[] buf2;

	}

    // sending a non-sense message, you need to change this
	stop = clock();
	double time_taken = double(stop - start) / double(CLOCKS_PER_SEC);
	cout << "time taken: " << fixed << time_taken << endl;
	// closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;

	if(new_channel){//if we have new channel send quit
        for(int i = 0; i < number_channels; i++){
            vec_chan.at(i)->cwrite(&m, sizeof(MESSAGE_TYPE));
            delete vec_chan.at(i);
        }
        vec_chan.clear();
	}//send quit to control no matter what
	control->cwrite(&m, sizeof(MESSAGE_TYPE));
	delete control;
	wait(NULL);
	cout << "client exited" << endl;
}
