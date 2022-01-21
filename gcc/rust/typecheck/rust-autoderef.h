// Copyright (C) 2020-2021 Free Software Foundation, Inc.

// This file is part of GCC.

// GCC is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3, or (at your option) any later
// version.

// GCC is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#ifndef RUST_AUTODEREF
#define RUST_AUTODEREF

#include "rust-tyty.h"

namespace Rust {
namespace Resolver {

class Adjustment
{
public:
  enum AdjustmentType
  {
    ERROR,

    IMM_REF,
    MUT_REF,
    DEREF_REF
  };

  // ctor for all adjustments except derefs
  Adjustment (AdjustmentType type, const TyTy::BaseType *expected)
    : Adjustment (type, expected, nullptr, nullptr, AdjustmentType::ERROR)
  {}

  static Adjustment get_op_overload_deref_adjustment (
    const TyTy::BaseType *expected, TyTy::FnType *fn, HIR::ImplItem *deref_item,
    Adjustment::AdjustmentType requires_ref_adjustment)
  {
    return Adjustment (Adjustment::DEREF_REF, expected, fn, deref_item,
		       requires_ref_adjustment);
  }

  AdjustmentType get_type () const { return type; }

  const TyTy::BaseType *get_expected () const { return expected; }

  std::string as_string () const
  {
    return Adjustment::type_string (get_type ()) + "->"
	   + get_expected ()->debug_str ();
  }

  static std::string type_string (AdjustmentType type)
  {
    switch (type)
      {
      case AdjustmentType::ERROR:
	return "ERROR";
      case AdjustmentType::IMM_REF:
	return "IMM_REF";
      case AdjustmentType::MUT_REF:
	return "MUT_REF";
      case AdjustmentType::DEREF_REF:
	return "DEREF_REF";
      }
    gcc_unreachable ();
    return "";
  }

  static Adjustment get_error () { return Adjustment{ERROR, nullptr}; }

  bool is_error () const { return type == ERROR; }

  bool is_deref_adjustment () const { return type == DEREF_REF; }

  bool has_operator_overload () const { return deref_operator_fn != nullptr; }

  TyTy::FnType *get_deref_operator_fn () const { return deref_operator_fn; }

  AdjustmentType get_deref_adjustment_type () const
  {
    return requires_ref_adjustment;
  }

  HIR::ImplItem *get_deref_hir_item () const { return deref_item; }

private:
  Adjustment (AdjustmentType type, const TyTy::BaseType *expected,
	      TyTy::FnType *deref_operator_fn, HIR::ImplItem *deref_item,
	      Adjustment::AdjustmentType requires_ref_adjustment)
    : type (type), expected (expected), deref_operator_fn (deref_operator_fn),
      deref_item (deref_item), requires_ref_adjustment (requires_ref_adjustment)
  {}

  AdjustmentType type;
  const TyTy::BaseType *expected;

  // - only used for deref operator_overloads
  //
  // the fn that we are calling
  TyTy::FnType *deref_operator_fn;
  HIR::ImplItem *deref_item;
  // operator overloads can requre a reference
  Adjustment::AdjustmentType requires_ref_adjustment;
};

class Adjuster
{
public:
  Adjuster (const TyTy::BaseType *ty) : base (ty) {}

  TyTy::BaseType *adjust_type (const std::vector<Adjustment> &adjustments);

  static bool needs_address (const std::vector<Adjustment> &adjustments);

  static Adjustment try_deref_type (const TyTy::BaseType *ty);

private:
  const TyTy::BaseType *base;
};

} // namespace Resolver
} // namespace Rust

#endif // RUST_AUTODEREF
