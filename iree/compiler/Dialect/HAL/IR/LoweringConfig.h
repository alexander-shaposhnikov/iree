// Copyright 2021 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//===- LoweringConfig.h - Declares configuration for lowering Linalg ops --===//
//
// This file declares an attribute that drives how a dispatch region containing
// a set of operations are lowered. The attribute itself is attached to Linalg
// operations, and help converting a Linalg operation into "scalar code".
//
//===----------------------------------------------------------------------===//

#ifndef IREE_COMPILER_CONVERSION_COMMON_LOWERINGCONFIG_H_
#define IREE_COMPILER_CONVERSION_COMMON_LOWERINGCONFIG_H_

#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinTypes.h"

// clang-format off
#include "iree/compiler/Dialect/HAL/IR/LoweringConfig.h.inc"
#include "iree/compiler/Dialect/HAL/IR/LoweringConfigEnums.h.inc"
// clang-format on

namespace mlir {
namespace iree_compiler {

namespace IREE {
namespace HAL {
/// Struct that for a given hal.target.executable defines how it is translated.
// TODO(ravishankarm): This could also be converted to an attribute on the
// hal.executable.target
struct TranslateExecutableInfo {
  DispatchLoweringPassPipeline passPipeline;
  SmallVector<int64_t, 3> workgroupSize;
};

inline bool operator==(const TranslateExecutableInfo &lhs,
                       const TranslateExecutableInfo &rhs) {
  return lhs.passPipeline == rhs.passPipeline &&
         lhs.workgroupSize == rhs.workgroupSize;
}

inline bool operator!=(const TranslateExecutableInfo &lhs,
                       const TranslateExecutableInfo &rhs) {
  return !(lhs == rhs);
}

}  // namespace HAL
}  // namespace IREE

//===----------------------------------------------------------------------===//
// Helpers for getting/setting the `hal.lowering.*` attributes that drive the
// linalg-based lowering.
// ===----------------------------------------------------------------------===//

/// Returns the lowering configuration set for an operation.
IREE::HAL::LoweringConfig getLoweringConfig(Operation *op);

/// Returns true if an operation has a lowering configuration set.
bool hasLoweringConfig(Operation *op);

/// Sets the lowering configuration if one isnt already set and returns
/// true. Returns false if a configuration already exists.
bool setLoweringConfig(Operation *op, IREE::HAL::LoweringConfig config);

/// Removes the lowering configuration on the operation if it exists.
void eraseLoweringConfig(Operation *op);

//===----------------------------------------------------------------------===//
// Helpers for accessing values from the LoweringConfig attribute.
//===----------------------------------------------------------------------===//

// TODO(ravishankarm): Struct attributes dont have a way of defining extra class
// methods. When they do, these could all be moved into the attribute definition
// itself.

/// Stores the tile sizes to use at different levels of tiling as a vector of
/// vectors.
/// - First level tiling maps to workgroups.
/// - Second level tiling maps to subgroups.
/// - Third level tiling maps to invocations.
using TileSizesListType = SmallVector<SmallVector<int64_t, 4>, 1>;
using TileSizesListTypeRef = ArrayRef<SmallVector<int64_t, 4>>;

/// Construct a lowering configuration.
IREE::HAL::LoweringConfig getConfigAttr(TileSizesListTypeRef tileSizes,
                                        ArrayRef<int64_t> nativeVectorSize,
                                        MLIRContext *context);

/// Get the tile sizes for all levels.
TileSizesListType getTileSizes(IREE::HAL::LoweringConfig config);

/// Get the tile sizes for all levels for an operation if the lowering
/// configuration is set.
inline TileSizesListType getTileSizes(Operation *op) {
  auto configAttr = getLoweringConfig(op);
  if (!configAttr) return {};
  return getTileSizes(configAttr);
}

/// Get the tile sizes for level `level`, if it is defined. Returns {} if tile
/// sizes are not set for that level.
SmallVector<int64_t, 4> getTileSizes(IREE::HAL::LoweringConfig config,
                                     unsigned level);

/// Get the tile sizes for level `level` for an operation if the lowering
/// configuration for the operation is set, and tile sizes are defined for that
/// level.
inline SmallVector<int64_t, 4> getTileSizes(Operation *op, unsigned level) {
  auto configAttr = getLoweringConfig(op);
  if (!configAttr) return {};
  return getTileSizes(configAttr, level);
}
SmallVector<Value, 4> getTileSizes(OpBuilder &b, Operation *op, unsigned level);

/// Gets the native vector size defined in the lowering configuration.
SmallVector<int64_t, 4> getNativeVectorSize(IREE::HAL::LoweringConfig config);

/// Gets the native vector size defined for lowering an operation, if the
/// lowering configuration is defined. If not returns empty vector.
inline SmallVector<int64_t, 4> getNativeVectorSize(Operation *op) {
  auto configAttr = getLoweringConfig(op);
  if (!configAttr) return {};
  return getNativeVectorSize(configAttr);
}

}  // namespace iree_compiler
}  // namespace mlir
#endif  // IREE_COMPILER_CONVERSION_COMMON_LOWERINGCONFIG_H_
