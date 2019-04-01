#ifndef UOIDX_H
#define UOIDX_H

#include <string>
#include <fstream>
#include <memory>


namespace uocf
{


struct UOIdx
{
    struct Entry
    {
        static constexpr unsigned int kInvalid = 0xFFFFFFFF;
        static constexpr unsigned int kSize = 4 + 4 + 4;

        unsigned int lookup;
        unsigned int size;
        unsigned int extra;
    };

    UOIdx(const std::string& filePath);

    void openStream();
    void closeStream();
    inline bool isStreamOpened() const {
        return m_stream.is_open();
    }

    inline bool hasCache() const noexcept {
        return bool(m_cache);
    }
    void clearCache();
    void cacheData();

    bool getLookup(unsigned int id, Entry *idxEntry);

    static bool getLookup(const std::string& filePath, unsigned int id, Entry *idxEntry);

private:
    std::string m_filePath;
    std::ifstream m_stream;

    unsigned int m_cachedCount;
    std::unique_ptr<Entry[]> m_cache;
};


}

#endif // UOIDX_H
