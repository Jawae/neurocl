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

#include "lenet.h"
#include "optimizer.h"
#include "conv_layer.h"
#include "full_layer.h"
#include "pool_layer.h"
#include "input_layer.h"
#include "output_layer.h"

#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <iostream>

namespace neurocl {

//#define VERBOSE_LENET

lenet::lenet() : m_training_samples( 0 )
{
    float learning_rate = 0.1f;
    float weight_decay = 0.f;

    // build optimizer given learning rate and weight decay
    m_optimizer = std::make_shared<optimizer>( learning_rate, weight_decay );
}

void lenet::add_layers( const std::vector<layer_descr>& layers )
{
    size_t conv_idx = 0;
    size_t pool_idx = 0;
    size_t full_idx = 0;
    
    for ( auto& _layer : layers )
    {
        std::shared_ptr<layer> l;
        switch( _layer.type )
        {
        case INPUT_LAYER:
            {
                std::shared_ptr<input_layer> in = std::make_shared<input_layer>();
                in->populate( _layer.sizeX, _layer.sizeY, _layer.sizeZ );
                l = in;
            }
            break;
        case CONV_LAYER:
            {
                std::shared_ptr<conv_layer> c = std::make_shared<conv_layer>( "c" + boost::lexical_cast<std::string>(++conv_idx) );
                c->set_filter_size( 5 ); // 5x5 // TODO-CNN : hardcoded :-(
                c->populate( m_layers.back(), _layer.sizeX, _layer.sizeY, _layer.sizeZ );
                l = c;
            }
            break;
        case POOL_LAYER:
            {
                std::shared_ptr<pool_layer> s = std::make_shared<pool_layer>( "s" + boost::lexical_cast<std::string>(++pool_idx) );
                s->populate( m_layers.back(), _layer.sizeX, _layer.sizeY, _layer.sizeZ );
                l = s;
            }
            break;
        case FULL_LAYER:
            {
                std::shared_ptr<full_layer> f = std::make_shared<full_layer>( "f" + boost::lexical_cast<std::string>(++full_idx) );
                f->populate( m_layers.back(), _layer.sizeX, _layer.sizeY, _layer.sizeZ );
                l = f;
            }
            break;
        case OUTPUT_LAYER:
            {
                std::shared_ptr<output_layer> out = std::make_shared<output_layer>(); // TODO-CNN -> smart naming
                out->populate( m_layers.back(), _layer.sizeX, _layer.sizeY, _layer.sizeZ );
                l = out;
            }
            break;  
        }
        m_layers.emplace_back( l );
    }
    
    //**** MLP ****//
    /*std::shared_ptr<input_layer> in = std::make_shared<input_layer>();
    in->populate( 784, 1, 1 );
    m_layers.emplace_back( in );

    std::shared_ptr<full_layer> f = std::make_shared<full_layer>( "f" );
    f->populate( m_layers.back(), 36, 1, 1 );
    m_layers.emplace_back( f );

    std::shared_ptr<output_layer> out = std::make_shared<output_layer>();
    out->populate( m_layers.back(), 10, 1, 1 );
    m_layers.emplace_back( out );*/

    //**** SIMPLE CONVNET ****//
    /*std::shared_ptr<input_layer> in = std::make_shared<input_layer>();
    in->populate( 28, 28, 1 );
    m_layers.emplace_back( in );

    std::shared_ptr<conv_layer> c = std::make_shared<conv_layer>( "c" );
    c->set_filter_size( 5 ); // 5x5
    c->populate( m_layers.back(), 24, 24, 3 );
    m_layers.emplace_back( c );

    std::shared_ptr<pool_layer> s = std::make_shared<pool_layer>( "s" );
    s->populate( m_layers.back(), 12, 12, 3 );
    m_layers.emplace_back( s );

    std::shared_ptr<full_layer> f = std::make_shared<full_layer>( "f" );
    f->populate( m_layers.back(), 100, 1, 1 );
    m_layers.emplace_back( f );

    std::shared_ptr<output_layer> out = std::make_shared<output_layer>();
    out->populate( m_layers.back(), 10, 1, 1 );
    m_layers.emplace_back( out );*/

    //**** LENET ****//
    /*std::shared_ptr<input_layer> in = std::make_shared<input_layer>();
    in->populate( 32, 32, 1 );
    m_layers.emplace_back( in );

    std::shared_ptr<conv_layer> c1 = std::make_shared<conv_layer>( "c1" );
    c1->set_filter_size( 5 ); // 5x5
    c1->populate( m_layers.back(), 28, 28, 6 );
    m_layers.emplace_back( c1 );

    std::shared_ptr<pool_layer> s2 = std::make_shared<pool_layer>( "s2" );
    s2->populate( m_layers.back(), 14, 14, 6 );
    m_layers.emplace_back( s2 );

    std::shared_ptr<conv_layer> c3 = std::make_shared<conv_layer>( "c3" );
    c3->set_filter_size( 5 ); // 5x5
    c3->populate( m_layers.back(), 10, 10, 16 );
    m_layers.emplace_back( c3 );

    std::shared_ptr<pool_layer> s4 = std::make_shared<pool_layer>( "s4" );
    s4->populate( m_layers.back(), 5, 5, 16 );
    m_layers.emplace_back( s4 );

    std::shared_ptr<conv_layer> c5 = std::make_shared<conv_layer>( "c5" );
    c5->set_filter_size( 5 ); // 5x5
    c5->populate( m_layers.back(), 1, 1, 120 );
    m_layers.emplace_back( c5 );

    std::shared_ptr<full_layer> f6 = std::make_shared<full_layer>( "f6" );
    f6->populate( m_layers.back(), 84, 1, 1 );
    m_layers.emplace_back( f6 );

    std::shared_ptr<output_layer> out = std::make_shared<output_layer>();
    out->populate( m_layers.back(), 10, 1, 1 );
    m_layers.emplace_back( out );*/
}

void lenet::set_input(  const size_t& in_size, const float* in )
{
    // TODO-CNN : for now works only because input layer has no depth for now!

    std::shared_ptr<layer> layer;
    std::shared_ptr<input_layer> input_layer = std::dynamic_pointer_cast<neurocl::input_layer>( m_layers.front() );

    if ( in_size > input_layer->size() )
        throw network_exception( "sample size exceeds allocated layer size!" );

#ifdef VERBOSE_LENET
    std::cout << "lenet::set_input - input (" << in << ") size = " << in_size << std::endl;
#endif

    input_layer->fill( 0, 0, in_size, in );
}

void lenet::set_output( const size_t& out_size, const float* out )
{
    // TODO-CNN : for now works only because output layer has no depth for now!

    std::shared_ptr<output_layer> output_layer = std::dynamic_pointer_cast<neurocl::output_layer>( m_layers.back() );

    if ( out_size > output_layer->size() )
        throw network_exception( "output size exceeds allocated layer size!" );

#ifdef VERBOSE_LENET
    std::cout << "lenet::set_output - output (" << out << ") size = " << out_size << std::endl;
#endif

    output_layer->fill( 0, 0, out_size, out );
}

const layer_ptr lenet::get_layer_ptr( const size_t layer_idx )
{
    if ( layer_idx >= m_layers.size() )
	{
        std::cerr << "lenet::get_layer_ptr - cannot access layer " << layer_idx << std::endl;
        throw network_exception( "invalid layer index" );
    }

    std::shared_ptr<layer> _layer = m_layers[layer_idx];

    size_t nw = _layer->nb_weights();
    size_t nb = _layer->nb_bias();

    std::cout << "lenet::set_layer_ptr - getting layer  " << _layer->type() << std::endl;

    layer_ptr l( nw, nb );
    //_layer->fill_w( l.weights.get() );
    //_layer->fill_b( layer.bias.get() );
    //std::copy( &weights.data()[0], &weights.data()[0] + ( weights.size1() * weights.size2() ), l.weights.get() );
    //std::copy( &bias[0], &bias[0] + bias.size(), l.bias.get() );

    return l;
}

void lenet::set_layer_ptr( const size_t layer_idx, const layer_ptr& l )
{
    if ( layer_idx >= m_layers.size() )
    {
        std::cerr << "lenet::set_layer_ptr - cannot access layer " << layer_idx << std::endl;
        throw network_exception( "invalid layer index" );
    }

    std::shared_ptr<layer> _layer = m_layers[layer_idx];

    std::cout << "lenet::set_layer_ptr - setting layer  " << _layer->type() << std::endl;

    //_layer->fill_w( l.weights.get() );
    //_layer->fill_b( layer.bias.get() );
    //std::copy( layer.weights.get(), layer.weights.get() + layer.num_weights, &weights.data()[0] );
    //std::copy( layer.bias.get(), layer.bias.get() + layer.num_bias, &bias.data()[0] );
}

const output_ptr lenet::output()
{
    // TODO-CNN : for now works only because last layer has no depth for now

    std::shared_ptr<output_layer> output_layer = std::dynamic_pointer_cast<neurocl::output_layer>( m_layers.back() );

    output_ptr o( output_layer->width() * output_layer->height() );
    output_layer->fill( 0, 0, o.outputs.get() );

    return o;
}

void lenet::prepare_training()
{
    for ( auto _layer : m_layers )
    {
        _layer->prepare_training();
    }

    m_training_samples = 0;
}

void lenet::feed_forward()
{
    for ( auto _layer : m_layers )
    {
#ifdef VERBOSE_LENET
        std::cout << "--> feed forwarding " << _layer->type() << " layer" << std::endl;
#endif
        _layer->feed_forward();
    }
}

void lenet::back_propagate()
{
    for ( auto _layer : boost::adaptors::reverse( m_layers ) )
    {
#ifdef VERBOSE_LENET
        std::cout << "--> back propagating " << _layer->type() << " layer" << std::endl;
#endif
        _layer->back_propagate();
    }

    for ( auto _layer : m_layers )
    {
#ifdef VERBOSE_LENET
        std::cout << "--> updating gradients " << _layer->type() << " layer" << std::endl;
#endif
        _layer->update_gradients();
    }

    ++m_training_samples;
}

void lenet::gradient_descent()
{
    m_optimizer->set_size( m_training_samples );

    for ( auto _layer : m_layers )
    {
        _layer->gradient_descent( m_optimizer );
    }
}

}; //namespace neurocl
