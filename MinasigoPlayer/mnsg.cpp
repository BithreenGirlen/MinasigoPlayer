

#include "mnsg.h"
#include "win_filesystem.h"
#include "win_text.h"

namespace mnsg
{
    void TextToLines(const std::wstring& wstrText, const wchar_t* wpzKey, size_t nKeyLen, std::vector<std::wstring>& lines)
    {
        if (wpzKey == nullptr)return;

        size_t nRead = 0;
        for (;;)
        {
            const wchar_t* p = wcsstr(&wstrText[nRead], wpzKey);
            if (p == nullptr)break;

            size_t nLen = p - &wstrText[nRead];
            lines.emplace_back(wstrText.substr(nRead, nLen));
            nRead += nLen + nKeyLen;
            if (nRead >= wstrText.size())break;
        }
    }

    std::wstring ExtractDirectory(const std::wstring& wstrFilePath)
    {
        size_t nPos = wstrFilePath.find_last_of(L"\\/");
        if (nPos != std::wstring::npos)
        {
            return wstrFilePath.substr(0, nPos);
        }
        return wstrFilePath;
    }
    std::wstring TruncateFilePath(const std::wstring& strRelativePath)
    {
        size_t nPos = strRelativePath.rfind(L'/');
        if (nPos != std::wstring::npos)
        {
            return strRelativePath.substr(nPos + 1);
        }
        return strRelativePath;
    }
}

bool mnsg::LoadScenario(const std::wstring &wstrFilePath, std::vector<adv::TextDatum>& textData)
{
    std::wstring wstrText = win_text::WidenUtf8(win_filesystem::LoadFileAsString(wstrFilePath.c_str()));

    /*大抵は\n\n区切りだが、\r\n\r\n区切りのもの、区切り無しの構成もあり*/
    constexpr wchar_t wszSeparator[] = L"clickwait";
    std::vector<std::wstring> lines;
    TextToLines(wstrText, wszSeparator, sizeof(wszSeparator) / sizeof(wchar_t) - 1, lines);

    constexpr wchar_t wszMsg0Key[] = L"msg,0,<";
    constexpr wchar_t wszMsg1Key[] = L"msg,1,<";
    constexpr wchar_t wszVoiceKey[] = L"playvoice,1,";
    constexpr wchar_t wszVoiceExtension[] = L".mp3";

    for (const std::wstring& line : lines)
    {
        size_t nRead = 0;
        adv::TextDatum t;
        const wchar_t* p = wcsstr(line.data(), wszMsg0Key);
        if (p == nullptr)
        {
            p = wcsstr(line.data(), wszMsg1Key);
        }

        if (p != nullptr)
        {
            /*併用*/
            nRead += p - &line[nRead] + sizeof(wszMsg0Key) / sizeof(wchar_t) - 1;
            p = wcschr(&line[nRead], '>');
            if (p != nullptr)
            {
                ++p;
                const wchar_t* pp = wcschr(p, '<');
                if (pp != nullptr)
                {
                    std::wstring wstr = std::wstring(p, pp - p);
                    for (;;)
                    {
                        size_t nPos = wstr.find(L"\\n");
                        if (nPos == std::wstring::npos)break;
                        wstr.replace(nPos, 2, L"\r\n");
                    }
                    t.wstrText = wstr;
                }
            }
        }

        nRead = 0;
        p = wcsstr(line.data(), wszVoiceKey);
        if (p != nullptr)
        {
            nRead += p - &line[nRead] + sizeof(wszVoiceKey) / sizeof(wchar_t) - 1;
            p = wcsstr(&line[nRead], L".mp3");
            if (p != nullptr)
            {
                p += sizeof(wszVoiceExtension) / sizeof(wchar_t) - 1;
                std::wstring wstrPath= line.substr(nRead, p - &line[nRead]);
                t.wstrVoicePath = mnsg::ExtractDirectory(wstrFilePath) + L"\\" + mnsg::TruncateFilePath(wstrPath);
            }
        }
        if (!t.wstrText.empty())
        {
            textData.emplace_back(t);
        }
    }

    return !textData.empty();
}
