#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <cstdlib>

#define INT64 int64_t

#define MAX_PTS_VAL ((0x1LL<<33) - 1)
#define MAX_SIGNED_TIMESTAMP_VAL 0x7FFFFFFFFFFFFFFFLL
#define MAX_HNS  0x7FFFFFFFFFFFFFFFLL

   enum eCompareOperation {COMPARE_GT,
                            COMPARE_LT,
                            COMPARE_GTE,
                            COMPARE_LTE,
                            COMPARE_EQ,
                            COMPARE_NE};

    enum eTimestampType {TIMESTAMP_PTS,
                         TIMESTAMP_HNS,
                         TIMESTAMP_MSEC};
                         //TODO? TIMESTAMP_27MHZ

    typedef struct tTimestamp {
        eTimestampType type;
        INT64 value;

        tTimestamp() : 
            type(TIMESTAMP_PTS),
            value(0) {};

        tTimestamp(eTimestampType _type, INT64 _value) :
            type(_type), value(_value) {};
    }tTimestamp;


   /**
     * Calculate time1 - time2
     * Returns a signed value in the range [-maxTimestamp/2, maxTimestamp/2]
     * Any difference greater than maxTimestamp/2 will be considered a timestamp wrap
     */
    INT64 DiffTimeSigned(const tTimestamp& time1, const tTimestamp& time2) {
        assert(time1.type == time2.type);
        INT64 maxTimestamp = (time1.type == TIMESTAMP_PTS) ? MAX_PTS_VAL : MAX_HNS;
        INT64 diff = (time1.value - time2.value);
        bool wrap = (llabs(diff) > (maxTimestamp >> 1));

        if (wrap) {
            if (diff > 0) {
                diff -= maxTimestamp;
            } else if (diff < 0) {
                diff += maxTimestamp;
            }
        }

        return (diff);        
    }



int main(void)
{
    tTimestamp a(TIMESTAMP_PTS, 10);
    tTimestamp b(TIMESTAMP_PTS, 15);
    tTimestamp c(TIMESTAMP_PTS, MAX_PTS_VAL - 5);
    tTimestamp f(TIMESTAMP_PTS, MAX_PTS_VAL + 5);
     // Hit a corner case, exactly 1/2 the PTS circle
    tTimestamp d(TIMESTAMP_PTS, 0);
    tTimestamp e(TIMESTAMP_PTS, MAX_PTS_VAL >> 1);

    // Jump forward (not across wrap)
    std::cout << "diff=" << DiffTimeSigned(b, a) << ", should be 5" << std::endl;

    // Jump backward (not across wrap)
    std::cout << "diff=" << DiffTimeSigned(a, b) << ", should be -5" << std::endl;

    // Jump forward (across wrap)
    std::cout << "diff=" << DiffTimeSigned(a, c) << ", should be 15" << std::endl;
    
    // Jump backward (across wrap)
    std::cout << "diff=" << DiffTimeSigned(c, a) << ", should be -15" << std::endl;

    std::cout << "diff=" << DiffTimeSigned(e, d) << ", should be " << (MAX_PTS_VAL >> 1) << std::endl;   
    std::cout << "diff=" << DiffTimeSigned(d, e) << ", should be " << ( -1 * (MAX_PTS_VAL >> 1)) << std::endl;  

    return 0;

}
