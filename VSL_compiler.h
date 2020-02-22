#ifndef MULTICHANNELCOMPILER_H
#define MULTICHANNELCOMPILER_H

#include <FastSin.h>
#include <MusicFreq.h>
#include <freqmesh.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <set>
#include <string>
#include <vector>

using std::exception;
using std::map;
using std::pair;
using std::set;
using std::stod;
using std::string;
using std::to_string;
using std::to_wstring;
using std::vector;
using std::wstring;

typedef enum { Notation_Algebraic, Notation_RPN } Notation_type;
static double phi = 1.61803398874989, M_PI3_2 = 3. * M_PI / 2,
              TWO_PI = 6.28318530717958623;

class Radionics {
 public:
  static double RadionicRate(std::wstring pw, int d) {  // unicode strings
    if (pw.empty()) return 0;

    size_t l = pw.size();

    // if it's a rate return it
    for (size_t i = 0; i < l && isdigit(pw[i]); i++)
      ;

    // calc rate  as sum( pw[i] * i^phi )
    double s = 0, phi = 1.618, mf;
    for (size_t i = 0; i < l && s < 1e200; i++)
      s += double(pw[i]) * pow(phi, i);

    // to 'd' digits
    if (log10(s) + 1 > d)
      mf = 0.5;
    else
      mf = 2;
    for (; int(floor(log10(s)) + 1) != d; s *= mf)
      ;

    return floor(s);
  }
};

class AuxFunc : public MusicFreq, public Radionics {
 public:
  static double factorial(double n) {
    if (n > 0)
      return n * factorial(n - 1);
    else
      return 1;
  }

  static double saw(double x, double alpha1) {
    static double tn, x1, a1;
    double ret, fm = x;

    if (alpha1 != a1) {
      tn = tan(alpha1 * M_PI / 180.);
      x1 = M_PI / (1 + tn * tn);
    }

    if (x > TWO_PI) fm = fmod(x, TWO_PI);
    if (fm <= x1)
      ret = fm * tn;
    else if (fm <= TWO_PI - x1)
      ret = (-1 / tn) * (fm - M_PI);
    else
      ret = tn * (fm - TWO_PI);
    return ret;
  }

  // saw(x)
  // evaluate between 0..2PI, return -1..+1
  static double saw(double x) {
    double ret, fm = x;
    if (x > TWO_PI) fm = fmod(x, TWO_PI);
    if (fm <= M_PI_2)
      ret = fm / M_PI_2;
    else if (fm <= M_PI3_2)
      ret = -2 * (fm / M_PI - 1);
    else
      ret = fm / M_PI_2 - 4;
    return ret;
  }

  //    double rate(double istr) {
  //      double r = 0;
  //      auto s = compiler->get_string(int(istr));
  //      r = RadionicRate(QString::fromStdWString(s), 8);
  //      r = FreqInOctave(r, 0);

  //      return r;
  //    }
  static double rate(wstring str) {
    double r = 0;
    r = RadionicRate(str, 8);
    r = FreqInOctave(r, 0);

    return r;
  }
};

template <class T>
class DataItem {
 public:
  typedef enum { t_double, t_string } data_type;
  data_type dt = t_double;
  union {
    T d = 0;
    wstring s;
  };
  DataItem() : d(0) {}
  DataItem(DataItem &o) { *this = o; }
  ~DataItem() {
    d = 0;
    s.~wstring();
  }
  inline DataItem &operator=(DataItem &o) {
    dt = o.dt;
    switch (dt) {
      case t_double:
        d = o.d;
        break;
      case t_string:
        s = o.s;
        break;
    }

    return *this;
  }
  inline DataItem &operator=(double d) {
    this->d = d;
    this->dt = t_double;
    return *this;
  }
  inline DataItem &operator=(wstring s) {
    this->s = s;
    this->dt = t_string;
    return *this;
  }
  inline DataItem &operator+=(DataItem &o) {
    switch (dt) {
      case t_double:
        d += o.d;
        break;
      case t_string:
        s += o.s;
        break;
    }
    return *this;
  }
  inline DataItem &operator-=(DataItem &o) {
    d -= o.d;
    return *this;
  }
  inline DataItem &operator*=(DataItem &o) {
    d *= o.d;
    return *this;
  }
  inline DataItem &operator*=(double o) {
    d *= o;
    return *this;
  }
  inline DataItem &operator/=(DataItem &o) {
    d /= o.d;
    return *this;
  }
  inline bool operator==(DataItem &o) {
    switch (dt) {
      case t_double:
        return d == o.d;
      case t_string:
        return s == o.s;
    }
  }
  inline bool operator!=(DataItem &o) {
    switch (dt) {
      case t_double:
        return d != o.d;
      case t_string:
        return s != o.s;
    }
  }
  inline bool operator<(DataItem &o) {
    switch (dt) {
      case t_double:
        return d < o.d;
      case t_string:
        return s < o.s;
    }
  }
  inline bool operator<=(DataItem &o) {
    switch (dt) {
      case t_double:
        return d <= o.d;
      case t_string:
        return s <= o.s;
    }
  }
  inline bool operator>(DataItem &o) {
    switch (dt) {
      case t_double:
        return d > o.d;
      case t_string:
        return s > o.s;
    }
  }
  inline bool operator>=(DataItem &o) {
    switch (dt) {
      case t_double:
        return d >= o.d;
      case t_string:
        return s >= o.s;
    }
  }
};

class VSL_compiler : public AuxFunc {
  static const int max_waves = 1024, max_channels = 32,
                   max_code = 1024 * 2 * max_channels;

  typedef enum {  // list of symbols
    SNULL = 0,
    CONST,
    LET,
    RPN,
    FUNC,
    RET,
    PARAM,
    ALGEBRAIC,
    NUMBER,
    IDENT,
    STRING,
    IDENT_t,
    PLUS,
    MINUS,
    MULT,
    DIV,
    OPAREN,
    CPAREN,

    OCURL,
    CCURL,
    OSQARE,
    CSQUARE,
    BACKSLASH,
    RANDOM,
    VERT_LINE,
    OLQUOTE,
    CLQUOTE,
    YINYANG,
    SEQUENCE,
    FREQ_MESH,

    FACT,
    TILDE,
    POWER,
    PERIOD,
    SEMICOLON,
    COMMA,
    COLON,

    EQ,
    GT,
    GE,
    LT,
    LE,
    NE,

    FSIN,
    FCOS,
    FTAN,
    FEXP,
    FLOG,
    FLOG10,
    FINT,
    FSQRT,
    FASIN,
    FACOS,
    FATAN,
    FABS,
    SPI,
    SPHI,
    SWAVE,
    TONE,
    NOTE,
    SEC,
    OSC,
    ABS,
    SAW,
    SAW1,
    LAP,
    HZ2OCT,
    MAGNETICRING,
    PUSH_CONST,
    PUSH_T,
    PUSH_ID,
    PUSH_STR,
    POP,
    NEG,
    SWAVE1,
    SWAVE2,
    FLOAT,
    RATE,

    NOTE_CONST,
    N_DO,  // notes
    N_RE,
    N_MI,
    N_FA,
    N_SOL,
    N_LA,
    N_SI,
    FLAT,
    SHARP
  } symbol;

