const long freqList[] = {868100000, 868300000, 868500000};
const int freqCount = sizeof(freqList) / sizeof(freqList[0]);
const long SYNC_FREQ = 866200000;

const uint32_t seed = 123456789;

unsigned long hopInterval = 10000;
unsigned long lastHopTime = 0;
int hopIndex = -1;