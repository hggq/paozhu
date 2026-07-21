#ifndef TTFTFONT_PARSER_H
#define TTFTFONT_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <stdexcept>
#include <fstream>

namespace pz {

class TTFParserException : public std::runtime_error {
public:
    explicit TTFParserException(const std::string& msg) : std::runtime_error("TTFParser error: " + msg) {}
};

struct TTFGlyph {
    uint16_t advanceWidth;
    int16_t lsb;
};

class TTFParser {
public:
    TTFParser(const std::string& file);
    ~TTFParser();

    void Parse();
    void Subset(const std::vector<uint32_t>& chars);
    std::string Build();

    std::map<uint32_t, uint16_t> chars;
    std::vector<TTFGlyph> glyphs;
    uint16_t unitsPerEm;
    int16_t xMin, yMin, xMax, yMax;
    std::string postScriptName;
    bool embeddable;
    bool bold;
    bool is_otf;
    int16_t typoAscender;
    int16_t typoDescender;
    int16_t capHeight;
    int16_t italicAngle;
    int16_t underlinePosition;
    int16_t underlineThickness;
    bool isFixedPitch;
    uint16_t numGlyphs;
    uint16_t numberOfHMetrics;
    int16_t hheaAscender;
    int16_t hheaDescender;
    uint16_t usWeightClass;
    std::map<uint16_t, uint16_t> glyphMap;
    std::vector<uint32_t> subsettedGlyphs;
    // For the OTF/CFF subset path: maps each retained Unicode code point to its
    // new (compact) GID, which also equals its CID in the emitted CID-keyed CFF.
    // The caller uses this to build the /W array (keyed by CID == Unicode).
    std::map<uint32_t, uint16_t> cffUniToGid;
    // new GID -> original GID, used to rebuild hmtx for the sfnt wrapper.
    std::vector<uint16_t> cffNewToOldGid;

private:
    std::ifstream f;
    std::map<std::string, std::pair<uint32_t, uint32_t>> tables;
    std::string built_font;

    void ParseOffsetTable();
    std::string _get_table(const std::string& tag);
    // Build a subsetted, CID-keyed bare CFF from the font's "CFF " table.
    // chars_vec holds the Unicode code points to keep. Glyphs are packed into
    // dense new GIDs and the charset maps each new GID to CID == Unicode.
    // cffUniToGid (Unicode -> new GID) and cffNewToOldGid (new GID -> original
    // GID) are populated as a side effect. Returns "" on failure.
    std::string _subset_cff(const std::string& cff, const std::vector<uint32_t>& chars_vec);
    // Wrap a subsetted CFF in an sfnt/OpenType container (tag 'OTTO') built from
    // the original font's head/hhea/maxp/OS/2/name tables plus a fresh hmtx,
    // cmap and post. Apple CoreGraphics/Preview only renders CID-keyed CFF fonts
    // that are embedded this way (not as a bare CFF). Returns "" on failure.
    std::string _build_otf_sfnt(const std::string& subset_cff);
    // Assemble an sfnt from the given tables (with checksums and the head
    // checkSumAdjustment). Only tags present in both tbls and tag_order are used.
    std::string _assemble_sfnt(uint32_t sfntVersion, std::map<std::string, std::string>& tbls,
                               const std::vector<std::string>& tag_order);
    // Build a cmap table (format 4, platform 3 encoding 1) from Unicode -> GID.
    std::string _build_cmap4(const std::map<uint32_t, uint16_t>& uni_to_gid);
    void _set_ushort(std::string& data, size_t pos, uint16_t val);
    uint16_t _get_ushort(const std::string& data, size_t pos);
    uint32_t _get_ulong(const std::string& data, size_t pos);

    void Seek(const std::string& tag);
    uint16_t ReadUShort();
    int16_t ReadShort();
    uint32_t ReadULong();
    int32_t ReadLong();
    std::string Read(int n);
    void Skip(int n);
};

}

#endif
