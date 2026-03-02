#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

struct Frame {
    int page = -1;
    uint32_t age = 0;
    bool ref = false;
};

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

static uint32_t makeMsbMask(int bits) {
    if(bits <= 0) return 0;
    if(bits >= 32) return (1u << 31);
    return (1u << (bits - 1));
}

static void doTick(vector<Frame>& frames, uint32_t msbMask) {
    for(int i = 0; i < (int)frames.size(); i++) {
        if(frames[i].page == -1) continue;

        frames[i].age >>= 1;

        if(frames[i].ref) frames[i].age |= msbMask;

        frames[i].ref = false;
    }
}

static int findPage(const vector<Frame>& frames, int pageNum) {
    for(int i = 0; i < (int)frames.size(); i++) {
        if(frames[i].page == pageNum) return i;
    }
    return -1;
}

static int findEmpty(const vector<Frame>& frames) {
    for(int i = 0; i < (int)frames.size(); i++) {
        if(frames[i].page == -1) return i;
    }
    return -1;
}

static int pickVictim(const vector<Frame>& frames) {
    int best = 0;
    for(int i = 1; i < (int)frames.size(); i++) {
        if(frames[i].age < frames[best].age) best = i;
    }
    return best;
}

static long long simulateOne(const vector<int>& trace, int frameCount, int tickEvery, uint32_t msbMask) {
    vector<Frame> frames(frameCount);

    long long faults = 0;
    long long refs = 0;

    for(int p : trace) {
        refs++;

        int hit = findPage(frames, p);
        if(hit != -1) {
            frames[hit].ref = true;
        } else {
            faults++;

            int slot = findEmpty(frames);
            if(slot == -1) slot = pickVictim(frames);

            frames[slot].page = p;
            frames[slot].age = 0;     // will get msb on next tick
            frames[slot].ref = true;
        }

        if(tickEvery > 0 && (refs % tickEvery == 0)) {
            doTick(frames, msbMask);
        }
    }

    return faults;
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

    uint32_t msbMask = makeMsbMask(bits);

    // for now just run one simulation as a check (using maxFrames)
    long long faults = simulateOne(trace, maxFrames, tickEvery, msbMask);

    cout << "trace refs: " << trace.size() << "\n";
    cout << "frames used (temp): " << maxFrames << "\n";
    cout << "faults: " << faults << "\n";
    cout << "csv output will be: " << outCsv << " (not written yet)\n";

    return 0;
}