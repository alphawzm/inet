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

#include "ScalarImplementation.h"
#include "IRadioChannel.h"

Define_Module(ScalarRadioSignalFreeSpaceAttenuation);
Define_Module(ScalarRadioBackgroundNoise);
Define_Module(ScalarRadioSignalReceiver);
Define_Module(ScalarRadioSignalTransmitter);

void ScalarRadioSignalTransmission::printToStream(std::ostream &stream) const
{
    RadioSignalTransmissionBase::printToStream(stream);
    stream << ", power = " << power << ", carrier frequency = " << carrierFrequency << ", bandwidth = " << bandwidth;
}

void ScalarRadioSignalReception::printToStream(std::ostream &stream) const
{
    RadioSignalReceptionBase::printToStream(stream);
    stream << ", power = " << power << ", carrier frequency = " << carrierFrequency << ", bandwidth = " << bandwidth;
}

W ScalarRadioSignalNoise::computeMaxPower(simtime_t startTime, simtime_t endTime) const
{
    W noisePower = W(0);
    W maxNoisePower = W(0);
    for (std::map<simtime_t, W>::const_iterator it = powerChanges->begin(); it != powerChanges->end(); it++)
    {
        noisePower += it->second;
        if (noisePower > maxNoisePower && startTime <= it->first && it->first <= endTime)
            maxNoisePower = noisePower;
    }
    return maxNoisePower;
}

const IRadioSignalReception *ScalarRadioSignalAttenuationBase::computeReception(const IRadio *receiverRadio, const IRadioSignalTransmission *transmission) const
{
    const IRadioChannel *channel = receiverRadio->getChannel();
    const IRadio *transmitterRadio = transmission->getTransmitter();
    const IRadioAntenna *receiverAntenna = receiverRadio->getAntenna();
    const IRadioAntenna *transmitterAntenna = transmitterRadio->getAntenna();
    const ScalarRadioSignalTransmission *scalarTransmission = check_and_cast<const ScalarRadioSignalTransmission *>(transmission);
    const IRadioSignalArrival *arrival = channel->getArrival(receiverRadio, transmission);
    const simtime_t receptionStartTime = arrival->getStartTime();
    const simtime_t receptionEndTime = arrival->getEndTime();
    const Coord receptionStartPosition = arrival->getStartPosition();
    const Coord receptionEndPosition = arrival->getEndPosition();
    // TODO: revise
    const Coord direction = receptionStartPosition - transmission->getStartPosition();
    double transmitterAntennaGain = transmitterAntenna->getGain(direction);
    double receiverAntennaGain = receiverAntenna->getGain(direction);
    const IRadioSignalLoss *loss = computeLoss(transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition);
    double lossFactor = check_and_cast<const ScalarRadioSignalLoss *>(loss)->getFactor();
    W transmissionPower = scalarTransmission->getPower();
    W receptionPower = transmitterAntennaGain * receiverAntennaGain * lossFactor * transmissionPower;
    delete loss;
    return new ScalarRadioSignalReception(receiverRadio, transmission, receptionStartTime, receptionEndTime, receptionStartPosition, receptionEndPosition, receptionPower, scalarTransmission->getCarrierFrequency(), scalarTransmission->getBandwidth());
}

const IRadioSignalLoss *ScalarRadioSignalFreeSpaceAttenuation::computeLoss(const IRadioSignalTransmission *transmission, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) const
{
    const ScalarRadioSignalTransmission *scalarTransmission = check_and_cast<const ScalarRadioSignalTransmission *>(transmission);
    double pathLoss = computePathLoss(transmission, startTime, endTime, startPosition, endPosition, scalarTransmission->getCarrierFrequency());
    return new ScalarRadioSignalLoss(pathLoss);
}

const IRadioSignalLoss *ScalarRadioSignalCompoundAttenuation::computeLoss(const IRadioSignalTransmission *transmission, simtime_t startTime, simtime_t endTime, Coord startPosition, Coord endPosition) const
{
    double totalLoss;
    for (std::vector<const IRadioSignalAttenuation *>::const_iterator it = elements->begin(); it != elements->end(); it++)
    {
        const IRadioSignalAttenuation *element = *it;
        const ScalarRadioSignalLoss *scalarLoss = check_and_cast<const ScalarRadioSignalLoss *>(element->computeLoss(transmission, startTime, endTime, startPosition, endPosition));
        totalLoss *= scalarLoss->getFactor();
    }
    return new ScalarRadioSignalLoss(totalLoss);
}

void ScalarRadioBackgroundNoise::initialize(int stage)
{
    cCompoundModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        power = mW(FWMath::dBm2mW(par("power")));
    }
}

