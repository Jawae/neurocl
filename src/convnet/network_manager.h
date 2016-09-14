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

#ifndef NETWORK_MANAGER_CONVNET_H
#define NETWORK_MANAGER_CONVNET_H

#include "common/network_manager_interface.h"
#include "common/network_sample.h"

#include <boost/shared_array.hpp>

#include <vector>

namespace neurocl {

class network_factory;
class samples_manager;

namespace convnet {

class network_interface;
class network_file_handler;

class network_manager : public network_manager_interface
{
private:

	friend network_factory;

	static std::shared_ptr<network_manager_interface> create()
	{
		struct make_shared_enabler : public network_manager {};
		return std::make_shared<make_shared_enabler>();
	}

    network_manager();

public:

	virtual ~network_manager() {}

    virtual void load_network( const std::string& topology_path, const std::string& weights_path ) override;
    virtual void save_network() override;

    virtual void train( const sample& s ) override;
    virtual void train( const std::vector<sample>& training_set ) override;
    virtual void batch_train(	const samples_manager& smp_manager,
								const size_t& epoch_size,
								const size_t& batch_size,
								t_progress_fct progress_fct = t_progress_fct() ) override;

    // prepare gradient descent
    virtual void prepare_training_iteration() override;
    // finalize gradient descent
    virtual void finalize_training_iteration() override;

    virtual void compute_output( sample& s ) override;

private:

	void _assert_loaded();
    void _train( const sample& s );

private:

	bool m_network_loaded;

    std::shared_ptr<network_interface> m_net;
    std::shared_ptr<network_file_handler> m_net_file_handler;
};

} /*namespace neurocl*/ } /*namespace convnet*/

#endif //NETWORK_MANAGER_CONVNET_H
