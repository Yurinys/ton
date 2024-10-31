/*
    This file is part of TON Blockchain Library.

    TON Blockchain Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    TON Blockchain Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TON Blockchain Library.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include "src-file.h"
#include <functional>
#include <memory>
#include <vector>

namespace tolk {

/*
 *
 *   SYMBOL VALUES (DECLARED)
 *
 */

typedef int var_idx_t;

enum class SymValKind { _Param, _Var, _Func, _Typename, _GlobVar, _Const };

struct SymValBase {
  SymValKind kind;
  int idx;
  SymValBase(SymValKind kind, int idx) : kind(kind), idx(idx) {
  }
  virtual ~SymValBase() = default;
};

/*
 *
 *   SYMBOL TABLE
 *
 */

enum class SymbolSubclass {
  undef = 0,
  dot_identifier = 1,    // begins with . (a const method)
  tilde_identifier = 2   // begins with ~ (a non-const method)
};

typedef int sym_idx_t;

struct Symbol {
  std::string str;
  sym_idx_t idx;
  SymbolSubclass subclass;

  Symbol(std::string str, sym_idx_t idx);

  static std::string unknown_symbol_name(sym_idx_t i);
};

class SymTable {
public:
  static constexpr int SIZE_PRIME = 100003;

private:
  sym_idx_t def_sym{0};
  std::unique_ptr<Symbol> sym[SIZE_PRIME + 1];
  sym_idx_t gen_lookup(std::string_view str, int mode = 0, sym_idx_t idx = 0);

  static constexpr int max_kw_idx = 10000;
  sym_idx_t keywords[max_kw_idx];

public:

  static constexpr sym_idx_t not_found = 0;
  sym_idx_t lookup(const std::string_view& str, int mode = 0) {
    return gen_lookup(str, mode);
  }
  sym_idx_t lookup_add(const std::string& str) {
    return gen_lookup(str, 1);
  }
  Symbol* operator[](sym_idx_t i) const {
    return sym[i].get();
  }
  bool is_keyword(sym_idx_t i) const {
    return sym[i] && sym[i]->idx < 0;
  }
  std::string get_name(sym_idx_t i) const {
    return sym[i] ? sym[i]->str : Symbol::unknown_symbol_name(i);
  }
  SymbolSubclass get_subclass(sym_idx_t i) const {
    return sym[i] ? sym[i]->subclass : SymbolSubclass::undef;
  }
  Symbol* get_keyword(int i) const {
    return ((unsigned)i < (unsigned)max_kw_idx) ? sym[keywords[i]].get() : nullptr;
  }

  SymTable() {
    std::memset(keywords, 0, sizeof(keywords));
  }
};

struct SymTableOverflow {
  int sym_def;
  explicit SymTableOverflow(int x) : sym_def(x) {
  }
};

struct SymTableKwRedef {
  std::string kw;
  SymTableKwRedef(std::string _kw) : kw(_kw) {
  }
};

extern SymTable symbols;

extern int scope_level;

struct SymDef {
  int level;
  sym_idx_t sym_idx;
  SymValBase* value;
  SrcLocation loc;
#ifdef TOLK_DEBUG
  std::string sym_name;
#endif
  SymDef(int lvl, sym_idx_t idx, SrcLocation _loc, SymValBase* val = nullptr)
      : level(lvl), sym_idx(idx), value(val), loc(_loc) {
  }
  bool has_name() const {
    return sym_idx;
  }
  std::string name() const {
    return symbols.get_name(sym_idx);
  }
};

extern SymDef* sym_def[symbols.SIZE_PRIME + 1];
extern SymDef* global_sym_def[symbols.SIZE_PRIME + 1];
extern std::vector<std::pair<int, SymDef>> symbol_stack;
extern std::vector<SrcLocation> scope_opened_at;

void open_scope(SrcLocation loc);
void close_scope(SrcLocation loc);
SymDef* lookup_symbol(sym_idx_t idx);

SymDef* define_global_symbol(sym_idx_t name_idx, bool force_new = false, SrcLocation loc = {});
SymDef* define_symbol(sym_idx_t name_idx, bool force_new, SrcLocation loc);

}  // namespace tolk
