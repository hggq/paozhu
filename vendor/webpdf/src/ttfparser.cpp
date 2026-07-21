/*******************************************************************************
* Class to parse and subset TrueType fonts                                     *
*                                                                              *
* Version: 1.11                                                                *
* Date:    2021-04-18                                                          *
* Author:  Olivier PLATHEY                                                     *
* Edit: Paozhu web framework transformation ttfparser.php                      *
*******************************************************************************/
#include "ttfparser.h"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <set>

namespace pz {

namespace {
// ---- CFF low-level primitives ---------------------------------------------
// A parsed DICT entry: operator id (two-byte ops are stored as 1200 + b1),
// decoded operands, plus the raw byte span (operands + operator) so entries
// can be re-emitted verbatim when we don't want to touch their encoding.
struct CFFDictEntry {
    int op;
    std::vector<double> ops;
    std::string raw;
};

uint16_t cff_u16(const std::string& d, size_t p) {
    if (p + 1 >= d.size()) return 0;
    return ((uint8_t)d[p] << 8) | (uint8_t)d[p + 1];
}

// Parse a CFF DICT (Top DICT / Private DICT / Font DICT).
void cff_parse_dict(const std::string& d, std::vector<CFFDictEntry>& out) {
    size_t i = 0, ops_start = 0;
    bool started = false;
    std::vector<double> operands;
    while (i < d.size()) {
        uint8_t b0 = (uint8_t)d[i];
        if (!started) { ops_start = i; started = true; }
        if (b0 <= 21) {
            int op = b0;
            size_t op_end;
            if (b0 == 12) { op = 1200 + (i + 1 < d.size() ? (uint8_t)d[i + 1] : 0); op_end = i + 2; }
            else op_end = i + 1;
            CFFDictEntry e;
            e.op = op;
            e.ops = operands;
            e.raw = d.substr(ops_start, op_end - ops_start);
            out.push_back(e);
            operands.clear();
            started = false;
            i = op_end;
        } else if (b0 == 28) {
            int16_t v = (int16_t)(((uint8_t)d[i + 1] << 8) | (uint8_t)d[i + 2]);
            operands.push_back(v); i += 3;
        } else if (b0 == 29) {
            int32_t v = ((uint8_t)d[i + 1] << 24) | ((uint8_t)d[i + 2] << 16) |
                        ((uint8_t)d[i + 3] << 8) | (uint8_t)d[i + 4];
            operands.push_back(v); i += 5;
        } else if (b0 == 30) {
            std::string num;
            size_t j = i + 1;
            bool done = false;
            while (j < d.size() && !done) {
                uint8_t byte = (uint8_t)d[j++];
                for (int k = 0; k < 2 && !done; k++) {
                    uint8_t nib = (k == 0) ? (byte >> 4) : (byte & 0xF);
                    if (nib <= 9) num += (char)('0' + nib);
                    else if (nib == 0xa) num += '.';
                    else if (nib == 0xb) num += 'E';
                    else if (nib == 0xc) num += "E-";
                    else if (nib == 0xe) num += '-';
                    else if (nib == 0xf) done = true;
                }
            }
            operands.push_back(atof(num.c_str()));
            i = j;
        } else if (b0 >= 32 && b0 <= 246) {
            operands.push_back((int)b0 - 139); i += 1;
        } else if (b0 >= 247 && b0 <= 250) {
            operands.push_back((b0 - 247) * 256 + (uint8_t)d[i + 1] + 108); i += 2;
        } else if (b0 >= 251 && b0 <= 254) {
            operands.push_back(-((int)(b0 - 251)) * 256 - (uint8_t)d[i + 1] - 108); i += 2;
        } else {
            i += 1;
        }
    }
}

const CFFDictEntry* cff_find(const std::vector<CFFDictEntry>& d, int op) {
    for (auto& e : d) if (e.op == op) return &e;
    return nullptr;
}

// Read a CFF INDEX at pos. Fills out with the objects, sets end_pos to the byte
// right after the INDEX. Returns false on malformed input.
bool cff_read_index(const std::string& d, size_t pos, std::vector<std::string>& out, size_t& end_pos) {
    out.clear();
    if (pos + 2 > d.size()) { end_pos = pos; return false; }
    uint16_t count = ((uint8_t)d[pos] << 8) | (uint8_t)d[pos + 1];
    if (count == 0) { end_pos = pos + 2; return true; }
    if (pos + 3 > d.size()) { end_pos = pos; return false; }
    uint8_t off_size = (uint8_t)d[pos + 2];
    if (off_size < 1 || off_size > 4) { end_pos = pos; return false; }
    size_t base = pos + 3;
    std::vector<uint32_t> offs(count + 1);
    for (int i = 0; i <= count; i++) {
        uint32_t v = 0;
        for (int b = 0; b < off_size; b++) {
            size_t q = base + (size_t)i * off_size + b;
            v = (v << 8) | (q < d.size() ? (uint8_t)d[q] : 0);
        }
        offs[i] = v;
    }
    size_t data_base = base + (size_t)(count + 1) * off_size - 1;
    for (int i = 0; i < count; i++) {
        if (offs[i + 1] < offs[i]) { end_pos = pos; return false; }
        size_t s = data_base + offs[i];
        size_t len = offs[i + 1] - offs[i];
        if (s + len > d.size()) { end_pos = pos; return false; }
        out.push_back(d.substr(s, len));
    }
    end_pos = data_base + offs[count];
    return true;
}

// Serialize a set of objects as a CFF INDEX.
std::string cff_write_index(const std::vector<std::string>& items) {
    std::string s;
    uint16_t count = (uint16_t)items.size();
    s += (char)((count >> 8) & 0xFF);
    s += (char)(count & 0xFF);
    if (count == 0) return s;
    size_t data_size = 0;
    for (auto& it : items) data_size += it.size();
    uint32_t max_off = (uint32_t)data_size + 1;
    int off_size = (max_off < 0x100) ? 1 : (max_off < 0x10000) ? 2 : (max_off < 0x1000000) ? 3 : 4;
    s += (char)off_size;
    auto put_off = [&](uint32_t v) {
        for (int b = off_size - 1; b >= 0; b--) s += (char)((v >> (8 * b)) & 0xFF);
    };
    uint32_t off = 1;
    put_off(off);
    for (auto& it : items) { off += (uint32_t)it.size(); put_off(off); }
    for (auto& it : items) s += it;
    return s;
}

// Encode an integer as a DICT operand (minimal form).
std::string cff_enc_int(int v) {
    std::string s;
    if (v >= -107 && v <= 107) {
        s += (char)(v + 139);
    } else if (v >= 108 && v <= 1131) {
        v -= 108; s += (char)((v >> 8) + 247); s += (char)(v & 0xFF);
    } else if (v >= -1131 && v <= -108) {
        v = -v - 108; s += (char)((v >> 8) + 251); s += (char)(v & 0xFF);
    } else if (v >= -32768 && v <= 32767) {
        s += (char)28; s += (char)((v >> 8) & 0xFF); s += (char)(v & 0xFF);
    } else {
        s += (char)29; s += (char)((v >> 24) & 0xFF); s += (char)((v >> 16) & 0xFF);
        s += (char)((v >> 8) & 0xFF); s += (char)(v & 0xFF);
    }
    return s;
}

// Encode an offset as a fixed-width (5-byte, int32) DICT operand so that the
// enclosing DICT keeps a constant length regardless of the actual value.
std::string cff_enc_offset(int v) {
    std::string s;
    s += (char)29;
    s += (char)((v >> 24) & 0xFF); s += (char)((v >> 16) & 0xFF);
    s += (char)((v >> 8) & 0xFF); s += (char)(v & 0xFF);
    return s;
}

std::string cff_enc_op(int op) {
    std::string s;
    if (op >= 1200) { s += (char)12; s += (char)(op - 1200); }
    else s += (char)op;
    return s;
}

// ---- Type2 charstring subr inliner ----------------------------------------
// Decode one Type2 charstring operand at p, returning its byte length and value.
size_t cs_operand_len(const std::string& cs, size_t p, double& val) {
    uint8_t b = (uint8_t)cs[p];
    if (b == 28) {
        int16_t v = (int16_t)(((uint8_t)cs[p + 1] << 8) | (uint8_t)cs[p + 2]);
        val = v; return 3;
    } else if (b >= 32 && b <= 246) {
        val = (int)b - 139; return 1;
    } else if (b >= 247 && b <= 250) {
        val = (b - 247) * 256 + (uint8_t)cs[p + 1] + 108; return 2;
    } else if (b >= 251 && b <= 254) {
        val = -((int)(b - 251)) * 256 - (uint8_t)cs[p + 1] - 108; return 2;
    } else if (b == 255) {
        int32_t v = ((uint8_t)cs[p + 1] << 24) | ((uint8_t)cs[p + 2] << 16) |
                    ((uint8_t)cs[p + 3] << 8) | (uint8_t)cs[p + 4];
        val = v / 65536.0; return 5;
    }
    val = 0; return 1;
}

// Recursively inline callsubr/callgsubr references so the resulting charstring
// contains no subr calls (letting us drop the Global/Local Subr INDEXes).
// The operand stack is shared across subr invocations, mirroring the real Type2
// interpreter, so stem counting stays correct for hintmask/cntrmask sizing.
struct CFFInliner {
    const std::vector<std::string>& gsubrs;
    int gbias;
    const std::vector<std::string>& lsubrs;
    int lbias;
    std::string out;
    std::string pending;          // buffered operand bytes not yet flushed
    std::vector<size_t> arg_len;  // byte length of each pending operand
    int numStems = 0;
    bool ok = true;

