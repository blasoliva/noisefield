#include "model/PresetJson.h"

#include <array>
#include <cctype>
#include <charconv>
#include <cstdio>
#include <map>
#include <optional>

namespace noisefield::model
{

namespace
{

// ---- writing ------------------------------------------------------------------------------

std::string escape(std::string_view s)
{
    std::string out;
    out.reserve(s.size() + 2);
    for (const char c : s)
    {
        switch (c)
        {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20)
            {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            }
            else
            {
                out += c;
            }
        }
    }
    return out;
}

std::string number(double v)
{
    // std::to_chars is locale-independent (always uses '.'), unlike snprintf's "%g" which
    // follows LC_NUMERIC — the reader below (std::from_chars) is locale-independent too, so
    // this keeps the pair symmetric regardless of the process locale (Main.cpp calls
    // std::setlocale(LC_ALL, "") for Xlib i18n, which would otherwise write e.g. "220,0").
    char buf[32];
    const auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v);
    return std::string(buf, ec == std::errc{} ? ptr : buf);
}

// ---- reading -----------------------------------------------------------------------------

struct Value
{
    enum class Type
    {
        String,
        Number,
        Bool,
        Null
    };
    Type type = Type::Null;
    std::string str;
    double num = 0.0;
    bool boolean = false;
};

class Parser
{
public:
    explicit Parser(std::string_view s) : s_(s) {}

    std::optional<std::map<std::string, Value>> parseObject()
    {
        std::map<std::string, Value> object;
        skipWs();
        if (!consume('{'))
            return std::nullopt;
        skipWs();
        if (consume('}'))
            return object;

        while (true)
        {
            skipWs();
            auto key = parseString();
            if (!key)
                return std::nullopt;
            skipWs();
            if (!consume(':'))
                return std::nullopt;
            skipWs();
            Value value;
            if (!parseValue(value))
                return std::nullopt;
            object[*key] = std::move(value);

            skipWs();
            if (consume(','))
                continue;
            if (consume('}'))
                return object;
            return std::nullopt;
        }
    }

private:
    void skipWs()
    {
        while (i_ < s_.size() && std::isspace(static_cast<unsigned char>(s_[i_])))
            ++i_;
    }

    bool consume(char c)
    {
        if (i_ < s_.size() && s_[i_] == c)
        {
            ++i_;
            return true;
        }
        return false;
    }

