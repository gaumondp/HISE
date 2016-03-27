/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#if STANDALONE_CONVOLUTION
#else
#include <regex>
#endif


ModulatorSamplerSound::ModulatorSamplerSound(StreamingSamplerSound *sound, int index_) :
index(index_),
wrappedSound(sound),
gain(1.0f),
isMultiMicSound(false),
centPitch(0),
pitchFactor(1.0),
maxRRGroup(1),
rrGroup(1),
normalizedPeak(-1.0f),
isNormalized(false),
upperVeloXFadeValue(0),
lowerVeloXFadeValue(0),
pan(0),
purged(false),
purgeChannels(0)
{
	soundList.add(wrappedSound.get());

	checkFileReference();

	setProperty(Pan, 0);
}


ModulatorSamplerSound::ModulatorSamplerSound(StreamingSamplerSoundArray &soundArray, int index_):
index(index_),
wrappedSound(soundArray.getFirst()),
isMultiMicSound(true),
gain(1.0f),
centPitch(0),
pitchFactor(1.0),
maxRRGroup(1),
rrGroup(1),
normalizedPeak(-1.0f),
isNormalized(false),
upperVeloXFadeValue(0),
lowerVeloXFadeValue(0),
pan(0),
purged(false),
purgeChannels(0)
{
	soundList.add(wrappedSound.get());

	for (int i = 1; i < soundArray.size(); i++)
	{
		wrappedSounds.add(soundArray[i]);
		soundList.add(soundArray[i]);
	}

	checkFileReference();
	setProperty(Pan, 0);
}

ModulatorSamplerSound::~ModulatorSamplerSound()
{
	masterReference.clear();
	soundList.clear();
	removeAllChangeListeners();
}

String ModulatorSamplerSound::getPropertyName(Property p)
{
	switch (p)
	{
	case ID:			return "ID";
	case FileName:		return "FileName";
	case RootNote:		return "Root";
	case KeyHigh:		return "HiKey";
	case KeyLow:		return "LoKey";
	case VeloLow:		return "LoVel";
	case VeloHigh:		return "HiVel";
	case RRGroup:		return "RRGroup";
	case Volume:		return "Volume";
	case Pan:			return "Pan";
	case Normalized:	return "Normalized";
	case Pitch:			return "Pitch";
	case SampleStart:	return "SampleStart";
	case SampleEnd:		return "SampleEnd";
	case SampleStartMod:return "SampleStartMod";
	case LoopEnabled:	return "LoopEnabled";
	case LoopStart:		return "LoopStart";
	case LoopEnd:		return "LoopEnd";
	case LoopXFade:		return "LoopXFade";
	case UpperVelocityXFade:	return "UpperVelocityXFade";
	case LowerVelocityXFade:	return "LowerVelocityXFade";
	case SampleState:	return "SampleState";
	default:			jassertfalse; return String::empty;
	}
}

Range<int> ModulatorSamplerSound::getPropertyRange(Property p) const
{
	switch (p)
	{
	case ID:			return Range<int>(0, INT_MAX);
	case FileName:		jassertfalse; return Range<int>();
	case RootNote:		return Range<int>(0, 127);
	case KeyHigh:		return Range<int>((int)getProperty(KeyLow), 127);
	case KeyLow:		return Range<int>(0, (int)getProperty(KeyHigh));
	case VeloLow:		return Range<int>(0, (int)getProperty(VeloHigh) - 1);
	case VeloHigh:		return Range<int>((int)getProperty(VeloLow) + 1, 127);
	case Volume:		return Range<int>(-100, 18);
	case Pan:			return Range<int>(-100, 100);
	case Normalized:	return Range<int>(0, 1);
	case RRGroup:		return Range<int>(1, maxRRGroup);
	case Pitch:			return Range<int>(-100, 100);
	case SampleStart:	return wrappedSound->isLoopEnabled() ? Range<int>(0, jmin<int>((int)wrappedSound->getLoopStart() - (int)wrappedSound->getLoopCrossfade(), (int)(wrappedSound->getSampleEnd() - (int)wrappedSound->getSampleStartModulation()))) :
		Range<int>(0, (int)wrappedSound->getSampleEnd() - (int)wrappedSound->getSampleStartModulation());
	case SampleEnd:		{
		const int sampleStartMinimum = (int)(wrappedSound->getSampleStart() + wrappedSound->getSampleStartModulation());
		const int upperLimit = (int)wrappedSound->getLengthInSamples();

		if (wrappedSound->isLoopEnabled())
		{
			const int lowerLimit = jmax<int>(sampleStartMinimum, (int)wrappedSound->getLoopEnd());
			return Range<int>(lowerLimit, upperLimit);

		}
		else
		{
			const int lowerLimit = sampleStartMinimum;
			return Range<int>(lowerLimit, upperLimit);

		}
	}
	case SampleStartMod:return Range<int>(0, (int)wrappedSound->getSampleLength());
	case LoopEnabled:	return Range<int>(0, 1);
	case LoopStart:		return Range<int>((int)wrappedSound->getSampleStart() + (int)wrappedSound->getLoopCrossfade(), (int)wrappedSound->getLoopEnd() - (int)wrappedSound->getLoopCrossfade());
	case LoopEnd:		return Range<int>((int)wrappedSound->getLoopStart() + (int)wrappedSound->getLoopCrossfade(), (int)wrappedSound->getSampleEnd());
	case LoopXFade:		return Range<int>(0, jmin<int>((int)(wrappedSound->getLoopStart() - wrappedSound->getSampleStart()), (int)wrappedSound->getLoopLength()));
	case UpperVelocityXFade: return Range < int >(0, (int)getProperty(VeloHigh) - ((int)getProperty(VeloLow) + lowerVeloXFadeValue));
	case LowerVelocityXFade: return Range < int >(0, (int)getProperty(VeloHigh) - upperVeloXFadeValue - (int)getProperty(VeloLow));
	case SampleState:	return Range<int>(0, (int)StreamingSamplerSound::numSampleStates - 1);
	default:			jassertfalse; return Range<int>();
	}
}