    // A constructor (rather than aggregate init) so callers only supply the four
    // reference/bias members; the rest keep their default member initializers
    // and no -Wmissing-field-initializers warning is produced.
    CFFInliner(const std::vector<std::string>& gs, int gb,
               const std::vector<std::string>& ls, int lb)
        : gsubrs(gs), gbias(gb), lsubrs(ls), lbias(lb) {}

    void flush() { out += pending; pending.clear(); arg_len.clear(); }

    void run(const std::string& cs, int depth) {
        if (!ok) return;
        if (depth > 60) { ok = false; return; }
        size_t i = 0, n = cs.size();
        while (i < n) {
            uint8_t b = (uint8_t)cs[i];
            if (b >= 32 || b == 28) {
                double val; size_t len = cs_operand_len(cs, i, val);
                if (i + len > n) { ok = false; return; }
                pending += cs.substr(i, len);
                arg_len.push_back(len);
                i += len;
            } else if (b == 10 || b == 29) { // callsubr / callgsubr
                if (arg_len.empty()) { ok = false; return; }
                size_t llen = arg_len.back();
                double idxval;
                cs_operand_len(pending, pending.size() - llen, idxval);
                pending.erase(pending.size() - llen);
                arg_len.pop_back();
                int idx = (int)idxval + (b == 10 ? lbias : gbias);
                const std::vector<std::string>& subrs = (b == 10) ? lsubrs : gsubrs;
                if (idx < 0 || idx >= (int)subrs.size()) { ok = false; return; }
                run(subrs[idx], depth + 1);
                if (!ok) return;
                i += 1;
            } else if (b == 11) { // return
                return;
            } else if (b == 14) { // endchar
                flush(); out += (char)14; i += 1;
            } else if (b == 19 || b == 20) { // hintmask / cntrmask
                numStems += (int)arg_len.size() / 2;
                flush();
                out += (char)b;
                size_t mask = (numStems + 7) / 8;
                if (i + 1 + mask > n) { ok = false; return; }
                out += cs.substr(i + 1, mask);
                i += 1 + mask;
            } else if (b == 1 || b == 3 || b == 18 || b == 23) { // stem hints
                numStems += (int)arg_len.size() / 2;
                flush();
                out += (char)b;
                i += 1;
            } else if (b == 12) { // escape (2-byte operator)
                flush();
                if (i + 1 >= n) { ok = false; return; }
                out += cs.substr(i, 2);
                i += 2;
            } else { // other single-byte operators (moveto/lineto/curveto/...)
                flush();
                out += (char)b;
                i += 1;
            }
        }
    }
};
} // namespace

TTFParser::TTFParser(const std::string& file) {
    f.open(file, std::ios::binary);
    if (!f.is_open())
        throw TTFParserException("Can't open file: " + file);
}

TTFParser::~TTFParser() {
    if (f.is_open())
        f.close();
}

void TTFParser::Parse() {
    ParseOffsetTable();

    std::string head = _get_table("head");
    unitsPerEm = _get_ushort(head, 18);
    xMin = (int16_t)_get_ushort(head, 36);
    yMin = (int16_t)_get_ushort(head, 38);
    xMax = (int16_t)_get_ushort(head, 40);
    yMax = (int16_t)_get_ushort(head, 42);

    std::string hhea = _get_table("hhea");
    numberOfHMetrics = _get_ushort(hhea, 34);
    hheaAscender = (int16_t)_get_ushort(hhea, 4);
    hheaDescender = (int16_t)_get_ushort(hhea, 6);

    std::string maxp = _get_table("maxp");
    numGlyphs = _get_ushort(maxp, 4);

    std::string cmap_table = _get_table("cmap");
    uint16_t cmapTableCount = _get_ushort(cmap_table, 2);
    uint32_t unicode_cmap_offset = 0;
    uint16_t cmap_format = 0;
    for (uint16_t i = 0; i < cmapTableCount; i++) {
        size_t pos = 4 + i * 8;
        uint16_t platformID = _get_ushort(cmap_table, pos);
        uint16_t encodingID = _get_ushort(cmap_table, pos + 2);
        uint32_t offset = _get_ulong(cmap_table, pos + 4);
        uint16_t format = _get_ushort(cmap_table, offset);
        if ((platformID == 3 && encodingID == 1) || platformID == 0) {
            if (format == 12) {
                unicode_cmap_offset = offset;
                cmap_format = 12;
                break;
            } else if (format == 4 && unicode_cmap_offset == 0) {
                unicode_cmap_offset = offset;
                cmap_format = 4;
            }
        }
    }
    if (unicode_cmap_offset == 0)
        throw TTFParserException("No Unicode cmap found");

    chars.clear();
    if (cmap_format == 12) {
        size_t p = unicode_cmap_offset + 12;
        uint32_t numGroups = _get_ulong(cmap_table, p);
        p += 4;
        for (uint32_t i = 0; i < numGroups && p + 11 < cmap_table.size(); i++) {
            uint32_t startCharCode = _get_ulong(cmap_table, p);
            uint32_t endCharCode = _get_ulong(cmap_table, p + 4);
            uint32_t startGlyphID = _get_ulong(cmap_table, p + 8);
            p += 12;
            for (uint32_t c = startCharCode; c <= endCharCode && startGlyphID + (c - startCharCode) < 0xFFFF; c++) {
                uint16_t gid = (uint16_t)(startGlyphID + (c - startCharCode));
                if (gid > 0) chars[c] = gid;
            }
        }
    } else {
        size_t fmt4 = unicode_cmap_offset;
        uint16_t segCount = _get_ushort(cmap_table, fmt4 + 6) / 2;
        std::vector<uint16_t> endCount, startCount, idRangeOffset;
        std::vector<int16_t> idDelta;
        size_t p = fmt4 + 14;
        for (uint16_t i = 0; i < segCount; i++) {
            endCount.push_back(_get_ushort(cmap_table, p));
            p += 2;
        }
        p += 2;
        for (uint16_t i = 0; i < segCount; i++) {
            startCount.push_back(_get_ushort(cmap_table, p));
            p += 2;
        }
        for (uint16_t i = 0; i < segCount; i++) {
            idDelta.push_back((int16_t)_get_ushort(cmap_table, p));
            p += 2;
        }
        size_t ro_base = p;
        for (uint16_t i = 0; i < segCount; i++) {
            idRangeOffset.push_back(_get_ushort(cmap_table, p));
            p += 2;
        }

        for (uint16_t i = 0; i < segCount; i++) {
            uint16_t c1 = startCount[i];
            uint16_t c2 = endCount[i];
            int16_t d = idDelta[i];
            uint16_t ro = idRangeOffset[i];
            if (c1 == 0xFFFF) break;
            for (uint32_t c = c1; c <= c2 && c < 0xFFFF; c++) {
                uint16_t gid;
                if (ro > 0) {
                    size_t glyphIndex = ro_base + i * 2 + ro + (c - c1) * 2;
                    if (glyphIndex + 1 < cmap_table.size()) {
                        gid = _get_ushort(cmap_table, glyphIndex);
                        if (gid != 0) {
                            int32_t g = (int32_t)gid + (int32_t)d;
                            if (g < 0) g += 65536;
                            gid = (uint16_t)(g & 0xFFFF);
                        }
                    } else {
                        gid = 0;
                    }
                } else {
                    int32_t g = (int32_t)c + (int32_t)d;
                    if (g < 0) g += 65536;
                    gid = (uint16_t)(g & 0xFFFF);
                }
                if (gid > 0) chars[c] = gid;
            }
        }
    }

    std::string hmtx_table = _get_table("hmtx");
    glyphs.resize(numGlyphs);
    const uint8_t* hp = (const uint8_t*)hmtx_table.data();
    size_t hsz = hmtx_table.size();
    uint16_t last_aw = 0;
    size_t nHM = std::min((size_t)numberOfHMetrics, hsz / 4);
    for (size_t i = 0; i < nHM; i++) {
        last_aw = (hp[i*4] << 8) | hp[i*4+1];
        glyphs[i].advanceWidth = last_aw;
        glyphs[i].lsb = (int16_t)((hp[i*4+2] << 8) | hp[i*4+3]);
    }
    size_t lsb_start = nHM * 4;
    for (size_t i = nHM; i < (size_t)numGlyphs; i++) {
        glyphs[i].advanceWidth = last_aw;
        size_t off = lsb_start + (i - nHM) * 2;
        if (off + 1 < hsz)
            glyphs[i].lsb = (int16_t)((hp[off] << 8) | hp[off+1]);
    }

    std::string name_table = _get_table("name");
    postScriptName = "";
    uint16_t nameCount = _get_ushort(name_table, 2);
    uint16_t stringOffset = _get_ushort(name_table, 4);
    for (uint16_t i = 0; i < nameCount; i++) {
        size_t pos = 6 + i * 12;
        uint16_t nameID = _get_ushort(name_table, pos + 6);
        uint16_t length = _get_ushort(name_table, pos + 8);
        uint16_t offset = _get_ushort(name_table, pos + 10);
        if (nameID == 6) {
            size_t name_start = (size_t)stringOffset + offset;
            if (name_start + length <= name_table.size()) {
                std::string s = name_table.substr(name_start, length);
                std::string cleaned;
                for (char c : s) {
                    if (c != 0 && c != ' ' && c != '[' && c != ']' && c != '(' && c != ')' &&
                        c != '{' && c != '}' && c != '<' && c != '>' && c != '/' && c != '%')
                        cleaned += c;
                }
                postScriptName = cleaned;
            }
            break;
        }
    }

    std::string os2_table = _get_table("OS/2");
    uint16_t fsType = _get_ushort(os2_table, 8);
    embeddable = (fsType != 2) && ((fsType & 0x200) == 0);
    uint16_t fsSelection = _get_ushort(os2_table, 62);
    bold = (fsSelection & 32) != 0;
    usWeightClass = _get_ushort(os2_table, 4);
    typoAscender = (int16_t)_get_ushort(os2_table, 68);
    typoDescender = (int16_t)_get_ushort(os2_table, 70);
    uint16_t os2_version = _get_ushort(os2_table, 0);
    if (os2_version >= 2)
        capHeight = (int16_t)_get_ushort(os2_table, 88);
    else
        capHeight = 0;

    std::string post_table = _get_table("post");
    italicAngle = (int16_t)_get_ushort(post_table, 4);
    underlinePosition = (int16_t)_get_ushort(post_table, 8);
    underlineThickness = (int16_t)_get_ushort(post_table, 10);
    isFixedPitch = (_get_ulong(post_table, 12) != 0);
}

void TTFParser::ParseOffsetTable() {
    std::string version = Read(4);
    is_otf = false;
    if (version == "OTTO")
        is_otf = true;
    else if (version != std::string("\x00\x01\x00\x00", 4))
        throw TTFParserException("Unrecognized file format");
    uint16_t numTables = ReadUShort();
    Skip(3 * 2);
    for (uint16_t i = 0; i < numTables; i++) {
        std::string tag = Read(4);
        ReadULong();
        uint32_t offset = ReadULong();
        uint32_t length = ReadULong();
        tables[tag] = std::make_pair(offset, length);
    }
}

std::string TTFParser::_get_table(const std::string& tag) {
    auto it = tables.find(tag);
    if (it == tables.end())
        return "";
    uint32_t offset = it->second.first;
    uint32_t length = it->second.second;
    // Validate against file size to avoid huge/OOB allocations from corrupt fonts
    f.seekg(0, std::ios::end);
    std::streamoff filesize = f.tellg();
    if (filesize < 0 || offset > (uint32_t)filesize || length > (uint32_t)filesize - offset)
        return "";
    f.seekg(offset, std::ios::beg);
    std::string data(length, '\0');
    if (length > 0)
        f.read(&data[0], length);
    return data;
}

void TTFParser::_set_ushort(std::string& data, size_t pos, uint16_t val) {
    if (pos + 1 < data.size()) {
        data[pos] = (val >> 8) & 0xFF;
        data[pos + 1] = val & 0xFF;
    }
}

uint16_t TTFParser::_get_ushort(const std::string& data, size_t pos) {
    if (pos + 1 >= data.size()) return 0;
    return ((uint8_t)data[pos] << 8) | (uint8_t)data[pos + 1];
}

uint32_t TTFParser::_get_ulong(const std::string& data, size_t pos) {
    if (pos + 3 >= data.size()) return 0;
    return ((uint32_t)(uint8_t)data[pos] << 24) | 
           ((uint32_t)(uint8_t)data[pos + 1] << 16) |
           ((uint32_t)(uint8_t)data[pos + 2] << 8) |
           (uint8_t)data[pos + 3];
}

void TTFParser::Subset(const std::vector<uint32_t>& chars_vec) {
    if (is_otf) {
        // OpenType/CFF: subset the "CFF " table into a compact CID-keyed CFF, then
        // wrap it in an sfnt/OpenType container. Apple CoreGraphics/Preview only
        // renders CID-keyed CFF fonts embedded as a full sfnt (/Subtype /OpenType),
        // not as a bare CFF. Falls back to the bare/whole CFF if wrapping fails.
        std::string cff = _get_table("CFF ");
        if (!cff.empty()) {
            std::string sub = _subset_cff(cff, chars_vec);
            if (!sub.empty()) {
                std::string otf = _build_otf_sfnt(sub);
                built_font = otf.empty() ? sub : otf;
                return;
            }
            built_font = cff;
            return;
        }
        f.seekg(0, std::ios::end);
        size_t size = f.tellg();
        f.seekg(0, std::ios::beg);
        built_font.resize(size);
        f.read(&built_font[0], size);
        return;
    }

    std::string head = _get_table("head");
    unitsPerEm = _get_ushort(head, 18);
    xMin = (int16_t)_get_ushort(head, 36);
    yMin = (int16_t)_get_ushort(head, 38);
    xMax = (int16_t)_get_ushort(head, 40);
    yMax = (int16_t)_get_ushort(head, 42);
    uint16_t indexToLocFormat = _get_ushort(head, 50);

    std::string hhea = _get_table("hhea");
    numberOfHMetrics = _get_ushort(hhea, 34);

    std::string maxp = _get_table("maxp");
    numGlyphs = _get_ushort(maxp, 4);

    std::string cmap_table = _get_table("cmap");
    uint16_t cmapTableCount = _get_ushort(cmap_table, 2);
    uint32_t unicode_cmap_offset = 0;
    uint16_t cmap_format = 0;
    for (uint16_t i = 0; i < cmapTableCount; i++) {
        size_t pos = 4 + i * 8;
        uint16_t platformID = _get_ushort(cmap_table, pos);
        uint16_t encodingID = _get_ushort(cmap_table, pos + 2);
        uint32_t offset = _get_ulong(cmap_table, pos + 4);
        uint16_t format = _get_ushort(cmap_table, offset);
        if ((platformID == 3 && encodingID == 1) || platformID == 0) {
            if (format == 12) {
                unicode_cmap_offset = offset;
                cmap_format = 12;
                break;
            } else if (format == 4 && unicode_cmap_offset == 0) {
                unicode_cmap_offset = offset;
                cmap_format = 4;
            }
        }
    }
    if (unicode_cmap_offset == 0)
        throw TTFParserException("No Unicode cmap found");

    chars.clear();
    if (cmap_format == 12) {
        size_t p = unicode_cmap_offset + 12;
        uint32_t numGroups = _get_ulong(cmap_table, p);
        p += 4;
        for (uint32_t i = 0; i < numGroups && p + 11 < cmap_table.size(); i++) {
            uint32_t startCharCode = _get_ulong(cmap_table, p);
            uint32_t endCharCode = _get_ulong(cmap_table, p + 4);
            uint32_t startGlyphID = _get_ulong(cmap_table, p + 8);
            p += 12;
            for (uint32_t c = startCharCode; c <= endCharCode && startGlyphID + (c - startCharCode) < 0xFFFF; c++) {
                uint16_t gid = (uint16_t)(startGlyphID + (c - startCharCode));
                if (gid > 0) chars[c] = gid;
            }
        }
    } else {
        size_t fmt4 = unicode_cmap_offset;
        uint16_t segCount = _get_ushort(cmap_table, fmt4 + 6) / 2;
        std::vector<uint16_t> endCount, startCount, idRangeOffset;
        std::vector<int16_t> idDelta;
        size_t p = fmt4 + 14;
        for (uint16_t i = 0; i < segCount; i++) {
            endCount.push_back(_get_ushort(cmap_table, p));
            p += 2;
        }
        p += 2;
        for (uint16_t i = 0; i < segCount; i++) {
            startCount.push_back(_get_ushort(cmap_table, p));
            p += 2;
        }
        for (uint16_t i = 0; i < segCount; i++) {
            idDelta.push_back((int16_t)_get_ushort(cmap_table, p));
            p += 2;
        }
        size_t ro_base = p;
        for (uint16_t i = 0; i < segCount; i++) {
            idRangeOffset.push_back(_get_ushort(cmap_table, p));
            p += 2;
        }

        for (uint16_t i = 0; i < segCount; i++) {
            uint16_t c1 = startCount[i];
            uint16_t c2 = endCount[i];
            int16_t d = idDelta[i];
            uint16_t ro = idRangeOffset[i];
            if (c1 == 0xFFFF) break;
            for (uint32_t c = c1; c <= c2 && c < 0xFFFF; c++) {
                uint16_t gid;
                if (ro > 0) {
                    size_t glyphIndex = ro_base + i * 2 + ro + (c - c1) * 2;
                    if (glyphIndex + 1 < cmap_table.size()) {
                        gid = _get_ushort(cmap_table, glyphIndex);
                        if (gid != 0) {
                            int32_t g = (int32_t)gid + (int32_t)d;
                            if (g < 0) g += 65536;
                            gid = (uint16_t)(g & 0xFFFF);
                        }
                    } else {
                        gid = 0;
                    }
                } else {
                    int32_t g = (int32_t)c + (int32_t)d;
                    if (g < 0) g += 65536;
                    gid = (uint16_t)(g & 0xFFFF);
                }
                if (gid > 0) chars[c] = gid;
            }
        }
    }

    std::string glyf_table = _get_table("glyf");
    std::string loca_table = _get_table("loca");
    std::vector<uint32_t> glyph_offsets(numGlyphs + 1, 0);
    const uint8_t* lp = (const uint8_t*)loca_table.data();
    size_t lsz = loca_table.size();
    if (indexToLocFormat == 0) {
        size_t max_i = std::min((size_t)numGlyphs + 1, lsz / 2);
        for (size_t i = 0; i < max_i; i++)
            glyph_offsets[i] = 2u * ((lp[i*2] << 8) | lp[i*2+1]);
    } else {
        size_t max_i = std::min((size_t)numGlyphs + 1, lsz / 4);
        for (size_t i = 0; i < max_i; i++)
            glyph_offsets[i] = ((uint32_t)lp[i*4] << 24) | ((uint32_t)lp[i*4+1] << 16) |
                               ((uint32_t)lp[i*4+2] << 8) | lp[i*4+3];
    }
    std::string hmtx_table = _get_table("hmtx");
    std::vector<uint16_t> advanceWidths(numGlyphs);
    std::vector<int16_t> lsbs(numGlyphs);
    const uint8_t* hp = (const uint8_t*)hmtx_table.data();
    size_t hsz = hmtx_table.size();
    uint16_t last_aw = 0;
    size_t nHM = std::min((size_t)numberOfHMetrics, hsz / 4);
    for (size_t i = 0; i < nHM; i++) {
        last_aw = (hp[i*4] << 8) | hp[i*4+1];
        advanceWidths[i] = last_aw;
        lsbs[i] = (int16_t)((hp[i*4+2] << 8) | hp[i*4+3]);
    }
    size_t lsb_start = nHM * 4;
    for (size_t i = nHM; i < (size_t)numGlyphs; i++) {
        advanceWidths[i] = last_aw;
        size_t off = lsb_start + (i - nHM) * 2;
        if (off + 1 < hsz)
            lsbs[i] = (int16_t)((hp[off] << 8) | hp[off+1]);
    }
    std::map<uint16_t, uint32_t> subsetglyphs;
    std::map<uint32_t, uint16_t> subsetCharToGlyph;
    subsetglyphs[0] = 0;
    for (uint32_t code : chars_vec) {
        auto it = chars.find(code);
        if (it != chars.end()) {
            subsetglyphs[it->second] = code;
            subsetCharToGlyph[code] = it->second;
        }
    }

    auto get_glyph_data = [&](uint16_t gid) -> std::string {
        if (gid >= glyph_offsets.size() - 1) return "";
        uint32_t start = glyph_offsets[gid];
        uint32_t end = glyph_offsets[gid + 1];
        if (end <= start || start >= glyf_table.size()) return "";
        return glyf_table.substr(start, end - start);
    };

    std::map<uint16_t, uint16_t> glyphSet;
    uint16_t n = 0;
    for (auto& kv : subsetglyphs) {
        glyphSet[kv.first] = n++;
    }
    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<uint16_t> to_add;
        for (auto& kv : subsetglyphs) {
            uint16_t originalGlyphIdx = kv.first;
            std::string data = get_glyph_data(originalGlyphIdx);
            if (data.size() > 2) {
                int16_t nContours = (int16_t)_get_ushort(data, 0);
                if (nContours < 0) {
                    size_t pos_in_glyph = 10;
                    uint16_t flags = 0x20;
                    while (flags & 0x20) {
                        if (pos_in_glyph + 4 > data.size()) break;
                        flags = _get_ushort(data, pos_in_glyph);
                        uint16_t glyphIdx = _get_ushort(data, pos_in_glyph + 2);
                        if (glyphSet.find(glyphIdx) == glyphSet.end()) {
                            to_add.push_back(glyphIdx);
                            changed = true;
                        }
                        pos_in_glyph += 4;
                        if (flags & 0x01) pos_in_glyph += 4;
                        else pos_in_glyph += 2;
                        if (flags & 0x08) pos_in_glyph += 2;
                        else if (flags & 0x40) pos_in_glyph += 4;
                        else if (flags & 0x80) pos_in_glyph += 8;
                    }
                }
            }
        }
        for (uint16_t gid : to_add) {
            if (glyphSet.find(gid) == glyphSet.end()) {
                glyphSet[gid] = n++;
                subsetglyphs[gid] = 0;
            }
        }
    }
    std::map<uint32_t, uint16_t> codeToGlyph;
    for (auto& kv : subsetCharToGlyph) {
        codeToGlyph[kv.first] = glyphSet[kv.second];
    }

