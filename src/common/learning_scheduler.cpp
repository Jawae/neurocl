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

#include "learning_scheduler.h"
#include "solver.h"
#include "logger.h"
#include "network_exception.h"

namespace neurocl {

#define EPOCH_DECREASE_FACTOR 0.99f

learning_scheduler::learning_scheduler()
    : m_enabled( false ), m_cached_rate( 0.f )
{
}

learning_scheduler::~learning_scheduler()
{
}

void learning_scheduler::register_solver( const std::shared_ptr<solver_base>& solver )
{
    m_solver = solver;
}

void learning_scheduler::enable_scheduling( const bool enable )
{
    _assert_solver();

    if ( enable )
    {
        if ( m_enabled )
            LOGGER(warning) << "learning_scheduler::enable_scheduling - scheduling is already enabled!" << std::endl;
        else
        {
            // store learning rate
            m_cached_rate = m_solver->get_learning_rate();
            m_enabled = enable;
        }
    }
    else
    {
        if ( m_enabled )
        {
            // restore learning rate
            m_solver->set_learning_rate( m_cached_rate );
            m_cached_rate = 0.f;
            m_enabled = enable;
        }
        else
            LOGGER(warning) << "learning_scheduler::enable_scheduling - scheduling is already disabled!" << std::endl;
    }
}

void learning_scheduler::push_error( const float error )
{
    if ( m_enabled )
    {
    	_assert_solver();

    	LOGGER(info) << "learning_scheduler::push_error - pushing error " << error << std::endl;

        float new_rate = EPOCH_DECREASE_FACTOR * m_solver->get_learning_rate();

        LOGGER(info) << "learning_scheduler::push_error - new learning rate set to " << new_rate << "!" << std::endl;

        m_solver->set_learning_rate( new_rate );
    }
}

void learning_scheduler::set_learning_rate( const float rate )
{
    _assert_solver();

    m_solver->set_learning_rate( rate );
}

const float& learning_scheduler::get_learning_rate()
{
    _assert_solver();

    return m_solver->get_learning_rate();
}

void learning_scheduler::_assert_solver()
{
    if ( !m_solver )
        throw network_exception( "no solver registered!" );
}

} /*namespace neurocl*/
