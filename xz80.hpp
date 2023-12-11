#if not +0
#error "use -fno-operator-names"
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace Xz80 {
// =======================================================================
// 条件記号の定義

struct CondBase {
  const int id;
  const char* str;
  CondBase(const char* str, int id) : id(id), str(str) {}
};

/// JR および JP 命令で使用できる分岐条件
struct AllCond : public CondBase {
  AllCond(const char* str, int id) : CondBase(str, id) {}
};

/// JP 命令でのみ使用できる分岐条件
struct JpCond : public CondBase {
  JpCond(const char* str, int id) : CondBase(str, id) {}
};

// =======================================================================
// レジスタを表す型と変数の定義

struct RegC;

struct RegCAddr {
  const RegC& m_reg;
  RegCAddr(const RegC& c) : m_reg(c) {}
};

struct Reg8 {
  const int id;
  const char* str;
  Reg8(const char* str, int id = -1) : id(id), str(str) {}
};

struct BasicReg8 : public Reg8 {
  BasicReg8(const char* str, int id) : Reg8(str, id) {}
};

struct RegA : public BasicReg8 {
  RegA(const char* str, int id) : BasicReg8(str, id) {}
};

struct RegC : public BasicReg8 {
  RegC(const char* str, int id) : BasicReg8(str, id) {}
  RegCAddr operator()(void) const { return RegCAddr(*this); }
};

struct RegI : public BasicReg8 {
  RegI(const char* str) : BasicReg8(str, -1) {}
};

struct RegR : public BasicReg8 {
  RegR(const char* str) : BasicReg8(str, -1) {}
};

struct Reg16;
struct BasicReg16;
struct RegHL;
struct RegSP;
struct IndexReg16;

struct Reg16Addr {
  const Reg16& m_reg;
  Reg16Addr(const Reg16& reg16) : m_reg(reg16) {}
};

struct BasicReg16Addr {
  const BasicReg16& m_reg;
  BasicReg16Addr(const BasicReg16& reg16) : m_reg(reg16) {}
};

struct RegHLAddr {
  const RegHL& m_reg;
  RegHLAddr(const RegHL& reg16) : m_reg(reg16) {}
};

struct RegSPAddr {
  const RegSP& m_reg;
  RegSPAddr(const RegSP& reg16) : m_reg(reg16) {}
};

struct IndenexReg16AddrOffset {
  const IndexReg16& m_reg;
  const int8_t m_offset;
  IndenexReg16AddrOffset(const IndexReg16& reg16, int8_t offset)
      : m_reg(reg16), m_offset(offset) {}
};

struct IndenexReg16Addr {
  const IndexReg16& m_reg;
  IndenexReg16Addr(const IndexReg16& reg16) : m_reg(reg16) {}
};

struct Reg16 {
  const int id;
  const char* str;
  Reg16(const char* str, int id = -1) : id(id), str(str) {}
  Reg16Addr operator()(void) const { return Reg16Addr(*this); }
};

struct BasicReg16 : public Reg16 {
  BasicReg16(const char* str, int id) : Reg16(str, id) {}
  BasicReg16Addr operator()(void) const { return BasicReg16Addr(*this); }
};

struct RegHL : public Reg16 {
  RegHL(const char* str, int id) : Reg16(str, id) {}
  RegHLAddr operator()(void) const { return RegHLAddr(*this); }
};

struct RegAF : public Reg16 {
  RegAF(const char* str, int id) : Reg16(str, id) {}
};

struct RegBC : public BasicReg16 {
  RegBC(const char* str, int id) : BasicReg16(str, id) {}
};

struct RegDE : public BasicReg16 {
  RegDE(const char* str, int id) : BasicReg16(str, id) {}
};

struct RegSP : public BasicReg16 {
  RegSP(const char* str, int id) : BasicReg16(str, id) {}
  RegSPAddr operator()(void) const { return RegSPAddr(*this); }
};

struct IndexReg16 : public Reg16 {
  const uint8_t m_prefix;
  IndexReg16(const char* str, int id, uint8_t prefix)
      : Reg16(str, id), m_prefix(prefix) {}
  IndenexReg16AddrOffset operator()(int8_t offset) const {
    return IndenexReg16AddrOffset(*this, offset);
  }

  IndenexReg16Addr operator()(void) const { return IndenexReg16Addr(*this); }
};

struct IoAddr {
  const uint8_t addr;
  explicit IoAddr(uint8_t io_addr)
      : addr(io_addr) {}
};

struct MemAddr {
  const uint16_t addr;
  const uint8_t h;
  const uint8_t l;
  const std::string label;
  explicit MemAddr(uint16_t addr)
      : addr(addr),
        h(static_cast<uint8_t>(addr >> 8)),
        l(static_cast<uint8_t>(addr)),
        label() {}

  explicit MemAddr(const std::string& label)
      : addr(0), h(0), l(0), label(label) {}

  bool isLabel(void) const { return !label.empty(); }
};

// =======================================================================
// ニーモニックフォーマッター

namespace Formatter {
enum Type {
  T_End = '\0',
  T_Insn = 'i',        // str Insn.
  T_Label = 'l',       // str Label
  T_Symbol = 's',      // str Symbol
  T_Indirect = 'I',    // str/word (nn) Indirect
  T_Reg = 'r',         // reg
  T_Condition = 'c',   // str condition
  T_Comma = ',',       // no-arg
  T_Dash = '\'',       // no-arg
  T_Dec = 'd',         // decimal number
  T_Hex = 'x',         // hex number
  T_AddrOffset = 'o',  // $+/-d number
  T_Bytes = 'b',       // bytes
  T_Words = 'w',       // words
  T_Text = 't',        // str
  T_Unknown = '?',     // ignore
};

class Formatter {
  const char* m_format;
  const char* m_p;
  std::string m_buffer;

  Type nextType(void) const {
    // return static_cast<Type>(*m_p);
    switch (*m_p) {
      case '\0':
        return T_End;
      case 'i':
        return T_Insn;
      case 'l':
        return T_Label;
      case 's':
        return T_Symbol;
      case 'I':
        return T_Indirect;
      case 'r':
        return T_Reg;
      case 'c':
        return T_Condition;
      case ',':
        return T_Comma;
      case '\'':
        return T_Dash;
      case 'd':
        return T_Dec;
      case 'x':
        return T_Hex;
      case 'o':
        return T_AddrOffset;
      case 'b':
        return T_Bytes;
      case 'w':
        return T_Words;
      case 't':
        return T_Text;
      default:
        break;
    }
    return T_Unknown;
  }

  void reduceNoArg(void) {
    while (true) {
      switch (nextType()) {
        case T_Comma:
          ++m_p;
          m_buffer.append(", ");
          break;

        case T_Dash:
          ++m_p;
          m_buffer.append("'");
          break;

        case T_Unknown:
          ++m_p;
          break;

        default:
          return;
      }  // switch
    }    // while
  }

 public:
  Formatter(const char* format)
      : m_format(format),
        m_p(&m_format[0]),
        m_buffer() {
    reduceNoArg();
  }

  const std::string& str(void) const { return m_buffer; }

  Formatter operator%(const std::string& str) {
    return *this % str.c_str();
  }

  Formatter operator%(const char* str) {
    switch (nextType()) {
      case T_Insn:
        ++m_p;
        m_buffer.append("    ").append(str).append(" ");
        break;

      case T_Label:
        ++m_p;
        m_buffer.append(str).append(":");
        break;

      case T_Symbol:
        ++m_p;
        m_buffer.append(str);
        break;

      case T_Text: {
        ++m_p;
        std::string s;
        bool outside = true;
        for (const char* p = str; *p != '\0'; ++p) {
          if (std::isprint(*p) && *p != '\'') {
            if (outside) {
              s.append(", '");
              outside = false;
            }
            const char b[2] = {*p, 0};
            s.append(b);
          } else {
            if (!outside) {
              s.append("'");
              outside = true;
            }
            char buf[8];
            std::sprintf(buf, ", 0%xh", (0xff & *p));
            s.append(buf);
          }
        }  // for
        if (!outside) {
          s.append("'");
        }
        if (s.empty()) {
          s = ", ''";
        }
        m_buffer.append(s.begin() + 2, s.end());

        break;
      }

      default:
        throw std::invalid_argument("const char*不正な組み合わせ");
        break;
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const Reg8& r) {
    switch (nextType()) {
      case T_Reg:
        ++m_p;
        m_buffer.append(r.str);
        break;
      default:
        throw std::invalid_argument("Reg8不正な組み合わせ");
        break;
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const RegCAddr& c) {
    switch (nextType()) {
      case T_Reg:
        ++m_p;
        m_buffer.append("(").append(c.m_reg.str).append(")");
        break;
      default:
        throw std::invalid_argument("Reg8不正な組み合わせ");
        break;
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const Reg16& r) {
    switch (nextType()) {
      case T_Reg:
        ++m_p;
        m_buffer.append(r.str);
        break;

      default:
        throw std::invalid_argument("Reg16不正な組み合わせ");
        break;
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const BasicReg16Addr& rp) {
    if (nextType() == T_Reg) {
      ++m_p;
      m_buffer.append("(").append(rp.m_reg.str).append(")");
    } else {
      throw std::invalid_argument("BasicReg16Addr不正な組み合わせ");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const RegHLAddr& hl) {
    if (nextType() == T_Reg) {
      ++m_p;
      m_buffer.append("(").append(hl.m_reg.str).append(")");
    } else {
      throw std::invalid_argument("RegHLAddr不正な組み合わせ");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const RegSPAddr& sp) {
    if (nextType() == T_Reg) {
      ++m_p;
      m_buffer.append("(").append(sp.m_reg.str).append(")");
    } else {
      throw std::invalid_argument("RegSPAddr不正な組み合わせ");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const IndenexReg16AddrOffset& idx_offset) {
    if (nextType() == T_Reg) {
      ++m_p;
      const int ofs = idx_offset.m_offset;
      char buf[16];
      if (ofs < 0) {
        std::sprintf(buf, "-0%xh)", -ofs);
      } else {
        std::sprintf(buf, "+0%xh)", ofs);
      }
      m_buffer.append("(").append(idx_offset.m_reg.str).append(buf);
    } else {
      throw std::invalid_argument("IndenexReg16AddrOffset不正な組み合わせ");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const IndenexReg16Addr& idx) {
    if (nextType() == T_Reg) {
      ++m_p;
      m_buffer.append("(").append(idx.m_reg.str).append(")");
    } else {
      throw std::invalid_argument("IndenexReg16Addr不正な組み合わせ");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const IoAddr& io) {
    if (nextType() == T_Indirect) {
      ++m_p;
      char buf[8];
      std::sprintf(buf, "(0%xh)", (0xff & io.addr));
      m_buffer.append(buf);
    } else {
      throw std::invalid_argument("IoAddr不正な組み合わせ");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const CondBase& cc) {
    if (nextType() == T_Condition) {
      ++m_p;
      m_buffer.append(cc.str);
    } else {
      throw std::invalid_argument("CondBase不正な組み合わせ");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(int n) {
    char buf[16];
    switch (nextType()) {
      case T_Dec:
        ++m_p;
        std::sprintf(buf, "%d", n);
        m_buffer.append(buf);
        break;

      case T_Hex:
        ++m_p;
        std::sprintf(buf, "0%xh", n);
        m_buffer.append(buf);
        break;

      case T_AddrOffset:
        ++m_p;
        std::sprintf(buf, "%+d", n);
        m_buffer.append("$").append(buf);
        break;

      default:
        throw std::invalid_argument("");
        break;
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const std::initializer_list<uint8_t>& bytes) {
    if (nextType() == T_Bytes) {
      ++m_p;
      std::string s;
      for (uint8_t b : bytes) {
        char buf[8];
        std::sprintf(buf, ", 0%xh", 0xff & b);
        s.append(buf);
      }
      m_buffer.append(s.begin() + 2, s.end());
    } else {
      throw std::invalid_argument("");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const std::initializer_list<uint16_t>& words) {
    if (nextType() == T_Words) {
      ++m_p;
      std::string s;
      for (uint16_t w : words) {
        char buf[12];
        std::sprintf(buf, ", 0%xh", 0xffff & w);
        s.append(buf);
      }
      m_buffer.append(s.begin() + 2, s.end());
    } else {
      throw std::invalid_argument("");
    }
    reduceNoArg();
    return *this;
  }

  Formatter operator%(const MemAddr& nn) {
    if (nextType() == T_Indirect) {
      ++m_p;
      if (nn.isLabel()) {
        m_buffer.append("(").append(nn.label).append(")");
      } else {
        char buf[16];
        std::sprintf(buf, "(0%xh)", nn.addr);
        m_buffer.append(buf);
      }
    } else {
      throw std::invalid_argument("");
    }
    reduceNoArg();
    return *this;
  }

#if 0
  //テンプレ
  Formatter operator%(const char* str) {
    switch (nextType()) {
      case T_End:
      case T_Insn:
      case T_Reg:
      case T_Comma:
      case T_Dec:
      case T_Hex:
      case T_Bytes:
      case T_Words:
      case T_Text:
      default:
        throw std::invalid_argument("");
        break;
    }
    reduceNoArg();
    return *this;
  }
#endif
};

}  // namespace Formatter

// =======================================================================
// コードジェネレータ

class Mnemonic {
  uint16_t m_addr;
  std::string m_mnemonic;
  std::vector<uint8_t> m_bytes;
  std::string m_label;
  size_t m_offset;
  bool m_rel;  ///< 「ラベル解決時に相対アドレスとして解決する」フラグ

 public:
  Mnemonic(uint16_t addr, const std::string& mnemonic,
           const std::vector<uint8_t>& bytes, const char* label = "",
           size_t offset = 0)
      : m_addr(addr),
        m_mnemonic(mnemonic),
        m_bytes(bytes),
        m_label(label),
        m_offset(offset),
        m_rel(false) {}

  uint16_t getAddr(void) const { return m_addr; }
  const std::string& getMnemonic(void) const { return m_mnemonic; }
  const std::vector<uint8_t>& getBytes(void) const { return m_bytes; }
  const std::string& getLabel(void) const { return m_label; }
  size_t getOffset(void) const { return m_offset; }

  void setLabel(const char* label, size_t offset, bool rel) {
    m_label = label;
    m_offset = offset;
    m_rel = rel;
  }
  void setLabel(const std::string& label, size_t offset, bool rel) {
    m_label = label;
    m_offset = offset;
    m_rel = rel;
  }

  void resolveAddr(uint16_t addr) {
    if (m_rel) {
      const int e = static_cast<int>(addr) - static_cast<int>(m_addr);
      if (e < -126 || 129 < e) {
        char buf[64];
        std::sprintf(buf, "Label resolve e=%d:out of range", e);
        throw std::out_of_range(buf);
      }
      m_bytes[m_offset] = static_cast<uint8_t>((e - 2) & 0xff);
    } else {
      MemAddr ma(addr);
      m_bytes[m_offset] = ma.l;
      m_bytes[m_offset + 1] = ma.h;
    }
    m_label.clear();
    m_offset = 0;
  }
};

class Generator {
  typedef Formatter::Formatter Fmt;
  const uint16_t m_org;
  uint16_t m_curr;
  std::vector<Mnemonic> m_mnemonics;

  /// ラベルの定義マップ
  std::map<std::string, uint16_t> m_labelMap;

 protected:
  const RegA A;
  const BasicReg8 B;
  const RegC C;
  const BasicReg8 D;
  const BasicReg8 E;
  const Reg8 F;
  const BasicReg8 H;
  const BasicReg8 L;
  const RegI I;
  const RegR R;
  const Reg8 IXH;
  const Reg8 IXL;
  const Reg8 IYH;
  const Reg8 IYL;

  const RegAF AF;
  const RegBC BC;
  const RegDE DE;
  const RegHL HL;
  const IndexReg16 IX;
  const IndexReg16 IY;
  const RegSP SP;

  AllCond NZ;  ///< Not Zero
  AllCond Z;   ///< Zero
  AllCond NC;  ///< Not Carry
  AllCond Cy;  ///< Carry *Cレジスタと名前が被るのでCyとした};
  JpCond PO;   ///< Parity Odd
  JpCond PE;   ///< Parity Even
  JpCond P;    ///< Plus
  JpCond M;    ///< Minus

 private:
  static uint8_t build(uint8_t a, const Reg8& d, const Reg8& s) {
    const auto ret = a << 6 | d.id << 3 | s.id;
    return ret;
  }
  static uint8_t build(uint8_t a, const Reg16& rp, uint8_t b) {
    const auto ret = a << 6 | rp.id << 4 | b;
    return ret;
  }
  static uint8_t build(uint8_t a, int d, const Reg16& s) {
    const auto ret = a << 6 | d << 3 | s.id;
    return ret;
  }
  static uint8_t build(uint8_t a, CondBase cc, uint8_t b) {
    const auto ret = a << 6 | cc.id << 3 | b;
    return ret;
  }
  static uint8_t build(uint8_t a, AllCond cc) {
    const auto ret = a | cc.id << 3;
    return ret;
  }

  void append(const std::string& mnemonic, const std::vector<uint8_t>& bytes) {
    Mnemonic m(m_curr, mnemonic, bytes);
    m_curr += bytes.size();
    m_mnemonics.push_back(m);
  }

  void append(const std::string& mnemonic) {
    Mnemonic m(m_curr, mnemonic, std::vector<uint8_t>());
    m_mnemonics.push_back(m);
  }

  void append(const Fmt& mnemonic) {
    Mnemonic m(m_curr, mnemonic.str(), std::vector<uint8_t>());
    m_mnemonics.push_back(m);
  }

  void append(const Fmt& mnemonic,
              const std::vector<uint8_t>& bytes) {
    append(mnemonic.str(), bytes);
  }
  void append(const Fmt& mnemonic, uint8_t byte) {
    append(mnemonic.str(), std::vector<uint8_t>{byte});
  }

  /// アドレス解決用の情報を登録する
  void resolve(const char* label, size_t offset, bool rel = false) {
    m_mnemonics.rbegin()->setLabel(label, offset, rel);
  }
  /// アドレス解決用の情報を登録する
  void resolve(const std::string& label, size_t offset, bool rel = false) {
    m_mnemonics.rbegin()->setLabel(label, offset, rel);
  }

 public:
  Generator(uint16_t org = 0x100)
      : m_org(org),
        m_curr(org),
        m_mnemonics(),  //
        A("A", 7),
        B("B", 0),
        C("C", 1),
        D("D", 2),
        E("E", 3),
        F("F", 6),
        H("H", 4),
        L("L", 5),
        I("I"),
        R("R"),
        IXH("IXH"),
        IXL("IXL"),
        IYH("IYH"),
        IYL("IYL"),
        AF("AF", -1),
        BC("BC", 0),
        DE("DE", 1),
        HL("HL", 2),
        IX("IX", 2, 0b1101'1101),
        IY("IY", 2, 0b1111'1101),
        SP("SP", 3),
        NZ("NZ", 0),
        Z("Z", 1),
        NC("NC", 2),
        Cy("C", 3),
        PO("PO", 4),
        PE("PE", 5),
        P("P", 6),
        M("M", 7) {}

  void dump() const {
    std::printf("ORG 0100h\n");
    for (const auto& m : m_mnemonics) {
      std::string s;
      char buf[8];
      for (const auto b : m.getBytes()) {
        std::sprintf(buf, "%02x ", b);
        s += buf;
      }
      std::printf("%-20s\t;%04Xh(%+d): %s\n",  //
                  m.getMnemonic().c_str(), m.getAddr(), m.getAddr() - m_org,
                  s.c_str());
    }
  }

  /// 生成されたコードをベタ形式でファイルに保存する
  void save(const char* fn) const {
    FILE* fp = fopen(fn, "wb");
    for (const auto& m : m_mnemonics) {
      const auto& bs = m.getBytes();
      std::fwrite(bs.data(), 1, bs.size(), fp);
    }
    fclose(fp);
  }

  /// 生成されたコードをMSXのBSAVE形式でファイルに保存する
  void bsave(const char* fn, uint16_t start_addr = 0x0000) {
    FILE* fp = fopen(fn, "wb");
    const auto wb = [&](uint8_t val)  // WriteByte
    { std::fwrite(&val, 1, 1, fp); };
    const auto ww = [&](uint16_t val)  // WriteWord
    {
      wb(val & 0xff);
      wb((val >> 8) & 0xff);
    };

    // ヘッダの出力
    wb(0xfe);
    ww(this->m_org);
    ww(this->m_curr - 1);
    ww(start_addr);

    // バイナリデータ本体の出力
    for (const auto& m : m_mnemonics) {
      const auto& bs = m.getBytes();
      std::fwrite(bs.data(), 1, bs.size(), fp);
    }

    fclose(fp);
  }

  /// ラベルのアドレス解決
  bool resolve(bool verbose = false) {
    int numError = 0;
    if (verbose) {
      std::printf(";\x1b[1;36mLabel address resolve..\x1b[0m\n");
    }
    for (auto& m : m_mnemonics) {
      if (m.getLabel().empty()) {
        continue;
      }

      const auto itr = this->m_labelMap.find(m.getLabel());
      if (itr == m_labelMap.end()) {
        // 解決不能なラベルだった
        if (verbose) {
          std::printf(
              ";0%04xh: %-20s\t;\x1b[1;31mLabel '%s' is not resolved.\x1b[0m\n",  //
              m.getAddr(), m.getMnemonic().c_str(), m.getLabel().c_str());
        }
        ++numError;
        continue;
      }

      // アドレスを埋め込む
      if (verbose) {
        std::printf(
            ";0%04xh: %-20s\t;\x1b[1;32mLabel '%s' = 0%04xh\x1b[0m\n",  //
            m.getAddr(), m.getMnemonic().c_str(), m.getLabel().c_str(),
            itr->second);
      }
      m.resolveAddr(itr->second);
    }  // for

    if (verbose) {
      if (numError != 0) {
        std::printf(";\x1b[1;36m%d unresolved mnemonic(s) found.\x1b[0m\n",
                    numError);
      } else {
        std::printf(";\x1b[1;36mAll mnemonic labels resolved.\x1b[0m\n");
      }
    }
    return numError == 0;
  }

  // -------------------------------------------------------------------
  // ニーモニック・疑似命令の実装

 protected:
  MemAddr mem(uint16_t addr) { return MemAddr(addr); }
  MemAddr mem(const char* label) { return MemAddr(label); }
  MemAddr mem(const std::string& label) { return MemAddr(label); }

  IoAddr io(uint8_t n) { return IoAddr(n); }

  // =========================================================================
  // 疑似命令

  /// DB byte | constant8
  void db(uint8_t byte) {
    append(Fmt("i b") % "DB" % std::initializer_list<uint8_t>{byte},  //
           byte);
  }

  /// DB byte | constant8 ...
  void db(std::initializer_list<uint8_t> bytes) {
    append(Fmt("i b") % "DB" % bytes,  //
           bytes);
  }

  /// DB byte | constant8 ...
  void db(const char* str) {
    const std::vector<uint8_t> bs(str, str + std::strlen(str));
    append(Fmt("i t") % "DB" % str,  //
           bs);
  }

  /// DW word | constant16
  void dw(uint16_t word) {
    const MemAddr m(word);
    append(Fmt("ix") % "DW" % word,  //
           {m.l, m.h});
  }

  /// DB word | constant16 ...
  void dw(std::initializer_list<uint16_t> words) {
    std::vector<uint8_t> bs;
    for (const uint16_t w : words) {
      const MemAddr m(w);
      bs.push_back(m.l);
      bs.push_back(m.h);
    }
    append(Fmt("i w") % "DW" % words,  //
           bs);
  }

  /// DB label | label ...
  void dw(std::string label) {
    append(Fmt("i s") % "DW" % label, {0x00, 0x00});
    resolve(label.c_str(), 0);
  }

  /// DB label | label ...
  void dw(std::initializer_list<const std::string> labels) {
    for (const std::string& l : labels) {
      append(Fmt("i s") % "DW" % l, {0x00, 0x00});
      resolve(l.c_str(), 0);
    }
  }

  /// $ | Get current address.
  uint16_t curr(void) const { return this->m_curr; }

  /// Label
  uint16_t l(const char* label) {
    m_labelMap.insert(std::make_pair(std::string(label), m_curr));
    append(Fmt("l") % label);
    return m_curr;
  }

  // =========================================================================
  // 8ビット転送命令

  /// LD r1, r2 | reg8 <- reg8
  void ld(const BasicReg8& r1, const BasicReg8& r2) {
    append(Fmt("i r,r") % "LD" % r1 % r2,  //
           build(0b01, r1, r2));
  }

  /// LD r, n | reg8 <- constant8
  void ld(const BasicReg8& r, uint8_t n) {
    append(Fmt("i r,x") % "LD" % r % n,  //
           {build(0b00, r, F), n});
  }

  /// LD r, (HL) | reg8 <- mem[HL]
  void ld(const BasicReg8& r, const RegHLAddr& hl_addr) {
    append(Fmt("i r,r") % "LD" % r % hl_addr,  //
           build(0b01, r, F));
  }

  /// LD r,(indexreg16+offset) | reg8 <- mem[(IX or IY)+offset8]
  void ld(const BasicReg8& r, const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r,r") % "LD" % r % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b01, r, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// LD (HL), r | mem[HL] <- reg8
  void ld(const RegHLAddr& hl_addr, const BasicReg8& r) {
    append(Fmt("i r,r") % "LD" % hl_addr % r,  //
           build(0b01, F, r));
  }

  /// LD (indexreg16+offset), r |  mem[(IX or IY)+offset8] <- reg8
  void ld(const IndenexReg16AddrOffset& ireg16_offset, const BasicReg8& r) {
    append(Fmt("i r,r") % "LD" % ireg16_offset % r,  //
           {ireg16_offset.m_reg.m_prefix, build(0b01, F, r),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// LD (HL), r | mem[HL] <- constant8
  void ld(const RegHLAddr& hl_addr, uint8_t n) {
    append(Fmt("i r,x") % "LD" % hl_addr % n,  //
           {build(0b00, F, F), n});
  }

  /// LD (indexreg16+offset), n |  mem[(IX or IY)+offset8] <- constant8
  void ld(const IndenexReg16AddrOffset& ireg16_offset, uint8_t n) {
    append(Fmt("i r,x") % "LD" % ireg16_offset % n,  //
           {ireg16_offset.m_reg.m_prefix, build(0b00, F, F),
            static_cast<uint8_t>(ireg16_offset.m_offset), n});
  }

  /// LD A, (BC or DE) | A <- mem[BC or DE]
  void ld(const RegA& a, const BasicReg16Addr& rr) {
    append(Fmt("i r,r") % "LD" % a % rr,  //
           build(0b00, rr.m_reg, 0b1010));
  }

  /// LD A, (nn) | A <- mem[constant16]
  void ld(const RegA& a, const MemAddr& nn) {
    if (nn.isLabel()) {
      append(Fmt("i r,I") % "LD" % a % nn,  //
             {build(0b00, A, D), nn.l, nn.h});
      resolve(nn.label, 1);
    } else {
      append(Fmt("i r,I") % "LD" % a % nn,  //
             {build(0b00, A, D), nn.l, nn.h});
    }
  }

  /// LD (BC or DE), A | mem[BC or DE] <- A
  void ld(const BasicReg16Addr& rr, const RegA& a) {
    append(Fmt("i r,r") % "LD" % rr % a,  //
           {build(0b00, rr.m_reg, 0b0010)});
  }

  /// LD (nn), A | mem[constant16] <- A
  void ld(const MemAddr& nn, const RegA& a) {
    append(Fmt("i I,r") % "LD" % nn % a,  //
           {build(0b00, F, D), nn.l, nn.h});
  }

  /// LD A, I | A <- I
  void ld(const RegA& a, const RegI& i) {
    append(Fmt("i r,r") % "LD" % a % i,  //
           {0xed, 0x57});
  }

  /// LD I, A | I <- A
  void ld(const RegI& i, const RegA& a) {
    append(Fmt("i r,r") % "LD" % i % a,  //
           {0xed, 0x47});
  }

  /// LD A, R | A <- R
  void ld(const RegA& a, const RegR& r) {
    append(Fmt("i r,r") % "LD" % a % r,  //
           {0xed, 0x5f});
  }

  /// LD R, A | R <- A
  void ld(const RegR& r, const RegA& a) {
    append(Fmt("i r,r") % "LD" % r % a,  //
           {0xed, 0x4f});
  }

  // =========================================================================
  // 16ビット転送命令

  /// LD rp, nn | reg16 <- constant16
  void ld(const Reg16& rp, uint16_t nn) {
    MemAddr m(nn);
    append(Fmt("i r,x") % "LD" % rp % nn,  //
           {build(0b00, rp, 0b0001), m.l, m.h});
  }
  void ld(const Reg16& rp, const std::string& label) {
    append(Fmt("i r,s") % "LD" % rp % label,  //
           {build(0b00, rp, 0b0001), 0x00, 0x00});
    resolve(label, 1);
  }

  /// LD indexreg16, nn | indexreg16 <- constant16
  void ld(const IndexReg16& rp, uint16_t nn) {
    MemAddr m(nn);
    append(Fmt("i r,x") % "LD" % rp % nn,  //
           {rp.m_prefix, build(0b00, HL, 0b0001), m.l, m.h});
  }
  void ld(const IndexReg16& rp, const std::string& label) {
    append(Fmt("i r,s") % "LD" % rp % label,  //
           {rp.m_prefix, build(0b00, HL, 0b0001), 0x00, 0x00});
    resolve(label, 2);
  }

  /// LD HL, (nn) | reg16 <- mem[constant16]
  void ld(const RegHL& hl, const MemAddr& nn) {
    append(Fmt("i r,I") % "LD" % hl % nn,  //
           {build(0b00, hl, 0b1010), nn.l, nn.h});
    if (nn.isLabel()) {
      resolve(nn.label, 1);
    }
  }

  /// LD rp, (nn) | reg16 <- mem[constant16]
  void ld(const BasicReg16& rp, const MemAddr& nn) {
    append(Fmt("i r,I)") % "LD" % rp % nn,  //
           {0xed, build(0b01, rp, 0b1011), nn.l, nn.h});
    if (nn.isLabel()) {
      resolve(nn.label, 2);
    }
  }

  /// LD IX or IY, (nn) | indexreg16 <- mem[constant16]
  void ld(const IndexReg16& rp, const MemAddr& nn) {
    append(Fmt("i r,I") % "LD" % rp % nn,  //
           {rp.m_prefix, build(0b00, rp, 0b1010), nn.l, nn.h});
    if (nn.isLabel()) {
      resolve(nn.label, 2);
    }
  }

  /// LD (nn), HL | mem[constant16] <- reg16
  void ld(const MemAddr& nn, const RegHL& hl) {
    append(Fmt("i I,r") % "LD" % nn % hl,  //
           {build(0b00, hl, 0b0010), nn.l, nn.h});
    if (nn.isLabel()) {
      resolve(nn.label, 1);
    }
  }

  /// LD (nn), rp | mem[constant16] <- reg16
  void ld(const MemAddr& nn, const BasicReg16& rp) {
    append(Fmt("i I,r") % "LD" % nn % rp,  //
           {0xed, build(0b01, rp, 0b0011), nn.l, nn.h});
    if (nn.isLabel()) {
      resolve(nn.label, 2);
    }
  }

  /// LD (nn), IX or IY | mem[constant16] <- indexreg16
  void ld(const MemAddr& nn, const IndexReg16& rp) {
    append(Fmt("i I,r") % "LD" % nn % rp,  //
           {rp.m_prefix, build(0b00, rp, 0b0010), nn.l, nn.h});
    if (nn.isLabel()) {
      resolve(nn.label, 2);
    }
  }

  /// LD SP, HL | SP <- HL
  void ld(const RegSP& sp, const RegHL& hl) {
    append(Fmt("i r,r") % "LD" % sp % hl,  //
           {0b11111001});
  }

  /// LD SP, IX or IY | SP <- IX or IY
  void ld(const RegSP& sp, const IndexReg16& rp) {
    append(Fmt("i r,r") % "LD" % sp % rp,  //
           {rp.m_prefix, 0b11111001});
  }

  // =========================================================================
  // ブロック転送命令

  /// LDI | mem[DE++] <- mem[HL++]; --BC;
  void ldi(void) {
    append(Fmt("i") % "LDI",  //
           {0b1110'1101, 0b1010'0000});
  }

  /// LDIR | while(BC!=0) { mem[DE++] <- mem[HL++]; --BC; }
  void ldir(void) {
    append(Fmt("i") % "LDIR",  //
           {0b1110'1101, 0b1011'0000});
  }

  /// LDD | mem[DE--] <- mem[HL--]; --BC;
  void ldd(void) {
    append(Fmt("i") % "LDD",  //
           {0b1110'1101, 0b1010'1000});
  }

  /// LDDR | while(BC!=0) { mem[DE--] <- mem[HL--]; --BC; }
  void lddr(void) {
    append(Fmt("i") % "LDDR",  //
           {0b1110'1101, 0b1011'1000});
  }

  // =========================================================================
  // 交換命令

  /// EX DE, HL | DE <=> HL
  void ex(const RegDE& de, const RegHL& hl) {
    append(Fmt("i r,r") % "EX" % de % hl,  //
           {0b1110'1011});
  }

  /// EX AF, AF' | AF <=> AF'
  void ex(const RegAF& af, const RegAF& afd) {
    append(Fmt("i r,r'") % "EX" % af % afd,  //
           {0b0000'1000});
  }

  /// EXX | (BC, DE, HL) <=> (BC', DE', HL')
  void exx(void) {
    append(Fmt("i") % "EXX",  //
           {0b1101'1001});
  }

  /// EX (SP), HL | mem[SP] <=> L; mem[SP+1] <=> H;
  void ex(const RegSPAddr& sp, const RegHL& hl) {
    append(Fmt("i r,r") % "EX" % sp % hl,  //
           {0b1110'0011});
  }

  /// EX (SP), IX or IY | mem[SP] <=> IXL or IYL; mem[SP+1] <=> IXH or IYH;
  void ex(const RegSPAddr& sp, const IndexReg16& rp) {
    append(Fmt("i r,r") % "EX" % sp % rp,  //
           {rp.m_prefix, 0b1110'0011});
  }

  // =========================================================================
  // スタック操作命令

 private:
  void push_rp_impl(const Reg16& rp) {
    append(Fmt("i r") % "PUSH" % rp,  //
           {build(0b11, rp, 0b0101)});
  }
  void pop_rp_impl(const Reg16& rp) {
    append(Fmt("i r") % "POP" % rp,  //
           {build(0b11, rp, 0b0001)});
  }

 public:
  /// PUSH BC | mem[SP-1] <- B; mem[SP-2] <- C; SP-= 2;
  void push(const RegBC& bc) { push_rp_impl(bc); }

  /// PUSH DE | mem[SP-1] <- D; mem[SP-2] <- E; SP-= 2;
  void push(const RegDE& de) { push_rp_impl(de); }

  /// PUSH HL | mem[SP-1] <- H; mem[SP-2] <- L; SP-= 2;
  void push(const RegHL& hl) { push_rp_impl(hl); }

  /// PUSH AF | mem[SP-1] <- A; mem[SP-2] <- F; SP-= 2;
  void push(const RegAF& af) { push_rp_impl(af); }

  /// PUSH IX or IY | mem[SP-1] <- IXH or IYH; mem[SP-2] <- IXL or IYL; SP-= 2;
  void push(const IndexReg16& rp) {
    append(Fmt("i r") % "PUSH" % rp,  //
           {rp.m_prefix, build(0b11, rp, 0b0101)});
  }

  /// POP BC | B <- mem[SP]; C <- mem[SP+1]; SP+= 2;
  void pop(const RegBC& bc) { pop_rp_impl(bc); }

  /// POP DE | D <- mem[SP]; E <- mem[SP+1]; SP+= 2;
  void pop(const RegDE& de) { pop_rp_impl(de); }

  /// POP HL | H <- mem[SP]; L <- mem[SP+1]; SP+= 2;
  void pop(const RegHL& hl) { pop_rp_impl(hl); }

  /// POP AF | A <- mem[SP]; F <- mem[SP+1]; SP+= 2;
  void pop(const RegAF& af) { pop_rp_impl(af); }

  /// POP IX or IY | IXH or IYH <- mem[SP]; IXL or IYL <- mem[SP+1]; SP+= 2;
  void pop(const IndexReg16& rp) {
    append(Fmt("i r") % "POP" % rp,  //
           {rp.m_prefix, build(0b11, rp, 0b0001)});
  }

  // =========================================================================
  // ローテート・シフト命令

  // ---------------------------------
  // 左巡回シフト命令

  /// RLCA |
  void rlca(void) {
    append(Fmt("i") % "RLCA",  //
           {0b0000'0111});
  }

  /// RLA |
  void rla(void) {
    append(Fmt("i") % "RLA",  //
           {0b0001'0111});
  }

  /// RLC r |
  void rlc(const BasicReg8& r) {
    append(Fmt("i r") % "RLC" % r,  //
           {0b1100'1011, build(0b00, B, r)});
  }

  /// RLC (HL) |
  void rlc(const RegHLAddr& hl) {
    append(Fmt("i r") % "RLC" % hl,  //
           {0b1100'1011, 0b0000'0110});
  }

  /// RLC (IX or IY + d) |
  void rlc(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "RLC" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset), 0b0000'0110});
  }

  /// RL r |
  void rl(const BasicReg8& r) {
    append(Fmt("i r") % "RL" % r,  //
           {0b1100'1011, build(0b00, D, r)});
  }

  /// RL (HL) |
  void rl(const RegHLAddr& hl) {
    append(Fmt("i r") % "RL" % hl,  //
           {0b1100'1011, build(0b00, D, F)});
  }

  /// RL (IX or IY + d) |
  void rl(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "RL" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset), 0b0001'0110});
  }

  // ---------------------------------
  // 右巡回シフト命令

  /// RRCA |
  void rrca(void) {
    append(Fmt("i") % "RRCA",  //
           {0b0000'1111});
  }

  /// RRA |
  void rra(void) {
    append(Fmt("i") % "RRA",  //
           {0b0001'1111});
  }

  /// RRC r |
  void rrc(const BasicReg8& r) {
    append(Fmt("i r") % "RRC" % r,  //
           {0b1100'1011, build(0b00, C, r)});
  }

  /// RRC (HL) |
  void rrc(const RegHLAddr& hl) {
    append(Fmt("i r") % "RRC" % hl,  //
           {0b1100'1011, build(0b00, C, F)});
  }

  /// RRC (IX or IY + d) |
  void rrc(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "RRC" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset), build(0b00, C, F)});
  }

  /// RR r |
  void rr(const BasicReg8& r) {
    append(Fmt("i r") % "RR" % r,  //
           {0b1100'1011, build(0b00, E, r)});
  }

  /// RR (HL) |
  void rr(const RegHLAddr& hl) {
    append(Fmt("i r") % "RR" % hl,  //
           {0b1100'1011, build(0b00, E, F)});
  }

  /// RR (IX or IY + d) |
  void rr(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "RR" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset), build(0b00, E, F)});
  }

  // ---------------------------------
  // 左シフト命令

  /// SLA r |
  void sla(const BasicReg8& r) {
    append(Fmt("i r") % "SLA" % r,  //
           {0b1100'1011, build(0b00, H, r)});
  }

  /// SLA (HL) |
  void sla(const RegHLAddr& hl) {
    append(Fmt("i r") % "SLA" % hl,  //
           {0b1100'1011, build(0b00, H, F)});
  }

  /// SLA (IX or IY + d) |
  void sla(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "SLA" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset), build(0b00, H, F)});
  }

  // ---------------------------------
  // 右シフト命令

  /// SRA r |
  void sra(const BasicReg8& r) {
    append(Fmt("i r") % "SRA" % r,  //
           {0b1100'1011, build(0b00, L, r)});
  }

  /// SRA (HL) |
  void sra(const RegHLAddr& hl) {
    append(Fmt("i r") % "SRA" % hl,  //
           {0b1100'1011, build(0b00, L, F)});
  }

  /// SRA (IX or IY + d) |
  void sra(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "SRA" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset), build(0b00, L, F)});
  }

  /// SRL r |
  void srl(const BasicReg8& r) {
    append(Fmt("i r") % "SRL" % r,  //
           {0b1100'1011, build(0b00, A, r)});
  }

  /// SRL (HL) |
  void srl(const RegHLAddr& hl) {
    append(Fmt("i r") % "SRL" % hl,  //
           {0b1100'1011, build(0b00, A, F)});
  }
  /// SRL (IX or IY + d) |
  void srl(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "SRL" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset), build(0b00, A, F)});
  }

  // =========================================================================
  // 8ビット算術演算

  // ---------------------------------
  // 加算・インクリメント命令

  /// ADD A, r | A <- A + reg8
  void add(const RegA& a, const BasicReg8& r) {
    append(Fmt("i r,r") % "ADD" % a % r,  //
           {build(0b10, B, r)});
  }

  /// ADD A, n | A <- A + constant8
  void add(const RegA& a, uint8_t n) {
    append(Fmt("i r,x") % "ADD" % a % n,  //
           {build(0b11, B, F), n});
  }

  /// ADD A, (HL) | A <- A + mem[HL]
  void add(const RegA& a, const RegHLAddr& r) {
    append(Fmt("i r,r") % "ADD" % a % r,  //
           {build(0b10, B, F)});
  }

  /// ADD A, (IX or IY + d) | A <- A + mem[IX or IY + d]
  void add(const RegA& a, const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r,r") % "ADD" % a % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b10, B, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// ADC A, r | A <- A + reg8 + carry
  void adc(const RegA& a, const BasicReg8& r) {
    append(Fmt("i r,r") % "ADC" % a % r,  //
           {build(0b10, C, r)});
  }

  /// ADC A, n | A <- A + constant8 + carry
  void adc(const RegA& a, uint8_t n) {
    append(Fmt("i r,x") % "ADC" % a % n,  //
           {build(0b11, C, F), n});
  }

  /// ADC A, (HL) | A <- A + mem[HL] + carry
  void adc(const RegA& a, const RegHLAddr& r) {
    append(Fmt("i r,r") % "ADC" % a % r,  //
           {build(0b10, C, F)});
  }

  /// ADC A, (IX or IY + d) | A <- A + mem[IX or IY + d] + carry
  void adc(const RegA& a, const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r,r") % "ADC" % a % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b10, C, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// INC r | reg8 <- reg8 + 1
  void inc(const BasicReg8& r) {
    append(Fmt("i r") % "INC" % r,  //
           {build(0b00, r, H)});
  }

  /// INC (HL) | mem[HL] <- mem[HL] + 1
  void inc(const RegHLAddr& r) {
    append(Fmt("i r") % "INC" % r,  //
           {build(0b00, F, H)});
  }

  /// INC (IX or IY + d) | mem[IX or IY + d] <- mem[IX or IY + d] + 1
  void inc(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "INC" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b00, F, H),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  // ---------------------------------
  // 減算・デクリメント命令

  /// SUB A, r | A <- A - reg8
  void sub(const RegA& a, const BasicReg8& r) {
    append(Fmt("i r,r") % "SUB" % a % r,  //
           {build(0b10, D, r)});
  }

  /// SUB A, n | A <- A - constant8
  void sub(const RegA& a, uint8_t n) {
    append(Fmt("i r,x") % "SUB" % a % n,  //
           {build(0b11, D, F), n});
  }

  /// SUB A, (HL) | A <- A - mem[HL]
  void sub(const RegA& a, const RegHLAddr& r) {
    append(Fmt("i r,r") % "SUB" % a % r,  //
           {build(0b10, D, F)});
  }

  /// SUB A, (IX or IY + d) | A <- A - mem[IX or IY + d]
  void sub(const RegA& a, const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r,r") % "SUB" % a % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b10, D, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// SBC A, r | A <- A - reg8 - carry
  void sbc(const RegA& a, const BasicReg8& r) {
    append(Fmt("i r,r") % "SBC" % a % r,  //
           {build(0b10, E, r)});
  }

  /// SBC A, n | A <- A - constant8 - carry
  void sbc(const RegA& a, uint8_t n) {
    append(Fmt("i r,x") % "SBC" % a % n,  //
           {build(0b11, E, F), n});
  }

  /// SBC A, (HL) | A <- A - mem[HL] - carry
  void sbc(const RegA& a, const RegHLAddr& r) {
    append(Fmt("i r,r") % "SBC" % a % r,  //
           {build(0b10, E, F)});
  }

  /// SBC A, (IX or IY + d) | A <- A - mem[IX or IY + d] - carry
  void sbc(const RegA& a, const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r,r") % "SBC" % a % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b10, E, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// DEC r | reg8 <- reg8 - 1
  void dec(const BasicReg8& r) {
    append(Fmt("i r") % "DEC" % r,  //
           {build(0b00, r, L)});
  }

  /// DEC (HL) | mem[HL] <- mem[HL] - 1
  void dec(const RegHLAddr& r) {
    append(Fmt("i r") % "DEC" % r,  //
           {build(0b00, F, L)});
  }

  /// DEC (IX or IY + d) | mem[IX or IY + d] <- mem[IX or IY + d] - 1
  void dec(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "DEC" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b00, F, L),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  // =========================================================================
  // 16ビット算術演算命令

  /// ADD HL, rp | HL <- HL + reg16
  void add(const RegHL& hl, const BasicReg16& rp) {
    append(Fmt("i r,r") % "ADD" % hl % rp,  //
           {build(0b00, rp, 0b1001)});
  }

  /// ADD HL, HL | HL <- HL + HL
  void add(const RegHL& hl, const RegHL& rp) {
    append(Fmt("i r,r") % "ADD" % hl % rp,  //
           {build(0b00, rp, 0b1001)});
  }

  /// ADC HL, rp | HL <- HL + reg16 + carry
  void adc(const RegHL& hl, const BasicReg16& rp) {
    append(Fmt("i r,r") % "ADC" % hl % rp,  //
           {0b1110'1101, build(0b01, rp, 0b1010)});
  }

  /// ADC HL, HL | HL <- HL + HL + carry
  void adc(const RegHL& hl, const RegHL& rp) {
    append(Fmt("i r,r") % "ADC" % hl % rp,  //
           {0b1110'1101, build(0b01, rp, 0b1010)});
  }

  /// ADD IX or IY, rp | IX or IY <- IX or IY + reg16
  void add(const IndexReg16& ireg16, const BasicReg16& rp) {
    append(Fmt("i r,r") % "ADD" % ireg16 % rp,  //
           {ireg16.m_prefix, build(0b00, rp, 0b1001)});
  }

  /// INC rp | reg16 <- reg16 + 1
  void inc(const BasicReg16& rp) {
    append(Fmt("i r") % "INC" % rp,  //
           {build(0b00, rp, 0b0011)});
  }

  /// INC HL | HL <- HL + 1
  void inc(const RegHL& rp) {
    append(Fmt("i r") % "INC" % rp,  //
           {build(0b00, rp, 0b0011)});
  }

  /// INC IX or IY | IX or IY <- IX or IY + 1
  void inc(const IndexReg16& rp) {
    append(Fmt("i r") % "INC" % rp,  //
           {rp.m_prefix, build(0b00, rp, 0b0011)});
  }

  /// SBC HL, rp | HL <- HL - reg16 - carry
  void sbc(const RegHL& hl, const BasicReg16& rp) {
    append(Fmt("i r,r") % "SBC" % hl % rp,  //
           {0b1110'1101, build(0b1, rp, 0b0010)});
  }

  /// SBC HL, HL | HL <- HL - HL - carry
  void sbc(const RegHL& hl, const RegHL& rp) {
    append(Fmt("i r,r") % "SBC" % hl % rp,  //
           {0b1110'1101, build(0b01, rp, 0b0010)});
  }

  /// DEC rp | reg16 <- reg16 - 1
  void dec(const BasicReg16& rp) {
    append(Fmt("i r") % "DEC" % rp,  //
           {build(0b00, rp, 0b1011)});
  }

  /// DEC HL | HL <- HL - 1
  void dec(const RegHL& rp) {
    append(Fmt("i r") % "DEC" % rp,  //
           {build(0b00, rp, 0b1011)});
  }

  /// DEC IX or IY |IX or IY <- IX or IY - 1
  void dec(const IndexReg16& rp) {
    append(Fmt("i r") % "DEC" % rp,  //
           {rp.m_prefix, build(0b00, rp, 0b1011)});
  }

  // =========================================================================
  // 論理演算命令

  /// AND r | A <- A & reg8
  void and (const BasicReg8& r) {
    append(Fmt("i r") % "AND" % r,  //
           {build(0b10, H, r)});
  }

  /// AND n | A <- A & constant8
  void and (uint8_t n) {
    append(Fmt("i x") % "AND" % n,  //
           {build(0b11, H, F), n});
  }

  /// AND (HL) | A <- A & mem[HL]
  void and (const RegHLAddr& hl) {
    append(Fmt("i r") % "AND" % hl,  //
           {build(0b10, H, F)});
  }

  /// AND (IX or IY + d) | A <- A & mem[IX or IY + d]
  void and (const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "AND" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b10, H, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// OR r | A <- A | reg8
  void or (const BasicReg8& r) {
    append(Fmt("i r") % "OR" % r,  //
           {build(0b10, F, r)});
  }

  /// OR n | A <- A | constant8
  void or (uint8_t n) {
    append(Fmt("i x") % "OR" % n,  //
           {build(0b11, F, F), n});
  }

  /// OR (HL) | A <- A | mem[HL]
  void or (const RegHLAddr& hl) {
    append(Fmt("i r") % "OR" % hl,  //
           {build(0b10, F, F)});
  }

  /// OR (IX or IY + d) | A <- A | mem[IX or IY + d]
  void or (const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "OR" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b10, F, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

#define XZ80_XOR xor

  /// XOR r | A <- A ^ reg8
  void XZ80_XOR(const BasicReg8& r) {
    append(Fmt("i r") % "XOR" % r,  //
           {build(0b10, L, r)});
  }

  /// XOR n | A <- A ^ constant8
  void XZ80_XOR(uint8_t n) {
    append(Fmt("i x") % "XOR" % n,  //
           {build(0b11, L, F), n});
  }

  /// XOR (HL) | A <- A ^ mem[HL]
  void XZ80_XOR(const RegHLAddr& hl) {
    append(Fmt("i r") % "XOR" % hl,  //
           {build(0b10, L, F)});
  }

  /// XOR (IX or IY + d) | A <- A ^ mem[IX or IY + d]
  void XZ80_XOR(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "XOR" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, build(0b10, L, F),
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  /// CPL | A <- ~A
  void cpl(void) {
    append(Fmt("i") % "CPL",  //
           {0b0010'1111});
  }

  /// NEG | A <- ~A + 1
  void neg(void) {
    append(Fmt("i") % "NEG",  //
           {0b1110'1101, 0b0100'0100});
  }

  // =========================================================================
  // ビット操作命令

  /// CCF | carry <- ~carry
  void ccf(void) {
    append(Fmt("i") % "CCF",  //
           {0b0011'1111});
  }

  /// SCF | carry <- 1
  void scf(void) {
    append(Fmt("i") % "SCF",  //
           {0b0011'0111});
  }

  /// BIT b, r | Z <- ~r_b
  void bit(uint8_t b, const BasicReg8& r) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "BIT %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "BIT" % b % r,  //
           {0b1100'1011, static_cast<uint8_t>(0b0100'0000 | b << 3 | r.id)});
  }

  /// BIT b, (HL) | Z <- ~mem[HL]_b
  void bit(uint8_t b, const RegHLAddr& hl) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "BIT %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "BIT" % b % hl,  //
           {0b1100'1011, static_cast<uint8_t>(0b0100'0000 | b << 3 | 0b110)});
  }

  /// BIT b, (IX or IY +d) | Z <- ~mem[IX or IY +d]_b
  void bit(uint8_t b, const IndenexReg16AddrOffset& ireg16_offset) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "BIT %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "BIT" % b % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset),
            static_cast<uint8_t>(0b0100'0000 | b << 3 | 0b110)});
  }

  /// SET b, r | r_b <- 1
  void set(uint8_t b, const BasicReg8& r) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "SET %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "SET" % b % r,  //
           {0b1100'1011, static_cast<uint8_t>(0b1100'0000 | b << 3 | r.id)});
  }

  /// SET b, (HL) | mem[HL]_b <- 1
  void set(uint8_t b, const RegHLAddr& hl) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "SET %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "SET" % b % hl,  //
           {0b1100'1011, static_cast<uint8_t>(0b1100'0000 | b << 3 | 0b110)});
  }

  /// SET b, (IX or IY +d) | mem[IX or IY +d]_b <- 1
  void set(uint8_t b, const IndenexReg16AddrOffset& ireg16_offset) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "SET %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "SET" % b % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset),
            static_cast<uint8_t>(0b1100'0000 | b << 3 | 0b110)});
  }

  /// RES b, r | r_b <- 0
  void res(uint8_t b, const BasicReg8& r) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "RES %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "RES" % b % r,  //
           {0b1100'1011, static_cast<uint8_t>(0b1000'0000 | b << 3 | r.id)});
  }

  /// RES b, (HL) | mem[HL]_b <- 0
  void res(uint8_t b, const RegHLAddr& hl) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "RES %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "RES" % b % hl,  //
           {0b1100'1011, static_cast<uint8_t>(0b1000'0000 | b << 3 | 0b110)});
  }

  /// RES b, (IX or IY +d) | mem[IX or IY +d]_b <- 0
  void res(uint8_t b, const IndenexReg16AddrOffset& ireg16_offset) {
    if (7 < b) {
      char buf[32];
      std::sprintf(buf, "RES %d:out of range", b);
      throw std::out_of_range(buf);
    }
    append(Fmt("i d,r") % "RES" % b % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1100'1011,
            static_cast<uint8_t>(ireg16_offset.m_offset),
            static_cast<uint8_t>(0b1000'0000 | b << 3 | 0b110)});
  }

  // =========================================================================
  // サーチ・比較命令

  /// CPI | Frag <- A - mem[HL]; HL <- HL+1; BC <- BC-1;
  void cpi(void) {
    append(Fmt("i") % "CPI",  //
           {0b1110'1101, 0b1010'0001});
  }

  /// CPIR | while BC!=0 and A!=mem[HL] { Frag <- A - mem[HL]; HL <- HL+1; BC <- BC-1; }
  void cpir(void) {
    append(Fmt("i") % "CPIR",  //
           {0b1110'1101, 0b1011'0001});
  }

  /// CPD | Frag <- A - mem[HL]; HL <- HL-1; BC <- BC-1;
  void cpd(void) {
    append(Fmt("i") % "CPD",  //
           {0b1110'1101, 0b1010'1001});
  }

  /// CPDR | while BC!=0 and A!=mem[HL] { Frag <- A - mem[HL]; HL <- HL-1; BC <- BC-1; }
  void cpdr(void) {
    append(Fmt("i") % "CPDR",  //
           {0b1110'1101, 0b1011'1001});
  }

  /// CP r | Frag <- A - reg8
  void cp(const BasicReg8& r) {
    append(Fmt("i r") % "CP" % r,  //
           {build(0b10, A, r)});
  }

  /// CP n | Frag <- A - constant8
  void cp(uint8_t n) {
    append(Fmt("i x") % "CP" % n,  //
           {0b1111'1110, n});
  }

  /// CP (HL) | Frag <- A - mem[HL]
  void cp(const RegHLAddr& hl) {
    append(Fmt("i r") % "CP" % hl,  //
           {build(0b10, A, F)});
  }

  /// CP (IX or IY +d) | Frag <- A - mem[IX or IY +d]
  void cp(const IndenexReg16AddrOffset& ireg16_offset) {
    append(Fmt("i r") % "CP" % ireg16_offset,  //
           {ireg16_offset.m_reg.m_prefix, 0b1011'1110,
            static_cast<uint8_t>(ireg16_offset.m_offset)});
  }

  // =========================================================================
  // 分岐命令

  /// JP nn | PC <- constant16
  void jp(uint16_t nn) {
    MemAddr addr(nn);
    append(Fmt("i x") % "JP" % nn,  //
           {0b1100'0011, addr.l, addr.h});
  }
  void jp(const std::string& label) {
    MemAddr addr(label);
    append(Fmt("i s") % "JP" % label,  //
           {0b1100'0011, addr.l, addr.h});
    resolve(label, 1);
  }

  /// JP cc,nn | PC <- constant16 if cc
  void jp(const CondBase& cc, uint16_t nn) {
    MemAddr addr(nn);
    append(Fmt("i c,x") % "JP" % cc % nn,  //
           {build(0b11, cc, 0b010), addr.l, addr.h});
  }
  void jp(const CondBase& cc, const std::string& label) {
    MemAddr addr(label);
    append(Fmt("i c,s") % "JP" % cc % label,  //
           {build(0b11, cc, 0b010), addr.l, addr.h});
    resolve(label, 1);
  }

  /// JR e | PC <- PC + e
  void jr(int16_t e) {
    if (e < -126 || 129 < e) {
      char buf[32];
      std::sprintf(buf, "JR %d:out of range", e);
      throw std::out_of_range(buf);
    }
    const int offset = static_cast<int>(e) - 2;
    append(Fmt("i o") % "JR" % e,  //
           {0b0001'1000, static_cast<uint8_t>(offset)});
  }
  void jr(const std::string& label) {
    append(Fmt("i s") % "JR" % label,  //
           {0b0001'1000, 0x00});
    resolve(label, 1, true);
  }

  /// JR cc,e | PC <- PC + e if cc
  void jr(const AllCond& cc, int16_t e) {
    if (e < -126 || 129 < e) {
      char buf[32];
      std::sprintf(buf, "JR %d:out of range", e);
      throw std::out_of_range(buf);
    }
    const int offset = static_cast<int>(e) - 2;
    append(Fmt("i c,o") % "JR" % cc % e,  //
           {build(0b0010'0000, cc), static_cast<uint8_t>(offset)});
  }
  void jr(const AllCond& cc, const std::string& label) {
    append(Fmt("i c,s") % "JR" % cc % label,  //
           {build(0b0010'0000, cc), 0x00});
    resolve(label, 1, true);
  }

  /// JP (HL) | PC <- mem[HL]
  void jp(const RegHLAddr& hl) {
    append(Fmt("i r") % "JP" % hl,  //
           {0b1110'1001});
  }

  /// JP (IX or IY) | PC <- mem[IX or IY]
  void jp(const IndenexReg16Addr& rp) {
    append(Fmt("i r") % "JP" % rp,  //
           {rp.m_reg.m_prefix, 0b1110'1001});
  }

  /// DJNZ e | if B!=0 then PC <- PC + e; B <- B -1; end
  void djnz(int16_t e) {
    if (e < -126 || 129 < e) {
      char buf[32];
      std::sprintf(buf, "DJNZ %d:out of range", e);
      throw std::out_of_range(buf);
    }
    const int offset = static_cast<int>(e) - 2;
    append(Fmt("i o") % "DJNZ" % e,  //
           {0b0001'0000, static_cast<uint8_t>(offset)});
  }
  void djnz(const std::string& label) {
    append(Fmt("i s") % "DJNZ" % label,  //
           {0b0001'0000, 0x00});
    resolve(label, 1, true);
  }

  /// CALL nn | mem[SP-1] <- PCH; mem[SP-2] <- PCL;
  ///         | SP <- SP - 2; PC <- constant16;
  void call(uint16_t nn) {
    const MemAddr ad(nn);
    append(Fmt("i x") % "CALL" % nn,  //
           {0b1100'1101, ad.l, ad.h});
  }
  void call(const std::string& label) {
    const MemAddr ad(label);
    append(Fmt("i s") % "CALL" % label,  //
           {0b1100'1101, ad.l, ad.h});
    resolve(label, 1);
  }

  /// CALL cc, nn | if cc then mem[SP-1] <- PCH; mem[SP-2] <- PCL;
  ///             | SP <- SP - 2; PC <- constant16; end
  void call(const CondBase& cc, uint16_t nn) {
    const MemAddr ad(nn);
    append(Fmt("i c,x") % "CALL" % cc % nn,  //
           {build(0b11, cc, 0b100), ad.l, ad.h});
  }
  void call(const CondBase& cc, const std::string& label) {
    const MemAddr ad(label);
    append(Fmt("i c,s") % "CALL" % cc % label,  //
           {build(0b11, cc, 0b100), ad.l, ad.h});
    resolve(label, 1);
  }

  /// RET | PCL <- mem[SP]; PCH <- mem[SP+1]; SP <- SP+2
  void ret(void) {
    append(Fmt("i") % "RET",  //
           {0b1100'1001});
  }

  /// RET cc | if cc then PCL <- mem[SP]; PCH <- mem[SP+1]; SP <- SP+2; end
  void ret(const CondBase& cc) {
    append(Fmt("i c") % "RET" % cc,  //
           {build(0b11, cc, 0b000)});
  }

  /// RETI | PCL <- mem[SP]; PCH <- mem[SP+1]; SP <- SP+2; IFF1 <- IFF2
  void reti(void) {
    append(Fmt("i") % "RETI",  //
           {0b1110'1101, 0b0100'1101});
  }

  /// RETN | PCL <- mem[SP]; PCH <- mem[SP+1]; SP <- SP+2; IFF1 <- IFF2
  void retn(void) {
    append(Fmt("i") % "RETN",  //
           {0b1110'1101, 0b0100'0101});
  }

  /// RST p | mem[SP-1] <- PCH; mem[SP-2] <- PCL; SP <- SP-2; PC <- p
  ///       | Only p = 0, 8, 16, 24, 32, 40, 48, 56
  void rst(uint16_t p) {
    if (p % 8 != 0 || 7 * 8 < p) {
      char buf[32];
      std::sprintf(buf, "RST 0%xh:invalid argument", p);
      throw std::invalid_argument(buf);
    }
    const uint8_t insn = 0b1100'0111 | p;
    append(Fmt("i x") % "RST" % p,  //
           {insn});
  }

  // =========================================================================
  // CPU制御命令

  // -------------------------------------------------------------------------
  // 動作・割り込み設定命令

  /// NOP | Do nothing.
  void nop(void) {
    append(Fmt("i") % "NOP",  //
           {0b0000'0000});
  }

  /// HALT | Halt.
  void halt(void) {
    append(Fmt("i") % "HALT",  //
           {0b0111'0110});
  }

  /// DI | Disable interrupt.
  void di(void) {
    append(Fmt("i") % "DI",  //
           {0b1111'0011});
  }

  /// EI | Enable interrupt.
  void ei(void) {
    append(Fmt("i") % "EI",  //
           {0b1111'1011});
  }

  /// IM 0 or 1 or 2 | Interrupt mode 0 or 1 or 2
  void im(uint8_t m) {
    const uint8_t code[3] = {
        0b010'00'110,
        0b010'10'110,
        0b010'11'110,
    };
    if (2 < m) {
      char buf[32];
      std::sprintf(buf, "IM %d:invalid argument", m);
    }
    append(Fmt("i d") % "IM" % m,  //
           {0b1110'1101, code[m]});
  }

  // -------------------------------------------------------------------------
  // 入力命令

  /// IN A, (n) | A <- io[constant8]
  void in(const RegA& a, const IoAddr& n) {
    append(Fmt("i r,I") % "IN" % a % n,  //
           {0b1101'1011, n.addr});
  }

  /// IN r, (C) | reg8 <- io[C]
  void in(const BasicReg8& r, const RegCAddr& c) {
    append(Fmt("i r,r") % "IN" % r % c,  //
           {0b1110'1101, build(0b01, r, B)});
  }

  /// INI | mem[HL] <- io[C]; B <- B-1; HL <- HL+1;
  void ini(void) {
    append(Fmt("i") % "INI",  //
           {0b1110'1101, 0b1010'0010});
  }

  /// INIR | while B!=0 { mem[HL] <- io[C]; B <- B-1; HL <- HL+1; }
  void inir(void) {
    append(Fmt("i") % "INIR",  //
           {0b1110'1101, 0b1011'0010});
  }

  /// IND | mem[HL] <- io[C]; B <- B-1; HL <- HL-1;
  void ind(void) {
    append(Fmt("i") % "IND",  //
           {0b1110'1101, 0b1010'1010});
  }

  /// INDR | while B!=0 { mem[HL] <- io[C]; B <- B-1; HL <- HL-1; }
  void indr(void) {
    append(Fmt("i") % "INDR",  //
           {0b1110'1101, 0b1011'1010});
  }

  // -------------------------------------------------------------------------
  // 出力命令

  /// OUT (n), A | io[constant8] <- A
  void out(const IoAddr& n, const RegA& a) {
    append(Fmt("i I,r") % "OUT" % n % a,  //
           {0b1101'0011, n.addr});
  }

  /// OUT (C), r | io[C] <- reg8
  void out(const RegCAddr& c, const BasicReg8& r) {
    append(Fmt("i r,r") % "OUT" % c % r,  //
           {0b1110'1101, build(0b01, r, C)});
  }

  /// OUTI | io[C] <- mem[HL]; B <- B-1; HL <- HL+1;
  void outi(void) {
    append(Fmt("i") % "OUTI",  //
           {0b1110'1101, 0b1010'0011});
  }

  /// OTIR | while B!=0 { io[C] <- mem[HL]; B <- B-1; HL <- HL+1; }
  void otir(void) {
    append(Fmt("i") % "OTIR",  //
           {0b1110'1101, 0b1011'0011});
  }

  /// OUTD | io[C] <- mem[HL]; B <- B-1; HL <- HL-1;
  void outd(void) {
    append(Fmt("i") % "OUTD",  //
           {0b1110'1101, 0b1010'1011});
  }

  /// OTDR | while B!=0 { io[C] <- mem[HL]; B <- B-1; HL <- HL-1; }
  void otdr(void) {
    append(Fmt("i") % "OTDR",  //
           {0b1110'1101, 0b1011'1011});
  }

  // =========================================================================
  // BCD命令

  /// DAA | Decimal Adjust Accumulator
  void daa(void) {
    append(Fmt("i") % "DAA",  //
           {0b0010'0111});
  }

  /// RLD | BCD left shift
  void rld(void) {
    append(Fmt("i") % "RLD",  //
           {0b1110'1101, 0b0110'1111});
  }

  /// RRD | BCD right shift
  void rrd(void) {
    append(Fmt("i") % "RRD",  //
           {0b1110'1101, 0b0110'0111});
  }
};
}  // namespace Xz80
