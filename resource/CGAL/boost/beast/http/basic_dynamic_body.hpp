//
// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_HTTP_BASIC_DYNAMIC_BODY_HPP
#define BOOST_BEAST_HTTP_BASIC_DYNAMIC_BODY_HPP

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/type_traits.hpp>
#include <boost/beast/core/detail/buffer.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <cstdint>
#include <utility>

namespace boost {
namespace beast {
namespace http {

/** A @b Body using a @b DynamicBuffer

    This body uses a @b DynamicBuffer as a memory-based container
    for holding message payloads. Messages using this body type
    may be serialized and parsed.
*/
template<class DynamicBuffer>
struct basic_dynamic_body
{
    static_assert(
        boost::asio::is_dynamic_buffer<DynamicBuffer>::value,
        "DynamicBuffer requirements not met");

    /** The type of container used for the body

        This determines the type of @ref message::body
        when this body type is used with a message container.
    */
    using value_type = DynamicBuffer;

    /** Returns the payload size of the body

        When this body is used with @ref message::prepare_payload,
        the Content-Length will be set to the payload size, and
        any chunked Transfer-Encoding will be removed.
    */
    static
    std::uint64_t
    size(value_type const& v)
    {
        return v.size();
    }

    /** The algorithm for parsing the body

        Meets the requirements of @b BodyReader.
    */
#if BOOST_BEAST_DOXYGEN
    using reader = implementation_defined;
#else
    class reader
    {
        value_type& body_;

    public:
        template<bool isRequest, class Fields>
        explicit
        reader(header<isRequest, Fields>&, value_type& b)
            : body_(b)
        {
        }

        void
        init(boost::optional<
            std::uint64_t> const&, error_code& ec)
        {
            ec.assign(0, ec.category());
        }

        template<class ConstBufferSequence>
        std::size_t
        put(ConstBufferSequence const& buffers,
            error_code& ec)
        {
            using boost::asio::buffer_copy;
            using boost::asio::buffer_size;
            auto const n = buffer_size(buffers);
            if(body_.size() > body_.max_size() - n)
            {
                ec = error::buffer_overflow;
                return 0;
            }
            auto const mb =
                beast::detail::dynamic_buffer_prepare(
                    body_, (std::min)(n,
                        body_.max_size() - body_.size()),
                            ec, error::buffer_overflow);
            if(ec)
                return 0;
            auto const bytes_transferred =
                buffer_copy(*mb, buffers);
            body_.commit(bytes_transferred);
            return bytes_transferred;
        }

        void
        finish(error_code& ec)
        {
            ec.assign(0, ec.category());
        }
    };
#endif

    /** The algorithm for serializing the body

        Meets the requirements of @b BodyWriter.
    */
#if BOOST_BEAST_DOXYGEN
    using writer = implementation_defined;
#else
    class writer
    {
        DynamicBuffer const& body_;

    public:
        using const_buffers_type =
            typename DynamicBuffer::const_buffers_type;

        template<bool isRequest, class Fields>
        explicit
        writer(header<isRequest, Fields> const&, value_type const& b)
            : body_(b)
        {
        }

        void
        init(error_code& ec)
        {
            ec.assign(0, ec.category());
        }

        boost::optional<std::pair<const_buffers_type, bool>>
        get(error_code& ec)
        {
            ec.assign(0, ec.category());
            return {{body_.data(), false}};
        }
    };
#endif
};

} // http
} // beast
} // boost

#endif
