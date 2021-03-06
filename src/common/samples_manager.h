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

#ifndef SAMPLES_MANAGER_H
#define SAMPLES_MANAGER_H

#include "export.h"

#include "network_sample.h"

#include <boost/shared_array.hpp>

#include <functional>
#include <vector>

namespace neurocl {

using t_preproc = std::function<void (float*,const size_t,const size_t)>;

class NEUROCL_PUBLIC samples_augmenter
{
public:
    samples_augmenter( const int sizeX, const int sizeY );

    //! noise a given sample
    neurocl::sample noise( const neurocl::sample& s, const float sigma ) const;
    //! rotate a given sample
    neurocl::sample rotate( const neurocl::sample& s, const float angle ) const;
    //! translate a given sample
    neurocl::sample translate( const neurocl::sample& s, const int sx, const int sy ) const;
    //! zoom a given sample
    neurocl::sample zoom( const neurocl::sample& s, const int zx ) const;

    //! returns random translation
    static int rand_shift();

private:
    const int m_sizeX;
    const int m_sizeY;
};

class NEUROCL_PUBLIC samples_manager
{
public:

    samples_manager() : m_end( false ), m_batch_index( 0 ), m_restrict_size( 0 ), m_sample_sizeX( 0 ), m_sample_sizeY( 0 ) {}
    virtual ~samples_manager() {}

    void restrict_dataset( const size_t size )
    {
        m_restrict_size = size;
    }

    //! load all training samples
    void load_samples( const std::string& input_filename, bool shuffle = false, t_preproc extra_preproc = t_preproc() );

    //! get number of loaded samples
    const size_t samples_size() const
    {
        return m_samples_set.size();
    }

    //! get samples list
    const std::vector<neurocl::sample>& get_samples() const noexcept
    {
        return m_samples_set;
    }

    //! get samples mini-batch
    const std::vector<neurocl::sample> get_next_batch( const size_t size ) const noexcept;

    //! rewind sample "cursor"
    void rewind() const noexcept;

    //! shuffle samples list
    void shuffle() const noexcept;

	//! get data augmenter
    std::shared_ptr<samples_augmenter> get_augmenter() const;

    /******************************************************/
    /******************* CUSTOM LOADERS *******************/
    /******************************************************/

    //! load all kaggle digit recognizer formatted training samples (https://www.kaggle.com/c/digit-recognizer)
    void load_kaggle_digit_recognizer( const std::string& input_filename );

private:

	void _assert_sample_size() const;

private:

    mutable bool m_end;
    mutable size_t m_batch_index;
    size_t m_restrict_size;

    size_t m_sample_sizeX;
    size_t m_sample_sizeY;

    std::vector< boost::shared_array<float> > m_input_samples;
    std::vector< boost::shared_array<float> > m_output_samples;
    mutable std::vector<neurocl::sample> m_samples_set;

	std::shared_ptr<samples_augmenter> m_augmenter;
};

} //namespace neurocl

#endif //SAMPLES_MANAGER_H
