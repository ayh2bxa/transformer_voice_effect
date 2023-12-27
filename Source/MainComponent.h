#pragma once

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
#include <juce_dsp/juce_dsp.h>

//==============================================================================
#include "pitchshift.h"
#include "chorus.h"
#include "reverb.h"

class MainContentComponent : public juce::AudioAppComponent,
                             public juce::ChangeListener {
public:
    MainContentComponent();
    ~MainContentComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;
    void releaseResources() override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    void initializeWriter();

private:
    enum TransportState {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState(TransportState newState);
    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void shiftButtonClicked();

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton shiftButton;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    bool shiftOn;

    PhaseVocoder PV;
    struct SchroederReverb *Reverb = initReverb(0.8f, 1.0f, 0.4f, 0.8f);
    Chorus Chrs;
                                
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