  class Symbols {
    class symbolmap : public map<wstring, symbol> {
     public:
      // constructors
      symbolmap() = default;
      symbolmap(const std ::initializer_list<pair<wstring, symbol>> inputs) {
        append(inputs);
      }
      symbolmap(const std ::initializer_list<symbolmap> inputs) {
        append(inputs);
      }

      symbolmap &operator+=(const symbolmap &other) {
        append(other);
        return *this;
      }

      symbolmap &operator<<(const symbolmap &other) {
        append(other);
        return *this;
      }

      symbolmap operator+(const symbolmap &other) {
        symbolmap sm;
        sm << *this << other;
        return sm;
      }

      void append(const symbolmap &other) {
        insert(other.begin(), other.end());
      }
      void append(const std ::initializer_list<pair<wstring, symbol>> inputs) {
        for (auto &i : inputs) append(i);
      }
      void append(const pair<wstring, symbol> input) { insert(input); }
      void append(const std ::initializer_list<symbolmap> inputs) {
        for (auto &inp : inputs)
          for (auto &i : inp) append(i);
      }

      symbol contains(wstring ws) {
        auto f = find(ws);
        return f != end() ? f->second : SNULL;
      }
      symbol contains(wchar_t ch) { return contains(wstring{ch}); }
      symbol contains(wchar_t ch0, wchar_t ch1) {
        return contains(wstring{ch0, ch1});
      }

      wstring rev_find(symbol s) {
        wstring fs;
        for (auto &sp : *this)
          if (sp.second == s) {
            fs = sp.first;
            break;
          }
        return fs;
      }
    };

    symbolmap chars = {{L"+", PLUS},     {L"*", MULT},       {L"·", MULT},
                       {L"/", DIV},      {L"(", OPAREN},     {L")", CPAREN},
                       {L"{", OCURL},    {L"}", CCURL},      {L"[", OSQARE},
                       {L"]", CSQUARE},  {L"\\", BACKSLASH}, {L"?", RANDOM},
                       {L"!", FACT},     {L"^", POWER},      {L".", PERIOD},
                       {L",", COMMA},    {L":", COLON},      {L";", SEMICOLON},
                       {L"=", EQ},       {L"~", TILDE},      {L"π", SPI},
                       {L"Ø", SPHI},     {L"|", VERT_LINE},  {L"‹", OLQUOTE},
                       {L"›", CLQUOTE},  {L"♪", NOTE},       {L"⬳", SAW},
                       {L"∿", FSIN},     {L"τ", IDENT_t},    {L"☯", YINYANG},
                       {L"§", SEQUENCE}, {L"✬", FREQ_MESH},  {L"➡", RET},
                       {L"♭", FLAT},     {L"♯", SHARP}},
              words = {{L"sin", FSIN},      {L"cos", FCOS},
                       {L"tan", FTAN},      {L"exp", FEXP},
                       {L"log", FLOG},      {L"log10", FLOG10},
                       {L"int", FINT},      {L"sqrt", FSQRT},
                       {L"asin", FASIN},    {L"acos", FACOS},
                       {L"atan", FATAN},    {L"abs", FABS},
                       {L"pi", SPI},        {L"phi", SPHI},
                       {L"wave", SWAVE},    {L"wave1", SWAVE1},
                       {L"wave2", SWAVE2},  {L"tone", TONE},
                       {L"note", NOTE},     {L"sec", SEC},
                       {L"osc", OSC},       {L"saw", SAW},
                       {L"saw1", SAW1},     {L"lap", LAP},
                       {L"hz2oct", HZ2OCT}, {L"magneticring", MAGNETICRING},
                       {L"rate", RATE},

                       {L"t", IDENT_t},     {L"const", CONST},
                       {L"rpn", RPN},       {L"algebraic", ALGEBRAIC},
                       {L"let", LET},       {L"float", FLOAT},
                       {L"func", FUNC}},

              _2ch = {{L">=", GE}, {L"<=", LE}, {L"<>", NE}, {L"->", RET}},

              reserved = {chars, words, _2ch};

   public:
    symbolmap initial = {{L"-", MINUS}, {L">", GT}, {L"<", LT}},
              notes = {{L"do", N_DO}, {L"re", N_RE},   {L"mi", N_MI},
                       {L"fa", N_FA}, {L"sol", N_SOL}, {L"la", N_LA},
                       {L"si", N_SI}};

    Symbols() = default;

    symbol is_reserved(wchar_t ch) {  // check is ch in map -> sym
      return reserved.contains(ch);
    }
    symbol is_reserved(wchar_t ch0, wchar_t ch1) {  // check is ch in map -> sym
      return reserved.contains(ch0, ch1);
    }

    symbol is_reserved(wstring id) {  // is a reserved word ?
      return reserved.contains(id);
    }
    symbol is_reserved(symbolmap sm, wchar_t ch) { return sm.contains(ch); }
    symbol is_reserved(symbolmap sm, wstring id) { return sm.contains(id); }

    wstring rev_find(symbol sym) {
      return symbolmap{reserved, initial}.rev_find(sym);
    }
  };

  class Parser {
   public:
    Parser(wstring s) : s(s) { getch(); }
    double get_value() { return nval; }
    wstring get_str() { return str; }
    wstring get_id() { return id; }

   private:
    wstring s;
    wchar_t ch = 0;
    size_t ich = 0, lineno = 0;
    wstring id, str;
    double nval = 0;
    int i0, i1;
    bool err = false;
    symbol sym = SNULL;
    Symbols syms;

    wchar_t getch() {
      if (ich < s.size())
        ch = s[ich++];
      else
        ch = 0;
      if (ch == '\n') lineno++;
      return ch;
    }

    void ungetch() {
      if (ich) ich--;
      ch = wchar_t(s[ich - 1]);
    }

    void lower_case(wstring &s) {
      std::transform(s.begin(), s.end(), s.begin(),
                     [](int c) { return std::tolower(c); });
    }

    void skip_blanks() {
      while (ch && ch <= ' ') getch();
    }

    void skip_to_eol() {
      while (ch && (ch != '\n' || ch == '\r')) getch();  // skip line comment
    }

