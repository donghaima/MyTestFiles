#include <stdio.h>
#include <string.h>

int main(void)
{
    char *args[7];
    int argIndex = 0;

    printf("argIndex = %d\n", argIndex);

    char* chanExe = new char[20];
    strcpy(chanExe, "./Channel");
    args[argIndex++] = chanExe;

    printf("argIndex = %d, args[%d]=%s\n", 
           argIndex, argIndex-1, args[argIndex-1]);

    int m_port = 9400;
    char portArg[50];
    //char* portArg = new char[50];
    sprintf(portArg, "--port=%d", m_port);
    args[argIndex++] = portArg;

    printf("argIndex = %d, args[%d]=%s\n", 
           argIndex, argIndex-1, args[argIndex-1]);

    char statusArg[50];
    //char* statusArg = new char[50];
    sprintf(statusArg, "--status_port=9300");
    args[argIndex++] = statusArg;

    printf("argIndex = %d, args[%d]=%s\n", 
           argIndex, argIndex-1, args[argIndex-1]);

    bool debug = false;
    if (debug == true) {
        args[argIndex++] = "--debug";

        printf("argIndex = %d, args[%d]=%s\n", 
               argIndex, argIndex-1, args[argIndex-1]);
    }

#define CHANNEL_UUID_STR_LEN 128
    char idArg[CHANNEL_UUID_STR_LEN];
    //char* idArg = new char[CHANNEL_UUID_STR_LEN];
    snprintf(idArg, sizeof(idArg), "--id=%s", "StreamDetect");
    args[argIndex++] = idArg;

    printf("argIndex = %d, args[%d]=%s\n", 
           argIndex, argIndex-1, args[argIndex-1]);

    // for the StreamDetect Process
    char detectArg[50];
    int m_maxDetects = 3;
    //char* detectArg = new char[50];
    if (m_maxDetects > 0) {
        sprintf(detectArg, "--max_detects=%d", m_maxDetects);
        args[argIndex++] = detectArg;

        printf("argIndex = %d, args[%d]=%s\n", 
               argIndex, argIndex-1, args[argIndex-1]);

    }

    if (1)
    {
      char restartArg[50];
      int restart_count = 3;
      // Include number of restarts if channel is restarting
      snprintf(restartArg, sizeof(restartArg), "--restart_count=%d", restart_count);
      args[argIndex++] = restartArg;
    }

    // NOTES:
    //  - This arg (channelName) must be last!!!
    //  - If new arguments are added, you may need to update ChannelMgrUtils::GetChannelArgs()
#define CHANNEL_NAME_STR_LEN 256
    char nameArg[CHANNEL_NAME_STR_LEN];
    //char* nameArg = new char[CHANNEL_NAME_STR_LEN];
    snprintf(nameArg, sizeof(nameArg), "--name=\"%s\"", "StreamDetect");
    args[argIndex++] = nameArg;

    printf("argIndex = %d, args[%d]=%s\n", 
           argIndex, argIndex-1, args[argIndex-1]);

    args[argIndex] = NULL;    

    printf("argIndex=%d, args[0]=%s, [1]=%s, [2]=%s, [3]=%s, [4]=%s, [5]=%s\n",
           argIndex, args[0], args[1], args[2], args[3], args[4], args[5]);

    delete chanExe;

    return 0;
}