    std::string name_table = _get_table("name");
    postScriptName = "";
    uint16_t nameCount = _get_ushort(name_table, 2);
    uint16_t stringOffset = _get_ushort(name_table, 4);
    for (uint16_t i = 0; i < nameCount; i++) {
        size_t pos = 6 + i * 12;
        uint16_t nameID = _get_ushort(name_table, pos + 6);
        uint16_t length = _get_ushort(name_table, pos + 8);
        uint16_t offset = _get_ushort(name_table, pos + 10);
        if (nameID == 6) {
            size_t name_start = (size_t)stringOffset + offset;
            if (name_start + length <= name_table.size()) {
                std::string s = name_table.substr(name_start, length);
                std::string cleaned;
                for (char c : s) {
                    if (c != 0 && c != ' ' && c != '[' && c != ']' && c != '(' && c != ')' &&
                        c != '{' && c != '}' && c != '<' && c != '>' && c != '/' && c != '%')
                        cleaned += c;
                }
                postScriptName = cleaned;
            }
            break;
        }
    }

    std::string os2_table = _get_table("OS/2");
    uint16_t fsType = _get_ushort(os2_table, 8);
    embeddable = (fsType != 2) && ((fsType & 0x200) == 0);
    uint16_t fsSelection = _get_ushort(os2_table, 62);
    bold = (fsSelection & 32) != 0;
    usWeightClass = _get_ushort(os2_table, 4);
    typoAscender = (int16_t)_get_ushort(os2_table, 68);
    typoDescender = (int16_t)_get_ushort(os2_table, 70);
    uint16_t os2_version = _get_ushort(os2_table, 0);
    if (os2_version >= 2)
        capHeight = (int16_t)_get_ushort(os2_table, 88);
    else
        capHeight = 0;

