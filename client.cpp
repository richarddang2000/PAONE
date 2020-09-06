/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"

using namespace std;


int main(int argc, char *argv[]) {
    int person;
    float seconds;
    int ecgno;
    string fname;
    bool file_transfer = false; //Determines whether to to get datamessage or file message
    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);

    //NEW CHANNEL
    //FIFORequestChannel chan2 ("control2", FIFORequestChannel::CLIENT_SIDE);

    // sending a non-sense message, you need to change this
    //char buf [MAX_MESSAGE];
    // datamsg* x = new datamsg (1, 0.004, 2);

    int opt;
    while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
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
    //int nbytes = chan.cread (buf, MAX_MESSAGE);
	//double reply = *(double *) buf;

	//FILE Transfer *************************************************************
	if (file_transfer) {
        string fpath = "received/" + fname ; //file to write to
	    ofstream myfile;

        int _offset = 0;
        int _len = 0; //window length


        filemsg fm(_offset, _len);
        char *buf = new char[MAX_MESSAGE];
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
        int loops = 0; // how many times loop below executes (195)
        while (file_length > 0) {

            if (file_length >= MAX_MESSAGE) {
                _len = MAX_MESSAGE;
            } else {
                _len = file_length;
            }
            filemsg newFM(index, _len);
            char *newBuf = new char[MAX_MESSAGE];
            //char fname[] = "teslkansdlkjflasjdf.dat";
            memcpy(newBuf, &newFM, sizeof(filemsg));
            strcpy(newBuf + sizeof(filemsg), fname.c_str());
            chan.cwrite(newBuf, sizeof(filemsg) + fname.size() + 1);

            char receiveBuf[_len];
            chan.cread(receiveBuf, sizeof(receiveBuf));
            cout << "Received: " << receiveBuf << endl << endl;
            myfile.write(receiveBuf, _len);
            //myfile << receiveBuf;
            loops +=1;
            file_length -= _len;
            index += _len;
            delete newBuf;

        }
        myfile.close();
        delete buf;
        cout << "loops: " << loops << endl;
    }


    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));
    //closing the new channel
    //chan2.cwrite (&m, sizeof (MESSAGE_TYPE));


}