    void skip_multiline_comment() {
      for (; ch && ch != '/'; getch()) {
        getch();
        while (ch && (ch != '*')) getch();
      }
      getch();  // skip last '/'
    }

    void skip_blank_comments() {  // skip blanks & comments // /**/
      for (bool in_comment = true; in_comment;) {
        skip_blanks();

        if (ch == '/') {  // skip comment
          if (getch() == '/')
            skip_to_eol();
          else if (ch == '*') {  // /**/
            skip_multiline_comment();
          } else {
            ungetch();
            in_comment = false;
          }
        } else
          break;
      }

      skip_blanks();
    }

    bool contains(const wchar_t *s) { return wcschr(s, ch) != nullptr; }

   public:
    int get_i0() { return i0; }
    int get_i1() { return i1; }

    int get_lineno() { return int(lineno); }
    wchar_t get_current_ch() { return ch; }

    symbol getsym(void) {  // -> sym
      id.clear();
      sym = SNULL;

      skip_blank_comments();
      if (ch == '\'') {  // 'string' -> str
        str.clear();
        while (getch() != '\'' && ch) str.push_back(ch);
        getch();
        sym = STRING;
      } else if (isalpha(ch) || ch == '_') {  // ident / res. word / func

        while (isalnum(ch) || ch == '_') {
          id.push_back(ch);
          getch();
        }

        // check intro, func
        if ((sym = syms.is_reserved(id)) == SNULL) {
          if ((sym = syms.is_reserved(syms.notes, id)) == SNULL)
            sym = IDENT;
          else {  // note#b@oct: do, do@3, do#, do#@2
            map<symbol, int> note_val = {{N_DO, 0}, {N_RE, 2},  {N_MI, 4},
                                         {N_FA, 5}, {N_SOL, 7}, {N_LA, 9},
                                         {N_SI, 11}};
            map<wchar_t, int> suffix = {{u'♭', -1}, {u'♯', +1}};

            i0 = note_val[sym] + suffix[ch];
            i1 = 0;
            if (suffix[ch]) getch();

            if (ch == '@') {
              id.clear();  // octave
              getch();
              int sign = 1;
              if (ch == '-') {
                sign = -1;
                getch();
              }
              while (isdigit(ch)) {
                id.push_back(ch);
                getch();
              }
              if (!id.empty()) i1 = sign * std::stoi(id);
            }
            sym = NOTE_CONST;  // i0,i1
          }
        }

      } else {
        if (isdigit(ch) || ch == '.') {  // number: dddd.ddde-dd
          do {                           //  mantisa
            id.push_back(ch);
            getch();
          } while (isdigit(ch) || ch == '.');
          if (contains(L"eE")) {  // exp
            do {
              id.push_back(ch);
              getch();
            } while (isdigit(ch) || contains(L"-+"));
          }
          sym = NUMBER;
          try {
            nval = std::stod(id);
          } catch (...) {
            nval = 0;
          }

        } else {
          if ((sym = syms.is_reserved(ch)) == SNULL) {
            auto ch_ant = ch;
            getch();
            if ((sym = syms.is_reserved(ch_ant, ch)) == SNULL) {
              ungetch();
              if ((sym = syms.is_reserved(syms.initial, ch)) == SNULL)  // -,<,>
                err = true;
            }
          }

          getch();
        }
      }

      return sym;
    }
  };

  class Table_Values {
   public:
    typedef enum { NUM_ID, PARAM, FUNC, STRING_ID } types;

    Table_Values() = default;
    Table_Values(wstring id) : id(id), type(NUM_ID) {}
    Table_Values(wstring id, double value) : id(id), type(NUM_ID) {
      *di = value;
    }
    Table_Values(wstring id, int str_ix)
        : id(id), type(STRING_ID), str_ix(str_ix) {}
    Table_Values(wstring id, types type) : id(id), type(type) {}
    Table_Values(wstring id, types type, int address, int n_params)
        : id(id), type(type), address(address), n_params(n_params) {}
    Table_Values(wstring id, types type, int param_ix)
        : id(id), type(type), param_ix(param_ix) {}

    bool operator==(wstring s) { return id == s; }  // required for find
    Table_Values &operator=(DataItem<double> &di) {
      *this->di = di;
      return *this;
    }

    wstring id;
    types type = NUM_ID;
    int str_ix = 0;
    DataItem<double> *di = new DataItem<double>;
    int address = 0, param_ix = 0, n_params = 0;
  };

  class Compiler {
    Parser *parser;
    bool err = false;
    symbol sym;

    vector<Table_Values> tab_values;  // symbol table,
    vector<wstring> string_depot;

    char code[max_code];
    int pc = 0;

    static const int max_chan = 32;
    int ch = 0;
    int nid = 0;
    Notation_type notation = Notation_Algebraic;
    jmp_buf jmp_env;

    void parse_id_eq_expr() {  // id=expr, id=expr;
      do {
        if (getsym() == IDENT) {
          wstring id = parser->get_id();
          if (getsym() == EQ) {
            getsym();
            symbol first_sym = sym;

            switch (notation) {
              case Notation_Algebraic:
                expr_0();
                break;
              case Notation_RPN:
                rpn_expr();
                break;
            }

            if (first_sym == STRING)  // first string parsed?
              tab_values.push_back(Table_Values(id, Table_Values::STRING_ID));
            else
              tab_values.push_back(Table_Values(id, Table_Values::NUM_ID));

            generate(POP, nid++);

          } else
            err = true;
        } else
          err = true;
      } while (sym == COMMA && !err);

      if (sym == SEMICOLON) {
        getsym();
      } else
        err = true;
    }

    void parse_const() {
      if (sym == CONST)  // const sample_rate=expr, bits_sample=expr;
        parse_id_eq_expr();
      blk_addr.set_const(0, pc);
    }

    void parse_let() {
      if (sym == LET) parse_id_eq_expr();
      blk_addr.set_let(pc);
    }

    void parse_funcs() {
      while (sym == FUNC) {
        getsym(IDENT);

        tab_values.push_back(
            Table_Values(parser->get_id(), Table_Values::FUNC, pc, 0));

        auto ixtv = tab_values.size();
        int param_ix = 0;

        if (getsym() == OPAREN) {
          do {
            getsym(IDENT);
            tab_values.push_back(Table_Values(parser->get_id(),
                                              Table_Values::PARAM, param_ix++));
          } while (getsym() == COMMA);
          check_getsym(CPAREN);
        }
        check_getsym(RET);  // ->

        switch (notation) {
          case Notation_RPN:
            rpn_expr();
            break;
          case Notation_Algebraic:
            expr_0();
            break;
        }
        check_getsym(SEMICOLON);

        tab_values.resize(ixtv);                // remove refs. to parameters
        tab_values.back().n_params = param_ix;  // save # of args in

        generate(RET, param_ix);
      }
      blk_addr.set_func(pc);
      // jump over fun def
    }

