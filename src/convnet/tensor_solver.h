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

#ifndef TENSOR_SOLVER_H
#define TENSOR_SOLVER_H

#include "tensor_operations.h"

#include "common/solver.h"
#include "common/network_config.h"

#include <boost/lexical_cast.hpp>

namespace neurocl { namespace convnet {

class tensor_solver_iface
{
public:
    virtual void set_size( const size_t& size ) = 0;
    virtual size_t get_cache_size() = 0;

    virtual void update( tensor& input, tensor** input_cache, const tensor& gradient ) = 0;
    virtual void update_redux( tensor& input, tensor** input_cache, const tensor& gradient ) = 0;
};

template<class solverT>
class tensor_solver : public tensor_solver_iface
{
public:

    tensor_solver()
    {
        m_solver = std::make_shared<solverT>();
        _init();
    }
    tensor_solver( std::initializer_list<float> params_list )
    {
        m_solver = std::make_shared<solverT>( params_list );
        _init();
    }
    virtual ~tensor_solver() {}

    void set_size( const size_t& size ) override
    {
        m_solver->set_size( size );
    }

    size_t get_cache_size() override
    {
        return m_solver->get_cache_size();
    }

    void update( tensor& input, tensor** input_cache, const tensor& gradient ) override
    {
        m_solver->update( input, input_cache, gradient );
    }

    void update_redux( tensor& input, tensor** input_cache, const tensor& gradient ) override
    {
        m_solver->update_redux( input, input_cache, gradient );
    }

private:

    void _init()
    {
        m_solver->register_for_scheduling();

        const network_config& nc = network_config::instance();
        nc.update_set_optional<std::reference_wrapper<float>>( m_solver->get_parameters_map(), "solver.<xmlattr>" );
    }

private:
    std::shared_ptr<solverT> m_solver;
};

class tensor_solver_factory
{
public:
    enum class t_solver_impl
    {
        SOLVER_IMPL_SGD = 0,
        SOLVER_IMPL_ADAGRAD,
        SOLVER_IMPL_ADADELTA,
        SOLVER_IMPL_ADAM,
        SOLVER_IMPL_ADAMAX,
        SOLVER_IMPL_RMSPROP
    };
public:

    static std::shared_ptr<tensor_solver_iface> build()
    {
        std::string str_impl = "undefined";

        try
        {
            const network_config& nc = network_config::instance();
            nc.update_mandatory( "solver.<xmlattr>.type", str_impl );

            return build( boost::lexical_cast<t_solver_impl>( str_impl ) );
        }
        catch(...)
        {
            throw network_exception( "unmanaged solver implementation in configuration file : " + str_impl );
        }
    }

    static std::shared_ptr<tensor_solver_iface> build( const t_solver_impl& impl )
    {
        switch( impl )
        {
        case t_solver_impl::SOLVER_IMPL_SGD:
            return std::make_shared< tensor_solver<solver_sgd> >();
            case t_solver_impl::SOLVER_IMPL_ADAGRAD:
                return std::make_shared< tensor_solver<solver_adagrad<tensor_operation>> >();
        case t_solver_impl::SOLVER_IMPL_ADADELTA:
            return std::make_shared< tensor_solver<solver_adadelta<tensor_operation>> >();
        case t_solver_impl::SOLVER_IMPL_ADAM:
            return std::make_shared< tensor_solver<solver_adam<tensor_operation>> >();
        case t_solver_impl::SOLVER_IMPL_ADAMAX:
            return std::make_shared< tensor_solver<solver_adamax<tensor_operation>> >();
        case t_solver_impl::SOLVER_IMPL_RMSPROP:
            return std::make_shared< tensor_solver<solver_rmsprop<tensor_operation>> >();
        default:
            throw network_exception( "unmanaged solver implementation!" );
        }
    }
};

inline std::istream& operator>> ( std::istream &input, tensor_solver_factory::t_solver_impl& impl )
{
    std::string impl_string;
    input >> impl_string;

    if ( impl_string == "SGD" )
        impl = tensor_solver_factory::t_solver_impl::SOLVER_IMPL_SGD;
    else if ( impl_string == "ADAGRAD" )
            impl = tensor_solver_factory::t_solver_impl::SOLVER_IMPL_ADAGRAD;
    else if ( impl_string == "ADADELTA" )
            impl = tensor_solver_factory::t_solver_impl::SOLVER_IMPL_ADADELTA;
    else if ( impl_string == "ADAM" )
            impl = tensor_solver_factory::t_solver_impl::SOLVER_IMPL_ADAM;
    else if ( impl_string == "ADAMAX" )
            impl = tensor_solver_factory::t_solver_impl::SOLVER_IMPL_ADAMAX;
    else if ( impl_string == "RMSPROP" )
        impl = tensor_solver_factory::t_solver_impl::SOLVER_IMPL_RMSPROP;
    else
        input.setstate( std::ios_base::failbit );

    return input;
}

} /*namespace neurocl*/ } /*namespace convnet*/

#endif //TENSOR_SOLVER_H
