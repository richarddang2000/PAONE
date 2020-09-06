/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/20
 */
#include "common.h"
#include "FIFOreqchannel.h"

using namespace std;


int main(int argc, char *argv[]){
  int person;
  float seconds;
  int ecgno;
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

    // sending a non-sense message, you need to change this
    //char buf [MAX_MESSAGE];
   // datamsg* x = new datamsg (1, 0.004, 2);

    int opt;
    while ((opt = getopt(argc, argv, "p:t:e:")) != -1) {
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
        }
    }

    datamsg* x = new datamsg (person, seconds, ecgno);

    chan.cwrite (x, sizeof (datamsg));

	double *reply = new double;
	chan.cread(reply, sizeof(double));

	cout << "Server reply is " << *reply << endl;

	delete x;
	delete reply;
    //int nbytes = chan.cread (buf, MAX_MESSAGE);
	//double reply = *(double *) buf;

	//FILE Transfer *******************************************
	ofstream myfile;
    string fname = "1.csv";
	string fpath = "received/" + fname;

	int _offset = 0;
	int _len = 0; //window length
	//max bytes transferred

    filemsg fm (_offset,_len);
	char *buf = new char [MAX_MESSAGE];
	//char fname[] = "teslkansdlkjflasjdf.dat";
	memcpy (buf, &fm, sizeof (filemsg));
	strcpy (buf + sizeof (filemsg), fname.c_str());
	chan.cwrite(buf, sizeof(filemsg) + fname.size() + 1);

	//GET FILE LENGTH
	__int64_t filelen;
	chan.cread(&filelen, sizeof(__int64_t));
	//cout << "File Length: " << filelen << endl;
	int file_length = filelen;
	cout << "file length: " << file_length << endl; //convert __int64_t to int

	//creating a new file message
	myfile.open(fpath);
	int index = 0; // index in the file
	int temp = 1000; //to be replaced with file length later
	while (temp > 0) {
	    if (temp >= 256){
	        _len = 256;
	    }
	    else {
	        _len = temp;
	    }
        filemsg newFM (index, _len);
        char *newBuf = new char [MAX_MESSAGE];
        //char fname[] = "teslkansdlkjflasjdf.dat";
        memcpy (newBuf, &newFM, sizeof (filemsg));
        strcpy (newBuf + sizeof (filemsg), fname.c_str());
        chan.cwrite(newBuf, sizeof(filemsg) + fname.size() + 1);

        char receiveBuf [256];
        chan.cread(receiveBuf, sizeof (receiveBuf));
        cout << "Received: " << receiveBuf << endl << endl;

        temp -= _len;
        index += _len;

        myfile << receiveBuf;
	}
	myfile.close();
/*
    char receiveBuf [256];
    chan.cread(receiveBuf, sizeof (receiveBuf));
    cout << "Received: " << receiveBuf << endl << endl;

    myfile << receiveBuf << endl;
    */

    /*
    filemsg fm (0,256);
    char *buf = new char [MAX_MESSAGE];
    //char fname[] = "teslkansdlkjflasjdf.dat";
    memcpy (buf, &fm, sizeof (filemsg));
    strcpy (buf + sizeof (filemsg), fname.c_str());
    chan.cwrite(buf, sizeof(filemsg) + fname.size() + 1);
     */

// file length
/*
    int tmp_size = 10000;
    while (_offset < tmp_size) {

        if (tmp_size < MAX_MESSAGE) { //if there is less than 256 bytes left
            _len = tmp_size;


        }
        else {
            _len  = MAX_MESSAGE; //is there is more that 256 bytes left

        }

        char recieveBuf [_len];
        chan.cread(recieveBuf, sizeof (recieveBuf));
        cout << "Recieved: " << recieveBuf << endl << endl;

        _offset += _len;
        tmp_size -= _len;
    }
    */




	delete buf;

    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));
}
