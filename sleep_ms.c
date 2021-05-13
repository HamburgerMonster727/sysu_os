#include <sys/time.h>

void sleep_ms(long int timegap_ms)
{
    struct timeval t;
/*  __time_t tv_sec;
    __suseconds_t tv_usec;
*/
    long curr_s, curr_ms, end_ms;

    gettimeofday(&t, 0);
    curr_s = t.tv_sec;
    curr_ms = (long)(t.tv_sec * 1000);
    end_ms = curr_ms + timegap_ms;
    while (1) {
        gettimeofday(&t, 0);
        curr_ms = (long)(t.tv_sec * 1000);
        if (curr_ms > end_ms) {
            break;
        }
    }
    return;
}

