#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

static vector<int> readTraceFile(const string& path) {
    vector<int> trace;
    ifstream in(path);

    if(!in.is_open()) {
        cerr << "cant open file: " << path << "\n";
        return trace;
    }

    int x;
    while(in >> x) {
        trace.push_back(x);
    }

    return trace;
}

int main(int argc , char* argv[]) {
    cout << "paging aging simulator (task 3)\n";

    if(argc < 2) {
        cout << "usage: " << argv[0] << " <trace_file>\n";
        return 0;
    }

    string filePath = argv[1];
    vector<int> trace = readTraceFile(filePath);

    if(trace.empty()) {
        cout << "trace is empty (or file format not plain integers)\n";
        return 0;
    }

    cout << "trace file: " << filePath << "\n";
    cout << "refs read: " << trace.size() << "\n";

    cout << "first few: ";
    for(int i = 0; i < (int)trace.size() && i < 10; i++) {
        cout << trace[i] << " ";
    }
    cout << "\n";

    return 0;
}
