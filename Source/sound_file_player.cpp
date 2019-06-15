/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "sound_file_player.h"
#include <iostream>

//==============================================================================
SoundFilePlayerComponent::SoundFilePlayerComponent()
{
	// State is initially "Stopped"
	state_ = Stopped;

	// Add open button, set text & onClick function
	addAndMakeVisible(&openButton_);
	openButton_.setButtonText("Open...");
	openButton_.onClick = [this] { openButtonClicked(); };

	// Add play button, set color, text, & onClick function, and then disable
	addAndMakeVisible(&playButton_);
	playButton_.setButtonText("Play");
	playButton_.onClick = [this] { playButtonClicked(); };
	playButton_.setColour(TextButton::buttonColourId, Colours::green);
	playButton_.setEnabled(false);

	// Add stop button, set color, text, & onClick function, and then disable
	addAndMakeVisible(&stopButton_);
	stopButton_.setButtonText("Stop");
	stopButton_.onClick = [this] { stopButtonClicked(); };
	stopButton_.setColour(TextButton::buttonColourId, Colours::red);
	stopButton_.setEnabled(false);
	
	// Add the loop toggle button, set text & onClick function
	addAndMakeVisible(&loopToggleButton_);
	loopToggleButton_.setButtonText("Loop");
	loopToggleButton_.onClick = [this] { loopButtonChanged(); };

	// Add the time label & specify the initial text
	addAndMakeVisible(&timeLabel_);
	timeLabel_.setText("Stopped", dontSendNotification);

	// Initialize & add progress bar and set initial progress value to 0.0
	currentProgress_ = 0;
	progressBar_.setValue(currentProgress_, dontSendNotification);
	progressBar_.setRange(0.0, 1.0);
	progressBar_.addListener(this);
	addAndMakeVisible(&progressBar_);

    setSize (300, 200);

	formatManager_.registerBasicFormats();
	transportSource_.addChangeListener(this);

	setAudioChannels(2, 2);
	startTimer(20);
}


SoundFilePlayerComponent::~SoundFilePlayerComponent()
{
	shutdownAudio();
}


void SoundFilePlayerComponent::changeState(TransportState newState) {

	if (state_ != newState) {
		state_ = newState;

		switch (state_) {
			case Stopped:
				playButton_.setButtonText("Play");
				stopButton_.setButtonText("Stop");
				stopButton_.setEnabled(false);
				transportSource_.setPosition(0.0);
				progressBar_.setValue(0.0);
				break;

			case Starting:
				transportSource_.start();
				break;

			case Playing:
				playButton_.setButtonText("Pause");
				stopButton_.setButtonText("Stop");
				stopButton_.setEnabled(true);
				break;

			case Pausing:
				transportSource_.stop();
				break;

			case Paused:
				playButton_.setButtonText("Resume");
				stopButton_.setButtonText("Return to beginning");
				break;

			case Stopping:
				transportSource_.stop();
				break;
		}
	}
}


void SoundFilePlayerComponent::updateLoopState(const bool &loopFlag) {
	if (readerSource_.get() != nullptr) {
		readerSource_->setLooping(loopFlag);
	}
}


/*
 * Function that gets called whenever the state of the transport changes
 */
void SoundFilePlayerComponent::changeListenerCallback(ChangeBroadcaster *source) {

	// If the source of the transport changes, either start or stop the player
	//   (depending on whether the transport is started or stopped)
	if (source == &transportSource_) {
		if (transportSource_.isPlaying())
			changeState(Playing);
		else if ((state_ == Stopping) || (state_ == Playing))
			changeState(Stopped);
		else
			changeState(Paused);
	}
}


/*
 * Processes the next audio block from the audio source file
 */
void SoundFilePlayerComponent::getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) {

	if (readerSource_.get() == nullptr) {
		bufferToFill.clearActiveBufferRegion();
		return;
	}

	transportSource_.getNextAudioBlock(bufferToFill);
}


/*
 * Function called periodically as a timer callback
 */
void SoundFilePlayerComponent::timerCallback() {

	if (transportSource_.isPlaying()) {
		RelativeTime pos(transportSource_.getCurrentPosition());

		int minutes = ((int) pos.inMinutes() % 60);
		int seconds = ((int) pos.inSeconds() % 60);
		int millis  = ((int) pos.inMilliseconds() % 60);

		// Get new progress value, only update its value if we aren't currently dragging it
		currentProgress_ = transportSource_.getCurrentPosition() / transportSource_.getLengthInSeconds();
		if (progressBar_.getThumbBeingDragged() < 0) {
			progressBar_.setValue(currentProgress_);
		}

		String pos_str = String::formatted("%02d:%02d:%03d -- %.1f%%", minutes, seconds, millis, currentProgress_ * 100);

		timeLabel_.setText(pos_str, dontSendNotification);
	}
	else {
		if (state_ == Stopping || state_ == Stopped)
			timeLabel_.setText("Stopped", dontSendNotification);
	}
}


/*
 * Updates the progress of the audio source when the slider is done being dragged
 */
void SoundFilePlayerComponent::sliderDragEnded(Slider *s) {
	transportSource_.setPosition(progressBar_.getValue() * transportSource_.getLengthInSeconds());
}


void SoundFilePlayerComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
	transportSource_.prepareToPlay(samplesPerBlockExpected, sampleRate);
}


void SoundFilePlayerComponent::releaseResources() {
	transportSource_.releaseResources();
}


void SoundFilePlayerComponent::openButtonClicked() {

	// Create a file chooser that only allows wav files
	FileChooser chooser("Select a .wav file to play...", {}, "*.wav");

	// Open up the file chooser, check if the user inputs a file
	if (chooser.browseForFileToOpen()) {

		// Grab the file, try to create a reader for it
		auto file = chooser.getResult();
		auto *reader = formatManager_.createReaderFor(file);

		if (reader != nullptr) {
			// Store a reader source object for the reader in a temporary unique_ptr
			std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
			transportSource_.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
			playButton_.setEnabled(true);

			// Now that the bookkeeping is done, set the readerSource to our new object
			readerSource_.reset(newSource.release());
		}
	}
}


void SoundFilePlayerComponent::playButtonClicked() {
	if ((state_ == Stopped) || (state_ == Paused))
		changeState(Starting);
	else if (state_ == Playing)
		changeState(Pausing);
}


void SoundFilePlayerComponent::stopButtonClicked() {
	if (state_ == Paused)
		changeState(Stopped);
	else
		changeState(Stopping);
}


void SoundFilePlayerComponent::loopButtonChanged() {
	updateLoopState(loopToggleButton_.getToggleState());
}


void SoundFilePlayerComponent::resized()
{
	openButton_.setBounds(10, 10, getWidth() - 20, 20);
	playButton_.setBounds(10, 40, getWidth() - 20, 20);
	stopButton_.setBounds(10, 70, getWidth() - 20, 20);
	loopToggleButton_.setBounds(10, 100, getWidth() - 20, 20);
	timeLabel_.setBounds(10, 130, getWidth() - 20, 20);
	progressBar_.setBounds(10, 160, getWidth() - 20, 20);
}


