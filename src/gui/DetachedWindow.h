#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>
#include <utility>

namespace noisefield::gui
{

/// A plain non-modal top-level window that owns a content component and calls `onClose` when
/// its close button is pressed (the owner uses that to drop its `unique_ptr`).
class DetachedWindow final : public juce::DocumentWindow
{
public:
    DetachedWindow(const juce::String& name,
                   std::unique_ptr<juce::Component> content,
                   std::function<void()> onClose)
        : juce::DocumentWindow(name,
                               juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(
                                   juce::ResizableWindow::backgroundColourId),
                               juce::DocumentWindow::closeButton),
          onClose_(std::move(onClose))
    {
        setUsingNativeTitleBar(true);
        setContentOwned(content.release(), true);
        setResizable(true, true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        if (onClose_)
            onClose_();
    }

private:
    std::function<void()> onClose_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetachedWindow)
};

} // namespace noisefield::gui
