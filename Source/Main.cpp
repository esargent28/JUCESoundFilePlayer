/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/
#define MIN_WIDTH 300
#define MIN_HEIGHT 250
#define MAX_WIDTH 10000
#define MAX_HEIGHT 10000

#include "../JuceLibraryCode/JuceHeader.h"
#include "sound_file_player.h"
//==============================================================================
class SoundFilePlayerApplication : public JUCEApplication
{
public:
	//==============================================================================
	SoundFilePlayerApplication() {}

	const String getApplicationName() override { return "Sound File Player"; }
	const String getApplicationVersion() override { return "1.0.0"; }

	void initialise(const String&) override {
		mainWindow.reset(new MainWindow("Sound File Player", new SoundFilePlayerComponent(), *this));
	}

	void shutdown() override {
		mainWindow = nullptr;
	}

private:
	class MainWindow : public DocumentWindow {
	public:
		MainWindow(const String &name, Component *c, JUCEApplication &a)
			: DocumentWindow(name, Desktop::getInstance().getDefaultLookAndFeel()
				.findColour(ResizableWindow::backgroundColourId),
				DocumentWindow::allButtons), app(a) {

			setUsingNativeTitleBar(true);
			setContentOwned(c, true);

		#if JUCE_ANDROID || JUCE_IOS
			setFullScreen(true);
		#else
			setResizable(true, false);
			setResizeLimits(MIN_WIDTH, MIN_HEIGHT, MAX_WIDTH, MAX_HEIGHT);
			centreWithSize(getWidth(), getHeight());
		#endif
			setVisible(true);
		}

		void closeButtonPressed() override {
			app.systemRequestedQuit();
		}

	private:
		JUCEApplication &app;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
	};

	std::unique_ptr<MainWindow> mainWindow;
};
//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (SoundFilePlayerApplication)
