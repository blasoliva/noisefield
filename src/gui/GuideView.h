#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace noisefield::gui
{

/// Content of the detached "Guide" window: renders the bundled Markdown user guide
/// (docs/guide.md, embedded as binary data) as scrollable formatted text. Supports the small
/// Markdown subset the guide uses: `#`/`##`/`###` headings, `-` bullets, `**bold**`,
/// `` `code` ``, `---` rules and blank-line paragraphs.
class GuideView final : public juce::Component
{
public:
    GuideView();

    void resized() override;

private:
    class Page final : public juce::Component
    {
    public:
        explicit Page(juce::String markdown);
        void layoutForWidth(int width);
        void paint(juce::Graphics& g) override;

    private:
        void build(float textWidth);

        juce::String source_;
        juce::TextLayout layout_;
        float margin_ = 22.0f;
    };

    juce::Viewport viewport_;
    Page page_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuideView)
};

} // namespace noisefield::gui