String ModulatorSamplerSound::getPropertyAsString(Property p) const
{
	switch (p)
	{
	case ID:			return String(index);
	case FileName:		return wrappedSound->getFileName(false);
	case RootNote:		return MidiMessage::getMidiNoteName(rootNote, true, true, 3);
	case KeyHigh:		return MidiMessage::getMidiNoteName(midiNotes.getHighestBit(), true, true, 3);
	case KeyLow:		return MidiMessage::getMidiNoteName(midiNotes.findNextSetBit(0), true, true, 3);
	case VeloHigh:		return String(velocityRange.getHighestBit());
	case VeloLow:		return String(velocityRange.findNextSetBit(0));
	case RRGroup:		return String(rrGroup);
	case Volume:		return String(Decibels::gainToDecibels(gain), 1) + " dB";
	case Pan:			return BalanceCalculator::getBalanceAsString(pan);
	case Normalized:	return isNormalized ? "Enabled" : "Disabled";
	case Pitch:			return String(centPitch, 0) + " ct";
	case SampleStart:	return String(wrappedSound->getSampleStart());
	case SampleEnd:		return String(wrappedSound->getSampleEnd());
	case SampleStartMod:return String(wrappedSound->getSampleStartModulation());
	case LoopEnabled:	return wrappedSound->isLoopEnabled() ? "Enabled" : "Disabled";
	case LoopStart:		return String(wrappedSound->getLoopStart());
	case LoopEnd:		return String(wrappedSound->getLoopEnd());
	case LoopXFade:		return String(wrappedSound->getLoopCrossfade());
	case UpperVelocityXFade: return String(upperVeloXFadeValue);
	case LowerVelocityXFade: return String(lowerVeloXFadeValue);
	case SampleState:	return wrappedSound->getSampleStateAsString();
	default:			jassertfalse; return String::empty;
	}
}

var ModulatorSamplerSound::getProperty(Property p) const
{
	switch (p)
	{
	case ID:			return var(index);
	case FileName:		return var(wrappedSound->getFileName(true));
	case RootNote:		return var(rootNote);
	case KeyHigh:		return var(midiNotes.getHighestBit());
	case KeyLow:		return var(midiNotes.findNextSetBit(0));
	case VeloHigh:		return var(velocityRange.getHighestBit());
	case VeloLow:		return var(velocityRange.findNextSetBit(0));
	case RRGroup:		return var(rrGroup);
	case Volume:		return var(Decibels::gainToDecibels(gain));
	case Pan:			return var(pan);
	case Normalized:	return var(isNormalized);
	case Pitch:			return var(centPitch);
	case SampleStart:	return var(wrappedSound->getSampleStart());
	case SampleEnd:		return var(wrappedSound->getSampleEnd());
	case SampleStartMod:return var(wrappedSound->getSampleStartModulation());
	case LoopEnabled:	return var(wrappedSound->isLoopEnabled());
	case LoopStart:		return var(wrappedSound->getLoopStart());
	case LoopEnd:		return var(wrappedSound->getLoopEnd());
	case LoopXFade:		return var(wrappedSound->getLoopCrossfade());
	case UpperVelocityXFade: return var(upperVeloXFadeValue);
	case LowerVelocityXFade: return var(lowerVeloXFadeValue);
	case SampleState:	return var(isPurged());
	default:			jassertfalse; return var::undefined();
	}
}

