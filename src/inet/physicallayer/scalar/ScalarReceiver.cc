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

#include "inet/physicallayer/scalar/ScalarReceiver.h"
#include "inet/physicallayer/scalar/ScalarReception.h"
#include "inet/physicallayer/scalar/ScalarNoise.h"
#include "inet/physicallayer/scalar/ScalarSNIR.h"
#include "inet/physicallayer/common/BandListening.h"

namespace inet {

namespace physicallayer {

Define_Module(ScalarReceiver);

ScalarReceiver::ScalarReceiver() :
    FlatReceiverBase()
{
}

void ScalarReceiver::printToStream(std::ostream& stream) const
{
    stream << "ScalarReceiver, ";
    FlatReceiverBase::printToStream(stream);
}

} // namespace physicallayer

} // namespace inet