    void init() {
      ch = pc = nid = 0;
      err = false;
      sym = SNULL;
      tab_values.clear();
      string_depot.clear();
      freq_mesh.clear();
    }

   public:
    class BlockAddress {
     public:
      typedef pair<int, int> from_to;

      from_to _const, _let, _func, _code[max_chan];
      int last_to;

      void set_const(int f, int t) {
        _const = from_to(f, t);
        last_to = t;
      }
      void set_let(int t) {
        _let = from_to(last_to, t);
        last_to = t;
      }
      void set_func(int t) {
        _func = from_to(last_to, t);
        last_to = t;
      }
      void set_code(int chan, int t) {
        _code[chan] = from_to(last_to, t);
        last_to = t;
      }

      from_to get_const() { return _const; }
      from_to get_let() { return _let; }
      from_to get_code(int chan) { return _code[chan]; }
      from_to *get_codes() { return _code; }
    };

    BlockAddress blk_addr;
    vector<FreqMesh> freq_mesh;

    bool compile(wstring expr) {
      parser = new Parser(expr);

      if (setjmp(jmp_env)) {
        return false;
      } else {
        init();
        getsym();

        switch (sym) {
          default:
            notation = Notation_Algebraic;
            return compile_algebraic();
          case ALGEBRAIC:
            getsym(SEMICOLON);
            getsym();
            notation = Notation_Algebraic;
            return compile_algebraic();
          case RPN:
            notation = Notation_RPN;
            getsym(SEMICOLON);
            getsym();
            return compile_rpn();
        }
      }
    }

    QString error_msg() {
      return err ? QString("syntax error near line %1 ")
                           .arg(parser->get_lineno() + 1) +
                       QChar(parser->get_current_ch())
                 : QString("ok");
    }

    bool compile_rpn() {
      parse_const();  // const let var0=expr, var1=expr;

      if (!err) {
        parse_let();
        parse_funcs();

        do {
          rpn_expr();

          check_getsym(SEMICOLON);
          blk_addr.set_code(ch++, pc);
        } while (sym != SNULL);
      }
      return !err;
    }

    bool compile_algebraic() {
      parse_const();  // const let var0=expr, var1=expr;

      if (!err) {
        parse_let();
        parse_funcs();

        symbol ls;
        do {
          expr_0();  //  expr per channel;

          ls = sym;

          if (!err && sym == SEMICOLON) {
            blk_addr.set_code(ch++, pc);
            check_sym(SEMICOLON);
            getsym();
          }
        } while (ls == SEMICOLON && !err);
      }
      return !err;
    }

    inline bool ok() { return !err; }
    inline int get_code_size() { return pc; }
    inline char *get_code() { return code; }
    inline int get_chans() { return ch; }
    Parser *get_parser() { return parser; }
    wstring get_id(int nid) { return tab_values[size_t(nid)].id; }
    wstring get_string(int istr) { return string_depot[size_t(istr)]; }

    void getvalue(wstring ident, int &id) {
      auto fp = std::find(tab_values.begin(), tab_values.end(), ident);
      if (fp != tab_values.end()) id = int(fp->di->d);
    }

    void getvalue(wstring ident, double &id) {
      auto fp = std::find(tab_values.begin(), tab_values.end(), ident);
      if (fp != tab_values.end()) id = fp->di->d;
    }

    DataItem<double> getvalueitem(int nid) {
      return *tab_values[size_t(nid)].di;
    }

    void setvalue(int nid, DataItem<double> di) {
      tab_values[size_t(nid)] = di;
    }

   private:
    bool starts_implicit_mult() {
      const set<symbol> implicit_mult_start = {
          IDENT, IDENT_t, OCURL,  OSQARE, OLQUOTE, OPAREN,
          SPI,   SPHI,    NUMBER, RANDOM, TILDE,   SEQUENCE};
      return (implicit_mult_start.find(sym) != implicit_mult_start.end()) ||
             (sym >= FSIN && sym <= MAGNETICRING);
    }

    symbol getsym(symbol check_symbol) {
      getsym();
      err = check_symbol != sym;
      if (err) longjmp(jmp_env, check_symbol);
      return sym;
    }

    symbol check_getsym(symbol check_symbol) {
      check_sym(check_symbol);
      return getsym();
    }

    symbol getsym() { return sym = parser->getsym(); }
    void check_sym(symbol check_symbol) {
      err = check_symbol != sym;
      if (err) longjmp(jmp_env, check_symbol);
    }
    [[noreturn]] void set_error() {
      err = true;
      longjmp(jmp_env, -1);
    }

    double getvalue(void) {
      auto fp =
          std::find(tab_values.begin(), tab_values.end(), parser->get_id());
      return fp == tab_values.end() ? 0 : fp->di->d;
    }

    int get_ident_index() {
      auto fp =
          std::find(tab_values.begin(), tab_values.end(), parser->get_id());
      return fp != tab_values.end() ? int(std::distance(tab_values.begin(), fp))
                                    : -1;
    }

    // generate code
    void generate(int token, double f) {
      code[pc++] = char(token);
      *reinterpret_cast<double *>(code + pc) = f;
      pc += sizeof(double);
    }

    void generate(int token, int i) {
      code[pc++] = char(token);
      *reinterpret_cast<int *>(code + pc) = i;
      pc += sizeof(int);
    }

    void generate(int token, int p0, int p1) {
      code[pc++] = char(token);
      *reinterpret_cast<int *>(code + pc) = p0;
      pc += sizeof(int);
      *reinterpret_cast<int *>(code + pc) = p1;
      pc += sizeof(int);
    }

    void generate(int token) { code[pc++] = char(token); }

    inline void next_pc() { pc++; }
    inline void next_pc_int() { pc += sizeof(int); }
    inline void next_pc_double() { pc += sizeof(double); }

    class SymbolSet : public set<symbol> {
     public:
      SymbolSet(const std ::initializer_list<symbol> &inputs) {
        insert(inputs);
      }
      bool contains(symbol sym) const { return find(sym) != end(); }
    };

    void expr_0(void) {
      if (!err) {
        bool is_neg = (sym == MINUS);
        if (is_neg) getsym();

        expr_1();

        if (is_neg) generate(NEG);

        static const SymbolSet op_set = {EQ, NE, LT, LE, GT, GE, PLUS, MINUS};
        do {
          symbol sym_op = sym;
          if (op_set.contains(sym)) {
            getsym();
            expr_1();
            generate(sym_op);
          }
        } while (op_set.contains(sym));
      }
    }

