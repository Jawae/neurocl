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

#ifndef INPUT_LAYER_H
#define INPUT_LAYER_H

#include "layer.h"

#include "common/logger.h"

namespace neurocl { namespace convnet {

class input_layer final : public layer
{
public:

    input_layer() {}
	virtual ~input_layer() {}

    const std::string type() const override { return "input"; }

    tensor d_activation( const tensor& in ) const override { /* NOTHING TO DO */return tensor{};  }

    void populate(  const size_t width,
                    const size_t height,
                    const size_t depth )
    {
        LOGGER(info) << "input_layer::populate - populating input layer" << std::endl;

        m_feature_maps.resize( width, height, 1, depth );
    }

    size_t width() const override { return m_feature_maps.w(); };
    size_t height() const override { return m_feature_maps.h(); };
    size_t depth() const override { return m_feature_maps.d2(); }

    size_t nb_weights() const override { return 0; }
    size_t nb_bias() const override { return 0; }

    void fill(  const size_t depth1,
                const size_t depth2,
                const size_t data_size,
                const float* data )
    {
        m_feature_maps.fill( depth1, depth2, data_size, data );
    }

    const tensor& feature_maps() const override
        { return m_feature_maps; }

    void feed_forward() override { /*NOTHING TO DO YET*/ }
    void back_propagate() override { /*NOTHING TO DO YET*/ }
    void update_gradients() override { /*NOTHING TO DO YET*/ }
    void clear_gradients() override { /*NOTHING TO DO YET*/ }
    void gradient_descent( const std::shared_ptr<tensor_solver_iface>& solver ) override { /*NOTHING TO DO YET*/ }

    // Fill weights
    void fill_w( const size_t data_size, const float* data ) override { /* NOTHING TO DO */ }
    void fill_w( float* data ) override { /* NOTHING TO DO */ }

    // Fill bias
    void fill_b( const size_t data_size, const float* data ) override { /* NOTHING TO DO */ }
    void fill_b( float* data ) override { /* NOTHING TO DO */ }

    tensor& error_maps( key_errors ) override
        { return m_error_maps; } // returns empty tensor

protected:

    size_t fan_in() const override { return 0; }

private:

    tensor m_feature_maps;
    tensor m_error_maps;
};

} /*namespace neurocl*/ } /*namespace convnet*/

#endif //INPUT_LAYER_BNU_H