void ModulatorSamplerSound::setProperty(Property p, int newValue, NotificationType notifyEditor/*=sendNotification*/)
{
	ScopedLock sl(lock);

	switch (p)
	{
	case ID:			jassertfalse; break;
	case FileName:		jassertfalse; break;
	case RootNote:		rootNote = newValue; break;
	case VeloHigh:	{	int low = jmin(velocityRange.findNextSetBit(0), newValue, 127);
		velocityRange.clear();
		velocityRange.setRange(low, newValue - low + 1, true); break; }
	case VeloLow:	{	int high = jmax(velocityRange.getHighestBit(), newValue, 0);
		velocityRange.clear();
		velocityRange.setRange(newValue, high - newValue + 1, true); break; }
	case KeyHigh:	{	int low = jmin(midiNotes.findNextSetBit(0), newValue, 127);
		midiNotes.clear();
		midiNotes.setRange(low, newValue - low + 1, true); break; }
	case KeyLow:	{	int high = jmax(midiNotes.getHighestBit(), newValue, 0);
		midiNotes.clear();
		midiNotes.setRange(newValue, high - newValue + 1, true); break; }
	case RRGroup:		rrGroup = newValue; break;
	case Normalized:	isNormalized = newValue == 1; 
						if (isNormalized && normalizedPeak < 0.0f) calculateNormalizedPeak();
						break;
	case Volume:	{	gain = Decibels::decibelsToGain((float)newValue);
		break;
	}
	case Pan:		{
		pan = (int)newValue;
		leftBalanceGain = BalanceCalculator::getGainFactorForBalance(pan, true);
		rightBalanceGain = BalanceCalculator::getGainFactorForBalance(pan, false);
		break;
	}
	case Pitch:		{	centPitch = newValue;
		pitchFactor = powf(2.0f, (float)centPitch / 1200.f);
		break;
	};
	case SampleStart:	FOR_EVERY_SOUND(setSampleStart(newValue)); break;
	case SampleEnd:		FOR_EVERY_SOUND(setSampleEnd(newValue)); break;
	case SampleStartMod: FOR_EVERY_SOUND(setSampleStartModulation(newValue)); break;

	case LoopEnabled:	FOR_EVERY_SOUND(setLoopEnabled(newValue == 1.0f)); break;
	case LoopStart:		FOR_EVERY_SOUND(setLoopStart(newValue)); break;
	case LoopEnd:		FOR_EVERY_SOUND(setLoopEnd(newValue)); break;
	case LoopXFade:		FOR_EVERY_SOUND(setLoopCrossfade(newValue)); break;
	case LowerVelocityXFade: lowerVeloXFadeValue = newValue; break;
	case UpperVelocityXFade: upperVeloXFadeValue = newValue; break;
	case SampleState:	setPurged(newValue == 1.0f); break;
	default:			jassertfalse; break;
	}

	if(notifyEditor) sendChangeMessage();
}

void ModulatorSamplerSound::toggleBoolProperty(ModulatorSamplerSound::Property p, NotificationType notifyEditor/*=sendNotification*/)
{
	

	switch (p)
	{
	case Normalized:
	{
		isNormalized = !isNormalized; 

		if (isNormalized) calculateNormalizedPeak();

		break;
	}
	case LoopEnabled:
	{
		const bool wasEnabled = wrappedSound->isLoopEnabled();
		FOR_EVERY_SOUND(setLoopEnabled(!wasEnabled)); break;
	}
		
	default:			jassertfalse; break;
	}

	if(notifyEditor == sendNotification) sendChangeMessage();
}

ValueTree ModulatorSamplerSound::exportAsValueTree() const
{
	ScopedLock sl(lock);
	ValueTree v("sample");

	for (int i = ID; i < numProperties; i++)
	{
		Property p = (Property)i;

		v.setProperty(getPropertyName(p), getProperty(p), nullptr);
	}

	if (isMultiMicSound)
	{
		v.removeProperty(getPropertyName(FileName), nullptr);

		for (int i = 0; i < soundList.size(); i++)
		{
			ValueTree fileChild("file");
			fileChild.setProperty("FileName", soundList[i]->getFileName(true), nullptr);
			v.addChild(fileChild, -1, nullptr);
		}
	}

	v.setProperty("NormalizedPeak", normalizedPeak, nullptr);

	v.setProperty("Duplicate", wrappedSound->getReferenceCount() >= 3, nullptr);

	return v;
}

void ModulatorSamplerSound::restoreFromValueTree(const ValueTree &v)
{
	for (int i = RootNote; i < numProperties; i++) // ID and filename must be passed to the constructor!
	{
		Property p = (Property)i;

		var x = v.getProperty(getPropertyName(p), var::undefined());

		if (!x.isUndefined()) setProperty(p, x, dontSendNotification);
	}

	checkFileReference();

	normalizedPeak = v.getProperty("NormalizedPeak", -1.0f);
}

