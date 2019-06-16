/*
  ==============================================================================

  sound_file_player.cpp -- implementations of sound file player component
	- Initial code based on JUCE's audio player tutorial 
	- Progress slider added by Erik Sargent

  ==============================================================================
*/

#include "sound_file_player.h"

//==============================================================================

// Constructor
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

	// Initialize & add progress bar and set initial progress value to 0.0
	currentProgress_ = 0;
	progressBar_.setValue(currentProgress_, dontSendNotification);
	progressBar_.setRange(0.0, 1.0);
	progressBar_.addListener(this);
	addAndMakeVisible(&progressBar_);
	progressBar_.setEnabled(false);

    setSize (300, 200);

	formatManager_.registerBasicFormats();
	transportSource_.addChangeListener(this);

	setAudioChannels(2, 2);
	startTimer(20);
}


// Destructor
SoundFilePlayerComponent::~SoundFilePlayerComponent()
{
	shutdownAudio();
}


/*
 * Changes the player's state to newState
 */
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


/* 
 * Updates the player's loop setting based on the provided bool flag
 */
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
	}
}


/*
 * Updates the progress of the audio source when the slider is done being dragged
 */
void SoundFilePlayerComponent::sliderDragEnded(Slider *) {
	transportSource_.setPosition(progressBar_.getValue() * transportSource_.getLengthInSeconds());
}


/*
 * Prepares the audio transport source to play bassed on the expected # of samples per block and
 *   sampling rate
 */
void SoundFilePlayerComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
	transportSource_.prepareToPlay(samplesPerBlockExpected, sampleRate);
}


/*
 * Releases the transport source's resources
 */
void SoundFilePlayerComponent::releaseResources() {
	transportSource_.releaseResources();
}


/*
 * Callback run when the player's Open button is clicked
 */
void SoundFilePlayerComponent::openButtonClicked() {

	// Pause player if not already paused or stopped (loading a new file while the
	//   transport source is still playing leads to weird behavior)
	if (transportSource_.isPlaying()) {
		changeState(Pausing);
	}

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

			// Update UI now that we have a file loaded
			playButton_.setEnabled(true);
			loopToggleButton_.setToggleState(false, dontSendNotification);
			progressBar_.setValue(0.0);
			progressBar_.setEnabled(true);

			// Now that the bookkeeping is done, set the readerSource to our new object
			readerSource_.reset(newSource.release());
		}
	}
}


/*
 * Callback run when the player's Play button is clicked
 */
void SoundFilePlayerComponent::playButtonClicked() {
	if ((state_ == Stopped) || (state_ == Paused))
		changeState(Starting);
	else if (state_ == Playing)
		changeState(Pausing);
}


/*
 * Callback run when the player's Stop button is clicked
 */
void SoundFilePlayerComponent::stopButtonClicked() {
	if (state_ == Paused)
		changeState(Stopped);
	else
		changeState(Stopping);
}


/*
 * Callback run when the player's Loop checkbox is toggled
 */
void SoundFilePlayerComponent::loopButtonChanged() {
	updateLoopState(loopToggleButton_.getToggleState());
}


/* 
 * Callback run when the player's window is resized
 */
void SoundFilePlayerComponent::resized()
{
	openButton_.setBounds(10, 10, getWidth() - 20, 20);
	playButton_.setBounds(10, 40, getWidth() - 20, 20);
	stopButton_.setBounds(10, 70, getWidth() - 20, 20);
	loopToggleButton_.setBounds((getWidth() / 2) - 35, 130, 70, 20);
	progressBar_.setBounds(10, 100, getWidth() - 20, 20);
}


