#include "app/MainComponent.h"
#include "app/NoisefieldLookAndFeel.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace noisefield::app
{

class NoisefieldApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override
    {
        return JUCE_APPLICATION_NAME_STRING;
    }

    const juce::String getApplicationVersion() override
    {
        return JUCE_APPLICATION_VERSION_STRING;
    }

    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    void initialise(const juce::String&) override
    {
        juce::LookAndFeel::setDefaultLookAndFeel(&lookAndFeel_);
        mainWindow_ = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow_ = nullptr;
        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name)
            : juce::DocumentWindow(name,
                                   juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                                       juce::ResizableWindow::backgroundColourId),
                                   juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);
            setResizable(true, true);
            setResizeLimits(480, 320, 4000, 3000);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    NoisefieldLookAndFeel lookAndFeel_;
    std::unique_ptr<MainWindow> mainWindow_;
};

} // namespace noisefield::app

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables) -- JUCE entry point
START_JUCE_APPLICATION(noisefield::app::NoisefieldApplication)
