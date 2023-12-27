#include "MainComponent.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <iostream>
#include <cassert>

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>

MainContentComponent::MainContentComponent()
    : state(Stopped), shiftOn(false), PV(1024, 16384, 0.5f, 0.667f, 2), Chrs(44100.0) {
    addAndMakeVisible (&openButton);
    openButton.setButtonText ("Open...");
    openButton.onClick = [this] { openButtonClicked(); };

    addAndMakeVisible (&playButton);
    playButton.setButtonText ("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled (false);

    addAndMakeVisible (&stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled (false);

    addAndMakeVisible (&shiftButton);
    shiftButton.setButtonText ("Effect On/Off");
    shiftButton.onClick = [this] { shiftButtonClicked(); };
    shiftButton.setColour (juce::TextButton::buttonColourId, juce::Colours::blue);
    shiftButton.setEnabled (false);
    
    setSize (300, 200);
    
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);
    setAudioChannels(0,2);
}

MainContentComponent::~MainContentComponent() {
    freeReverb(Reverb);
    shutdownAudio();
}

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void MainContentComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    transportSource.getNextAudioBlock (bufferToFill);
    if (shiftOn) {
        /* main processing loop */
        for (int channel = 0; channel < 2; channel++) {
            float *channelData = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
            PV.applyPV(channelData, bufferToFill.numSamples, channel);
            Chrs.applyChorus(channelData, channel, bufferToFill.numSamples, 0.03, 1.0, 441000);
        }
        float *channelDataL = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
        float *channelDataR = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
        applyReverb(Reverb, channelDataL, channelDataR, bufferToFill.numSamples);
    }
}

void MainContentComponent::releaseResources() {
    transportSource.releaseResources();
}

void MainContentComponent::resized() {
    openButton.setBounds (10, 10, getWidth() - 20, 20);
    playButton.setBounds (10, 40, getWidth() - 20, 20);
    stopButton.setBounds (10, 70, getWidth() - 20, 20);
    shiftButton.setBounds (10, 100, getWidth() - 20, 20);
}

void MainContentComponent::changeListenerCallback(juce::ChangeBroadcaster *source) {
    if (source == &transportSource)
    {
        if (transportSource.isPlaying())
            changeState (Playing);
        else
            changeState (Stopped);
    }
}

void MainContentComponent::changeState(MainContentComponent::TransportState newState) {
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
            case Stopped:                           // [3]
                stopButton.setEnabled (false);
                playButton.setEnabled (true);
                transportSource.setPosition (0.0);
                break;

            case Starting:                          // [4]
                playButton.setEnabled (false);
                transportSource.start();
                break;

            case Playing:                           // [5]
                stopButton.setEnabled (true);
                shiftButton.setEnabled(true);
                break;

            case Stopping:                          // [6]
                transportSource.stop();
                shiftButton.setEnabled(false);
                break;
        }
    }
}

void MainContentComponent::openButtonClicked() {
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                   juce::File{},
                                                   "*.wav");                     // [7]
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)     // [8]
    {
        auto file = fc.getResult();

        if (file != juce::File{})                                                // [9]
        {
            auto* reader = formatManager.createReaderFor (file);                 // [10]

            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);   // [11]
                transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
                playButton.setEnabled (true);                                                      // [13]
                readerSource.reset (newSource.release());                                          // [14]
            }
        }
    });
}

void MainContentComponent::playButtonClicked() {
    changeState (Starting);
}

void MainContentComponent::stopButtonClicked() {
    changeState (Stopping);
}

void MainContentComponent::shiftButtonClicked() {
    shiftOn = !shiftOn;
}
