#include <iostream>
#include "MyDList.hpp"

using namespace std;

class RecordingLink
{
public:
    RecordingLink(uint32_t rate) {m_rate = rate;}
    ~RecordingLink() {} 

    void dump() 
    {
        cout << "recording: m_rate=" << m_rate << endl; 
    }

    DListNode    m_link;       // somehow m_link has to be public

private:
    uint32_t     m_rate; 
};


class DListTester
{
public:
    DListTester()
    {
        // Add one to the list first
        RecordingLink *rlink = new RecordingLink(4200000);
        m_recordingList.addTail(rlink);


        // try to walk the list
        RecordingLink *rlink1;

        for (rlink1 = m_recordingList.getHead();
             rlink1 != m_recordingList.Null();
             rlink1 = m_recordingList.getNext(rlink))
        {
            rlink1->dump();
            
        }
    }

    ~DListTester() {} 

    DLIST_TYPE(RecordingLink, m_link) m_recordingList;
};


int main (void)
{
    DListTester test;
    
    return 0;
}
