//
// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_IATTENUATION_H
#define __INET_IATTENUATION_H

#include "inet/physicallayer/contract/IRadio.h"
#include "inet/physicallayer/contract/IListening.h"
#include "inet/physicallayer/contract/ITransmission.h"
#include "inet/physicallayer/contract/IReception.h"
#include "inet/physicallayer/contract/IInterference.h"
#include "inet/physicallayer/contract/ISNIR.h"

namespace inet {

namespace physicallayer {

/**
 * This interface models how a radio signal attenuates during propagation. It
 * includes various effects such as free-space path loss, shadowing, refraction,
 * reflection, absorption, diffraction and others.
 */
class INET_API IAttenuation : public IPrintableObject
{
  public:
    /**
     * Returns the reception for the provided transmission at the receiver.
     * The result incorporates all modeled attenuation. This function never
     * returns NULL.
     */
    virtual const IReception *computeReception(const IRadio *receiver, const ITransmission *transmission) const = 0;

    /**
     * Returns the total noise summing up all the interfering receptions and
     * noises. This function never returns NULL.
     */
    virtual const INoise *computeNoise(const IListening *listening, const IInterference *interference) const = 0;

    /**
     * Returns the signal to noise and interference ratio. This function never
     * returns NULL.
     */
    virtual const ISNIR *computeSNIR(const IReception *reception, const INoise *noise) const = 0;
};

} // namespace physicallayer

} // namespace inet

#endif // ifndef __INET_IATTENUATION_H

