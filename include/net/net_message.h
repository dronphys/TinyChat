#pragma once
#include "net_common.h"

namespace net
{
	//Message header. Sent in the beginning of the message.
	// id has type of the custom enum class specified by the user
	// size is the size of the header + size of the message body. 
	template<typename T>
	struct message_header
	{
		T id{};
		uint32_t size = 0;
	};

	template <typename T>
	struct message
	{
		message_header<T> header;
		std::vector<uint8_t> body; // the main data of the message
		size_t size() const
		{
			return body.size();
		}
		// override for std::cout for debugging.
		friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
		{
			os << "ID: " << static_cast<int>(msg.header.id) << " Size: " << msg.size() << '\n';
			return os;
		}

		//Serialising the data.
		//Writing custum DataType to the msg
		template<typename DataType>
		friend message<T>& operator<< (message<T>& msg, const DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is to complex");

			size_t i = msg.body.size();
			//resizing data vector to stor more information
			msg.body.resize(msg.body.size() + sizeof(DataType));
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
			msg.header.size = msg.size();
			return msg;
		}

		//Reading custom data type from the msg
		//Data must be extracted in the reverse order
		template<typename DataType>
		friend message<T>& operator>> (message<T>& msg, DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is to complex");

			size_t newSize = msg.body.size() - sizeof(DataType);
			std::memcpy(&data, msg.body.data() + newSize, sizeof(DataType));
			msg.body.resize(newSize);
			msg.header.size = msg.size();
			return msg;
		}
	};
	template<typename T>
	class connection;

	template<typename T>
	class owned_message
	{
	public:
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;

		friend std::ostream& operator<<(std::ostream& os, const owned_message& o_msg)
		{
			os << o_msg.msg;
			return os;
		}

	};

}
