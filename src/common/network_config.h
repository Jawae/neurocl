/*
The MIT License

Copyright (c) 2015-2017 Albert Murienne

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

#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include "network_exception.h"
#include "logger.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#include <map>

namespace neurocl {

static const std::string s_neurocl_config_file = "neurocl.xml";

// network configuration class
class network_config
{
public:
    static const network_config& instance()
    {
        static network_config s;
        return s;
    }
    template<typename T>
    boost::optional<T> get_param( const std::string& key ) const
    {
        return m_ptree.get_optional<T>( "neurocl." + key );
    }
    template<typename T>
    void update_mandatory( const std::string& key, T& param ) const
    {
        boost::optional<T> opt_param = get_param<T>( key );

        if ( !opt_param )
            throw network_exception( "missing mandatory param key " + key );

        param = opt_param.get();
        LOGGER(info) << "network_config::update_mandatory - key " << key << " configured to value " << param << std::endl;
    }
    template<typename T>
    void update_optional( const std::string& key, T& param ) const
    {
        boost::optional<T> opt_param = get_param<T>( key );
        if ( opt_param )
        {
            param = opt_param.get();
            LOGGER(info) << "network_config::update_optional - key " << key << " configured to value " << param << std::endl;
        }
    }
    template<class T>
    void update_set_optional( std::map<const std::string,T>& parameters_set, const std::string& prefix = "" ) const
    {
        for ( auto& _param : parameters_set )
        {
            update_optional( prefix + "." + _param.first, _param.second.get() ); // works only because T is std::reference_wrapper<>
        }
    }

private:
    network_config()
    {
        try
        {
            char* var = nullptr;
            const std::string env_neurocl_config_file( ( var = getenv( "NEUROCL_RESOURCE_PATH" ) ) == nullptr ? "" : var );
            const auto _neurocl_config_file = env_neurocl_config_file.empty() ? s_neurocl_config_file : ( env_neurocl_config_file + "/" + s_neurocl_config_file );

            LOGGER(info) << "network_config::network_config - config lookup path is : " << _neurocl_config_file << std::endl;

        	if ( bfs::exists( _neurocl_config_file ) )
            	boost::property_tree::read_xml( _neurocl_config_file, m_ptree );
            else
                LOGGER(error) << "network_config::network_config - missing configuration file : " << _neurocl_config_file << std::endl;
        }
        catch( const boost::property_tree::ptree_error& e )
        {
            LOGGER(error) << "network_config::network_config - error parsing xml configuration file (" << e.what() << std::endl;
        }
    }
    virtual ~network_config() {}

private:
    boost::property_tree::ptree m_ptree;
};

} //namespace neurocl

#endif //NETWORK_CONFIG_H
