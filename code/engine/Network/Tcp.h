/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BFG_NETWORK_TCP_H
#define BFG_NETWORK_TCP_H

#include <cstring>

#include <boost/array.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <Network/Checksum.h>
#include <Network/Enums.hh>

namespace BFG {
namespace Network {

struct NetworkEventHeader
{
	typedef boost::array<char, 12> SerializationT;

	void serialize(SerializationT& output) const
	{
		char* p = output.data();
		memcpy(p, &mTimestamp, sizeof(mTimestamp));
		p += sizeof(mTimestamp);
		memcpy(p, &mDataChecksum, sizeof(mDataChecksum));
		p += sizeof(mDataChecksum);
		memcpy(p, &mHeaderChecksum, sizeof(mHeaderChecksum));
		p += sizeof(mHeaderChecksum);
		memcpy(p, &mDataLength, sizeof(mDataLength));
	}

	void deserialize(const SerializationT& input)
	{
		const char* p = input.data();
		memcpy(&mTimestamp, p, sizeof(mTimestamp));
		p += sizeof(mTimestamp);
		memcpy(&mDataChecksum, p, sizeof(mDataChecksum));
		p += sizeof(mDataChecksum);
		memcpy(&mHeaderChecksum, p, sizeof(mHeaderChecksum));
		p += sizeof(mHeaderChecksum);
		memcpy(&mDataLength, p, sizeof(mDataLength));
	}

	f32 mTimestamp;
	u32 mDataChecksum;
	u16 mHeaderChecksum;
	u16 mDataLength;
};

class TcpHeaderFactory;

struct Tcp
{
	static const std::size_t       MAX_PACKET_SIZE_BYTES = 2000;
	static const ID::NetworkAction EVENT_ID_FOR_SENDING  = ID::NE_SEND;

	//! Max size a packet can expand to before it will be flushed (Q3: rate)
	static const u32 MAX_BYTE_RATE = 100000;

	typedef NetworkEventHeader HeaderT;
	typedef TcpHeaderFactory HeaderFactoryT;
	
	typedef boost::asio::ip::tcp::socket SocketT;

	static std::size_t headerSize()
	{
		return HeaderT::SerializationT::size();
	}
};

class TcpHeaderFactory
{
public:
	typedef NetworkEventHeader HeaderT;
	
	//! Creates a NetworkEventHeader from a provided buffer. The
	//! to-transmitted data must've already been written into the buffer.
	//! \param[in] buffer The buffer used to generate the header. No extra
	//!                   bytes allowed. Its size must fit exactly the data
	//!                   part of the packet.
	static NetworkEventHeader createFrom(boost::asio::const_buffer data)
	{
		using namespace boost::asio;

		// Checksum of data
		BFG::u16 dataLength = boost::asio::buffer_size(data);
		u32 dataChecksum = calculateChecksum(buffer_cast<const char*>(data), dataLength);

		// Construct header (without header checksum)
		const BFG::u16 headerChecksum = 0;
		NetworkEventHeader neh = {0.0f, dataChecksum, headerChecksum, dataLength};
		
		// Insert header checksum
		neh.mHeaderChecksum = calculateHeaderChecksum(neh);
		return neh;
	}
};

} // namespace Network
} // namespace BFG

#endif