    std::string post_table = _get_table("post");
    italicAngle = (int16_t)_get_ushort(post_table, 4);
    underlinePosition = (int16_t)_get_ushort(post_table, 8);
    underlineThickness = (int16_t)_get_ushort(post_table, 10);
    isFixedPitch = (_get_ulong(post_table, 12) != 0);


    glyphs.resize(numGlyphs);
    for (uint16_t i = 0; i < numGlyphs; i++) {
        glyphs[i].advanceWidth = advanceWidths[i];
        glyphs[i].lsb = lsbs[i];
    }

    std::map<std::string, std::string> new_tables;

    new_tables["name"] = name_table;
    for (auto tag : {"cvt ", "fpgm", "prep", "gasp"}) {
        std::string t = _get_table(tag);
        if (!t.empty()) new_tables[tag] = t;
    }

    std::string post_prefix = (post_table.size() > 4) ? post_table.substr(4, 12) : std::string();
    if (post_prefix.size() < 12) post_prefix.resize(12, '\x00');
    std::string new_post = std::string("\x00\x03\x00\x00", 4) + post_prefix + 
                           std::string(16, '\x00');
    new_tables["post"] = new_post;

    std::map<uint32_t, std::vector<uint16_t>> range;
    uint32_t prevcid = 0xFFFFFFFF;
    uint16_t prevglidx = 0xFFFF;
    uint32_t rangeid = 0;
    for (auto& kv : codeToGlyph) {
        uint32_t cid = kv.first;
        uint16_t glidx = kv.second;
        if (cid == (prevcid + 1) && glidx == (prevglidx + 1)) {
            range[rangeid].push_back(glidx);
        } else {
            rangeid = cid;
            range[rangeid].clear();
            range[rangeid].push_back(glidx);
        }
        prevcid = cid;
        prevglidx = glidx;
    }

    uint16_t newSegCount = (uint16_t)range.size() + 1;
    uint16_t searchRange = 1;
    uint16_t entrySelector = 0;
    while (searchRange * 2 <= newSegCount) {
        searchRange *= 2;
        entrySelector++;
    }
    searchRange *= 2;
    uint16_t rangeShift = newSegCount * 2 - searchRange;

    uint16_t segCountX2 = (uint16_t)(newSegCount * 2);
    uint16_t fmt4_length = (uint16_t)(16 + newSegCount * 8);

    std::vector<uint16_t> cmap_vec = {0, 1, 3, 1, 0, 12, 4, fmt4_length, 0, segCountX2,
                                       searchRange, entrySelector, rangeShift};
    std::vector<uint16_t> endCodes, startCodes, idDeltas, idRangeOffsets;

    for (auto& kv : range) {
        uint32_t start = kv.first;
        auto& subrange = kv.second;
        uint32_t endCode = start + (subrange.size() - 1);
        endCodes.push_back((uint16_t)endCode);
    }
    endCodes.push_back(0xFFFF);

    for (auto& kv : range) {
        startCodes.push_back((uint16_t)kv.first);
    }
    startCodes.push_back(0xFFFF);

    for (auto& kv : range) {
        uint32_t start = kv.first;
        int16_t idDelta = (int16_t)(-(int32_t)start + kv.second[0]);
        idDeltas.push_back(idDelta);
    }
    idDeltas.push_back(1);

