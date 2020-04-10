/*

Copyright (c) 2008, 2010-2019, Arvid Norberg
Copyright (c) 2016, 2018, Alden Torres
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TORRENT_SSL_STREAM_HPP_INCLUDED
#define TORRENT_SSL_STREAM_HPP_INCLUDED

#ifdef TORRENT_USE_OPENSSL

#include "libtorrent/socket.hpp"
#include "libtorrent/error_code.hpp"
#include "libtorrent/io_context.hpp"
#include "libtorrent/aux_/openssl.hpp"

#include <functional>

namespace libtorrent {

template <class Stream>
struct ssl_stream
{
	ssl_stream(io_context& io_context, ssl::context& ctx)
		: m_sock(new ssl::stream<Stream>(io_context, ctx))
	{}

	template <typename S>
	ssl_stream(S&& s, ssl::context& ctx)
		: m_sock(new ssl::stream<Stream>(std::forward<S>(s), ctx))
	{}

	ssl_stream(ssl_stream&&) = default;

	using sock_type = typename boost::asio::ssl::stream<Stream>;
	using next_layer_type = typename sock_type::next_layer_type;
	using lowest_layer_type = typename Stream::lowest_layer_type;
	using endpoint_type = typename Stream::endpoint_type;
	using protocol_type = typename Stream::protocol_type;
	using executor_type = typename sock_type::executor_type;
	executor_type get_executor() { return m_sock->get_executor(); }

	void set_host_name(std::string const& name)
	{
		aux::openssl_set_tlsext_hostname(m_sock->native_handle(), name.c_str());
	}

	template <class T>
	void set_verify_callback(T const& fun, error_code& ec)
	{ m_sock->set_verify_callback(fun, ec); }

	SSL* native_handle() { return m_sock->native_handle(); }

	template <class Handler>
	void async_connect(endpoint_type const& endpoint, Handler const& h)
	{
		// the connect is split up in the following steps:
		// 1. connect to peer
		// 2. perform SSL client handshake

		m_sock->next_layer().async_connect(endpoint, wrap_allocator(
			[this](error_code const& ec, Handler hn) {
				connected(ec, std::move(hn));
			}, std::move(h)));
	}

	template <class Handler>
	void async_accept_handshake(Handler const& h)
	{
		// this is used for accepting SSL connections
		m_sock->async_handshake(ssl::stream_base::server, wrap_allocator(
			[this](error_code const& ec, Handler hn) {
				handshake(ec, std::move(hn));
			}, std::move(h)));
	}

	void accept_handshake(error_code& ec)
	{
		// this is used for accepting SSL connections
		m_sock->handshake(ssl::stream_base::server, ec);
	}

	template <class Handler>
	void async_shutdown(Handler const& handler)
	{
		error_code ec;
		m_sock->next_layer().cancel(ec);
		m_sock->async_shutdown(handler);
	}

	void shutdown(error_code& ec)
	{
		m_sock->shutdown(ec);
	}

	template <class Mutable_Buffers, class Handler>
	void async_read_some(Mutable_Buffers const& buffers, Handler const& handler)
	{
		m_sock->async_read_some(buffers, handler);
	}

	template <class Mutable_Buffers>
	std::size_t read_some(Mutable_Buffers const& buffers, error_code& ec)
	{
		return m_sock->read_some(buffers, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class SettableSocketOption>
	void set_option(SettableSocketOption const& opt)
	{
		m_sock->next_layer().set_option(opt);
	}
#endif

	template <class SettableSocketOption>
	void set_option(SettableSocketOption const& opt, error_code& ec)
	{
		m_sock->next_layer().set_option(opt, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class GettableSocketOption>
	void get_option(GettableSocketOption& opt)
	{
		m_sock->next_layer().get_option(opt);
	}
#endif

	template <class GettableSocketOption>
	void get_option(GettableSocketOption& opt, error_code& ec)
	{
		m_sock->next_layer().get_option(opt, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	template <class Mutable_Buffers>
	std::size_t read_some(Mutable_Buffers const& buffers)
	{
		return m_sock->read_some(buffers);
	}

	template <class IO_Control_Command>
	void io_control(IO_Control_Command& ioc)
	{
		m_sock->next_layer().io_control(ioc);
	}
#endif

	template <class IO_Control_Command>
	void io_control(IO_Control_Command& ioc, error_code& ec)
	{
		m_sock->next_layer().io_control(ioc, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	void non_blocking(bool b) { m_sock->next_layer().non_blocking(b); }
#endif

	void non_blocking(bool b, error_code& ec)
	{ m_sock->next_layer().non_blocking(b, ec); }

	template <class Const_Buffers, class Handler>
	void async_write_some(Const_Buffers const& buffers, Handler const& handler)
	{
		m_sock->async_write_some(buffers, handler);
	}

	template <class Const_Buffers>
	std::size_t write_some(Const_Buffers const& buffers, error_code& ec)
	{
		return m_sock->write_some(buffers, ec);
	}

	// the SSL stream may cache 17 kiB internally, and there's no way of
	// asking how large its buffer is. 17 kiB isn't very much though, so it
	// seems fine to potentially over-estimate the number of bytes available.
#ifndef BOOST_NO_EXCEPTIONS
	std::size_t available() const
	{ return 17 * 1024 + const_cast<sock_type&>(*m_sock).next_layer().available(); }
#endif

	std::size_t available(error_code& ec) const
	{ return 17 * 1024 + const_cast<sock_type&>(*m_sock).next_layer().available(ec); }

#ifndef BOOST_NO_EXCEPTIONS
	void bind(endpoint_type const& endpoint)
	{
		m_sock->next_layer().bind(endpoint);
	}
#endif

	void bind(endpoint_type const& endpoint, error_code& ec)
	{
		m_sock->next_layer().bind(endpoint, ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	void open(protocol_type const& p)
	{
		m_sock->next_layer().open(p);
	}
#endif

	void open(protocol_type const& p, error_code& ec)
	{
		m_sock->next_layer().open(p, ec);
	}

	bool is_open() const
	{
		return m_sock->next_layer().is_open();
	}

#ifndef BOOST_NO_EXCEPTIONS
	void close()
	{
		m_sock->next_layer().close();
	}
#endif

	void close(error_code& ec)
	{
		m_sock->next_layer().close(ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	endpoint_type remote_endpoint() const
	{
		return m_sock->next_layer().remote_endpoint();
	}
#endif

	endpoint_type remote_endpoint(error_code& ec) const
	{
		return m_sock->next_layer().remote_endpoint(ec);
	}

#ifndef BOOST_NO_EXCEPTIONS
	endpoint_type local_endpoint() const
	{
		return m_sock->next_layer().local_endpoint();
	}
#endif

	endpoint_type local_endpoint(error_code& ec) const
	{
		return m_sock->next_layer().local_endpoint(ec);
	}

	lowest_layer_type& lowest_layer()
	{
		return m_sock->lowest_layer();
	}

	lowest_layer_type const& lowest_layer() const
	{
		return m_sock->lowest_layer();
	}

	next_layer_type& next_layer()
	{
		return m_sock->next_layer();
	}

	next_layer_type const& next_layer() const
	{
		return m_sock->next_layer();
	}

private:

	template <typename Handler>
	void connected(error_code const& e, Handler h)
	{
		if (e)
		{
			h(e);
			return;
		}

		m_sock->async_handshake(ssl::stream_base::client, wrap_allocator(
			[this](error_code const& ec, Handler hn) {
				handshake(ec, std::move(hn));
			}, std::move(h)));
	}

	template <typename Handler>
	void handshake(error_code const& e, Handler h)
	{
		h(e);
	}

	// to make us movable
	std::unique_ptr<ssl::stream<Stream>> m_sock;
};

}

#endif // TORRENT_USE_OPENSSL

#endif
