#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "conf_global.h"
#include "conf_tables.h"
#include <AdpSharedTypes.h>

#ifndef CONF_COUNT_ADP_BUFFERS_1280
  #define CONF_COUNT_ADP_BUFFERS_1280   1
#endif

#ifndef CONF_COUNT_ADP_BUFFERS_400
  #define CONF_COUNT_ADP_BUFFERS_400    3
#endif

#ifndef CONF_COUNT_ADP_BUFFERS_100
  #define CONF_COUNT_ADP_BUFFERS_100    3
#endif

#ifndef CONF_ADP_FRAGMENTED_TRANSFER_TABLE_SIZE
  #define CONF_ADP_FRAGMENTED_TRANSFER_TABLE_SIZE    1
#endif

#define COUNT_BUFFERS_1280   CONF_COUNT_ADP_BUFFERS_1280
#define COUNT_BUFFERS_400    CONF_COUNT_ADP_BUFFERS_400
#define COUNT_BUFFERS_100    CONF_COUNT_ADP_BUFFERS_100

#define PROCESS_QUEUE_SIZE COUNT_BUFFERS_1280 + COUNT_BUFFERS_400 + COUNT_BUFFERS_100

// The number of fragmented transfers which can be handled in parallel
#define ADP_FRAGMENTED_TRANSFER_TABLE_SIZE CONF_ADP_FRAGMENTED_TRANSFER_TABLE_SIZE

/**********************************************************************************************************************/
/** Buffers and Queues
 **********************************************************************************************************************/
struct TDataSend1280 DataSend1280Buffers[COUNT_BUFFERS_1280];
struct TDataSend400 DataSend400Buffers[COUNT_BUFFERS_400];
struct TDataSend100 DataSend100Buffers[COUNT_BUFFERS_100];
struct TProcessQueueEntry ProcessQueueEntries[PROCESS_QUEUE_SIZE];


struct TLowpanFragmentedData FragmentedTransferTable[ADP_FRAGMENTED_TRANSFER_TABLE_SIZE];

/**********************************************************************************************************************/
/** Get sizes and pointers functions
 **********************************************************************************************************************/
struct TDataSend1280* AdpConfGet1280BufPtr(void)
{
  memset(DataSend1280Buffers, 0, sizeof(DataSend1280Buffers));
  return DataSend1280Buffers;
}

struct TDataSend400* AdpConfGet400BufPtr(void)
{
  memset(DataSend400Buffers, 0, sizeof(DataSend400Buffers));
  return DataSend400Buffers;
}

struct TDataSend100* AdpConfGet100BufPtr(void)
{
  memset(DataSend100Buffers, 0, sizeof(DataSend100Buffers));
  return DataSend100Buffers;
}

uint8_t AdpConfGet1280BufCount(void)
{
  return COUNT_BUFFERS_1280;
}

uint8_t AdpConfGet400BufCount(void)
{
  return COUNT_BUFFERS_400;
}

uint8_t AdpConfGet100BufCount(void)
{
  return COUNT_BUFFERS_100;
}

struct TProcessQueueEntry* AdpConfGetProcessQueuePtr(void)
{
  memset(ProcessQueueEntries, 0, sizeof(ProcessQueueEntries));
  return ProcessQueueEntries;
}

uint8_t AdpConfGetProcessQueueCount(void)
{
  return PROCESS_QUEUE_SIZE;
}

struct TLowpanFragmentedData* AdpConfGetFragmentedTransferTablePtr(void)
{
  memset(FragmentedTransferTable, 0, sizeof(FragmentedTransferTable));
  return &FragmentedTransferTable[0];
}

uint8_t AdpConfGetFragmentedTransferTableCount(void)
{
  return ADP_FRAGMENTED_TRANSFER_TABLE_SIZE;
}