    void expr_1(void) {
      if (!err) {
        expr_2();
        do {
          symbol sym_op = sym;
          if (starts_implicit_mult()) {  // not operator-> implicit *, i,e.
                                         // 2{440}
            expr_2();
            generate(MULT);
          } else {
            switch (sym) {
              case MULT:
              case DIV:
                getsym();
                expr_2();
                generate(sym_op);
                break;

              default:
                break;
            }
          }
        } while (sym == MULT || sym == DIV || starts_implicit_mult());
      }
    }

    void expr_2(void) {
      if (!err) {
        expr_3();
        do {
          if (sym == POWER) {
            getsym();
            expr_3();
            generate(POWER);
          }
        } while (sym == POWER);
      }
    }

    void expr_3(void) {
      if (!err) {
        switch (sym) {
          case OPAREN:
            getsym();
            expr_0();
            check_sym(CPAREN);
            getsym();
            break;
          case NUMBER:
            generate(PUSH_CONST, parser->get_value());
            getsym();
            break;
          case STRING:
            string_depot.push_back(parser->get_str());
            generate(PUSH_STR, int(string_depot.size() - 1));
            getsym();
            break;
          case FLOAT:
            generate(PUSH_CONST,
                     -32.);  // this is the floating_point=true value
            getsym();
            break;

          case IDENT_t:  //  't' special var is the parameter in eval call
            generate(PUSH_T);
            getsym();
            break;

          case IDENT: {
            auto idix = get_ident_index();
            if (idix != -1) {
              auto &tv = tab_values[size_t(idix)];
              switch (tv.type) {
                case Table_Values::STRING_ID:
                  generate(PUSH_ID, idix);
                  break;
                case Table_Values::NUM_ID:
                  generate(PUSH_ID, idix);
                  break;
                case Table_Values::PARAM:
                  generate(PARAM, tv.param_ix);
                  break;
                case Table_Values::FUNC:
                  if (tv.n_params) {
                    getsym(OPAREN);
                    getsym();
                    for (int np = 0; np < tv.n_params - 1; np++) {
                      expr_0();
                      check_getsym(COMMA);
                    }
                    expr_0();
                    check_sym(CPAREN);
                  }
                  generate(FUNC, tv.address, tv.n_params);
                  break;
              }
            } else
              set_error();
          }
            getsym();
            break;
          case MINUS:
            getsym();
            expr_3();
            generate(NEG);
            break;
          case PLUS:
            getsym();
            expr_3();  // +expr nothing to generate
            break;
          case FACT:
            getsym();
            expr_3();
            generate(FACT);
            break;
          case TILDE:
            getsym();
            expr_3();
            generate(SWAVE1);
            break;
          case YINYANG:
            getsym();
            expr_3();
            generate(YINYANG);
            break;

          case SEQUENCE:  // (from, to, inc)
            getsym(OPAREN);
            getsym();
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(COMMA);
            generate(SEQUENCE);
            break;

          case FREQ_MESH:  // (base, slope, islope, n)
            getsym(OPAREN);
            getsym();
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(CPAREN);
            generate(FREQ_MESH, int(freq_mesh.size()));
            freq_mesh.push_back(FreqMesh());
            break;

          case RANDOM:
            generate(PUSH_CONST, double(rand()) / RAND_MAX);
            getsym();
            break;

          case OCURL:  // {hz}, {amp,hz}, {amp, hz, phase}
            getsym();
            expr_0();
            if (sym == COMMA) {
              getsym();
              expr_0();
              if (sym == COMMA) {
                getsym();
                expr_0();
                generate(SWAVE);
              } else {
                generate(SWAVE2);
              }
            } else {
              generate(SWAVE1);
            }
            check_getsym(CCURL);
            break;

          case OSQARE:  // []==sec
            getsym();
            expr_0();

            generate(SEC);
            check_getsym(CSQUARE);
            break;

          case VERT_LINE:  // |abs|
            getsym();
            expr_0();

            generate(ABS);
            check_getsym(VERT_LINE);
            break;

          case OLQUOTE:  // «f»  -> exp(f*t)
            getsym();
            expr_0();
            check_getsym(CLQUOTE);

            generate(PUSH_T);
            generate(MULT);
            generate(FEXP);
            break;

          case BACKSLASH:  // \s:e\ -> lap(start, end)
            getsym();
            if (sym == COLON)  // \:e\ -> lap(0, end)
              generate(PUSH_CONST, 0.0);
            else
              expr_0();
            getsym();  // :
            expr_0();
            check_getsym(BACKSLASH);  // '\'
            generate(LAP);
            break;

          case RATE:
          case FSIN:
          case FCOS:
          case FTAN:
          case FASIN:
          case FACOS:
          case FATAN:
          case FEXP:
          case FINT:
          case FABS:
          case FLOG:
          case FLOG10:
          case FSQRT:
          case SEC:
          case OSC:
          case ABS: {
            symbol tsym = sym;
            getsym();
            expr_3();
            generate(tsym);
          } break;

          case SPI:
            getsym();
            generate(PUSH_CONST, M_PI);
            break;

          case SPHI:
            getsym();
            generate(PUSH_CONST, phi);
            break;

          case SWAVE:  // wave(amp, hz, phase)
            getsym(OPAREN);
            getsym();
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(CPAREN);

            generate(SWAVE);
            break;

          case NOTE_CONST:
            generate(NOTE_CONST, parser->get_i0(), parser->get_i1());
            getsym();
            break;

          // 2 parameter funcs.
          case NOTE:    // note(note#,oct)
          case TONE:    // tone(note#,oct)
          case LAP:     // lap(time1,time2)
          case HZ2OCT:  // hz2oct(hz,oct)
          {
            symbol tsym = sym;
            getsym(OPAREN);
            getsym();
            expr_0();
            check_getsym(COMMA);
            expr_0();
            check_getsym(CPAREN);
            generate(tsym);
          } break;

          case SAW:  // saw(freq, alpha)
            getsym();
            getsym();
            expr_0();
            if (sym == COMMA) {
              getsym();
              expr_0();
              getsym();
              generate(SAW);
            } else {
              getsym();
              generate(SAW1);
            }
            break;

          case MAGNETICRING:  // MagnetRing(Vol, Hz, Phase, on_count,
                              // off_count)
            getsym();
            getsym();
            expr_0();
            getsym();
            expr_0();
            getsym();
            expr_0();
            getsym();
            expr_0();
            getsym();
            expr_0();
            getsym();
            generate(MAGNETICRING);
            break;

          case SNULL:
            break;
          default:
            err = true;
            break;  // syntax error
        }
      }
    }

