/* Copyright 2020 Pawel Ordyna
 *
 * This file is part of PIConGPU.
 *
 * PIConGPU is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PIConGPU is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PIConGPU.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "picongpu/simulation_defines.hpp"

namespace picongpu
{
namespace plugins
{
namespace xrayScattering
{
namespace beam
{
namespace beamProfiles
{
    //! Homogeneous beam profile.
    struct ConstProfile
    {
        static HDINLINE constexpr float_X getFactor(
            const float_X & positionX,
            const float_X & positionY
        )
        {
            return float_X( 1.0 );
        }
    };
} // namespace beamProfiles
} // namespace beam
} // namespace xrayScattering
} // namespace plugins
} // namespace picongpu
