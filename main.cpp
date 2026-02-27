#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

static void printUsage(const char* prog) {
    cout << "usage:\n";
    cout << "  " << prog << " <trace_file> <max_frames> <tick_every> <bits> <output_csv>\n";
    cout << "example:\n";
    cout << "  " << prog << " trace.txt 50 10 8 results.csv\n";
}

static vector<int> readTraceFile(const string& path) {
    vector<int> trace;
    ifstream in(path);

    if(!in.is_open()) {
        cerr << "cant open file: " << path << "\n";
        return trace;
    }

    int x;
    while(in >> x) trace.push_back(x);

    return trace;
}

int main(int argc , char* argv[]) {
    cout << "paging aging simulator (task 3)\n";

    if(argc < 6) {
        printUsage(argv[0]);
        return 0;
    }

    string tracePath = argv[1];
    int maxFrames = 0;
    int tickEvery = 0;
    int bits = 0;
    string outCsv = argv[5];

    try {
        maxFrames = stoi(argv[2]);
        tickEvery = stoi(argv[3]);
        bits = stoi(argv[4]);
    } catch(...) {
        cout << "bad number in args\n";
        printUsage(argv[0]);
        return 0;
    }

    if(maxFrames <= 0 || tickEvery <= 0 || bits <= 0 || bits > 32) {
        cout << "args not valid (max_frames>0, tick_every>0, bits 1..32)\n";
        return 0;
    }

    vector<int> trace = readTraceFile(tracePath);
    if(trace.empty()) {
        cout << "trace is empty (or file format not plain integers)\n";
        return 0;
    }

    cout << "trace: " << tracePath << "  refs=" << trace.size() << "\n";
    cout << "maxFrames=" << maxFrames << " tickEvery=" << tickEvery << " bits=" << bits << "\n";
    cout << "csv output will be: " << outCsv << "\n";

    return 0;
}