    void rpn_expr() {
      do {
        switch (sym) {
          case NUMBER:
            generate(PUSH_CONST, parser->get_value());
            break;
            //  't' special var is the parameter in eval call
          case IDENT_t:
            generate(PUSH_T);
            break;
          case IDENT: {
            auto idix = get_ident_index();
            if (idix != -1) {
              auto &tv = tab_values[size_t(idix)];
              switch (tv.type) {
                case Table_Values::NUM_ID:
                  generate(PUSH_ID, idix);
                  break;
                case Table_Values::STRING_ID:
                  generate(PUSH_ID, idix);
                  break;
                case Table_Values::PARAM:
                  generate(PARAM, tv.param_ix);
                  break;
                case Table_Values::FUNC:
                  generate(FUNC, tv.address, tv.n_params);
                  break;
              }
            } else
              set_error();
          } break;

          case STRING:
            string_depot.push_back(parser->get_id());
            generate(STRING, int(string_depot.size() - 1));
            getsym();
            break;

          case SPI:
            generate(PUSH_CONST, M_PI);
            break;

          case SPHI:
            generate(PUSH_CONST, phi);
            break;

          case TILDE:
            generate(SWAVE1);
            break;
          case SEQUENCE:
            generate(SEQUENCE);
            break;
          case FREQ_MESH:
            generate(FREQ_MESH, int(freq_mesh.size()));
            freq_mesh.push_back(FreqMesh());
            break;

          case BACKSLASH: {  // \{operator:+-*/} compress stack w/operator
            static set<symbol> operators = {PLUS, MINUS, MULT, DIV, TILDE};
            generate(BACKSLASH);
            if (operators.find(getsym()) != operators.end())
              generate(sym);
            else
              err = true;
          } break;

          case YINYANG:

          case MINUS:
          case PLUS:
          case DIV:
          case MULT:

          case RATE:
          case FSIN:
          case FCOS:
          case FTAN:
          case FASIN:
          case FACOS:
          case FATAN:
          case FEXP:
          case FINT:
          case FABS:
          case FLOG:
          case FLOG10:
          case FSQRT:
          case SEC:
          case OSC:
          case ABS:
            generate(sym);
            break;

          case NOTE_CONST:
            generate(NOTE_CONST, parser->get_i0(), parser->get_i1());
            break;

          case SNULL:
            break;
          default:
            err = true;
            break;
        }

        getsym();
      } while (sym != SEMICOLON && sym != COMMA && sym != SNULL);
    }
  };

  class Decompiler {
    int pc = 0;
    char *code;

    wstring gen_token(wstring s) {
      wchar_t buff[200];
      swprintf(buff, 200, L"%5d: %ls", pc, s.data());
      return wstring(buff);
    }
    wstring param0() { return to_wstring(*reinterpret_cast<int *>(code + pc)); }
    int iparam0() { return *reinterpret_cast<int *>(code + pc); }
    wstring param0d() {
      return to_wstring(*reinterpret_cast<double *>(code + pc));
    }

    typedef enum { type_none, type_int, type_id, type_double } data_type;
    typedef struct {
      symbol sym;
      wstring mnemo;
      data_type dt;
      int n_param;
    } decomp;

    vector<decomp> dec = {{PUSH_CONST, L"push#", type_double, 1},
                          {PUSH_T, L"push t", type_none, 0},
                          {PUSH_ID, L"push", type_id, 1},
                          {PUSH_STR, L"push$", type_int, 1},
                          {PARAM, L"param", type_int, 1},
                          {FUNC, L"func", type_int, 2},
                          {RET, L"ret", type_int, 1},
                          {POP, L"pop", type_id, 1},
                          {FREQ_MESH, L"mesh", type_int, 1}};

   public:
    wstring decompile(Compiler *compiler) {
      wstring s, spc = L" ";
      Symbols syms;

      try {
        if (compiler && compiler->ok()) {
          code = compiler->get_code();
          for (pc = 0; pc < compiler->get_code_size();) {
            bool found = false;
            for (size_t i = 0; i < dec.size(); i++) {
              if (dec[i].sym == symbol(code[pc])) {
                s += gen_token(dec[i].mnemo);
                pc++;

                switch (dec[i].dt) {
                  case type_none:
                    break;
                  case type_int:
                    for (int p = 0; p < dec[i].n_param; p++) {
                      s += spc + param0();
                      pc += sizeof(int);
                    }
                    break;
                  case type_id:
                    for (int p = 0; p < dec[i].n_param; p++) {
                      s += spc + compiler->get_id(iparam0());
                      pc += sizeof(int);
                    }
                    break;
                  case type_double:
                    for (int p = 0; p < dec[i].n_param; p++) {
                      s += spc + param0d();
                      pc += sizeof(double);
                    }
                    break;
                }
                s += L"\n";
                found = true;
                break;
              }
            }

            if (!found) {
              auto token = syms.rev_find(symbol(code[pc]));
              if (!token.empty()) {
                s += gen_token(token) + L"\n";
                pc++;
              } else
                break;
            }
          }
        }
      } catch (...) {
      }

      return s;
    }
  };

  Compiler *compiler = nullptr;
  Decompiler decompiler;
  wstring decomp_string;

 public:
  ~VSL_compiler() {
    if (compiler) delete compiler;
  }

 private:
  template <class T>
  class Stack {
    typedef DataItem<T> _DataItem;

   public:
    static const size_t max_stack = 4 * 1024;
    DataItem<T> *data = new DataItem<T>[max_stack];  //[max_stack];
    int sp = 0;

    Stack() {}
    ~Stack() {
      if (data != nullptr) delete[] data;
      data = nullptr;
    }

    inline void push(T d) { data[sp++] = d; }
    inline void push(wstring s) { data[sp++] = s; }
    inline void push(_DataItem &&_data) { data[sp++] = _data; }
    inline void push(_DataItem &_data) { data[sp++] = _data; }
    inline _DataItem &pop() { return data[--sp]; }

    inline _DataItem &operator[](int ix) { return data[sp + ix]; }
    inline const _DataItem &operator[](int ix) const { return data[sp + ix]; }
    inline _DataItem &at(int ix) { return data[ix]; }

