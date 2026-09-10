#include "gui/GuideView.h"

#include <BinaryData.h>

#include <cmath>

namespace noisefield::gui
{

namespace
{
const juce::Colour kBackground{0xff16181c};
const juce::Colour kBody{0xffdfe3e8};
const juce::Colour kHeading{0xffffffff};
const juce::Colour kCode{0xff8bd5ff};

juce::Font bodyFont(float size, bool bold)
{
    return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Font codeFont(float size)
{
    return juce::Font(
        juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain));
}

/// Appends one line of text, interpreting `**bold**` and `` `code` `` spans.
void appendInline(juce::AttributedString& out,
                  const juce::String& text,
                  float size,
                  juce::Colour colour,
                  bool boldByDefault)
{
    juce::String run;
    bool bold = boldByDefault;
    bool code = false;

    auto flush = [&]
    {
        if (run.isNotEmpty())
        {
            out.append(run, code ? codeFont(size) : bodyFont(size, bold), code ? kCode : colour);
            run.clear();
        }
    };

    for (int i = 0; i < text.length();)
    {
        if (text[i] == '*' && i + 1 < text.length() && text[i + 1] == '*')
        {
            flush();
            bold = !bold;
            i += 2;
        }
        else if (text[i] == '`')
        {
            flush();
            code = !code;
            i += 1;
        }
        else
        {
            run += text[i];
            i += 1;
        }
    }
    flush();
}
} // namespace

GuideView::Page::Page(juce::String markdown) : source_(std::move(markdown)) {}

void GuideView::Page::build(float textWidth)
{
    juce::AttributedString s;
    s.setWordWrap(juce::AttributedString::byWord);
    s.setLineSpacing(2.5f);

    const auto lines = juce::StringArray::fromLines(source_);
    bool first = true;

    for (const auto& raw : lines)
    {
        const auto line = raw.trim();

        if (line.isEmpty())
        {
            s.append("\n", bodyFont(7.0f, false), kBody);
            continue;
        }

        auto heading = [&](int prefixLength, float size)
        {
            if (!first)
                s.append("\n", bodyFont(8.0f, false), kBody);
            appendInline(s, line.substring(prefixLength), size, kHeading, true);
            s.append("\n", bodyFont(size, true), kHeading);
        };

        if (line.startsWith("### "))
            heading(4, 15.0f);
        else if (line.startsWith("## "))
            heading(3, 17.0f);
        else if (line.startsWith("# "))
            heading(2, 21.0f);
        else if (line == "---" || line == "***")
            s.append("\n", bodyFont(8.0f, false), kBody);
        else if (line.startsWith("- ") || line.startsWith("* "))
        {
            s.append(juce::String::fromUTF8("  \xe2\x80\xa2  "), bodyFont(13.0f, false), kBody);
            appendInline(s, line.substring(2), 13.0f, kBody, false);
            s.append("\n", bodyFont(13.0f, false), kBody);
        }
        else
        {
            appendInline(s, line, 13.0f, kBody, false);
            s.append("\n", bodyFont(13.0f, false), kBody);
        }

        first = false;
    }

    layout_.createLayout(s, textWidth);
}

void GuideView::Page::layoutForWidth(int width)
{
    const int usableWidth = juce::jmax(120, width);
    build(static_cast<float>(usableWidth) - 2.0f * margin_);
    setSize(usableWidth, static_cast<int>(std::ceil(layout_.getHeight() + 2.0f * margin_)));
}

void GuideView::Page::paint(juce::Graphics& g)
{
    g.fillAll(kBackground);
    layout_.draw(g,
                 juce::Rectangle<float>(margin_,
                                        margin_,
                                        static_cast<float>(getWidth()) - 2.0f * margin_,
                                        static_cast<float>(getHeight()) - 2.0f * margin_));
}

GuideView::GuideView()
    : page_(juce::String::fromUTF8(BinaryData::guide_md, BinaryData::guide_mdSize))
{
    viewport_.setViewedComponent(&page_, false);
    viewport_.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport_);
    setSize(560, 680);
}

void GuideView::resized()
{
    viewport_.setBounds(getLocalBounds());
    page_.layoutForWidth(viewport_.getMaximumVisibleWidth());
}

} // namespace noisefield::gui
