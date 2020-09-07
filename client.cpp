/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"
#include<sys/wait.h>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {

    //FORK and EXECVP
    char* args [] = {"./server", "p:t:e:f:m:"};
    pid_t pid = fork();
    cout << "pid = " << pid << endl;
    if (pid == 0){
        execvp(args[0], args);//run server
    }

    int person;
    float seconds;
    int ecgno;
    string fname;
    int buffer_capacity;
    bool file_transfer = false; //Determines whether to to get datamessage or file message
    bool changeBuffer = false; //indicates whether original buffer capacity was changed


    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

    //FIFORequestChannel chan2 ("control2", FIFORequestChannel::CLIENT_SIDE);
    int opt;
    while ((opt = getopt(argc, argv, "p:t:e:f:m:")) != -1) {
        switch (opt) {
            case 'p':
                person = atoi(optarg);
                break;
            case 't':
                seconds = atof(optarg);
                break;
            case 'e':
                ecgno = atoi(optarg);
                break;
            case 'f':
                fname = optarg;
                file_transfer = true;
                break;
            case 'm':
                buffer_capacity = atoi(optarg);
                changeBuffer = true;
                break;
        }
    }


    //if data message chosen, execute loop below
    if (!file_transfer) {
        datamsg *x = new datamsg(person, seconds, ecgno);

        chan.cwrite(x, sizeof(datamsg));

        double *reply = new double;
        chan.cread(reply, sizeof(double));

        cout << "Server reply is " << *reply << endl;

        delete x;
        delete reply;
    }
	//FILE Transfer *************************************************************
	//change length of buffer depending on whether -m was called or not
	int buffer_length;

    if (changeBuffer == true) {
        buffer_length = buffer_capacity;

    }
    else {
        buffer_length = MAX_MESSAGE;
    }

    cout << "buffer_length: " << buffer_length << endl;
	if (file_transfer) {
        string fpath = "received/" + fname ; //file to write to
	    ofstream myfile;

        int _offset = 0;
        int _len = 0; //window length


        filemsg fm(_offset, _len);
        char *buf = new char[buffer_length];
        //char fname[] = "teslkansdlkjflasjdf.dat";
        memcpy(buf, &fm, sizeof(filemsg));
        strcpy(buf + sizeof(filemsg), fname.c_str());
        chan.cwrite(buf, sizeof(filemsg) + fname.size() + 1);

        //GET FILE LENGTH
        __int64_t filelen;
        chan.cread(&filelen, sizeof(__int64_t));
        int file_length = filelen;
        cout << "file length: " << file_length << endl; //convert __int64_t to int

        //WRITING TO FILE FROM EXTRACTING CHANNEL DATA
        myfile.open(fpath);
        int index = 0; // index in the file
        //int temp = 1000; //to be replaced with file length later
        while (file_length > 0) {

            if (file_length >= buffer_length) {
                _len = buffer_length;
            } else {
                _len = file_length;
            }
            filemsg newFM(index, _len);
            char *newBuf = new char[buffer_length];
            //char fname[] = "teslkansdlkjflasjdf.dat";
            memcpy(newBuf, &newFM, sizeof(filemsg));
            strcpy(newBuf + sizeof(filemsg), fname.c_str());
            chan.cwrite(newBuf, sizeof(filemsg) + fname.size() + 1);

            char receiveBuf[_len];
            chan.cread(receiveBuf, sizeof(receiveBuf));
            //cout << "Received: " << receiveBuf << endl << endl;
            myfile.write(receiveBuf, _len);
            //myfile << receiveBuf;
            file_length -= _len;
            index += _len;
            delete newBuf;

        }
        myfile.close();
        delete buf;
    }


    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));
    //closing the new channel
    //chan2.cwrite (&m, sizeof (MESSAGE_TYPE));

}
