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

#include "lenet_bnu.h"
#include "network_config.h"
#include "network_exception.h"
#include "network_utils.h"

#include <boost/optional.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/range/adaptor/reversed.hpp>

namespace bnu = boost::numeric::ublas;

namespace neurocl {

const matrixF layer_iface::empty::matrix = matrixF();
const vectorF layer_iface::empty::vector = vectorF();

float sigmoid( float x )
{
    return 1.f / ( 1.f + std::exp(-x) );
}

template<class T>
void random_normal_init( T& container, const float stddev = 1.f )
{
    utils::rand_gaussian_generator rgg( 0.f, stddev );

    for( auto& element : container.data() )
    {
        element = rgg();
    }
}

input_layer_bnu::input_layer_bnu()
{
}

void input_layer_bnu::populate(  const size_t width,
                                const size_t height,
                                const size_t depth  )
{
    std::cout << "populating input layer " << std::endl;

    m_inputs.resize( boost::extents[depth] );
    for ( auto&& _input : m_inputs )
    {
        _input = matrixF( width, height );
    }
}

full_layer_bnu::full_layer_bnu()
{
}

// WARNING : size is the square side size
void full_layer_bnu::populate(  const layer_iface* prev_layer,
                                const layer_size& lsize )
{
    std::cout << "populating full layer" << std::endl;

    m_prev_layer = prev_layer;

    if ( m_prev_layer ) // create a separate layer type for input???
    {
        m_weights = matrixF( lsize.size(), m_prev_layer->size() );
        // cf. http://neuralnetworksanddeeplearning.com/chap3.html#weight_initialization
        random_normal_init( m_weights, 1.f / std::sqrt( m_prev_layer->size() ) );
        m_deltas_weight = matrixF( lsize.size(), m_prev_layer->size() );
        m_deltas_weight.clear();

        m_bias = vectorF( lsize.size() );
        random_normal_init( m_bias, 1.f );
        m_deltas_bias = vectorF( lsize.size() );
        m_deltas_bias.clear();
    }

    m_activations = vectorF( lsize.size() );
    m_activations.clear();
    m_errors = vectorF( lsize.size() ); // not needed for input layer...?
    m_errors.clear();
}

void full_layer_bnu::feed_forward()
{
    if ( m_prev_layer->has_feature_maps() )
    {
        vectorF reconstructed_vector( m_prev_layer->width() * m_prev_layer->height() * m_prev_layer->depth() );

        for ( auto i=0; i<m_prev_layer->depth(); i++ )
        {
            auto& feature_map = m_prev_layer->feature_map(i).data();

            std::copy( feature_map.begin(), feature_map.end(),
                reconstructed_vector.data().begin() + ( i * m_prev_layer->width() * m_prev_layer->height() ) );
        }
    }
    else
    {
        const vectorF& prev_activations = m_prev_layer->activations();

        // apply weights and bias
        m_activations = bnu::prod( m_weights, prev_activations )
            + m_bias;
    }

    // apply sigmoid function
    std::for_each( m_activations.data().begin(), m_activations.data().end(), std::ptr_fun( sigmoid ) );
}

void full_layer_bnu::back_propagate()
{
    /*m_layers[i].errors() = bnu::element_prod(
        bnu::element_prod(  m_layers[i].activations(),
                            ( bnu::scalar_vector<float>( m_layers[i].activations().size(), 1.f ) - m_layers[i].activations() ) ),
        bnu::prod( bnu::trans( m_layers[i].weights() ), m_layers[i+1].errors() ) );*/
}

conv_layer_bnu::conv_layer_bnu() : m_filter_size( 0 ), m_filter_stride( 0 )
{
}

void conv_layer_bnu::set_filter_size( const size_t filter_size, const size_t filter_stride )
{
    m_filter_size = filter_size;
    m_filter_stride = filter_stride;
}

void conv_layer_bnu::populate(  const layer_iface* prev_layer,
                                const size_t width,
                                const size_t height,
                                const size_t depth )
{
    std::cout << "populating convolutional layer " << m_filter_size << " " << m_filter_stride << std::endl;

    std::cout << width << " " << height << " " << prev_layer->width() << " " << prev_layer->height() << std::endl;

    if ( ( width != ( prev_layer->width() - m_filter_size + 1 ) ) ||
        ( height != ( prev_layer->height() - m_filter_size + 1 ) ) )
    {
        std::cerr << "conv_layer_bnu::populate - zero padding not managed for now, \
            so layer size should be consistent with filter size and previous layer size" << std::endl;
        throw network_exception( "inconsistent convolutional layer size" );
    }

    m_prev_layer = prev_layer;

    m_filters.resize( boost::extents[depth][prev_layer->depth()] );
    m_feature_maps.resize( boost::extents[depth] );
    for ( size_t i = 0; i<depth; i++ )
    {
        m_feature_maps[i] = matrixF( width, height );
        for ( auto& _filter : m_filters[i] )
        {
            _filter = matrixF( m_filter_size, m_filter_size );
            random_normal_init( _filter, 1.f / std::sqrt( m_filter_size * m_filter_size ) );
        }
    }
}

void conv_layer_bnu::_convolve_add( const matrixF& prev_feature_map,
                                    const matrixF& filter, const size_t stride,
                                    matrixF& feature_map )
{
    using namespace boost::numeric::ublas;

    // assumption stepsX = stepsY could be easily made...
    auto stepsX = prev_feature_map.size1() - filter.size1() + 1;
    auto stepsY = prev_feature_map.size2() - filter.size2() + 1;
    for ( auto j=0; j<stepsY; j++ )
        for ( auto i=0; i<stepsX; i++ )
        {
            matrixF conv = element_prod( filter,
                project( prev_feature_map,
                    range( i, i+m_filter_size ),
                    range( j, j+m_filter_size ) ) );

            feature_map(i,j) += std::accumulate( conv.data().begin(), conv.data().end(), 0.f );
        }
}

void conv_layer_bnu::feed_forward()
{
    // TODO-CNN : what if previous layer has no feature maps!

    int j = 0;
    for ( auto& feature_map : m_feature_maps )
    {
        feature_map.clear();

        for ( auto i=0; i<m_prev_layer->depth(); i++ )
        {
            const matrixF& prev_feature_map = m_prev_layer->feature_map(i);
            matrixF& filter = m_filters[j][i];

            _convolve_add( prev_feature_map, filter, m_filter_stride, feature_map );
        }
        j++;
    }
}

void conv_layer_bnu::back_propagate()
{

}

pool_layer_bnu::pool_layer_bnu() : m_subsample( 1 )
{

}

void pool_layer_bnu::populate(  const layer_iface* prev_layer,
                                const size_t width,
                                const size_t height,
                                const size_t depth )
{
    std::cout << "populating pooling layer" << std::endl;

    m_prev_layer = prev_layer;

    // compute subsampling rate, throw error if not integer
    if ( ( prev_layer->width() % width) == 0 )
        m_subsample = prev_layer->width() / width;
    else
        throw network_exception( "invalid subsampling for max pooling" );

    m_feature_maps.resize( boost::extents[depth] );
    for ( auto& _feature : m_feature_maps )
    {
        _feature = matrixF( width, height );
        random_normal_init( _feature, 1.f / std::sqrt( width * height ) );
    }
}

void pool_layer_bnu::feed_forward()
{
    for ( auto i = 0; i < m_feature_maps.shape()[0]; i++ )
    {
        const matrixF& prev_feature_map = m_prev_layer->feature_map(i);
        matrixF& feature_map = m_feature_maps[i];
        auto prev_width = prev_feature_map.size1();
        auto prev_it1 = prev_feature_map.begin1();
        for( auto it1 = feature_map.begin1(); it1 != feature_map.end1(); it1++, prev_it1 += m_subsample )
        {
            auto prev_it2 = prev_it1.begin();
            for( auto it2 = it1.begin(); it2 !=it1.end(); it2++, prev_it2 += m_subsample )
            {
                float max_value = std::numeric_limits<float_t>::lowest();

                // could use ublas::project + std::accumulate + std::max for more compact expression

                // compute max in subsampling zone
                for ( auto i =0; i<m_subsample; i++ )
                    for ( auto j =0; j<m_subsample; j++ )
                    {
                        const float& value = *(prev_it2 + i + (j*prev_width) );
                        if ( value > max_value )
                            max_value = value;
                    }

                // update value in the destination feature map
                *it2 = max_value;
            }
        }
    }
}

void pool_layer_bnu::back_propagate()
{
    // TODO-CNN : what if previous layer has no error maps!

    for ( auto i = 0; i < m_feature_maps.shape()[0]; i++ )
    {
        const matrixF& prev_feature_map = m_prev_layer->feature_map(i);

        matrixF& prev_error_map = m_prev_layer->error_map(i);

        const matrixF& error_map = m_error_maps[i];

		// NOT IMPLEMEMENTED YET
    }
}

lenet_bnu::lenet_bnu() : m_learning_rate( 3.0f/*0.01f*/ ), m_weight_decay( 0.0f ), m_training_samples( 0 )
{
    const network_config& nc = network_config::instance();
    nc.update_optional( "learning_rate", m_learning_rate );
}

void lenet_bnu::set_input(  const size_t& in_size, const float* in )
{
    if ( in_size > m_layers[0]->size() )
        throw network_exception( "sample size exceeds allocated layer size!" );

    std::cout << "lenet_bnu::set_input - input (" << in << ") size = " << in_size << std::endl;

    // TODO-CNN : for now works only because first layer is one dimensional

    auto input_feature_map = m_layers[0]->feature_map(0).data();
    std::copy( in, in + in_size, input_feature_map.begin() );
}

void lenet_bnu::set_output( const size_t& out_size, const float* out )
{
    // TODO-CNN : for now works only because last layer is one dimensional fcnn

    if ( out_size > m_training_output.size() )
        throw network_exception( "output size exceeds allocated layer size!" );

    std::cout << "lenet_bnu::set_output - output (" << out << ") size = " << out_size << std::endl;

    std::copy( out, out + out_size, m_training_output.begin() );
}

void lenet_bnu::add_layers_2d( const std::vector<layer_size>& layer_sizes )
{
    m_layer_input.populate( 32, 32, 1 );
    m_layer_c1.set_filter_size( 5 ); // 5x5
    m_layer_c1.populate( &m_layer_input, 28, 28, 6 );               //conv
    m_layer_s2.populate( &m_layer_c1, 14, 14, 6 );                  //pool
    m_layer_c3.set_filter_size( 5 ); // 5x5
    m_layer_c3.populate( &m_layer_s2, 10, 10, 16 );                 //conv
    m_layer_s4.populate( &m_layer_c3, 5, 5, 16 );                   //pool
    m_layer_c5.set_filter_size( 5 );
    m_layer_c5.populate( &m_layer_s4, 1, 1, 120 );                  //conv
    m_layer_f6.populate( &m_layer_c5, layer_size( 84, 1 ) );        //full
    m_layer_output.populate( &m_layer_f6, layer_size( 10, 1 ) );

    m_layers = { &m_layer_input, &m_layer_c1, &m_layer_s2, &m_layer_c3, &m_layer_s4, &m_layer_c5, &m_layer_f6, &m_layer_output };
}

const layer_ptr lenet_bnu::get_layer_ptr( const size_t layer_idx )
{
    // TODO-CNN
    /*if ( layer_idx >= m_layers.size() )
    {
        std::cerr << "network_bnu_base::get_layer_ptr - cannot access layer " << layer_idx << std::endl;
        throw network_exception( "invalid layer index" );
    }

    matrixF& weights = m_layers[layer_idx].weights();
    vectorF& bias = m_layers[layer_idx].bias();
    layer_ptr l( weights.size1() * weights.size2(), bias.size() );
    std::copy( &weights.data()[0], &weights.data()[0] + ( weights.size1() * weights.size2() ), l.weights.get() );
    std::copy( &bias[0], &bias[0] + bias.size(), l.bias.get() );

    return l;*/
    return layer_ptr(0,0);
}

void lenet_bnu::set_layer_ptr( const size_t layer_idx, const layer_ptr& layer )
{
    // TODO-CNN
    /*if ( layer_idx >= m_layers.size() )
    {
        std::cerr << "network_bnu_base::set_layer_ptr - cannot access layer " << layer_idx << std::endl;
        throw network_exception( "invalid layer index" );
    }

    std::cout << "network_bnu_base::set_layer_ptr - setting layer  " << layer_idx << std::endl;

    matrixF& weights = m_layers[layer_idx].weights();
    std::copy( layer.weights.get(), layer.weights.get() + layer.num_weights, &weights.data()[0] );
    vectorF& bias = m_layers[layer_idx].bias();
    std::copy( layer.bias.get(), layer.bias.get() + layer.num_bias, &bias.data()[0] );*/
}

const output_ptr lenet_bnu::output()
{
    // TODO-CNN : for now works only because last layer is one dimensional fcnn

    const vectorF& output = m_layers.back()->activations();
    output_ptr o( output.size() );
    std::copy( &output[0], &output[0] + output.size(), o.outputs.get() );

    return o;
}

void lenet_bnu::prepare_training()
{
    // TODO-CNN
    /*
    // Clear gradients
    for ( size_t i=0; i<m_layers.size()-1; i++ )
    {
        m_layers[i].w_deltas().clear();
        m_layers[i].b_deltas().clear();
    }

    m_training_samples = 0;
    */
}

void lenet_bnu::feed_forward()
{
    for ( auto& _layer : m_layers )
    {
        std::cout << "--> feed forwarding" << std::endl;
        _layer->feed_forward();
    }
}

void lenet_bnu::back_propagate()
{
    for ( auto& _layer : boost::adaptors::reverse( m_layers ) )
    {
        _layer->back_propagate();
    }
}

void lenet_bnu::gradient_descent()
{

}

const std::string lenet_bnu::dump_weights()
{
    return "NOT IMPLEMENTED YET";
}

const std::string lenet_bnu::dump_bias()
{
    return "NOT IMPLEMENTED YET";
}

const std::string lenet_bnu::dump_activations()
{
    return "NOT IMPLEMENTED YET";
}

}; //namespace neurocl
