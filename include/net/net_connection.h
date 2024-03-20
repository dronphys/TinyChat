#pragma once
#include "net_tsqueue.h"
#include "net_message.h"
namespace net
{
	// Forward declaration
	template<typename T>
	class server_interface;
}


namespace net
{
	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>>
	{
	public:
		enum class owner
		{
			server,
			client
		};

		connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
			:m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
		{
			// the connection is owned either by server or client
			m_nOwnerType = parent;

			// constructing validation handshake check
			if (m_nOwnerType == owner::server)
			{
				// random data for handshake
				m_nHandshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

				m_nHandshakeCheck = scramble(m_nHandshakeOut);
			}
			else
			{
				m_nHandshakeOut = 0;
				m_nHandshakeIn = 0;
			}
		}

		virtual ~connection()
		{}

		uint32_t GetID() const
		{
			return id;
		}

	public:
		// Only ever called by servers
		void ConnectToClient(net::server_interface<T>* server, uint32_t uid = 0)
		{
			if (m_nOwnerType == owner::server)
			{
				if (m_socket.is_open())
				{
					id = uid;

					// client has connected to server
					// Server is sending client data for handshake
					WriteValidation();

					// reading the handshake sent back by the client
					ReadValidation(server);
				}
			}
		}

		//Only Clients can connect to server
		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (m_nOwnerType == owner::client)
			{
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							// first when we connect to server we need to receive
							// the key for the handshake
							ReadValidation();
						}
					});
			}
		}

		void Disconnect()
		{
			if (IsConnected())
			{
				asio::post(m_asioContext, [this]() {m_socket.close(); });
			}
		}

		bool IsConnected() const
		{
			return m_socket.is_open();
		}



	public:
		void Send(const message<T>& msg)
		{
			asio::post(m_asioContext,
				[this, msg]()
				{
					bool WritingMessage = !m_qMessagesOut.empty();
					// Adding msg to the queue
					m_qMessagesOut.push_back(msg);
					
					// We want only to start writing header if no other writing header currently happening
					// That's why we first check is the Out messages queue is empty or not.
					// If it's not empty, it means that there is already one WriteHeader() function working 
					if (!WritingMessage)
					{
						WriteHeader();
					}

				});
		}


	private:

		//ASYNC
		void ReadHeader()
		{
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t lenght)
				{
					if (!ec)
					{
						
						if (m_msgTemporaryIn.header.size > 0)
						{

							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
							ReadBody();
						}
						else
						{
							AddToIncomingMessageQueue();
						}
					}
					else
					{
						std::cout << "[SERVER] Read Header Failed; id: " << id << '\n';
						m_socket.close();
					}
				});
		}

		//ASYNC
		void ReadBody()
		{
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t lenght)
				{
					if (!ec)
					{
						
						AddToIncomingMessageQueue();
					}
					else
					{
						std::cout << "[SERVER] Read Body Failed; id: " << id << '\n';
						m_socket.close();
					}
				});
		}
		//ASYNC
		void WriteHeader()
		{
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t lenght)
				{
					if (!ec)
					{
						if (m_qMessagesOut.front().body.size() > 0) {
							WriteBody();
						}
						else
						{
							// We are done with the message and can remove it from the queue
							m_qMessagesOut.pop_front();
							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
					}
					else
					{
						std::cout << "[SERVER] Write Header Failed; id: " << id << '\n';
					}


				});
		}

		//ASYNC
		void WriteBody()
		{
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
				[this](std::error_code ec, std::size_t lenght)
				{
					if (!ec)
					{
						m_qMessagesOut.pop_front();

						if (!m_qMessagesOut.empty())
						{
							WriteHeader();
						}
					}
					else
					{
						std::cout << "[SERVER] Write Body Failed; id: " << id << '\n';
					}


				});
		}

		void AddToIncomingMessageQueue()
		{
			// for server user constructing owned_message
			if (m_nOwnerType == owner::server) {
				// Initializing list to construct owned_message type and passing share pointer
				m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
			}
			//for client packing with nullptr
			else
			{
				m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });
			}
			ReadHeader();
		}

		// function that calculates "encryption"
		// to test if the client connecting to the server is valid
		uint64_t scramble(uint64_t nInput)
		{
			uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0) >> 4 | (out & 0xF0F0F0F0F0F0) << 4;
			return out ^ 0xC0DEFACE11111211;
		}

		void WriteValidation()
		{
			asio::async_write(m_socket, asio::buffer(&m_nHandshakeOut, sizeof (uint64_t)),
				[this](std::error_code ec, std::size_t lenght)
				{
					if (!ec)
					{
						// if the owner is client we can only start received info.
						if (m_nOwnerType == owner::client) { ReadHeader(); }

					}
					else
					{
						std::cout << "[" << id << "] Error in writing validation\n";
						m_socket.close();
					}
				});
		}

		void ReadValidation(net::server_interface<T>* server = nullptr)
		{
			asio::async_read(m_socket, asio::buffer(&m_nHandshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t lenght)
				{
					if (!ec)
					{
						// if the owner is client we can only start received info.
						if (m_nOwnerType == owner::client)
						{ 
							m_nHandshakeOut = scramble(m_nHandshakeIn);
							WriteValidation();
						}
						if (m_nOwnerType == owner::server)
						{
							//checking if the received handshake from the client is correct
							if (m_nHandshakeIn == m_nHandshakeCheck)
							{
								std::cout << "Client Validated\n";
								server->OnClientValidated(this->shared_from_this());

								ReadHeader();
							}
							else
							{
								//The handshake was not correct
								std::cout << "Validation failed. ID: " << id << '\n';
								m_socket.close();
							}
						}

					}
					else
					{
						std::cout << "[" << id << "] Error in reading validation\n";
						m_socket.close();
					}
				});
		}

	protected:
		asio::ip::tcp::socket m_socket;

		asio::io_context& m_asioContext;

		tsqueue<message<T>> m_qMessagesOut;

		tsqueue<owned_message<T>>& m_qMessagesIn;

		owner m_nOwnerType = owner::server;
		uint32_t id = 0;

		message<T> m_msgTemporaryIn;


		// Handshake validation variables
		uint64_t m_nHandshakeOut = 0;
		uint64_t m_nHandshakeIn = 0;
		uint64_t m_nHandshakeCheck = 0;
	};

}