void ModulatorSamplerSound::startPropertyChange(Property p, int newValue)
{
	String x;
	x << getPropertyName(p) << ": " << getPropertyAsString(p) << " -> " << String(newValue);
	if (undoManager != nullptr) undoManager->beginNewTransaction(x);
}

void ModulatorSamplerSound::endPropertyChange(Property p, int startValue, int endValue)
{
	String x;
	x << getPropertyName(p) << ": " << String(startValue) << " -> " << String(endValue);
	if (undoManager != nullptr) undoManager->setCurrentTransactionName(x);
}

void ModulatorSamplerSound::endPropertyChange(const String &actionName)
{
	if (undoManager != nullptr) undoManager->setCurrentTransactionName(actionName);
}

void ModulatorSamplerSound::setPropertyWithUndo(Property p, var newValue)
{
	if (undoManager != nullptr)
	{
		undoManager->perform(new PropertyChange(this, p, newValue));
	}
	else					   setProperty(p, (int)newValue);
}

void ModulatorSamplerSound::openFileHandle()
{
	FOR_EVERY_SOUND(openFileHandle());
}

void ModulatorSamplerSound::closeFileHandle()
{
	FOR_EVERY_SOUND(closeFileHandle());
}

Range<int> ModulatorSamplerSound::getNoteRange() const			{ ScopedLock sl(lock); return Range<int>(midiNotes.findNextSetBit(0), midiNotes.getHighestBit() + 1); }
Range<int> ModulatorSamplerSound::getVelocityRange() const		{ ScopedLock sl(lock); return Range<int>(velocityRange.findNextSetBit(0), velocityRange.getHighestBit() + 1); }
float ModulatorSamplerSound::getPropertyVolume() const noexcept { ScopedLock sl(lock); return gain; }
double ModulatorSamplerSound::getPropertyPitch() const noexcept { ScopedLock sl(lock); return pitchFactor; }

void ModulatorSamplerSound::setMaxRRGroupIndex(int newGroupLimit)
{
	maxRRGroup = newGroupLimit;
	rrGroup = jmin(rrGroup, newGroupLimit);
}

void ModulatorSamplerSound::setMappingData(MappingData newData)
{
	rootNote = newData.rootNote;
	velocityRange.clear();
	velocityRange.setRange(newData.loVel, newData.hiVel - newData.loVel + 1, true);
	midiNotes.clear();
	midiNotes.setRange(newData.loKey, newData.hiKey - newData.loKey + 1, true);
	rrGroup = newData.rrGroup;
}

void ModulatorSamplerSound::calculateNormalizedPeak(bool forceScan /*= false*/)
{
	if (forceScan || normalizedPeak < 0.0f)
	{
		float highestPeak = 0.0f;

		for (int i = 0; i < soundList.size(); i++)
		{
			highestPeak = jmax<float>(highestPeak, soundList[i]->calculatePeakValue());
		}

		if (highestPeak != 0.0f)
		{
			normalizedPeak = 1.0f / highestPeak;
		}

		
	}
}

float ModulatorSamplerSound::getNormalizedPeak() const
{
	return (isNormalized && normalizedPeak != -1.0f) ? normalizedPeak : 1.0f;
}

float ModulatorSamplerSound::getBalance(bool getRightChannelGain) const
{
	return getRightChannelGain ? rightBalanceGain : leftBalanceGain;
}

void ModulatorSamplerSound::setVelocityXFade(int crossfadeLength, bool isUpperSound)
{
	if (isUpperSound) lowerVeloXFadeValue = crossfadeLength;
	else upperVeloXFadeValue = crossfadeLength;
}

void ModulatorSamplerSound::setPurged(bool shouldBePurged) 
{
	purged = shouldBePurged;
	FOR_EVERY_SOUND(setPurged(shouldBePurged));
}

void ModulatorSamplerSound::checkFileReference()
{
	allFilesExist = true;

	FOR_EVERY_SOUND(checkFileReference())

	for (int i = 0; i < soundList.size(); i++)
	{
		if (soundList[i]->isMissing())
		{
			allFilesExist = false;
			break;
		}
	}
}

float ModulatorSamplerSound::getGainValueForVelocityXFade(int velocity)
{
	if (upperVeloXFadeValue == 0 && lowerVeloXFadeValue == 0) return 1.0f;

	Range<int> upperRange = Range<int>(velocityRange.getHighestBit() - upperVeloXFadeValue, velocityRange.getHighestBit());
	Range<int> lowerRange = Range<int>(velocityRange.findNextSetBit(0), velocityRange.findNextSetBit(0) + lowerVeloXFadeValue);

	float delta = 1.0f;

	if (upperRange.contains(velocity))
	{
		delta = (float)(velocity - upperRange.getStart()) / (upperRange.getLength());

		return Interpolator::interpolateLinear(1.0f, 0.0f, delta);
	}
	else if (lowerRange.contains(velocity))
	{
		delta = (float)(velocity - lowerRange.getStart()) / (lowerRange.getLength());

		return Interpolator::interpolateLinear(0.0f, 1.0f, delta);
	}
	else
	{
		return 1.0f;
	}
}