const IRadioSignalNoise *ScalarRadioBackgroundNoise::computeNoise(const IRadioSignalListening *listening) const
{
    const ScalarRadioSignalListening *scalarListening = check_and_cast<const ScalarRadioSignalListening *>(listening);
    simtime_t startTime = listening->getStartTime();
    simtime_t endTime = listening->getEndTime();
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
    powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
    return new ScalarRadioSignalNoise(startTime, endTime, powerChanges, scalarListening->getCarrierFrequency(), scalarListening->getBandwidth());
}

const IRadioSignalNoise *ScalarRadioBackgroundNoise::computeNoise(const IRadioSignalReception *reception) const
{
    const ScalarRadioSignalReception *scalarReception = check_and_cast<const ScalarRadioSignalReception *>(reception);
    simtime_t startTime = reception->getStartTime();
    simtime_t endTime = reception->getEndTime();
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
    powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
    return new ScalarRadioSignalNoise(startTime, endTime, powerChanges, scalarReception->getCarrierFrequency(), scalarReception->getBandwidth());
}

void ScalarRadioSignalListeningDecision::printToStream(std::ostream &stream) const
{
    RadioSignalListeningDecision::printToStream(stream);
    stream << ", maximum power = " << powerMax;
}

void ScalarRadioSignalTransmitter::initialize(int stage)
{
    cCompoundModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        headerBitLength = par("headerBitLength");
        bitrate = bps(par("bitrate"));
        power = W(par("power"));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}

void ScalarRadioSignalTransmitter::printToStream(std::ostream &stream) const
{
    stream << "scalar radio signal transmitter, headerBitLength = " << headerBitLength << ", "
           << "bitrate = " << bitrate << ", "
           << "power = " <<  power << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

const IRadioSignalTransmission *ScalarRadioSignalTransmitter::createTransmission(const IRadio *radio, const cPacket *packet, const simtime_t startTime) const
{
    simtime_t duration = (packet->getBitLength() + headerBitLength) / bitrate.get();
    simtime_t endTime = startTime + duration;
    IMobility *mobility = radio->getAntenna()->getMobility();
    Coord startPosition = mobility->getPosition(startTime);
    Coord endPosition = mobility->getPosition(endTime);
    return new ScalarRadioSignalTransmission(radio, startTime, endTime, startPosition, endPosition, headerBitLength, packet->getBitLength(), bitrate, power, carrierFrequency, bandwidth);
}

void ScalarRadioSignalReceiver::initialize(int stage)
{
    SNIRRadioSignalReceiverBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL)
    {
        energyDetection = mW(FWMath::dBm2mW(par("energyDetection")));
        sensitivity = mW(FWMath::dBm2mW(par("sensitivity")));
        carrierFrequency = Hz(par("carrierFrequency"));
        bandwidth = Hz(par("bandwidth"));
    }
}

void ScalarRadioSignalReceiver::printToStream(std::ostream &stream) const
{
    stream << "scalar radio signal receiver, energyDetection = " << energyDetection << ", "
           << "sensitivity = " <<  sensitivity << ", "
           << "carrierFrequency = " << carrierFrequency << ", "
           << "bandwidth = " << bandwidth;
}

const IRadioSignalListening *ScalarRadioSignalReceiver::createListening(const IRadio *radio, const simtime_t startTime, const simtime_t endTime, const Coord startPosition, const Coord endPosition) const
{
    return new ScalarRadioSignalListening(radio, startTime, endTime, startPosition, endPosition, carrierFrequency, bandwidth);
}

bool ScalarRadioSignalReceiver::areOverlappingBands(Hz carrierFrequency1, Hz bandwidth1, Hz carrierFrequency2, Hz bandwidth2) const
{
    return carrierFrequency1 + bandwidth1 / 2 >= carrierFrequency2 - bandwidth2 / 2 &&
           carrierFrequency1 - bandwidth1 / 2 <= carrierFrequency2 + bandwidth2 / 2;
}

bool ScalarRadioSignalReceiver::computeIsReceptionPossible(const IRadioSignalReception *reception) const
{
    const ScalarRadioSignalReception *scalarReception = check_and_cast<const ScalarRadioSignalReception *>(reception);
    if (carrierFrequency == scalarReception->getCarrierFrequency() && bandwidth == scalarReception->getBandwidth())
        return scalarReception->getPower() >= sensitivity;
    else if (areOverlappingBands(carrierFrequency, bandwidth, scalarReception->getCarrierFrequency(), scalarReception->getBandwidth()))
        throw cRuntimeError("Overlapping bands are not supported");
    else
        return false;
}

