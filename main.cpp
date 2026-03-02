#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <iomanip>

using namespace std;

struct Frame {
    int page = -1;
    uint32_t age = 0;
    bool ref = false;
};

struct Result {
    long long faults = 0;
    long long refs = 0;
    double per1000 = 0.0;
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

static double calcPer1000(long long faults, long long refs) {
    if(refs == 0) return 0.0;
    return (double)faults * 1000.0 / (double)refs;
}

static Result simulateOne(const vector<int>& trace, int frameCount, int tickEvery, uint32_t msbMask) {
    vector<Frame> frames(frameCount);

    Result r;

    for(int p : trace) {
        r.refs++;

        int hit = findPage(frames, p);
        if(hit != -1) {
            frames[hit].ref = true;
        } else {
            r.faults++;

            int slot = findEmpty(frames);
            if(slot == -1) slot = pickVictim(frames);

            frames[slot].page = p;
            frames[slot].age = 0;
            frames[slot].ref = true;
        }

        if(tickEvery > 0 && (r.refs % tickEvery == 0)) {
            doTick(frames, msbMask);
        }
    }

    r.per1000 = calcPer1000(r.faults, r.refs);
    return r;
}

static bool writeCsvAllFrames(const string& outCsv,
                             const vector<int>& trace,
                             int maxFrames,
                             int tickEvery,
                             uint32_t msbMask) {

    ofstream out(outCsv);
    if(!out.is_open()) {
        cerr << "cant write csv: " << outCsv << "\n";
        return false;
    }

    out << "frames,faults,refs,faults_per_1000\n";

    for(int f = 1; f <= maxFrames; f++) {
        Result r = simulateOne(trace, f, tickEvery, msbMask);

        out << f << "," << r.faults << "," << r.refs << ",";
        out << fixed << setprecision(3) << r.per1000 << "\n";
    }

    return true;
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

    cout << "trace refs: " << trace.size() << "\n";
    cout << "running frames 1.." << maxFrames << "\n";

    bool ok = writeCsvAllFrames(outCsv, trace, maxFrames, tickEvery, msbMask);
    if(ok) cout << "wrote csv: " << outCsv << "\n";

    return 0;
}