int ModulatorSamplerSound::getNumMultiMicSamples() const noexcept { return soundList.size(); }

bool ModulatorSamplerSound::isChannelPurged(int channelIndex) const { return purgeChannels[channelIndex]; }

void ModulatorSamplerSound::setChannelPurged(int channelIndex, bool shouldBePurged)
{
	if (purged) return;

	purgeChannels.setBit(channelIndex, shouldBePurged);
	soundList[channelIndex]->setPurged(shouldBePurged);
}

int ModulatorSamplerSound::getRRGroup() const {	return rrGroup; }

void ModulatorSamplerSound::selectSoundsBasedOnRegex(const String &regexWildcard, ModulatorSampler *sampler, SelectedItemSet<WeakReference<ModulatorSamplerSound>> &set)
{
	bool subtractMode = false;

	bool addMode = false;

	String wildcard = regexWildcard;

	if (wildcard.startsWith("sub:"))
	{
		subtractMode = true;
		wildcard = wildcard.fromFirstOccurrenceOf("sub:", false, true);
	}
	else if (wildcard.startsWith("add:"))
	{
		addMode = true;
		wildcard = wildcard.fromFirstOccurrenceOf("add:", false, true);
	}
	else
	{
		set.deselectAll();
	}

	
#if STANDALONE_CONVOLUTION
#else
    try
    {
		std::regex reg(wildcard.toStdString());

		for (int i = 0; i < sampler->getNumSounds(); i++)
		{
			const String name = sampler->getSound(i)->getPropertyAsString(Property::FileName);

			if (std::regex_search(name.toStdString(), reg))
			{
				if (subtractMode)
				{
					set.deselect(sampler->getSound(i));
				}
				else
				{
					set.addToSelection(sampler->getSound(i));
				}


			}

		}
	}
	catch (std::regex_error e)
	{
		debugError(sampler, e.what());
	}
#endif
}


// ====================================================================================================================



ModulatorSamplerSound::PropertyChange::PropertyChange(ModulatorSamplerSound *soundToChange, Property p, var newValue) :
changedProperty(p),
currentValue(newValue),
sound(soundToChange),
lastValue(sound->getProperty(p))
{

}


bool ModulatorSamplerSound::PropertyChange::perform()
{
	if (sound != nullptr)
	{
		sound->setProperty(changedProperty, currentValue);
		return true;
	}
	else return false;
}

bool ModulatorSamplerSound::PropertyChange::undo()
{
	if (sound != nullptr)
	{
		sound->setProperty(changedProperty, lastValue);
		return true;
	}
	else return false;
}

// ====================================================================================================================

ModulatorSamplerSoundPool::ModulatorSamplerSoundPool(MainController *mc_) :
mc(mc_),
debugProcessor(nullptr),
mainAudioProcessor(nullptr),
numOpenFileHandles(0),
updatePool(true),
searchPool(true)
{
	afm.registerBasicFormats();
}

void ModulatorSamplerSoundPool::setDebugProcessor(Processor *p)
{
	debugProcessor = p;
}

ModulatorSamplerSound * ModulatorSamplerSoundPool::addSound(const ValueTree &soundDescription, int index, bool forceReuse /*= false*/)
{
	if (soundDescription.getNumChildren() > 1)
	{
		return addSoundWithMultiMic(soundDescription, index, forceReuse);
	}
	else
	{
		return addSoundWithSingleMic(soundDescription, index, forceReuse);
	}
}

void ModulatorSamplerSoundPool::deleteSound(ModulatorSamplerSound *soundToDelete)
{
	for (int i = 0; i < soundToDelete->getNumMultiMicSamples(); i++)
	{
		StreamingSamplerSound *sound = soundToDelete->getReferenceToSound(i);

		jassert(sound != nullptr);



		if (sound->getReferenceCount() == 2) // one for the array and two for the Synthesiser::Ptr from &delete()
		{
			pool.removeObject(sound);
		}
	}



	if(updatePool) sendChangeMessage();
}

void ModulatorSamplerSoundPool::loadMonolithicSound(const ValueTree &soundDescription, const File& data)
{
	jassert(data.existsAsFile());

	const String fileName = soundDescription.getProperty(ModulatorSamplerSound::getPropertyName(ModulatorSamplerSound::FileName));

	for (int i = 0; i < pool.size(); i++)
	{
		if (fileName == pool.getUnchecked(i)->getFileName(true))
		{
			return;
		};
	}

	int start = soundDescription.getProperty("mono_sample_start", 0);
	int length = soundDescription.getProperty("mono_sample_length", 0);



	StreamingSamplerSound *s = new StreamingSamplerSound(data, fileName, start, length, this);

	pool.add(s);
}

