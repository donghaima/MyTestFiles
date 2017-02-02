#include <stdio.h>
#include <string.h>

#define MAX_LOG_FORMAT_LENGTH (2048)
#define MAX_TIMESTAMP_LENGTH (20)

void bar(const char* combinedFormat)
{
    printf("bar(): combinedFormat=%s\n", combinedFormat);    
}

void foo(const char* tag, const char* format)
{
    char timestamp[MAX_TIMESTAMP_LENGTH] = "13:01:42.950:";
    char channelDebugPrefix[] = "chan1:0000-0001-0002-012121";

    // The combined format string
    static char combinedFormat[MAX_LOG_FORMAT_LENGTH] = {0};

    // Add the timestamp and threadId to the format string
    snprintf(
        combinedFormat,
        MAX_LOG_FORMAT_LENGTH,
        "[%s],%s%s:%s",
        timestamp,
        tag,
	channelDebugPrefix,
        format);

    bar(combinedFormat);

    printf("foo(): combinedFormat=%s\n", combinedFormat);
}


int main (void)
{
    const char file[] = "src/HTTPSession_linux.cpp";
    const char function[] = "WriteData";
    int linenumber = 481;
    const char tag[] = "HttpSession";
    const char format[] = "Resetting HTTP";

    static char buffer[MAX_LOG_FORMAT_LENGTH];
    static int marker = 0;

    // Get the length of the format string
    marker = strlen(format);

    strcpy(buffer, format);
    sprintf(buffer + marker, " File: %s, Function: %s, Line: %i", file, function, linenumber);

    
    foo(tag, buffer);

    return 0;
}
