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

#ifndef LENET_H
#define LENET_H

#include "lenet_interface.h"

#include <memory>
#include <vector>

namespace neurocl { namespace convnet {

class layer;
class optimizer;

class lenet : public lenet_interface
{
public:

	lenet();
	virtual ~lenet() {}

    void add_layers( const std::vector<layer_descr>& layers );

	void set_input(  const size_t& in_size, const float* in );
    void set_output( const size_t& out_size, const float* out );
    const output_ptr output();

    void prepare_training();

    // pure compute-critic virtuals to be implemented in inherited classes
    void feed_forward();
    void back_propagate();
    void gradient_descent();

    const size_t count_layers() { return m_layers.size(); }
	const layer_ptr get_layer_ptr( const size_t layer_idx );
    void set_layer_ptr( const size_t layer_idx, const layer_ptr& l );

protected:

    size_t m_training_samples;

	std::shared_ptr<optimizer> m_optimizer;

    std::vector<std::shared_ptr<layer>> m_layers;
};

} /*namespace neurocl*/ } /*namespace convnet*/

#endif //LENET_H
