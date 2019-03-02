#ifndef UOIDX_H
#define UOIDX_H

#include <string>
#include <fstream>
#include <vector>


namespace uocf
{


struct UOIdx
{
    struct Entry
    {
        static const unsigned int kInvalid = 0xFFFFFFFF;
        static const int kSize = 4 + 4 + 4;

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

    bool hasCache();
    void clearCache();
    void cacheData();

    bool getLookup(unsigned int id, Entry *idxEntry);

    static bool getLookup(const std::string& filePath, unsigned int id, Entry *idxEntry);

private:
    std::string m_filePath;
    std::ifstream m_stream;
    std::vector<Entry> m_cache;
};


}

#endif // UOIDX_H
