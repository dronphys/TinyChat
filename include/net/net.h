#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_client.h"
#include "net_connection.h"
#include "net_server.h"

#define MSG_MAX_LENGHT 250
#define NICK_MAX_LENGHT 11
enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
    ChangeNickname
};
using Message = net::message<CustomMsgTypes>;