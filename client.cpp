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
#include <chrono>
#include <sys/time.h>

using namespace std;

int main(int argc, char *argv[]) {

    //FORK and EXECVP
    char* args [] = {"./server", "p:t:e:f:m:c"};
    pid_t pid = fork();
    cout << "pid = " << pid << endl;
    if (pid == 0){
        execvp(args[0], args );
        //run server
    }


    int person;
    float seconds;
    int ecgno;

    string fname;
/*
    cout << "server name: " << server << endl;
    //cout << "string is: ./server -m 256" << endl;
    cout << (server_array == "./server -m 256") << endl;
    cout << (1 == 1) << endl;
*/
    int buffer_capacity;
    char newChannelName[100];
    bool newChannel = false; // was -c and argument?
    bool file_transfer = false; //Determines whether to to get datamessage or file message
    bool changeBuffer = false; //indicates whether original buffer capacity was changed

    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

    int opt;
    while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
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
            case 'c':
                newChannel = true;

                MESSAGE_TYPE m = NEWCHANNEL_MSG;
                chan.cwrite(&m, sizeof(MESSAGE_TYPE));

                char newChannelName[100];
                chan.cread(newChannelName, sizeof (char[100]));

                cout << "new channel name: " << newChannelName << endl;
                FIFORequestChannel(newChannelName, FIFORequestChannel::CLIENT_SIDE);

                break;
        }
    }

    //if data message chosen, execute loop below
    if (!file_transfer && !newChannel) {
        datamsg *x = new datamsg(person, seconds, ecgno);

        chan.cwrite(x, sizeof(datamsg));

        double *reply = new double;
        chan.cread(reply, sizeof(double));

        cout << "Server reply is " << *reply << endl;

        //REQUEST 1000 data points and move to x1.csv

        struct timeval start, stop;
        int person = 1;
        double seconds  = 0.000;
        string fname = "x1.csv";
        string fpath = "received/" + fname ; //file to write to
        ofstream myfile;
        myfile.open(fpath);
        while (seconds < 5) {
            gettimeofday(&start, NULL);
            int ecgno = 1;
            myfile << seconds << "\t";
            while (ecgno <= 2) {
                datamsg *x = new datamsg(1, seconds, ecgno);
                chan.cwrite(x, sizeof(datamsg));
                double *reply = new double;
                chan.cread(reply, sizeof(double));

                myfile << *reply << "\t";
                ecgno++;
            }
            myfile << endl;
            seconds += 0.004;
        }
        gettimeofday(&stop, NULL);
        double time = (stop.tv_usec - start.tv_usec);
        cout << "time to transfer 1250 data points: " << time << " microseconds" << endl;
        myfile.close();
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

    //cout << "buffer_length: " << buffer_length << endl;
	if (file_transfer) {
	    auto start = chrono::steady_clock::now(); //clock start
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
        auto end = chrono::steady_clock::now();
        auto time = chrono::duration<double, milli>(end - start);
        cout << "Window Size: " << buffer_length << ", executed in " << time.count() << " milliseconds" << endl;
        myfile.close();
        delete buf;
    }


    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));


    if (newChannel) {
        //creating new channel
        cout << "Reading from channel 2 now..." << endl;
        FIFORequestChannel chan2("data_1", FIFORequestChannel::CLIENT_SIDE);

        datamsg *x = new datamsg(1, 0.000, 1);
        chan2.cwrite(x, sizeof(datamsg));
        double *reply2 = new double;
        chan2.cread(reply2, sizeof(double));
        cout << "Server reply is " << *reply2 << endl;


        delete reply2;

        // closing the  NEW channel
        MESSAGE_TYPE n = QUIT_MSG;
        chan2.cwrite (&n, sizeof (MESSAGE_TYPE));
    }


    sleep(3);
    wait(NULL);
}
