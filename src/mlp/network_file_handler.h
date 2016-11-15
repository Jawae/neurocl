/*
The MIT License

Copyright (c) 2015-2016 Albert Murienne

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef NETWORK_FILE_HANDLER_MLP_H
#define NETWORK_FILE_HANDLER_MLP_H

#include "common/network_file_handler_interface.h"

#include <boost/cstdint.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/array.hpp>

namespace neurocl { namespace mlp {

class network_interface_mlp;

class network_file_handler : public network_file_handler_interface
{
private:

    class layer_storage
    {
    public:
        layer_storage()
            :   m_num_weights( 0u ), m_num_bias( 0u ) {}
        layer_storage( boost::uint32_t nw, boost::shared_array<float> w, boost::uint32_t nb, boost::shared_array<float> b )
            :   m_num_weights( nw ), m_weights( w ),
                m_num_bias( nb ), m_bias( b ) {}
        layer_storage( layer_storage const& ) = delete; // no copy construct
        layer_storage& operator=( layer_storage const& ) = delete; // no assignment
        ~layer_storage() {}

    protected:

        friend class network_file_handler;

        boost::uint32_t m_num_weights;
        boost::shared_array<float> m_weights;
        boost::uint32_t m_num_bias;
        boost::shared_array<float> m_bias;

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & m_num_weights;
            if ( Archive::is_loading::value )
            {
                assert( m_weights == 0 );
                m_weights.reset( new float[m_num_weights] );
            }
            ar & boost::serialization::make_array<float>( m_weights.get(), m_num_weights );

            ar & m_num_bias;
            if ( Archive::is_loading::value )
            {
                assert( m_bias == 0 );
                m_bias.reset( new float[m_num_bias] );
            }
            ar & boost::serialization::make_array<float>( m_bias.get(), m_num_bias );
        }
    };

public:

    network_file_handler( const std::shared_ptr<network_interface_mlp>& net );
    virtual ~network_file_handler();

    virtual void load_network_topology( const std::string& topology_path ) override;
    virtual void load_network_weights( const std::string& weights_path ) override;
    virtual void save_network_weights() override;

private:

    size_t m_layers;

    std::string m_weights_path;

    std::shared_ptr<network_interface_mlp> m_net;
};

} /*namespace neurocl*/ } /*namespace mlp*/

#endif //NETWORK_FILE_HANDLER_MLP_H
