#include "sync_client.h"
#include "network.h"
#include "core.h"
#include "packetize.h"
#include "mcp.h"

#include <iostream>

using namespace std;

SyncClient::SyncClient(Core *core)
      : m_core(core)
      , m_network(core->getNetwork())
{
}

SyncClient::~SyncClient()
{
}

void SyncClient::mutexInit(carbon_mutex_t *mux)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_MUTEX_INIT;

   m_send_buff << msg_type;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(carbon_mutex_t));

   *mux = *((carbon_mutex_t*)recv_pkt.data);

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::mutexLock(carbon_mutex_t *mux)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_MUTEX_LOCK;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << *mux << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   // Set the CoreState to 'STALLED'
   m_network->getCore()->setState(Core::STALLED);

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(unsigned int) + sizeof(UInt64));

   // Set the CoreState to 'RUNNING'
   m_network->getCore()->setState(Core::WAKING_UP_STAGE1);

   unsigned int dummy;
   UInt64 time;
   m_recv_buff << make_pair(recv_pkt.data, recv_pkt.length);
   m_recv_buff >> dummy;
   assert(dummy == MUTEX_LOCK_RESPONSE);

   m_recv_buff >> time;

   if (time > start_time)
      m_core->getPerformanceModel()->queueDynamicInstruction(new SyncInstruction(time - start_time));

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::mutexUnlock(carbon_mutex_t *mux)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_MUTEX_UNLOCK;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << *mux << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(unsigned int));

   unsigned int dummy;
   m_recv_buff << make_pair(recv_pkt.data, recv_pkt.length);
   m_recv_buff >> dummy;
   assert(dummy == MUTEX_UNLOCK_RESPONSE);

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::condInit(carbon_cond_t *cond)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_COND_INIT;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << *cond << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(carbon_cond_t));

   *cond = *((carbon_cond_t*)recv_pkt.data);

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::condWait(carbon_cond_t *cond, carbon_mutex_t *mux)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_COND_WAIT;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << *cond << *mux << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   // Set the CoreState to 'STALLED'
   m_network->getCore()->setState(Core::STALLED);

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(unsigned int) + sizeof(UInt64));

   // Set the CoreState to 'RUNNING'
   m_network->getCore()->setState(Core::WAKING_UP_STAGE1);

   unsigned int dummy;
   m_recv_buff << make_pair(recv_pkt.data, recv_pkt.length);
   m_recv_buff >> dummy;
   assert(dummy == COND_WAIT_RESPONSE);

   UInt64 time;
   m_recv_buff >> time;

   if (time > start_time)
      m_core->getPerformanceModel()->queueDynamicInstruction(new SyncInstruction(time - start_time));

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::condSignal(carbon_cond_t *cond)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_COND_SIGNAL;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << *cond << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(unsigned int));

   unsigned int dummy;
   m_recv_buff << make_pair(recv_pkt.data, recv_pkt.length);
   m_recv_buff >> dummy;
   assert(dummy == COND_SIGNAL_RESPONSE);

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::condBroadcast(carbon_cond_t *cond)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_COND_BROADCAST;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << *cond << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(unsigned int));

   unsigned int dummy;
   m_recv_buff << make_pair(recv_pkt.data, recv_pkt.length);
   m_recv_buff >> dummy;
   assert(dummy == COND_BROADCAST_RESPONSE);

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::barrierInit(carbon_barrier_t *barrier, UInt32 count)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_BARRIER_INIT;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << count << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(carbon_barrier_t));

   *barrier = *((carbon_barrier_t*)recv_pkt.data);

   delete [](Byte*) recv_pkt.data;
}

void SyncClient::barrierWait(carbon_barrier_t *barrier)
{
   // Reset the buffers for the new transmission
   m_recv_buff.clear();
   m_send_buff.clear();

   int msg_type = MCP_MESSAGE_BARRIER_WAIT;

   UInt64 start_time = m_core->getPerformanceModel()->getCycleCount();

   m_send_buff << msg_type << *barrier << start_time;

   m_network->netSend(Config::getSingleton()->getMCPCoreNum(), MCP_REQUEST_TYPE, m_send_buff.getBuffer(), m_send_buff.size());

   // Set the CoreState to 'STALLED'
   m_network->getCore()->setState(Core::STALLED);

   NetPacket recv_pkt;
   recv_pkt = m_network->netRecv(Config::getSingleton()->getMCPCoreNum(), MCP_RESPONSE_TYPE);
   assert(recv_pkt.length == sizeof(unsigned int) + sizeof(UInt64));

   // Set the CoreState to 'RUNNING'
   m_network->getCore()->setState(Core::WAKING_UP_STAGE1);

   unsigned int dummy;
   m_recv_buff << make_pair(recv_pkt.data, recv_pkt.length);
   m_recv_buff >> dummy;
   assert(dummy == BARRIER_WAIT_RESPONSE);

   UInt64 time;
   m_recv_buff >> time;

   if (time > start_time)
      m_core->getPerformanceModel()->queueDynamicInstruction(new SyncInstruction(time - start_time));

   delete [](Byte*) recv_pkt.data;
}