    inline void operator_sp1(symbol op) {
      sp--;
      auto &data_sp_1 = data[sp - 1], &data_sp = data[sp];
      switch (op) {
        case PLUS:
          data_sp_1 += data_sp;
          break;
        case MINUS:
          data_sp_1 -= data_sp;
          break;
        case MULT:
          data_sp_1 *= data_sp;
          break;
        case DIV:
          data_sp_1 /= data_sp;
          break;
        case EQ:
          data_sp_1 = data_sp_1 == data_sp;
          break;
        case NE:
          data_sp_1 = data_sp_1 != data_sp;
          break;
        case LT:
          data_sp_1 = data_sp_1 < data_sp;
          break;
        case GT:
          data_sp_1 = data_sp_1 > data_sp;
          break;
        case LE:
          data_sp_1 = data_sp_1 <= data_sp;
          break;
        case GE:
          data_sp_1 = data_sp_1 >= data_sp;
          break;
        case POWER:
          data_sp_1 = pow(data_sp_1.d, data_sp.d);
          break;

        default:
          break;
      }
    }

    inline void operator_sp0(symbol op, double t) {
      auto &data_sp_1 = data[sp - 1];
      switch (op) {
        default:
          break;
        case FACT:
          data_sp_1 = factorial(floor(data_sp_1.d));
          break;
        case NEG:
          data_sp_1 = -data_sp_1.d;
          break;
        case RATE:
          data_sp_1.d = rate(data_sp_1.s);
          break;
        case FSIN:
          data_sp_1 = sin(data_sp_1.d);
          break;
        case FCOS:
          data_sp_1 = cos(data_sp_1.d);
          break;
        case FTAN:
          data_sp_1 = tan(data_sp_1.d);
          break;
        case FASIN:
          data_sp_1 = asin(data_sp_1.d);
          break;
        case FACOS:
          data_sp_1 = acos(data_sp_1.d);
          break;
        case FATAN:
          data_sp_1 = atan(data_sp_1.d);
          break;
        case FEXP:
          data_sp_1 = exp(data_sp_1.d);
          break;
        case FINT:
          data_sp_1 = floor(data_sp_1.d);
          break;
        case FABS:
          data_sp_1 = fabs(data_sp_1.d);
          break;
        case FLOG:
          if (data_sp_1.d > 0)
            data_sp_1 = log(data_sp_1.d);
          else
            data_sp_1 = 0;
          break;
        case FLOG10:
          if (data_sp_1.d > 0)
            data_sp_1 = log10(data_sp_1.d);
          else
            data_sp_1 = 0;
          break;
        case FSQRT:
          if (data_sp_1.d >= 0)
            data_sp_1 = sqrt(data_sp_1.d);
          else
            data_sp_1 = 0;
          break;
        case SEC:
          data_sp_1 = data_sp_1.d * 2 * M_PI;
          break;
        case OSC:
          data_sp_1 = sin(t * data_sp_1.d);
          break;
        case ABS:
          data_sp_1 = fabs(data_sp_1.d);
          break;
      }
    }
  };

  Stack<double> _stack;

  int sample_rate = 44100, bits_sample = 16;
  int floating_point = 0;
  double seconds = 5, volume = 1;

  //  static const int max_stack = 1024 * 4;
  //  double stack[max_stack];
  //  int sp = 0;
  int err_number = 0, n_ch = 0;
  char *code = nullptr;
  int sec_eval = 0;
  Compiler::BlockAddress::from_to blk_let, *blk_code;

 public:
  wstring get_decompiled() { return decompiler.decompile(compiler); }

  void clear() {
    delete compiler;
    compiler = nullptr;
  }

  QString error_msg() { return compiler ? compiler->error_msg() : ""; }

  QString get_wave_mesh() {
    QString s;
    for (auto &m : compiler->freq_mesh) s += m.gen_clip_wave() + "\n";
    return s;
  }

  int get_sample_rate() { return sample_rate; }
  int get_bits_sample() { return bits_sample; }
  int get_n_channels() { return compiler ? compiler->get_chans() : 0; }
  bool get_floating_point() { return floating_point; }
  double get_seconds() { return seconds; }
  double get_volume() { return volume; }

  bool is_ok() { return compiler ? compiler->ok() : false; }

  void init_defaults() {
    sample_rate = 44100;
    floating_point = 0;
    bits_sample = 16;
    seconds = 5;
    volume = 0.4;
    sec_eval = 0;
  }

  bool compile(wstring expr) {
    try {
      (compiler = new Compiler)->compile(expr);

      if (compiler->ok()) {
        code = compiler->get_code();
        n_ch = compiler->get_chans();

        blk_let = compiler->blk_addr.get_let();
        blk_code = compiler->blk_addr.get_codes();

        // get wave def values from const
        init_defaults();

        execute_const();  // execute const values

        compiler->getvalue(L"sample_rate", sample_rate);
        compiler->getvalue(L"bits_sample", bits_sample);
        if (bits_sample == -32) {
          bits_sample = 32;
          floating_point = 1;
        } else
          floating_point = 0;
        compiler->getvalue(L"seconds", seconds);
        compiler->getvalue(L"volume", volume);
        if (volume > 1) volume = 1;
      }

      return compiler->ok();
    } catch (...) {
      return false;
    }
  }

  void execute_const() {
    auto bc = compiler->blk_addr.get_const();
    execute(0, bc.first, bc.second);
  }

  void execute_let(double x) { execute(x, blk_let.first, blk_let.second); }

  double execute(double x, int chan) {
    execute_let(x);  // execute let
    return execute(x, blk_code[chan].first, blk_code[chan].second);
  }