bool ModulatorSamplerSoundPool::loadMonolithicData(ValueTree &sampleMaps)
{
	for (int i = 0; i < sampleMaps.getNumChildren(); i++)
	{
		ValueTree sampleMap = sampleMaps.getChild(i);

		String fileNameString = sampleMap.getProperty("FileName", String::empty);

		jassert(fileNameString == String::empty);

		String productId = sampleMap.getProperty("UniqueId", String::empty);

		String sampleMapId = sampleMap.getProperty("SampleMapIdentifier", String::empty);

		String sampleFolder = PresetHandler::getSampleDataFolder(productId).getFullPathName();

		if (sampleFolder.isEmpty()) return false;

		File f = File(sampleFolder + "/" + sampleMapId + ".dat");

		if (!f.existsAsFile())
		{
			f = PresetHandler::getSampleFolder(productId).getFullPathName() + "/" + sampleMapId + ".dat";


		}

		if (f.existsAsFile())
		{
			for (int j = 0; j < sampleMap.getNumChildren(); j++)
			{
				loadMonolithicSound(sampleMap.getChild(j), f);
			}
		}
		else
		{



			return false;
		}


	}

	sendChangeMessage();

	return true;
}

void ModulatorSamplerSoundPool::clearUnreferencedSamples()
{
	for (int i = 0; i < pool.size(); i++)
	{
		if (pool[i]->getReferenceCount() == 2)
		{
			pool.remove(i--);
		}
	}

	sendChangeMessage();
}

int ModulatorSamplerSoundPool::getNumSoundsInPool() const noexcept
{
	return pool.size();
}

void ModulatorSamplerSoundPool::getMissingSamples(Array<StreamingSamplerSound*> &missingSounds) const
{
	for (int i = 0; i < pool.size(); i++)
	{
		if (pool[i]->isMissing())
		{
			missingSounds.add(pool[i]);
		}
	}
}

void ModulatorSamplerSoundPool::deleteMissingSamples()
{
	if (PresetHandler::showYesNoWindow("Delete missing samples", "Do you really want to delete the missing samples?"))
	{

	}
}


class SampleResolver : public ThreadWithAsyncProgressWindow
{
public:

	SampleResolver(ModulatorSamplerSoundPool *pool_, Processor *synthChain_):
		ThreadWithAsyncProgressWindow("Sample Resolver"),
		pool(pool_),
		mainSynthChain(synthChain_)
	{
		pool->getMissingSamples(missingSounds);

		if (missingSounds.size() == 0)
		{
			showStatusMessage("No missing samples detected.");
			addBasicComponents(false);
		}
		else
		{
			numMissingSounds = missingSounds.size();

			remainingSounds = numMissingSounds;

			String text = "Remaining missing sounds: " + String(remainingSounds) + " / " + String(numMissingSounds) + " missing sounds.";

			showStatusMessage(text);

			String fileNames = missingSounds[0]->getFileName(true);
            
            String path;

            if(File::isAbsolutePath(fileNames))
            {
                path = File(fileNames).getParentDirectory().getFullPathName();

            }
            else path = fileNames;
            
			addTextEditor("fileNames", fileNames, "Filenames:");

			addTextEditor("search", path, "Search for:");
			addTextEditor("replace", path, "Replace with:");

			addBasicComponents(true);
		}
	};


	void run() override
	{
		const String search = getTextEditorContents("search");
		const String replace = getTextEditorContents("replace");

		pool->setUpdatePool(false);

		int foundThisTime = 0;

		showStatusMessage("Replacing references");

		try
		{
            const double numMissingSoundsDouble = (double)missingSounds.size();
            
			for (int i = 0; i < missingSounds.size(); i++)
			{
				
				if (threadShouldExit()) return;

                setProgress(double(i) / numMissingSoundsDouble);

				StreamingSamplerSound *sound = missingSounds[i];

				String newFileName = sound->getFileName(true).replace(search, replace, true);

                String newFileNameSanitized = newFileName.replace("\\", "/");
                
				if (File(newFileNameSanitized).existsAsFile())
				{
					sound->replaceFileReference(newFileNameSanitized);

					foundThisTime++;
					missingSounds.remove(i);
					i--;
				}
			}
		}
		catch (StreamingSamplerSound::LoadingError e)
		{
			errorMessage = "There was an error at preloading.";
			return;
		}

		remainingSounds -= foundThisTime;
		
		showStatusMessage("Replacing references");

		Processor::Iterator<ModulatorSampler> iter(mainSynthChain);

		int numSamplers = iter.getNumProcessors();
		int index = 0;

		while (ModulatorSampler *s = iter.getNextProcessor())
		{
			setProgress((double)index / (double)numSamplers);

			for (int i = 0; i < s->getNumSounds(); i++)
			{
				s->getSound(i)->checkFileReference();
			}

			s->sendChangeMessage();

			index++;
		}
	}

