#ifndef LANG_IMAGE_DEFD
#define LANG_IMAGE_DEFD

#include <ostream>
#include <string>
#include <vector>

//##############################################################################
// CLangImage
//##############################################################################
//! Creates a binary image, as shown in the following table, of the translations
//! for a given language, supporting fast look-up given a string ID.
//##############################################################################

class CLangImage {
public:
    CLangImage();
    CLangImage(const CLangImage &other);
    CLangImage(CLangImage &&other);
    ~CLangImage();

    void Fill(const std::string &language,
        const std::vector<std::wstring> &translations,
        uint8_t langOrder, uint8_t charSet = 0);
    const uint8_t *Image() const { return mpImage; }
    const void SaveAsCode(const std::vector<std::string> &enums,
        std::ostream &stream, const char *indent) const;
    uint32_t Size() const { return mTotalSize; }
    void Show() const;
private:
    std::string mLanguage;
    uint8_t *mpImage;
    uint8_t *mpWrite;
    uint32_t mTotalSize;

    void Write(uint32_t value, uint32_t size);
    static void WriteLine(const uint8_t **ppimage, uint32_t maxBytes, uint32_t bytes,
        const std::string &comment, std::ostream &file, const char *indent);
};

#endif // LANG_IMAGE_DEFD
