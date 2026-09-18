#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace noisefield::gui
{

/// Content of the detached "Guide" window: renders the bundled Markdown user guide
/// (docs/guide.md, embedded as binary data) as scrollable formatted text. Supports the small
/// Markdown subset the guide uses: `#`/`##`/`###` headings, `-` bullets, `**bold**`,
/// `` `code` ``, `---` rules and blank-line paragraphs.
///
/// Text size is adjustable with **Ctrl +** / **Ctrl -** (**Ctrl 0** resets it) since the guide
/// has no on-screen zoom control; `onZoomChanged` lets the owner persist the chosen level.
class GuideView final : public juce::Component
{
public:
    static constexpr float kMinZoom = 0.6f;
    static constexpr float kMaxZoom = 2.5f;

    explicit GuideView(float initialZoom = 1.0f);

    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void visibilityChanged() override;

    std::function<void(float)> onZoomChanged;

private:
    class Page final : public juce::Component
    {
    public:
        Page(juce::String markdown, float initialZoom);
        void layoutForWidth(int width);
        void setZoom(float zoom);
        void paint(juce::Graphics& g) override;

    private:
        void build(float textWidth);

        juce::String source_;
        juce::TextLayout layout_;
        float margin_ = 22.0f;
        float zoom_;
    };

    void applyZoom(float zoom);

    juce::Viewport viewport_;
    Page page_;
    float zoom_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuideView)
};

} // namespace noisefield::gui
