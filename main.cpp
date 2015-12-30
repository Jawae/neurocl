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

#include "network_manager.h"
#include "network_exception.h"

#include <boost/chrono.hpp>

int main()
{
    std::cout << "Welcome to neurocl!" << std::endl;

    try
    {
        neurocl::network_manager net_manager( neurocl::network_manager::NEURAL_IMPL_VEXCL );

        net_manager.load_network( "titi" );

        float test_sample[16*16];
        float test_output = 0.5f;

        for ( size_t i=0; i<16*16; i++ )
            test_sample[i] = std::rand()/float(RAND_MAX);

        std::vector<neurocl::sample> training_set;
        training_set.push_back( neurocl::sample( 16*16, test_sample, 1, &test_output ) );

        boost::chrono::system_clock::time_point start = boost::chrono::system_clock::now();

        net_manager.train( training_set );

        typedef boost::chrono::milliseconds bcms;
        bcms duration = boost::chrono::duration_cast<bcms>( boost::chrono::system_clock::now() - start );

        std::cout << "execution in "  << duration.count() << "ms"<< std::endl;

        std::cout << "Bye bye neurocl!" << std::endl;
    }
    catch( neurocl::network_exception& e )
    {
        std::cerr << "network exception : " << e.what() << std::endl;
    }
    catch( std::exception& e )
    {
        std::cerr << "std::exception : " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "unknown exception" << std::endl;
    }
    return 0;
}