    std::optional<std::string> parseString()
    {
        if (!consume('"'))
            return std::nullopt;
        std::string out;
        while (i_ < s_.size())
        {
            const char c = s_[i_++];
            if (c == '"')
                return out;
            if (c == '\\')
            {
                if (i_ >= s_.size())
                    return std::nullopt;
                const char e = s_[i_++];
                switch (e)
                {
                case '"':
                    out += '"';
                    break;
                case '\\':
                    out += '\\';
                    break;
                case '/':
                    out += '/';
                    break;
                case 'n':
                    out += '\n';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 't':
                    out += '\t';
                    break;
                case 'b':
                    out += '\b';
                    break;
                case 'f':
                    out += '\f';
                    break;
                case 'u':
                {
                    if (i_ + 4 > s_.size())
                        return std::nullopt;
                    unsigned code = 0;
                    for (int k = 0; k < 4; ++k)
                    {
                        const char h = s_[i_++];
                        code <<= 4;
                        if (h >= '0' && h <= '9')
                            code |= static_cast<unsigned>(h - '0');
                        else if (h >= 'a' && h <= 'f')
                            code |= static_cast<unsigned>(h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F')
                            code |= static_cast<unsigned>(h - 'A' + 10);
                        else
                            return std::nullopt;
                    }
                    // Minimal: only the BMP ASCII/Latin range is needed for preset names.
                    if (code < 0x80)
                    {
                        out += static_cast<char>(code);
                    }
                    else
                    {
                        out += static_cast<char>(0xC0 | (code >> 6));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default:
                    return std::nullopt;
                }
            }
            else
            {
                out += c;
            }
        }
        return std::nullopt;
    }

    bool parseValue(Value& out)
    {
        if (i_ >= s_.size())
            return false;
        const char c = s_[i_];

        if (c == '"')
        {
            auto str = parseString();
            if (!str)
                return false;
            out.type = Value::Type::String;
            out.str = std::move(*str);
            return true;
        }
        if (c == 't' || c == 'f')
        {
            const std::string_view rest = s_.substr(i_);
            if (rest.starts_with("true"))
            {
                i_ += 4;
                out.type = Value::Type::Bool;
                out.boolean = true;
                return true;
            }
            if (rest.starts_with("false"))
            {
                i_ += 5;
                out.type = Value::Type::Bool;
                out.boolean = false;
                return true;
            }
            return false;
        }
        if (c == 'n')
        {
            if (s_.substr(i_).starts_with("null"))
            {
                i_ += 4;
                out.type = Value::Type::Null;
                return true;
            }
            return false;
        }
        if (c == '{' || c == '[')
            return skipContainer(); // forward-compat: ignore nested structures

        // number
        const size_t start = i_;
        if (i_ < s_.size() && (s_[i_] == '-' || s_[i_] == '+'))
            ++i_;
        while (i_ < s_.size() &&
               (std::isdigit(static_cast<unsigned char>(s_[i_])) || s_[i_] == '.' ||
                s_[i_] == 'e' || s_[i_] == 'E' || s_[i_] == '-' || s_[i_] == '+'))
            ++i_;
        if (i_ == start)
            return false;
        const std::string_view token = s_.substr(start, i_ - start);
        double value = 0.0;
        const auto [ptr, ec] = std::from_chars(token.data(), token.data() + token.size(), value);
        if (ec != std::errc{} || ptr != token.data() + token.size())
            return false;
        out.type = Value::Type::Number;
        out.num = value;
        return true;
    }

    bool skipContainer()
    {
        const char open = s_[i_];
        const char close = open == '{' ? '}' : ']';
        int depth = 0;
        bool inString = false;
        while (i_ < s_.size())
        {
            const char c = s_[i_++];
            if (inString)
            {
                if (c == '\\' && i_ < s_.size())
                    ++i_;
                else if (c == '"')
                    inString = false;
            }
            else if (c == '"')
                inString = true;
            else if (c == open)
                ++depth;
            else if (c == close && --depth == 0)
                return true;
        }
        return false;
    }

    std::string_view s_;
    size_t i_ = 0;
};

} // namespace

std::string toJson(const Preset& p)
{
    std::string out = "{\n";
    out += "  \"schemaVersion\": " + std::to_string(Preset::kSchemaVersion) + ",\n";
    out += "  \"name\": \"" + escape(p.name) + "\",\n";
    out += "  \"toneEnabled\": " + std::string(p.toneEnabled ? "true" : "false") + ",\n";
    out += "  \"toneFrequencyHz\": " + number(p.toneFrequencyHz) + ",\n";
    out += "  \"toneGainDb\": " + number(p.toneGainDb) + ",\n";
    out += "  \"noiseEnabled\": " + std::string(p.noiseEnabled ? "true" : "false") + ",\n";
    out += "  \"noiseColour\": \"" + std::string(dsp::noiseColourName(p.noiseColour)) + "\",\n";
    out += "  \"noiseGainDb\": " + number(p.noiseGainDb) + ",\n";
    out += "  \"noiseSeed\": " + std::to_string(p.noiseSeed) + ",\n";
    out += "  \"masterMute\": " + std::string(p.masterMute ? "true" : "false") + ",\n";
    out += "  \"masterGainDb\": " + number(p.masterGainDb) + ",\n";
    out += "  \"limiterEnabled\": " + std::string(p.limiterEnabled ? "true" : "false") + "\n";
    out += "}\n";
    return out;
}

bool fromJson(std::string_view json, Preset& out)
{
    Parser parser(json);
    const auto object = parser.parseObject();
    if (!object)
        return false;

    const auto& obj = *object;
    auto getNumber = [&](const char* key, double& target)
    {
        if (auto it = obj.find(key); it != obj.end() && it->second.type == Value::Type::Number)
            target = it->second.num;
    };
    auto getBool = [&](const char* key, bool& target)
    {
        if (auto it = obj.find(key); it != obj.end() && it->second.type == Value::Type::Bool)
            target = it->second.boolean;
    };

    if (auto it = obj.find("name"); it != obj.end() && it->second.type == Value::Type::String)
        out.name = it->second.str;

    getBool("toneEnabled", out.toneEnabled);
    getNumber("toneFrequencyHz", out.toneFrequencyHz);
    getNumber("toneGainDb", out.toneGainDb);

    getBool("noiseEnabled", out.noiseEnabled);
    if (auto it = obj.find("noiseColour");
        it != obj.end() && it->second.type == Value::Type::String)
        out.noiseColour = dsp::noiseColourFromName(it->second.str);
    getNumber("noiseGainDb", out.noiseGainDb);
    if (auto it = obj.find("noiseSeed"); it != obj.end() && it->second.type == Value::Type::Number)
        out.noiseSeed = static_cast<std::uint64_t>(it->second.num);

    getBool("masterMute", out.masterMute);
    getNumber("masterGainDb", out.masterGainDb);
    getBool("limiterEnabled", out.limiterEnabled);

    return true;
}

} // namespace noisefield::model
