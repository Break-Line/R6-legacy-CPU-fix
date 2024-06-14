#include <cstdio>
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>
#include <sstream>
#include <winuser.h>
#include <string>

using namespace std;

const char PROCESS_NAME[] = "RainbowSix.exe";

struct pStatus {
    bool pFound;
    bool pFixed;
};

/**
 * return sleep time in seconds
*/
int sleep(float x) {
    int timeSpan;
    int totTime;
    
    x = x * 1000;
    if (x <= 1000) 
        timeSpan = 500;
    else
        timeSpan = 1000;
    for (int i=0; i<x; i+=timeSpan) {
        Sleep(timeSpan);
        cout << '.';
        totTime = i;
    }
    cout << endl;
    return totTime/1000 + 1;
}

bool fixProcess(PROCESSENTRY32 process) {
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process.th32ProcessID);

    // set affinity to 1st core only
    if (!SetProcessAffinityMask(hProcess, (DWORD_PTR)1)) {
        throw string("Cannot set the first core");
    }

    // set affinity to all cores, 15 does the trick :)
    if (!SetProcessAffinityMask(hProcess, (DWORD_PTR)15)) {
        throw string("Cannot set all cores");
    }
    CloseHandle(hProcess);
}

bool getProcess(int sleepingTimeParam, PROCESSENTRY32 * const &process) {
    process->dwSize = sizeof(PROCESSENTRY32);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (Process32First(snapshot, process) == TRUE) {
        while (Process32Next(snapshot, process) == TRUE)
        {
            if (_tcsicmp(process->szExeFile, PROCESS_NAME) == 0)
            {
                cout << endl;
                return true;
            }
        }
        cout << endl;
        return false;
    }
}

void describeProcess(PROCESSENTRY32 process) {
    cout << "[==============]" << endl;
    cout << "|PID: " << process.th32ProcessID << endl;
    cout << "|Threads: " << process.cntThreads << endl;
    cout << "|Parent PID: " << process.th32ParentProcessID << endl;
    cout << "|Base Priority: " << process.pcPriClassBase << endl;
    cout << "[==============]" << endl;
}

/**
 * argv[1] = set sleeping time in minutes (min 1)
 * TODO: if process is killed, then close script;
 * 
*/
int main( int, char* argv[] )
{
    PROCESSENTRY32 process;
    /* tagLASTINPUTINFO lastInput = {sizeof(LASTINPUTINFO), 0};
    if(!GetLastInputInfo(&lastInput)) {
        cout << endl << "failed retrieving lastInput" << endl;
    } else {
        cout << endl << lastInput.dwTime << endl;
        return 0;
    } */
    // note, cannot use bool because of specialization
    /* vector<unsigned char> previous(256);
    vector<unsigned char> current(256);

    // in update_keys or similar:
    current.swap(previous); // constant time, yay
    GetKeyboardState(&current[0]); // normally do error checking
 */
    
    const int vKey = VK_END;
    const int sleepingTimeParam = atoi(argv[1]) * 60;
    pStatus pStatus = {false, false};
    int tries = 0;

    while (tries < 5) {
        if (!pStatus.pFixed) {
            cout << "waiting key..." << endl;
            while ((GetKeyState(VK_END) & 0x8000) == 0) { //0x8000 captures it every time
                // not pressed
            }
        }

        pStatus = {false, false};
        
        try {
            if(getProcess(sleepingTimeParam, &process)) {
                cout << PROCESS_NAME << " FOUND" << endl;
                if (tries == 0)
                describeProcess(process);
            }
            else {
                cout << PROCESS_NAME << " NOT FOUND" << endl;
                tries++;
                sleep(3);
                continue;
            }
            if(fixProcess(process)) {
                cout << PROCESS_NAME << " FIXED" << endl;
                pStatus.pFixed = true;
                tries = 0;
            }
        } catch(string e) {
            cout << endl << "ERR: " << e << endl;
        }

        int runningTime = 0; // seconds
        int pingAliveTime = 5; // seconds
        int minSleepingTime = 60; //seconds
        int sleepingTime = 300; // seconds

        if (sleepingTimeParam >= minSleepingTime ) {
            sleepingTime = sleepingTimeParam;
        }
        
        cout << "sleeping(" + to_string(sleepingTime) + "sec)..." << endl;
        runningTime = sleep(sleepingTime);

        cout << endl << "wakeup after " + to_string(runningTime) + "sec" << endl << endl;
    }
    cout << endl << "Closing";
    sleep(3);
    return 0;
}
