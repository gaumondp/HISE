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

#ifndef SATURATOR_H_INCLUDED
#define SATURATOR_H_INCLUDED

/** A simple gain effect that allows time variant modulation. */
class SaturatorEffect : public MasterEffectProcessor
{
public:

	SET_PROCESSOR_NAME("Saturator", "Saturator")

	enum InternalChains
	{
		SaturationChain = 0,
		numInternalChains
	};

	enum EditorStates
	{
		SaturationChainShown = Processor::numEditorStates,
		numEditorStates
	};

	enum Parameters
	{
		Saturation = 0,
		WetAmount,
		PreGain,
		PostGain,
		numParameters
	};

	SaturatorEffect(MainController *mc, const String &uid);;

	~SaturatorEffect()
	{};

	void setInternalAttribute(int parameterIndex, float newValue) override;;
	float getAttribute(int parameterIndex) const override;
	float getDefaultValue(int parameterIndex) const override;

	void restoreFromValueTree(const ValueTree &v) override;;
	ValueTree exportAsValueTree() const override;

	bool hasTail() const override { return false; };

	Processor *getChildProcessor(int /*processorIndex*/) override { return saturationChain; };
	const Processor *getChildProcessor(int /*processorIndex*/) const override { return saturationChain; };
	int getNumInternalChains() const override { return numInternalChains; };
	int getNumChildProcessors() const override { return numInternalChains; };

	ProcessorEditorBody *createEditor(BetterProcessorEditor *parentEditor)  override;

	virtual void renderNextBlock(AudioSampleBuffer &buffer, int startSample, int numSamples);;
	void prepareToPlay(double sampleRate, int samplesPerBlock);
	void applyEffect(AudioSampleBuffer &/*b*/, int /*startSample*/, int /*numSamples*/) override {};

private:

	float dry;
	float wet;
	float saturation;
	float preGain;
	float postGain;

	Saturator saturator;

	ScopedPointer<ModulatorChain> saturationChain;

	AudioSampleBuffer saturationBuffer;
};



#endif  // SATURATOR_H_INCLUDED