    for (size_t i = 0; i < range.size(); i++) {
        idRangeOffsets.push_back(0);
    }
    idRangeOffsets.push_back(0);

    for (uint16_t v : endCodes) cmap_vec.push_back(v);
    cmap_vec.push_back(0);
    for (uint16_t v : startCodes) cmap_vec.push_back(v);
    for (int16_t v : idDeltas) cmap_vec.push_back((uint16_t)v);
    for (uint16_t v : idRangeOffsets) cmap_vec.push_back(v);
    cmap_vec.push_back(0);

    std::string cmapstr;
    cmapstr.reserve(cmap_vec.size() * 2);
    for (uint16_t v : cmap_vec) {
        cmapstr += (char)((v >> 8) & 0xFF);
        cmapstr += (char)(v & 0xFF);
    }
    new_tables["cmap"] = cmapstr;

    std::vector<uint16_t> ordered_glyphs;
    for (auto& kv : glyphSet) ordered_glyphs.push_back(kv.first);
    std::sort(ordered_glyphs.begin(), ordered_glyphs.end(),
              [&](uint16_t a, uint16_t b) { return glyphSet[a] < glyphSet[b]; });
    // 预计算 glyf 总大小，避免 O(n²) 拼接
    size_t total_glyf_size = 0;
    for (uint16_t gid : ordered_glyphs) {
        if (gid < glyph_offsets.size() - 1) {
            uint32_t gstart = glyph_offsets[gid];
            uint32_t gend = glyph_offsets[gid + 1];
            if (gend > gstart && gstart < glyf_table.size()) {
                size_t gsize = gend - gstart;
                total_glyf_size += gsize;
                total_glyf_size += (4 - gsize % 4) % 4; // padding
            }
        }
    }

    std::string glyf;
    glyf.reserve(total_glyf_size);
    std::string hmtxstr;
    hmtxstr.reserve(ordered_glyphs.size() * 4);
    std::vector<uint32_t> offsets;
    offsets.reserve(ordered_glyphs.size() + 1);
    uint32_t pos = 0;

    for (uint16_t originalGlyphIdx : ordered_glyphs) {
        uint16_t aw = 0;
        int16_t lsb = 0;
        if (originalGlyphIdx < advanceWidths.size()) {
            aw = advanceWidths[originalGlyphIdx];
            lsb = lsbs[originalGlyphIdx];
        }
        hmtxstr += (char)((aw >> 8) & 0xFF);
        hmtxstr += (char)(aw & 0xFF);
        hmtxstr += (char)(((uint16_t)lsb >> 8) & 0xFF);
        hmtxstr += (char)((uint16_t)lsb & 0xFF);

        offsets.push_back(pos);
        std::string data = get_glyph_data(originalGlyphIdx);

        if (!data.empty()) {
            int16_t nContours = (int16_t)_get_ushort(data, 0);
            if (nContours < 0) {
                size_t pos_in_glyph = 10;
                uint16_t flags = 0x20;
                while (flags & 0x20) {
                    if (pos_in_glyph + 4 > data.size()) break;
                    flags = _get_ushort(data, pos_in_glyph);
                    uint16_t glyphIdx = _get_ushort(data, pos_in_glyph + 2);
                    _set_ushort(data, pos_in_glyph + 2, glyphSet[glyphIdx]);
                    pos_in_glyph += 4;
                    if (flags & 0x01) pos_in_glyph += 4;
                    else pos_in_glyph += 2;
                    if (flags & 0x08) pos_in_glyph += 2;
                    else if (flags & 0x40) pos_in_glyph += 4;
                    else if (flags & 0x80) pos_in_glyph += 8;
                }
            }
        }

        glyf += data;
        pos += (uint32_t)data.size();
        while (pos % 4 != 0) {
            glyf += '\x00';
            pos++;
        }
    }
    offsets.push_back(pos);
    new_tables["glyf"] = glyf;
    new_tables["hmtx"] = hmtxstr;

    uint16_t newNumGlyphs = (uint16_t)ordered_glyphs.size();
    uint16_t newNumberOfHMetrics = newNumGlyphs;

    std::string locastr;
    uint16_t newIndexToLocFormat;
    if (((pos + 1) >> 1) > 0xFFFF) {
        newIndexToLocFormat = 1;
        for (uint32_t offset : offsets) {
            locastr += (char)((offset >> 24) & 0xFF);
            locastr += (char)((offset >> 16) & 0xFF);
            locastr += (char)((offset >> 8) & 0xFF);
            locastr += (char)(offset & 0xFF);
        }
    } else {
        newIndexToLocFormat = 0;
        for (uint32_t offset : offsets) {
            uint16_t v = (uint16_t)(offset / 2);
            locastr += (char)((v >> 8) & 0xFF);
            locastr += (char)(v & 0xFF);
        }
    }
    new_tables["loca"] = locastr;

    std::string new_head = head;
    _set_ushort(new_head, 50, newIndexToLocFormat);
    new_tables["head"] = new_head;

    std::string new_hhea = hhea;
    _set_ushort(new_hhea, 34, newNumberOfHMetrics);
    new_tables["hhea"] = new_hhea;

    std::string new_maxp = maxp;
    _set_ushort(new_maxp, 4, newNumGlyphs);
    new_tables["maxp"] = new_maxp;

    new_tables["OS/2"] = os2_table;

    std::vector<std::string> tag_order = {"cmap", "cvt ", "fpgm", "gasp", "glyf", "head", "hhea",
                                          "hmtx", "loca", "maxp", "name", "OS/2", "post", "prep"};
    std::vector<std::string> final_tags;
    std::map<std::string, std::string> final_data;
    for (auto& tag : tag_order) {
        if (new_tables.find(tag) != new_tables.end()) {
            final_tags.push_back(tag);
            final_data[tag] = new_tables[tag];
        }
    }

    for (auto& kv : final_data) {
        while (kv.second.size() % 4 != 0)
            kv.second += '\x00';
    }

