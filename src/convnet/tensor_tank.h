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

#ifndef TENSOR_TANK_H
#define TENSOR_TANK_H

#include <boost/format.hpp>
#include <boost/container/stable_vector.hpp>

#include <map>

namespace neurocl { namespace convnet {

class tensor_tank
{
public:

    using shared_tensor_tank = std::map<std::string,tensor>;
    using multi_tensor_tank = std::map<std::string,boost::container::stable_vector<tensor>>;

public:
    static tensor_tank& instance() { static tensor_tank tt; return tt; }

    tensor* get_shared( const std::string& key,
                        const size_t width,
                        const size_t height,
                        const size_t depth1,
                        const size_t depth2 )
    {
        auto& t_map = m_shared_tensor_tank[key];

        const std::string size_key = boost::str( boost::format{"%1%x%2%x%3%x%4%"} % width % height % depth1 % depth2 );

        bool populate = false;

        tensor& t = _find_or_emplace( size_key, t_map, populate );
        if ( populate )
            t.resize( width, height, depth1, depth2 );
        return &t;
    }

    tensor* get_standard(   const std::string& key,
                            const size_t width,
                            const size_t height,
                            const size_t depth1,
                            const size_t depth2 )
    {
        auto& t_map = m_standard_tensor_tank[key];

        const std::string size_key = boost::str( boost::format{"%1%x%2%x%3%x%4%"} % width % height % depth1 % depth2 );

        tensor& t = _emplace( size_key, t_map );
        t.resize( width, height, depth1, depth2 );
        return &t;
    }

    tensor* get_cumulative( const std::string& key,
                            const size_t width,
                            const size_t height,
                            const size_t depth1,
                            const size_t depth2 )
    {
        auto& t_map = m_cumulative_tensor_tank[key];

        const std::string size_key = boost::str( boost::format{"%1%x%2%x%3%x%4%"} % width % height % depth1 % depth2 );

        tensor& t = _emplace( size_key, t_map );
        t.resize( width, height, depth1, depth2 );
        return &t;
    }

    void accumulate()
    {
        for ( auto& tank : m_cumulative_tensor_tank )
        {
            for ( auto& elem : tank.second )
            {
                auto& v_tensor = elem.second;
                tensor& ref_tensor = v_tensor.front();
                std::for_each(  v_tensor.begin()+1,
                                v_tensor.end(),
                                [&ref_tensor]( tensor& t ) { ref_tensor += t; } );
            }
        }
    }

private:
    tensor_tank() {}
    virtual ~tensor_tank() {}

    tensor& _emplace( const std::string& key, multi_tensor_tank& map )
    {
        auto& t_vec = map[key];
        t_vec.emplace_back( tensor{} );
        return t_vec.back();
    }

    tensor& _find_or_emplace( const std::string& key, shared_tensor_tank& map, bool& populate )
    {
        auto iter = map.find( key );
        if ( iter != map.end() )
        {
            populate = false;
            return map.at( key );
        }
        else
        {
            populate = true;
            map.emplace( key, tensor{} );
            return map.at( key );
        }
    }

private:

    std::map<std::string,shared_tensor_tank> m_shared_tensor_tank;
    std::map<std::string,multi_tensor_tank> m_cumulative_tensor_tank;
    std::map<std::string,multi_tensor_tank> m_standard_tensor_tank;

};

} /*namespace neurocl*/ } /*namespace convnet*/

#endif //TENSOR_TANK_H