	void threadFinished()
	{
		if (errorMessage.isEmpty())
		{
			PresetHandler::showMessageWindow("Missing Samples resolved", String(numMissingSounds - remainingSounds) + " out of " + String(numMissingSounds) + " were resolved.");
		}
		else
		{
			PresetHandler::showMessageWindow("Error", errorMessage);
		}

		pool->setUpdatePool(true);
		pool->sendChangeMessage();
	}

private:

	Array<StreamingSamplerSound*> missingSounds;

	int remainingSounds;
	int numMissingSounds;

	String errorMessage;

	ModulatorSamplerSoundPool *pool;
	WeakReference<Processor> mainSynthChain;


};


void ModulatorSamplerSoundPool::resolveMissingSamples(Component *childComponentOfMainEditor)
{
	BackendProcessorEditor *editor = dynamic_cast<BackendProcessorEditor*>(childComponentOfMainEditor);
	
	if(editor == nullptr) editor = childComponentOfMainEditor->findParentComponentOfClass<BackendProcessorEditor>();

	SampleResolver *r = new SampleResolver(this, editor->getMainSynthChain());

	r->setModalComponentOfMainEditor(childComponentOfMainEditor);


#if 0

	Array<StreamingSamplerSound*> missingSounds;

	getMissingSamples(missingSounds);

	if (missingSounds.size() == 0)
	{
		PresetHandler::showMessageWindow("No missing samples detected!", "All sample references are OK");
		return;
	}

	const int numMissingSounds = missingSounds.size();

	int remainingSounds = numMissingSounds;

	String text = "Remaining missing sounds: " + String(remainingSounds) + " / " + String(numMissingSounds) + " missing sounds.";

	double progress = 0.0;

	AlertWindowLookAndFeel laf;

	AlertWindow w("Resolve Missing Samples", String::empty, AlertWindow::AlertIconType::NoIcon);

	w.setLookAndFeel(&laf);
	w.setUsingNativeTitleBar(true);

	w.setColour(AlertWindow::backgroundColourId, Colour(0xff222222));
	w.setColour(AlertWindow::textColourId, Colours::white);
	w.addTextBlock(text);

	String fileNames = missingSounds[0]->getFileName(true);

	String path = File(fileNames).getParentDirectory().getFullPathName();

	w.addTextEditor("fileNames", fileNames, "Filenames:");

	w.addTextEditor("search", path, "Search for:");
	w.addTextEditor("replace", path, "Replace with:");
	w.addProgressBarComponent(progress);
	w.addButton("Replace current Wildcard", 1, KeyPress(KeyPress::returnKey));
	w.addButton("Close", 0, KeyPress(KeyPress::escapeKey));


	while (w.runModalLoop())
	{
		const String search = w.getTextEditorContents("search");
		const String replace = w.getTextEditorContents("replace");
		
		int foundThisTime = 0;

		try
		{


			for (int i = 0; i < missingSounds.size(); i++)
			{
				StreamingSamplerSound *sound = missingSounds[i];

				String newFileName = sound->getFileName(true).replace(search, replace, true);

				if (File(newFileName).existsAsFile())
				{
					sound->replaceFileReference(newFileName);


					foundThisTime++;
					missingSounds.remove(i);
					i--;
				}
			}

		}
		catch (StreamingSamplerSound::LoadingError e)
		{
			PresetHandler::showMessageWindow("Error at preloading.", "There was an error at preloading.");
		}

		remainingSounds -= foundThisTime;

		if (remainingSounds == 0) break;

		text.clear();
		text.append("Remaining missing sounds: " + String(remainingSounds) + " / " + String(numMissingSounds) + " missing sounds.", 2000);
		
		progress = (double)(numMissingSounds - remainingSounds) / (double)numMissingSounds;

		

	}
#endif
	
}

StringArray ModulatorSamplerSoundPool::getFileNameList() const
{
	StringArray sa;

	for (int i = 0; i < pool.size(); i++)
	{
		sa.add(pool[i]->getFileName(true));
	}

	return sa;
}

size_t ModulatorSamplerSoundPool::getMemoryUsageForAllSamples() const noexcept
{
	size_t memoryUsage = 0;

	for (int i = 0; i < pool.size(); i++)
	{
		memoryUsage += pool[i]->getActualPreloadSize();
	}

	return memoryUsage;
}