    auto calc_checksum = [](const std::string& data) -> uint32_t {
        uint32_t sum = 0;
        for (size_t i = 0; i < data.size(); i += 4) {
            uint32_t b0 = (i < data.size()) ? (uint8_t)data[i] : 0;
            uint32_t b1 = (i + 1 < data.size()) ? (uint8_t)data[i + 1] : 0;
            uint32_t b2 = (i + 2 < data.size()) ? (uint8_t)data[i + 2] : 0;
            uint32_t b3 = (i + 3 < data.size()) ? (uint8_t)data[i + 3] : 0;
            sum += (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
        }
        return sum;
    };

    uint16_t numTables = (uint16_t)final_tags.size();
    uint16_t sr = 1;
    uint16_t es = 0;
    while ((uint16_t)(16 * (1 << (es + 1))) <= (uint16_t)(16 * numTables)) {
        es++;
    }
    sr = (uint16_t)(16 * (1 << es));
    uint16_t rs = (uint16_t)(16 * numTables - sr);

    std::string offset_table;
    offset_table.reserve(12 + 16 * numTables);
    offset_table += '\x00';
    offset_table += '\x01';
    offset_table += '\x00';
    offset_table += '\x00';
    offset_table += (char)((numTables >> 8) & 0xFF);
    offset_table += (char)(numTables & 0xFF);
    offset_table += (char)((sr >> 8) & 0xFF);
    offset_table += (char)(sr & 0xFF);
    offset_table += (char)((es >> 8) & 0xFF);
    offset_table += (char)(es & 0xFF);
    offset_table += (char)((rs >> 8) & 0xFF);
    offset_table += (char)(rs & 0xFF);

    uint32_t table_offset = 12 + 16 * numTables;
    // 预计算所有表数据总大小
    size_t total_table_size = 0;
    for (auto& kv : final_data) total_table_size += kv.second.size();
    std::string table_data_concat;
    table_data_concat.reserve(total_table_size);
    for (auto& tag : final_tags) {
        auto& data = final_data[tag];
        uint32_t cs = calc_checksum(data);
        offset_table += tag;
        offset_table += (char)((cs >> 24) & 0xFF);
        offset_table += (char)((cs >> 16) & 0xFF);
        offset_table += (char)((cs >> 8) & 0xFF);
        offset_table += (char)(cs & 0xFF);
        uint32_t actual_len = 0;
        if (new_tables.find(tag) != new_tables.end()) {
            actual_len = (uint32_t)new_tables[tag].size();
        } else {
            auto it = tables.find(tag);
            if (it != tables.end())
                actual_len = it->second.second;
        }
        offset_table += (char)((table_offset >> 24) & 0xFF);
        offset_table += (char)((table_offset >> 16) & 0xFF);
        offset_table += (char)((table_offset >> 8) & 0xFF);
        offset_table += (char)(table_offset & 0xFF);
        offset_table += (char)((actual_len >> 24) & 0xFF);
        offset_table += (char)((actual_len >> 16) & 0xFF);
        offset_table += (char)((actual_len >> 8) & 0xFF);
        offset_table += (char)(actual_len & 0xFF);
        table_offset += (uint32_t)data.size();
        table_data_concat += data;
    }

    uint32_t master_checksum = calc_checksum(offset_table) + calc_checksum(table_data_concat);
    uint32_t checkSumAdjustment = 0xB1B0AFBA - master_checksum;
    _set_ushort(final_data["head"], 8, (uint16_t)((checkSumAdjustment >> 16) & 0xFFFF));
    _set_ushort(final_data["head"], 10, (uint16_t)(checkSumAdjustment & 0xFFFF));

    std::string result;
    result.reserve(offset_table.size() + total_table_size);
    result = offset_table;
    for (auto& tag : final_tags) {
        result += final_data[tag];
    }

    built_font = result;
    this->glyphMap = glyphSet;
    subsettedGlyphs.clear();
    for (uint16_t g : ordered_glyphs) subsettedGlyphs.push_back(g);
}

std::string TTFParser::Build() {
    return built_font;
}

std::string TTFParser::_subset_cff(const std::string& cff, const std::vector<uint32_t>& chars_vec) {
    if (cff.size() < 4) return "";
    uint8_t hdr_size = (uint8_t)cff[2];
    size_t pos = hdr_size;
    size_t np;

    std::vector<std::string> name_index, top_index, string_index, gsubr_index;
    if (!cff_read_index(cff, pos, name_index, np)) return "";
    pos = np;
    if (!cff_read_index(cff, pos, top_index, np)) return "";
    pos = np;
    if (!cff_read_index(cff, pos, string_index, np)) return "";
    pos = np;
    if (!cff_read_index(cff, pos, gsubr_index, np)) return "";
    pos = np;
    if (top_index.empty()) return "";

    std::vector<CFFDictEntry> top;
    cff_parse_dict(top_index[0], top);

    const CFFDictEntry* e_cs = cff_find(top, 17); // CharStrings
    if (!e_cs || e_cs->ops.empty()) return "";
    std::vector<std::string> char_strings;
    if (!cff_read_index(cff, (size_t)e_cs->ops[0], char_strings, np)) return "";
    uint16_t num_glyphs = (uint16_t)char_strings.size();
    if (num_glyphs == 0) return "";

    bool is_cid = cff_find(top, 1230) != nullptr; // ROS

    // Per-font-dict data we keep verbatim (Private DICT stripped of Subrs, plus
    // its local subrs and the source Font DICT entries).
    struct FDData {
        std::string priv_no_subrs;
        std::vector<std::string> local_subrs;
        bool has_subrs = false;
        std::vector<CFFDictEntry> font_dict; // source Font DICT entries (CID only)
    };
    std::vector<FDData> fds;
    std::vector<uint8_t> gid_to_fd(num_glyphs, 0);

    auto load_private = [&](size_t priv_size, size_t priv_off, FDData& fd) {
        if (priv_off + priv_size > cff.size()) return;
        std::string priv = cff.substr(priv_off, priv_size);
        std::vector<CFFDictEntry> pe;
        cff_parse_dict(priv, pe);
        const CFFDictEntry* subrs = cff_find(pe, 19);
        if (subrs && !subrs->ops.empty()) {
            size_t ls_off = priv_off + (size_t)subrs->ops[0];
            size_t tmp;
            if (cff_read_index(cff, ls_off, fd.local_subrs, tmp))
                fd.has_subrs = true;
        }
        std::string nb;
        for (auto& en : pe) if (en.op != 19) nb += en.raw;
        fd.priv_no_subrs = nb;
    };

    if (is_cid) {
        const CFFDictEntry* e_fda = cff_find(top, 1236); // FDArray
        const CFFDictEntry* e_fds = cff_find(top, 1237); // FDSelect
        if (!e_fda || !e_fds || e_fda->ops.empty() || e_fds->ops.empty()) return "";
        std::vector<std::string> fd_array;
        if (!cff_read_index(cff, (size_t)e_fda->ops[0], fd_array, np)) return "";
        if (fd_array.empty() || fd_array.size() > 255) return "";

        size_t fdsel_off = (size_t)e_fds->ops[0];
        if (fdsel_off >= cff.size()) return "";
        uint8_t fmt = (uint8_t)cff[fdsel_off];
        if (fmt == 0) {
            for (uint16_t g = 0; g < num_glyphs; g++) {
                size_t q = fdsel_off + 1 + g;
                gid_to_fd[g] = (q < cff.size()) ? (uint8_t)cff[q] : 0;
            }
        } else if (fmt == 3) {
            uint16_t n_ranges = cff_u16(cff, fdsel_off + 1);
            size_t rp = fdsel_off + 3;
            for (uint16_t r = 0; r < n_ranges; r++) {
                uint16_t first = cff_u16(cff, rp);
                uint8_t fdi = (rp + 2 < cff.size()) ? (uint8_t)cff[rp + 2] : 0;
                uint16_t next = cff_u16(cff, rp + 3);
                for (uint16_t g = first; g < next && g < num_glyphs; g++) gid_to_fd[g] = fdi;
                rp += 3;
            }
        } else {
            return "";
        }

        fds.resize(fd_array.size());
        for (size_t i = 0; i < fd_array.size(); i++) {
            cff_parse_dict(fd_array[i], fds[i].font_dict);
            const CFFDictEntry* pv = cff_find(fds[i].font_dict, 18); // Private
            if (pv && pv->ops.size() >= 2)
                load_private((size_t)pv->ops[0], (size_t)pv->ops[1], fds[i]);
        }
    } else {
        FDData fd;
        const CFFDictEntry* pv = cff_find(top, 18); // Private
        if (pv && pv->ops.size() >= 2)
            load_private((size_t)pv->ops[0], (size_t)pv->ops[1], fd);
        fds.push_back(fd);
        // gid_to_fd already all zero.
    }

    // Build a compact subset: each distinct Unicode becomes a new dense GID and
    // the charset is identity (new GID i -> CID i). The CFF is wrapped in an
    // sfnt (OpenType) container by _build_otf_sfnt. The Unicode -> CID(==GID)
    // relationship is NOT carried by the charset (pdfium/CoreGraphics ignore a
    // non-identity charset and treat CID as GID directly under Identity
    // ordering); instead _putfonts emits a custom PDF Encoding CMap that maps
    // the 2-byte Unicode content codes to these CIDs. cffUniToGid (Unicode ->
    // new GID == CID) and cffNewToOldGid (new GID -> original GID) are returned
    // for that CMap, the /W array and for rebuilding hmtx.
    std::map<uint32_t, uint16_t> cid_to_old; // CID(Unicode) -> original GID
    for (uint32_t code : chars_vec) {
        auto it = chars.find(code);
        if (it != chars.end() && it->second < num_glyphs && code <= 0xFFFF)
            cid_to_old[code] = it->second;
    }

    // new GID 0 == .notdef (original GID 0, CID 0). Remaining new GIDs follow in
    // ascending CID order (std::map is ordered).
    std::vector<std::pair<uint32_t, uint16_t>> order; // (CID == Unicode, original GID)
    for (auto& kv : cid_to_old) order.push_back({kv.first, kv.second});

    // Remap the font dicts that are actually referenced by the subset.
    std::map<uint8_t, uint8_t> fd_remap; // old FD -> new FD
    auto use_fd = [&](uint8_t f) {
        if (f < fds.size() && !fd_remap.count(f)) {
            uint8_t nf = (uint8_t)fd_remap.size();
            fd_remap[f] = nf;
        }
    };
    use_fd(gid_to_fd[0]);
    for (auto& pr : order) use_fd(gid_to_fd[pr.second]);
    if (fd_remap.empty()) fd_remap[0] = 0;
    std::vector<uint8_t> new_to_old(fd_remap.size());
    for (auto& kv : fd_remap) new_to_old[kv.second] = kv.first;

    // Attempt to inline every referenced subr so the Global/Local Subr INDEXes
    // can be dropped entirely (a big win for CJK fonts whose FDs carry tens of
    // thousands of local subrs). If any glyph fails to inline cleanly we fall
    // back to keeping the subrs verbatim.
    auto bias_of = [](size_t nsub) -> int {
        return nsub < 1240 ? 107 : (nsub < 33900 ? 1131 : 32768);
    };
    int gbias = bias_of(gsubr_index.size());
    static const std::vector<std::string> empty_subrs;
    auto inline_glyph = [&](uint16_t oldGid, std::string& result) -> bool {
        uint8_t of = gid_to_fd[oldGid];
        const std::vector<std::string>& ls =
            (of < fds.size() && fds[of].has_subrs) ? fds[of].local_subrs : empty_subrs;
        CFFInliner in{gsubr_index, gbias, ls, bias_of(ls.size())};
        in.run(char_strings[oldGid], 0);
        if (!in.ok) return false;
        result = std::move(in.out);
        return true;
    };

    std::vector<std::string> inlined_cs;
    bool inlined = true;
    {
        std::string tmp;
        if (inline_glyph(0, tmp)) inlined_cs.push_back(std::move(tmp));
        else inlined = false;
        if (inlined) {
            for (auto& pr : order) {
                std::string t;
                if (inline_glyph(pr.second, t)) inlined_cs.push_back(std::move(t));
                else { inlined = false; break; }
            }
        }
    }

    // CharStrings, FDSelect (format 0) and charset (format 0). The charset lists
    // CID == Unicode for each new GID 1,2,3,...
    std::vector<std::string> new_charstrings;
    std::string fdselect;
    fdselect += (char)0;
    std::string charset;
    charset += (char)0;

    new_charstrings.push_back(inlined ? inlined_cs[0] : char_strings[0]); // .notdef, CID 0
    fdselect += (char)(fd_remap.count(gid_to_fd[0]) ? fd_remap[gid_to_fd[0]] : 0);
    cffUniToGid.clear();
    cffNewToOldGid.clear();
    cffNewToOldGid.push_back(0); // new GID 0 -> original GID 0
    size_t ci = 1;
    for (auto& pr : order) {
        new_charstrings.push_back(inlined ? inlined_cs[ci++] : char_strings[pr.second]);
        uint8_t of = gid_to_fd[pr.second];
        fdselect += (char)(fd_remap.count(of) ? fd_remap[of] : 0);
        uint16_t newGid = (uint16_t)(new_charstrings.size() - 1);
        // Identity charset format 0: new GID i is assigned CID i. Both a viewer
        // that maps CID->GID directly (pdfium/CoreGraphics) and one that honours
        // the charset (Acrobat) then resolve to the same glyph.
        charset += (char)((newGid >> 8) & 0xFF);
        charset += (char)(newGid & 0xFF);
        cffUniToGid[pr.first] = newGid;
        cffNewToOldGid.push_back(pr.second);
    }
    // Dense CIDs 0..N-1 (identity with GID).
    int cid_count = (int)new_charstrings.size();

    // Per-FD Private DICT + local subrs. When inlining succeeded no local subrs
    // are emitted; otherwise they are placed immediately after their Private
    // DICT so the Subrs offset equals the Private size.
    struct FDOut { std::string priv; std::string local_subrs_idx; int priv_size; };
    std::vector<FDOut> fdouts(new_to_old.size());
    for (size_t nf = 0; nf < new_to_old.size(); nf++) {
        FDData& fd = fds[new_to_old[nf]];
        FDOut o;
        if (!inlined && fd.has_subrs) {
            o.local_subrs_idx = cff_write_index(fd.local_subrs);
            int psize = (int)fd.priv_no_subrs.size() + 6; // 5-byte offset + 1-byte op(19)
            o.priv = fd.priv_no_subrs + cff_enc_offset(psize) + cff_enc_op(19);
            o.priv_size = psize;
        } else {
            o.priv = fd.priv_no_subrs;
            o.priv_size = (int)fd.priv_no_subrs.size();
        }
        fdouts[nf] = o;
    }

    // String INDEX: keep originals, append "Adobe" and "Identity" for the ROS.
    std::vector<std::string> new_strings = string_index;
    int sid_registry = 391 + (int)new_strings.size();
    new_strings.push_back("Adobe");
    int sid_ordering = 391 + (int)new_strings.size();
    new_strings.push_back("Identity");

    static const std::set<int> drop_top = {1230, 15, 16, 17, 18, 1234, 1236, 1237};

    auto build_top = [&](int off_charset, int off_charstrings, int off_fdarray, int off_fdselect) {
        std::string t;
        // ROS must be the FIRST operator in a CID-keyed Top DICT (Adobe TN5176).
        // Strict parsers such as Apple CoreGraphics/Preview only recognise the
        // font as CID-keyed when ROS comes first; otherwise they treat it as a
        // name-keyed font, mis-read the charset and fall back to a system font.
        t += cff_enc_int(sid_registry) + cff_enc_int(sid_ordering) + cff_enc_int(0) + cff_enc_op(1230); // ROS
        for (auto& en : top) if (!drop_top.count(en.op)) t += en.raw;
        t += cff_enc_int(cid_count) + cff_enc_op(1234); // CIDCount
        t += cff_enc_offset(off_charset) + cff_enc_op(15);      // charset
        t += cff_enc_offset(off_fdselect) + cff_enc_op(1237);   // FDSelect
        t += cff_enc_offset(off_charstrings) + cff_enc_op(17);  // CharStrings
        t += cff_enc_offset(off_fdarray) + cff_enc_op(1236);    // FDArray
        return t;
    };

    auto build_font_dict = [&](int priv_size, int priv_off, const std::vector<CFFDictEntry>& src) {
        std::string d;
        for (auto& en : src) if (en.op != 18) d += en.raw;
        d += cff_enc_offset(priv_size) + cff_enc_offset(priv_off) + cff_enc_op(18); // Private size offset
        return d;
    };

    std::string header;
    header += (char)1; header += (char)0; header += (char)4; header += (char)4;
    std::string name_bytes = cff_write_index(name_index);
    std::string string_bytes = cff_write_index(new_strings);
    std::string gsubr_bytes = cff_write_index(inlined ? std::vector<std::string>{} : gsubr_index);
    std::string cs_index_bytes = cff_write_index(new_charstrings);

    // Fixed-width encodings keep the Top DICT / FDArray lengths independent of
    // the offsets, so we can size the layout with placeholders first.
    std::string top_dummy = cff_write_index({build_top(0, 0, 0, 0)});
    size_t prefix_len = header.size() + name_bytes.size() + top_dummy.size() +
                        string_bytes.size() + gsubr_bytes.size();

    std::vector<CFFDictEntry> empty_dict;
    std::vector<std::string> fd_dummy;
    for (size_t nf = 0; nf < fdouts.size(); nf++) {
        const std::vector<CFFDictEntry>& src = is_cid ? fds[new_to_old[nf]].font_dict : empty_dict;
        fd_dummy.push_back(build_font_dict(fdouts[nf].priv_size, 0, src));
    }
    std::string fdarray_dummy = cff_write_index(fd_dummy);

    int off_charset = (int)prefix_len;
    int off_fdselect = off_charset + (int)charset.size();
    int off_charstrings = off_fdselect + (int)fdselect.size();
    int off_fdarray = off_charstrings + (int)cs_index_bytes.size();
    int off_privs = off_fdarray + (int)fdarray_dummy.size();

    std::vector<int> priv_offsets(fdouts.size());
    int cur = off_privs;
    for (size_t nf = 0; nf < fdouts.size(); nf++) {
        priv_offsets[nf] = cur;
        cur += (int)fdouts[nf].priv.size() + (int)fdouts[nf].local_subrs_idx.size();
    }

    std::vector<std::string> fd_dicts;
    for (size_t nf = 0; nf < fdouts.size(); nf++) {
        const std::vector<CFFDictEntry>& src = is_cid ? fds[new_to_old[nf]].font_dict : empty_dict;
        fd_dicts.push_back(build_font_dict(fdouts[nf].priv_size, priv_offsets[nf], src));
    }
    std::string fdarray_bytes = cff_write_index(fd_dicts);
    std::string top_bytes = cff_write_index({build_top(off_charset, off_charstrings, off_fdarray, off_fdselect)});
    if (top_bytes.size() != top_dummy.size() || fdarray_bytes.size() != fdarray_dummy.size())
        return ""; // layout assumption broke; fall back

    std::string out;
    out.reserve(off_privs + (cur - off_privs));
    out += header;
    out += name_bytes;
    out += top_bytes;
    out += string_bytes;
    out += gsubr_bytes;
    out += charset;
    out += fdselect;
    out += cs_index_bytes;
    out += fdarray_bytes;
    for (size_t nf = 0; nf < fdouts.size(); nf++) {
        out += fdouts[nf].priv;
        out += fdouts[nf].local_subrs_idx;
    }
    return out;
}

std::string TTFParser::_build_cmap4(const std::map<uint32_t, uint16_t>& uni_to_gid) {
    auto u16 = [](std::string& s, uint16_t v) {
        s += (char)((v >> 8) & 0xFF);
        s += (char)(v & 0xFF);
    };
    auto u32 = [](std::string& s, uint32_t v) {
        s += (char)((v >> 24) & 0xFF);
        s += (char)((v >> 16) & 0xFF);
        s += (char)((v >> 8) & 0xFF);
        s += (char)(v & 0xFF);
    };
    // One segment per BMP code point plus the mandatory 0xFFFF terminator. Codes
    // above the BMP are skipped (the PDF selects glyphs via the CFF, not cmap).
    std::vector<std::pair<uint16_t, uint16_t>> ents; // (code, gid)
    for (auto& kv : uni_to_gid)
        if (kv.first <= 0xFFFF) ents.push_back({(uint16_t)kv.first, kv.second});
    std::sort(ents.begin(), ents.end());

    uint16_t segCount = (uint16_t)(ents.size() + 1);
    uint16_t es = 0, p = 1;
    while ((uint16_t)(p * 2) <= segCount) { p = (uint16_t)(p * 2); es++; }
    uint16_t searchRange = (uint16_t)(2 * p);
    uint16_t rangeShift = (uint16_t)(2 * segCount - searchRange);

    std::string sub;
    u16(sub, 4);                       // format
    uint16_t length = (uint16_t)(16 + 8 * segCount);
    u16(sub, length);                  // length
    u16(sub, 0);                       // language
    u16(sub, (uint16_t)(2 * segCount));// segCountX2
    u16(sub, searchRange);
    u16(sub, es);                      // entrySelector
    u16(sub, rangeShift);
    for (auto& e : ents) u16(sub, e.first);   // endCode
    u16(sub, 0xFFFF);                          // endCode terminator
    u16(sub, 0);                               // reservedPad
    for (auto& e : ents) u16(sub, e.first);   // startCode
    u16(sub, 0xFFFF);                          // startCode terminator
    for (auto& e : ents) u16(sub, (uint16_t)((e.second - e.first) & 0xFFFF)); // idDelta
    u16(sub, 1);                               // idDelta terminator
    for (size_t i = 0; i < ents.size(); i++) u16(sub, 0); // idRangeOffset
    u16(sub, 0);                               // idRangeOffset terminator

    std::string cmap;
    u16(cmap, 0);   // version
    u16(cmap, 1);   // numTables
    u16(cmap, 3);   // platformID (Windows)
    u16(cmap, 1);   // encodingID (Unicode BMP)
    u32(cmap, 12);  // offset to subtable
    cmap += sub;
    return cmap;
}

std::string TTFParser::_assemble_sfnt(uint32_t sfntVersion, std::map<std::string, std::string>& tbls,
                                      const std::vector<std::string>& tag_order) {
    std::vector<std::string> final_tags;
    std::map<std::string, std::string> final_data;
    std::map<std::string, uint32_t> real_len;
    for (auto& tag : tag_order) {
        auto it = tbls.find(tag);
        if (it != tbls.end()) {
            final_tags.push_back(tag);
            final_data[tag] = it->second;
            real_len[tag] = (uint32_t)it->second.size();
        }
    }
    if (final_tags.empty()) return "";

    // The head checkSumAdjustment must be zero while table checksums are computed.
    if (final_data.count("head") && final_data["head"].size() >= 12) {
        _set_ushort(final_data["head"], 8, 0);
        _set_ushort(final_data["head"], 10, 0);
    }
    for (auto& kv : final_data)
        while (kv.second.size() % 4 != 0) kv.second += '\x00';

    auto calc_checksum = [](const std::string& data) -> uint32_t {
        uint32_t sum = 0;
        for (size_t i = 0; i < data.size(); i += 4) {
            uint32_t b0 = (i < data.size()) ? (uint8_t)data[i] : 0;
            uint32_t b1 = (i + 1 < data.size()) ? (uint8_t)data[i + 1] : 0;
            uint32_t b2 = (i + 2 < data.size()) ? (uint8_t)data[i + 2] : 0;
            uint32_t b3 = (i + 3 < data.size()) ? (uint8_t)data[i + 3] : 0;
            sum += (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
        }
        return sum;
    };

    uint16_t numTables = (uint16_t)final_tags.size();
    uint16_t es = 0;
    while ((uint16_t)(16 * (1 << (es + 1))) <= (uint16_t)(16 * numTables)) es++;
    uint16_t sr = (uint16_t)(16 * (1 << es));
    uint16_t rs = (uint16_t)(16 * numTables - sr);

    std::string offset_table;
    offset_table += (char)((sfntVersion >> 24) & 0xFF);
    offset_table += (char)((sfntVersion >> 16) & 0xFF);
    offset_table += (char)((sfntVersion >> 8) & 0xFF);
    offset_table += (char)(sfntVersion & 0xFF);
    offset_table += (char)((numTables >> 8) & 0xFF);
    offset_table += (char)(numTables & 0xFF);
    offset_table += (char)((sr >> 8) & 0xFF);
    offset_table += (char)(sr & 0xFF);
    offset_table += (char)((es >> 8) & 0xFF);
    offset_table += (char)(es & 0xFF);
    offset_table += (char)((rs >> 8) & 0xFF);
    offset_table += (char)(rs & 0xFF);

    uint32_t table_offset = 12u + 16u * numTables;
    std::string table_data_concat;
    for (auto& tag : final_tags) {
        auto& data = final_data[tag];
        uint32_t cs = calc_checksum(data);
        offset_table += tag;
        offset_table += (char)((cs >> 24) & 0xFF);
        offset_table += (char)((cs >> 16) & 0xFF);
        offset_table += (char)((cs >> 8) & 0xFF);
        offset_table += (char)(cs & 0xFF);
        offset_table += (char)((table_offset >> 24) & 0xFF);
        offset_table += (char)((table_offset >> 16) & 0xFF);
        offset_table += (char)((table_offset >> 8) & 0xFF);
        offset_table += (char)(table_offset & 0xFF);
        uint32_t len = real_len[tag];
        offset_table += (char)((len >> 24) & 0xFF);
        offset_table += (char)((len >> 16) & 0xFF);
        offset_table += (char)((len >> 8) & 0xFF);
        offset_table += (char)(len & 0xFF);
        table_offset += (uint32_t)data.size();
        table_data_concat += data;
    }

    uint32_t master_checksum = calc_checksum(offset_table) + calc_checksum(table_data_concat);
    uint32_t checkSumAdjustment = 0xB1B0AFBAu - master_checksum;
    if (final_data.count("head") && final_data["head"].size() >= 12) {
        _set_ushort(final_data["head"], 8, (uint16_t)((checkSumAdjustment >> 16) & 0xFFFF));
        _set_ushort(final_data["head"], 10, (uint16_t)(checkSumAdjustment & 0xFFFF));
    }

    std::string result = offset_table;
    for (auto& tag : final_tags) result += final_data[tag];
    return result;
}

std::string TTFParser::_build_otf_sfnt(const std::string& subset_cff) {
    std::string head = _get_table("head");
    std::string hhea = _get_table("hhea");
    std::string maxp = _get_table("maxp");
    std::string os2 = _get_table("OS/2");
    std::string name = _get_table("name");
    if (head.size() < 54 || hhea.size() < 36 || maxp.size() < 6 || name.empty())
        return ""; // required tables missing; caller falls back to the bare CFF

    uint16_t newNumGlyphs = (uint16_t)cffNewToOldGid.size();
    if (newNumGlyphs == 0) return "";

    // Rebuild hmtx for the new dense GID order from the original advances/lsb.
    std::string orig_hmtx = _get_table("hmtx");
    uint16_t orig_numHM = _get_ushort(hhea, 34);
    auto orig_adv = [&](uint16_t g) -> uint16_t {
        uint16_t idx = (g < orig_numHM) ? g : (orig_numHM ? (uint16_t)(orig_numHM - 1) : 0);
        size_t off = (size_t)idx * 4;
        return (off + 1 < orig_hmtx.size()) ? _get_ushort(orig_hmtx, off) : 0;
    };
    auto orig_lsb = [&](uint16_t g) -> int16_t {
        size_t off;
        if (g < orig_numHM) off = (size_t)g * 4 + 2;
        else off = (size_t)orig_numHM * 4 + (size_t)(g - orig_numHM) * 2;
        return (off + 1 < orig_hmtx.size()) ? (int16_t)_get_ushort(orig_hmtx, off) : 0;
    };
    std::string hmtx;
    hmtx.reserve((size_t)newNumGlyphs * 4);
    for (uint16_t ng = 0; ng < newNumGlyphs; ng++) {
        uint16_t og = cffNewToOldGid[ng];
        uint16_t a = orig_adv(og);
        int16_t l = orig_lsb(og);
        hmtx += (char)((a >> 8) & 0xFF);
        hmtx += (char)(a & 0xFF);
        hmtx += (char)(((uint16_t)l >> 8) & 0xFF);
        hmtx += (char)((uint16_t)l & 0xFF);
    }

    // maxp version 0.5 (CFF) with the new glyph count.
    std::string new_maxp = maxp.substr(0, 6);
    new_maxp[0] = 0x00; new_maxp[1] = 0x00; new_maxp[2] = 0x50; new_maxp[3] = 0x00;
    _set_ushort(new_maxp, 4, newNumGlyphs);

    // hhea with numberOfHMetrics == full glyph count (hmtx has one metric each).
    std::string new_hhea = hhea;
    _set_ushort(new_hhea, 34, newNumGlyphs);

    // Minimal post table, version 3.0 (no glyph names).
    std::string post(32, '\0');
    post[1] = 0x03; // version 0x00030000

    std::map<std::string, std::string> nt;
    nt["CFF "] = subset_cff;
    nt["cmap"] = _build_cmap4(cffUniToGid);
    nt["head"] = head;
    nt["hhea"] = new_hhea;
    nt["hmtx"] = hmtx;
    nt["maxp"] = new_maxp;
    nt["name"] = name;
    if (!os2.empty()) nt["OS/2"] = os2;
    nt["post"] = post;

    std::vector<std::string> tag_order = {"CFF ", "OS/2", "cmap", "head", "hhea",
                                          "hmtx", "maxp", "name", "post"};
    return _assemble_sfnt(0x4F54544Fu /* 'OTTO' */, nt, tag_order);
}

void TTFParser::Seek(const std::string& tag) {
    if (tables.find(tag) == tables.end())
        throw TTFParserException("Table not found: " + tag);
    f.seekg(tables[tag].first, std::ios::beg);
}

uint16_t TTFParser::ReadUShort() {
    uint8_t b1 = (uint8_t)f.get();
    uint8_t b2 = (uint8_t)f.get();
    return (b1 << 8) | b2;
}

int16_t TTFParser::ReadShort() {
    return (int16_t)ReadUShort();
}

uint32_t TTFParser::ReadULong() {
    uint8_t b1 = (uint8_t)f.get();
    uint8_t b2 = (uint8_t)f.get();
    uint8_t b3 = (uint8_t)f.get();
    uint8_t b4 = (uint8_t)f.get();
    return ((uint32_t)b1 << 24) | ((uint32_t)b2 << 16) | ((uint32_t)b3 << 8) | b4;
}

int32_t TTFParser::ReadLong() {
    return (int32_t)ReadULong();
}

std::string TTFParser::Read(int n) {
    if (n <= 0) return "";
    std::string s(n, '\0');
    f.read(&s[0], n);
    return s;
}

void TTFParser::Skip(int n) {
    f.seekg(n, std::ios::cur);
}

}