const IRadioSignalNoise *ScalarRadioSignalReceiver::computeNoise(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *receptions, const IRadioSignalNoise *backgroundNoise) const
{
    const ScalarRadioSignalListening *scalarListening = check_and_cast<const ScalarRadioSignalListening *>(listening);
    Hz carrierFrequency = scalarListening->getCarrierFrequency();
    Hz bandwidth = scalarListening->getBandwidth();
    simtime_t noiseStartTime = SimTime::getMaxTime();
    simtime_t noiseEndTime = 0;
    std::map<simtime_t, W> *powerChanges = new std::map<simtime_t, W>();
    for (std::vector<const IRadioSignalReception *>::const_iterator it = receptions->begin(); it != receptions->end(); it++)
    {
        const ScalarRadioSignalReception *reception = check_and_cast<const ScalarRadioSignalReception *>(*it);
        if (carrierFrequency == reception->getCarrierFrequency() && bandwidth == reception->getBandwidth())
        {
            W power = reception->getPower();
            simtime_t startTime = reception->getStartTime();
            simtime_t endTime = reception->getEndTime();
            if (startTime < noiseStartTime)
                noiseStartTime = startTime;
            if (endTime > noiseEndTime)
                noiseEndTime = endTime;
            std::map<simtime_t, W>::iterator itStartTime = powerChanges->find(startTime);
            if (itStartTime != powerChanges->end())
                itStartTime->second += power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(startTime, power));
            std::map<simtime_t, W>::iterator itEndTime = powerChanges->find(endTime);
            if (itEndTime != powerChanges->end())
                itEndTime->second -= power;
            else
                powerChanges->insert(std::pair<simtime_t, W>(endTime, -power));
        }
        else if (areOverlappingBands(carrierFrequency, bandwidth, reception->getCarrierFrequency(), reception->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    if (backgroundNoise)
    {
        const ScalarRadioSignalNoise *scalarBackgroundNoise = check_and_cast<const ScalarRadioSignalNoise *>(backgroundNoise);
        if (carrierFrequency == scalarBackgroundNoise->getCarrierFrequency() && bandwidth == scalarBackgroundNoise->getBandwidth())
        {
            const std::map<simtime_t, W> *backgroundNoisePowerChanges = check_and_cast<const ScalarRadioSignalNoise *>(backgroundNoise)->getPowerChanges();
            for (std::map<simtime_t, W>::const_iterator it = backgroundNoisePowerChanges->begin(); it != backgroundNoisePowerChanges->end(); it++)
            {
                std::map<simtime_t, W>::iterator jt = powerChanges->find(it->first);
                if (jt != powerChanges->end())
                    jt->second += it->second;
                else
                    powerChanges->insert(std::pair<simtime_t, W>(it->first, it->second));
            }
        }
        else if (areOverlappingBands(carrierFrequency, bandwidth, scalarBackgroundNoise->getCarrierFrequency(), scalarBackgroundNoise->getBandwidth()))
            throw cRuntimeError("Overlapping bands are not supported");
    }
    return new ScalarRadioSignalNoise(noiseStartTime, noiseEndTime, powerChanges, carrierFrequency, bandwidth);
}

const IRadioSignalListeningDecision *ScalarRadioSignalReceiver::computeListeningDecision(const IRadioSignalListening *listening, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    const ScalarRadioSignalNoise *scalarNoise = check_and_cast<const ScalarRadioSignalNoise *>(computeNoise(listening, interferingReceptions, backgroundNoise));
    W maxPower = scalarNoise->computeMaxPower(listening->getStartTime(), listening->getEndTime());
    return new ScalarRadioSignalListeningDecision(listening, maxPower >= energyDetection, maxPower);
}

const IRadioSignalReceptionDecision *ScalarRadioSignalReceiver::computeReceptionDecision(const IRadioSignalListening *listening, const IRadioSignalReception *reception, const std::vector<const IRadioSignalReception *> *interferingReceptions, const IRadioSignalNoise *backgroundNoise) const
{
    // TODO: factor with base class
    const ScalarRadioSignalListening *scalarListening = check_and_cast<const ScalarRadioSignalListening *>(listening);
    const ScalarRadioSignalReception *scalarReception = check_and_cast<const ScalarRadioSignalReception *>(reception);
    if (scalarListening->getCarrierFrequency() == scalarReception->getCarrierFrequency() && scalarListening->getBandwidth() == scalarReception->getBandwidth())
        SNIRRadioSignalReceiverBase::computeReceptionDecision(listening, reception, interferingReceptions, backgroundNoise);
    else if (areOverlappingBands(scalarListening->getCarrierFrequency(), scalarListening->getBandwidth(), scalarReception->getCarrierFrequency(), scalarReception->getBandwidth()))
        throw cRuntimeError("Overlapping bands are not supported");
    else
        return new RadioSignalReceptionDecision(reception, false, false, NaN);
}

double ScalarRadioSignalReceiver::computeSNIRMin(const IRadioSignalReception *reception, const IRadioSignalNoise *noise) const
{
    const ScalarRadioSignalNoise *scalarNoise = check_and_cast<const ScalarRadioSignalNoise *>(noise);
    const ScalarRadioSignalReception *scalarReception = check_and_cast<const ScalarRadioSignalReception *>(reception);
    return (scalarReception->getPower() / scalarNoise->computeMaxPower(reception->getStartTime(), reception->getEndTime())).get();
}
