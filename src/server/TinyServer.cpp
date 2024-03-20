#include "net.h"




class CustomServer : public net::server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t port)
		: net::server_interface<CustomMsgTypes>(port)
	{
	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<net::connection<CustomMsgTypes>> client) override
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);

		for (auto& [id, nick] : m_nicknames)
		{
			Message nicknameMsg;
			char nickname[NICK_MAX_LENGHT];
			std::memcpy(nickname, nick.data(), sizeof(nickname));
			nicknameMsg.header.id = CustomMsgTypes::ChangeNickname;
			nicknameMsg << nickname;
			nicknameMsg << id;
			client->Send(nicknameMsg);
		}

		return true;
	}

	virtual void OnClientDisconnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}
	virtual void OnMessage(std::shared_ptr<net::connection<CustomMsgTypes>> client, net::message<CustomMsgTypes>& msg) override
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			client->Send(msg);
		}
		break;
		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All\n";
			msg.header.id = CustomMsgTypes::ServerMessage;
			msg << client->GetID();
			MessageAllClients(msg);
		}
		break;
		case CustomMsgTypes::ChangeNickname:
		{
			char nickname[NICK_MAX_LENGHT];
			msg >> nickname;
			m_nicknames[client->GetID()] = std::string(nickname);
			
			// putting the nichname back in the message with the id;
			msg << nickname << client->GetID();
			MessageAllClients(msg);
		}
		break;
		}
	}

private:
	std::map<uint32_t, std::string> m_nicknames;
};



int main()
{
	CustomServer server(60000);
	server.Start();
	while (1)
	{
		server.Update(-1, true);
	}
	return 0;
}