  double execute(double t, int from_pc, int to_pc) {
    bool err = false;
    vector<int> n_params, sp_base;
    auto &sp = _stack.sp;

    sec_eval++;

    sp = 0;
    err_number = 0;

    try {
      for (int pc = from_pc; pc < to_pc && !err_number;) {
        switch (code[pc]) {
          case PUSH_CONST:
            pc++;
            _stack.push(*reinterpret_cast<double *>(code + pc));
            pc += sizeof(double);
            break;
          case PUSH_T:
            pc++;
            _stack.push(t);
            break;
          case PUSH_ID:
            pc++;
            _stack.push(
                compiler->getvalueitem(*reinterpret_cast<int *>(code + pc)));
            pc += sizeof(int);
            break;
          case PUSH_STR:  // push_str #string_depot
            pc++;
            _stack.push(
                compiler->get_string(*reinterpret_cast<int *>(code + pc)));
            pc += sizeof(int);
            break;

          case PARAM:  // push param
            pc++;
            _stack.push(_stack.at(int(sp_base.back() - 1 - n_params.back() +
                                      *reinterpret_cast<int *>(code + pc))));
            pc += sizeof(int);
            break;
          case FUNC:  // pc, nparams
            _stack.push(double(pc + 1 + 2 * int(sizeof(int))));
            n_params.push_back(
                *reinterpret_cast<int *>(code + pc + 1 + sizeof(int)));
            sp_base.push_back(sp);
            pc = *reinterpret_cast<int *>(code + pc + 1);
            break;
          case RET: {
            int nr = *reinterpret_cast<int *>(code + pc + 1);
            pc = int(_stack[-2].d);
            _stack[-(nr + 2)] = _stack[-1];
            sp -= nr + 2 - 1;
            sp_base.pop_back();
            n_params.pop_back();
          } break;

          case POP:
            pc++;
            compiler->setvalue(*reinterpret_cast<int *>(code + pc), _stack[-1]);
            pc += sizeof(int);
            break;

          case PLUS:
          case MINUS:
          case MULT:
          case DIV:
          case EQ:
          case NE:
          case LT:
          case LE:
          case GT:
          case GE:
          case POWER:
            _stack.operator_sp1(symbol(code[pc]));
            pc++;
            break;

          case FACT:
          case NEG:
          case RATE:
          case FSIN:
          case FCOS:
          case FTAN:
          case FASIN:
          case FACOS:
          case FATAN:
          case FEXP:
          case FINT:
          case FABS:
          case FLOG:
          case FLOG10:
          case FSQRT:
          case SEC:
          case OSC:
          case ABS:
            _stack.operator_sp0(symbol(code[pc]), t);
            pc++;
            break;

          case SWAVE1:  // wave(hz)
            pc++;
            _stack[-1] = sin(t * _stack[-1].d);
            break;
          case SWAVE2:  // wave(amp, hz)
            pc++;
            _stack[-2] *= sin(t * _stack[-1].d);
            sp--;
            break;
          case SWAVE:  // wave(amp, freq, phase)
            pc++;
            _stack[-3] = _stack[-3].d * sin(t * _stack[-2].d + _stack[-1].d);
            sp -= 2;
            break;

          case YINYANG: {
            auto &f = _stack[-1].d, k = 6. * M_PI;
            pc++;
            f = sin(t * f) * sin(f / (t + k));
          } break;

          case FREQ_MESH: {  // *(base, slope, islope, n)
            pc++;
            FreqMesh &fm =
                compiler
                    ->freq_mesh[size_t(*reinterpret_cast<int *>(code + pc))];
            pc += sizeof(int);

            if (sp >= 4) {
              if (!fm.is_init())
                fm.set_param(_stack[-4].d, _stack[-3].d, uint(_stack[-2].d),
                             uint(_stack[-1].d));

              _stack[-4] = fm.gen_sample(t);
              sp -= 3;
            } else {
              err = true;
              err_number = 1;
            }
          } break;

          case BACKSLASH: {  // \{}operator
            pc++;
            double res = 0;
            if (sp > 1) {
              if (code[pc] == TILDE) {
                for (sp--; sp >= 0; sp--) res += sin(t * _stack[sp].d);
              } else {
                res = _stack[-1].d;
                for (sp -= 2; sp >= 0; sp--) {
                  switch (code[pc]) {
                    case PLUS:
                      res += _stack[sp].d;
                      break;
                    case MINUS:
                      res -= _stack[sp].d;
                      break;
                    case MULT:
                      res *= _stack[sp].d;
                      break;
                    case DIV:
                      res /= _stack[sp].d;
                      break;
                  }
                }
              }
              _stack[0] = res;
              sp = 1;
            }

            pc++;
          } break;

          case SEQUENCE: {
            auto n = _stack[-1].d, end = _stack[-2].d, ini = _stack[-3].d;
            if (n < _stack.max_stack - 10 && end != ini && sp > 2) {
              if (end < ini) std::swap(ini, end);
              auto inc = (end - ini) / (n - 1);
              sp -= 3;
              for (auto i = ini; i < end; i += inc) _stack.push(i);
              _stack.push(end);
            }
            pc++;
          } break;

          case NOTE_CONST:
            pc++;
            _stack.push(sin(NoteOct2Freq(*reinterpret_cast<int *>(code + pc),
                                         *reinterpret_cast<int *>(
                                             code + pc + sizeof(int))) *
                            t));
            pc += sizeof(int) * 2;
            break;

          case NOTE:  // note(note#, octave)
          {
            double &s2 = _stack[-2].d, &s1 = _stack[-1].d;
            if (s2 > 12 || s2 < 0) s2 = 0;
            if (s1 < 0 || s1 > 10) s1 = 0;
            //          s2 = sin(x * NoteOct2Freq(int(s2), int(s1)));
            s2 = NoteOct2Freq(int(s2), int(s1));
            sp--;
            pc++;
          } break;
          case TONE:  // tone(note#, octave)
          {
            double &s2 = _stack[-2].d, &s1 = _stack[-1].d;
            if (s2 > 12 || s2 < 0) s2 = 0;
            if (s1 < 0 || s1 > 10) s1 = 0;
            s2 = NoteOct2Freq(int(s2), int(s1));
            sp--;
            pc++;
          } break;

          case LAP:  // lap(time1,time2)
          {
            double &s2 = _stack[-1].d, &s1 = _stack[-2].d;
            if (s2 <= s1)
              s1 = 0;
            else
              s1 = (t >= s1 * (2 * M_PI)) && (t <= s2 * (2 * M_PI));
            sp--;
            pc++;
          } break;

          case SAW1:
            _stack[-1] = saw(t * _stack[-1].d);
            pc++;
            break;
          case SAW:  // saw(freq, alpha1)
          {
            double &s2 = _stack[-2].d, &s1 = _stack[-1].d;
            if (s2 == 0.) s2 = 0.1;
            if (s1 < 0 || s1 > 90) s1 = 0;
            s2 = saw(t * s2, s1);
            sp--;
            pc++;
          } break;

          case HZ2OCT:  // hz2oct(freq, oct)
          {
            double &s2 = _stack[-2].d, &s1 = _stack[-1].d;
            s2 = FreqInOctave(int(s2), int(s1));
            sp--;
            pc++;
          } break;

          case MAGNETICRING:  // MagnetRing(Vol, Hz, Phase, on_count,
                              // off_count)
          {                   // vol is top of _stack

            double &vol = _stack[-5].d, hz = _stack[-4].d, ph = _stack[-3].d,
                   onc = _stack[-2].d, offc = _stack[-1].d;

            // double delta=(hz * 2 * M_PI) / samp;
            double delta = hz / sample_rate;
            if (fmod(sec_eval * delta, (onc + offc)) <= onc)
              vol *= sin(t * hz + ph);
            else
              vol = 0;
            sp -= 4;
            pc++;
          } break;

          default:
            err = true;
            err_number = 1;
            break;
        }
      }
    } catch (...) {
      err_number = 1;
    }

    if (err_number) {  // any error??
      if (sp > 0) _stack[-1] = 0;
      err = true;
    }
    return sp == 1 ? _stack[-1].d : 0;
  }
};

#endif  // MULTICHANNELCOMPILER_H