String ModulatorSamplerSoundPool::getTextForPoolTable(int columnId, int indexInPool)
{
#if USE_BACKEND

	if (indexInPool < pool.size())
	{
		switch (columnId)
		{
		case SamplePoolTable::FileName:	return pool[indexInPool]->getFileName();
		case SamplePoolTable::Memory:	return String((int)(pool[indexInPool]->getActualPreloadSize() / 1024)) + " kB";
		case SamplePoolTable::State:	return String(pool[indexInPool]->getSampleStateAsString());
		case SamplePoolTable::References:	return String(pool[indexInPool]->getReferenceCount() - 2);
		default:						jassertfalse; return "";
		}
	}
	else
	{
		jassertfalse;

		return "Invalid Index";

	}
#else

	return "";

#endif
}

void ModulatorSamplerSoundPool::increaseNumOpenFileHandles()
{
	numOpenFileHandles++;

	if(updatePool) sendChangeMessage();
}

void ModulatorSamplerSoundPool::decreaseNumOpenFileHandles()
{
	numOpenFileHandles--; 
	
	if (numOpenFileHandles < 0) numOpenFileHandles = 0;

	if(updatePool) sendChangeMessage();
}

bool ModulatorSamplerSoundPool::isFileBeingUsed(int poolIndex)
{
	if (poolIndex < pool.size())
	{
		return pool[poolIndex]->isOpened();
	}

	return false;
}

int ModulatorSamplerSoundPool::getSoundIndexFromPool(int64 hashCode)
{
	if (!searchPool) return -1;

	for (int i = 0; i < pool.size(); i++)
	{
		if (pool[i]->getHashCode() == hashCode) return i;
	}

	return -1;
}

ModulatorSamplerSound * ModulatorSamplerSoundPool::addSoundWithSingleMic(const ValueTree &soundDescription, int index, bool forceReuse /*= false*/)
{
	String fileName = GET_PROJECT_HANDLER(mc->getMainSynthChain()).getFilePath(soundDescription.getProperty(ModulatorSamplerSound::getPropertyName(ModulatorSamplerSound::FileName)), ProjectHandler::SubDirectories::Samples);

	//String fileName = soundDescription.getChild(0).getProperty(ModulatorSamplerSound::getPropertyName(ModulatorSamplerSound::FileName));

	if (forceReuse)
	{
        int64 hash = fileName.hashCode64();
		int i = getSoundIndexFromPool(hash);

		if (i != -1)
		{
			if(updatePool) sendChangeMessage();
			return new ModulatorSamplerSound(pool[i], index);
		}
		else
		{
			jassertfalse;
			return nullptr;
		}
	}
	else
	{
        static Identifier duplicate("Duplicate");
        
        const bool isDuplicate = soundDescription.getProperty(duplicate, true);
        
        const bool skipPoolSearch = isDuplicate ? false : true;
        
        if(!skipPoolSearch)
        {
            int64 hash = fileName.hashCode64();
            int i = getSoundIndexFromPool(hash);
            
            if (i != -1)
            {
                ModulatorSamplerSound *sound = new ModulatorSamplerSound(pool[i], index);
                if(updatePool) sendChangeMessage();
                return sound;
            }
        }
        
		StreamingSamplerSound *s = new StreamingSamplerSound(fileName, -1, -1, this);

		pool.add(s);

		if(updatePool) sendChangeMessage();

		return new ModulatorSamplerSound(s, index);
	}
}

ModulatorSamplerSound * ModulatorSamplerSoundPool::addSoundWithMultiMic(const ValueTree &soundDescription, int index, bool forceReuse /*= false*/)
{
	StreamingSamplerSoundArray multiMicArray;

	for (int i = 0; i < soundDescription.getNumChildren(); i++)
	{
        String fileName = GET_PROJECT_HANDLER(mc->getMainSynthChain()).getFilePath(soundDescription.getChild(i).getProperty(ModulatorSamplerSound::getPropertyName(ModulatorSamplerSound::FileName)).toString(), 
														   ProjectHandler::SubDirectories::Samples);

		if (forceReuse)
		{
            int64 hash = fileName.hashCode64();
			int i = getSoundIndexFromPool(hash);

			jassert(i != -1);

			multiMicArray.add(pool[i]);
			if(updatePool) sendChangeMessage();
		}
		else
		{
            static Identifier duplicate("Duplicate");
            
            const bool skipPoolSearch = soundDescription.getProperty(duplicate, true) ? false : true;
            
            if(!skipPoolSearch)
            {
                int64 hash = fileName.hashCode64();
                int i = getSoundIndexFromPool(hash);
                
                if (i != -1)
                {
                    multiMicArray.add(pool[i]);
                    continue;
                }
            }
            
			else
			{
				StreamingSamplerSound *s = new StreamingSamplerSound(fileName, -1, -1, this);

				multiMicArray.add(s);
				pool.add(s);
			}
		}
	}

	if(updatePool) sendChangeMessage();

	return new ModulatorSamplerSound(multiMicArray, index);
}