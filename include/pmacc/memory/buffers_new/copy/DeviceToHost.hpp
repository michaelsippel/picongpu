/* Copyright 2013-2020 Felix Schmitt, Rene Widera, Wolfgang Hoenig,
 *                     Benjamin Worpitz, Michael Sippel
 *
 * This file is part of PMacc.
 *
 * PMacc is free software: you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License or
 * the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PMacc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License and the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and the GNU Lesser General Public License along with PMacc.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <iomanip>
#include <pmacc/types.hpp>
#include <pmacc/Environment.hpp>

#include <pmacc/memory/buffers_new/HostBuffer.hpp>
#include <pmacc/memory/buffers_new/DeviceBuffer.hpp>

namespace pmacc
{
namespace mem
{
namespace buffer
{

namespace device2host_detail
{

template < typename T >
void fast_copy(
    T * dst,
    T const * src,
    size_t size
)
{
    cudaStream_t cuda_stream = 0;
    CUDA_CHECK(cudaMemcpyAsync(dst,
                               src,
                               size * sizeof (T),
                               cudaMemcpyDeviceToHost,
                               cuda_stream));
}

template <
    typename T,
    typename T_DstDataAccessPolicy,
    typename T_SrcDataAccessPolicy
>
void copy(
    host_buffer::data::WriteGuard< T, DIM1, T_DstDataAccessPolicy > const & dst,
    device_buffer::data::ReadGuard< T, DIM1, T_SrcDataAccessPolicy > const & src,
    DataSpace<DIM1> const & size
)
{
    cudaStream_t cuda_stream = 0;
    CUDA_CHECK(cudaMemcpyAsync(dst.getPointer(),
                               src.getPointer(),
                               size[0] * sizeof (T),
                               cudaMemcpyDeviceToHost,
                               cuda_stream));
}

template <
    typename T,
    typename T_DstDataAccessPolicy,
    typename T_SrcDataAccessPolicy
>
void copy(
    host_buffer::data::WriteGuard< T, DIM2, T_DstDataAccessPolicy> const & dst,
    device_buffer::data::ReadGuard< T, DIM2, T_SrcDataAccessPolicy> const & src,
    DataSpace<DIM2> const & size
)
{
    cudaStream_t cuda_stream = 0;
    CUDA_CHECK(cudaMemcpy2DAsync(
        dst.getPointer(),
        dst.getPitch(),
        src.getPointer(),
        src.getPitch(),
        size[0] * sizeof (T),
        size[1],
        cudaMemcpyDeviceToHost,
        cuda_stream));
}

template <
    typename T,
    typename T_DstDataAccessPolicy,
    typename T_SrcDataAccessPolicy
>
void copy(
    host_buffer::data::WriteGuard< T, DIM3, T_DstDataAccessPolicy > const & dst,
    device_buffer::data::ReadGuard< T, DIM3, T_SrcDataAccessPolicy > const & src,
    DataSpace<DIM3> const & size
)
{
    cudaStream_t cuda_stream = 0;

    cudaPitchedPtr hostPtr;
    hostPtr.ptr = dst.getBasePointer();
    hostPtr.pitch = src.getPitch();
    hostPtr.xsize = src.getDataSpace()[0] * sizeof(T);
    hostPtr.ysize = src.getDataSpace()[1];

    cudaMemcpy3DParms params;
    params.srcArray = nullptr;
    params.srcPos = make_cudaPos(
        src.getOffset()[0] * sizeof(T),
        src.getOffset()[1],
        src.getOffset()[2]
    );
    params.srcPtr = src.getCudaPitched();

    params.dstArray = nullptr;
    params.srcPos = make_cudaPos(
        dst.getOffset()[0] * sizeof(T),
        dst.getOffset()[1],
        dst.getOffset()[2]
    );
    params.dstPtr = hostPtr;

    params.extent = make_cudaExtent(
        size[0] * sizeof(T),
        size[1],
        size[2]);
    params.kind = cudaMemcpyDeviceToHost;

    CUDA_CHECK(cudaMemcpy3DAsync(&params, cuda_stream))
}

} // namespace device2host_detail

template <
    typename T_Item,
    std::size_t T_dim,
    typename T_DataAccessPolicy
>
void copy(
    host_buffer::WriteGuard< T_Item, T_dim, T_DataAccessPolicy > const & dst,
    device_buffer::ReadGuard< T_Item, T_dim, T_DataAccessPolicy > const & src
)
{
    Environment<>::task(
        []( auto dst, auto src, auto cuda_stream )
        {
            dst.size().set( src.size().get() );
            DataSpace< T_dim > devCurrentSize = src.size().getCurrentDataSpace();

            if( src.is1D() && dst.is1D() )
                device2host_detail::fast_copy(
                    dst.data().getPointer(),
                    src.data().getPointer(),
                    devCurrentSize.productOfComponents()
                );
            else
                device2host_detail::copy(dst.data(), src.data(), devCurrentSize);

            cuda_stream->sync();
        },
        TaskProperties::Builder().label("pmacc::mem::copy(dst: Host, src: Device)"),
        dst.write(),
        src.read(),
        Environment<>::get().cuda_stream()
    );
}

} // namespace buffer

} // namespace mem

} // namespace pmacc
