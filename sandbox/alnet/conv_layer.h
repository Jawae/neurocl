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

#ifndef CONV_LAYER_H
#define CONV_LAYER_H

#include "network_exception.h"
#include "layer.h"

namespace neurocl {

using nto = neurocl::tensor_operation;

class conv_layer  : public layer
{
public:

    conv_layer( const std::string& name ) : m_name( name ) {}
	virtual ~conv_layer() {}

    virtual const std::string type() const override { return "conv " + m_name; }

    void set_filter_size( const size_t filter_size, const size_t filter_stride = 1 )
    {
        m_filter_size = filter_size;
        m_filter_stride = filter_stride;
    }

    void populate(  const std::shared_ptr<layer>& prev_layer,
                    const size_t width,
                    const size_t height,
                    const size_t depth )
    {
        std::cout << "populating convolutional layer " << m_filter_size << " " << m_filter_stride << std::endl;

        std::cout << width << " " << height << " " << prev_layer->width() << " " << prev_layer->height() << std::endl;

        if ( ( width != ( prev_layer->width() - m_filter_size + 1 ) ) ||
            ( height != ( prev_layer->height() - m_filter_size + 1 ) ) )
        {
            std::cerr << "conv_layer::populate - zero padding not managed for now, \
                so layer size should be consistent with filter size and previous layer size" << std::endl;
            throw network_exception( "inconsistent convolutional layer size" );
        }

        m_prev_layer = prev_layer;

        m_filters.resize( m_filter_size, m_filter_size, prev_layer->depth(), depth );
        m_filters_delta.resize( m_filter_size, m_filter_size, prev_layer->depth(), depth );
        m_feature_maps.resize( width, height, 1, depth );
        m_error_maps.resize( width, height, 1, depth );
    }

    virtual size_t width() const override { return m_feature_maps.w(); }
    virtual size_t height() const override { return m_feature_maps.h(); }
    virtual size_t depth() const override { return m_feature_maps.d2(); }

    virtual const tensor& feature_maps() const override
        { return m_feature_maps; }

    virtual void prepare_training() override
    {

    }

    virtual void feed_forward() override
    {
        m_feature_maps = nto::convolve_add<nto::kernel_flip,nto::pad_valid>(
            m_prev_layer->feature_maps(),
            m_filters,
            m_filter_stride );

        nto::sig( m_feature_maps );
    }

    virtual void back_propagate() override
    {
        // Compute errors

        m_prev_layer->error_maps() = nto::convolve_add<nto::kernel_std,nto::pad_full>(
            m_error_maps,
            m_filters,
            m_filter_stride );

        // multiply by sigma derivative
        m_prev_layer->error_maps() = nto::elemul(
            nto::d_sig( m_prev_layer->feature_maps() ),
            m_prev_layer->error_maps()
        );
    }

    virtual void update_gradients() override
    {
        // Compute gradients

        auto&& grad = nto::convolve<nto::kernel_flip,nto::pad_valid>(
            m_prev_layer->error_maps(),
            m_feature_maps,
            m_filter_stride);

        m_filters_delta += grad / static_cast<float>( m_filters_delta.d2() );
    }

    virtual void gradient_descent( const std::shared_ptr<optimizer>& optimizer ) override
    {
        // Optimize gradients

        nto::optimize( optimizer, m_filters, m_filters_delta );
    }

protected:

    virtual tensor& error_maps() override
        { return m_error_maps; }

private:

    std::shared_ptr<layer> m_prev_layer;

    size_t m_filter_size;
    size_t m_filter_stride;

    tensor m_filters;
    tensor m_filters_delta;
    tensor m_feature_maps;
    tensor m_error_maps;

    const std::string m_name;
};

} //namespace neurocl

#endif //CONV_LAYER_H
