/*
  ==============================================================================

  sound_file_player.h -- interface for sound file player component
	- Initial code based on JUCE's audio player tutorial
	- Progress slider added by Erik Sargent

  ==============================================================================
*/
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class SoundFilePlayerComponent : public AudioAppComponent,
						         public ChangeListener,
								 public Timer,
								 public Slider::Listener
{
public:
	SoundFilePlayerComponent();
    ~SoundFilePlayerComponent();

	// Redefinitions of TransportSource methods
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) override;
	void releaseResources() override;

	// Callback functions for reader state change & timer
	void changeListenerCallback(ChangeBroadcaster *source) override;
	void timerCallback() override;

	// Callback functions for progress bar listeners
	void sliderDragEnded(Slider *) override;
	void sliderDragStarted(Slider *) override { }
	void sliderValueChanged(Slider *) override { }

    void resized() override;

private:
    // Transport state options:
    enum TransportState {
		Starting, Playing,
		Stopping, Stopped,
		Pausing, Paused
	};
    
	// Private helper functions
	void changeState(TransportState newState);
	void openButtonClicked();
	void playButtonClicked();
	void stopButtonClicked();
	void loopButtonChanged();
	void updateLoopState(const bool &loopFlag);

	// ===== PRIVATE MEMBER VARIABLES =====

	// Interface buttons
	TextButton openButton_;
	TextButton playButton_;
	TextButton stopButton_;
	ToggleButton loopToggleButton_;
	
	// Progress bar and progress value
	Slider progressBar_;
	double currentProgress_;

	// Managers, sources, & transport state
	AudioFormatManager formatManager_;
	std::unique_ptr<AudioFormatReaderSource> readerSource_;
	AudioTransportSource transportSource_;
	TransportState state_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundFilePlayerComponent)
};
