//===- AMDGPULegalizerInfo.cpp -----------------------------------*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for
/// AMDGPU.
/// \todo This should be generated by TableGen.
//===----------------------------------------------------------------------===//

#include "AMDGPU.h"
#include "AMDGPULegalizerInfo.h"
#include "AMDGPUTargetMachine.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace LegalizeActions;
using namespace LegalizeMutations;
using namespace LegalityPredicates;

AMDGPULegalizerInfo::AMDGPULegalizerInfo(const GCNSubtarget &ST,
                                         const GCNTargetMachine &TM) {
  using namespace TargetOpcode;

  auto GetAddrSpacePtr = [&TM](unsigned AS) {
    return LLT::pointer(AS, TM.getPointerSizeInBits(AS));
  };

  const LLT S1 = LLT::scalar(1);
  const LLT S16 = LLT::scalar(16);
  const LLT S32 = LLT::scalar(32);
  const LLT S64 = LLT::scalar(64);
  const LLT S128 = LLT::scalar(128);
  const LLT S256 = LLT::scalar(256);
  const LLT S512 = LLT::scalar(512);

  const LLT V2S16 = LLT::vector(2, 16);
  const LLT V4S16 = LLT::vector(4, 16);
  const LLT V8S16 = LLT::vector(8, 16);

  const LLT V2S32 = LLT::vector(2, 32);
  const LLT V3S32 = LLT::vector(3, 32);
  const LLT V4S32 = LLT::vector(4, 32);
  const LLT V5S32 = LLT::vector(5, 32);
  const LLT V6S32 = LLT::vector(6, 32);
  const LLT V7S32 = LLT::vector(7, 32);
  const LLT V8S32 = LLT::vector(8, 32);
  const LLT V9S32 = LLT::vector(9, 32);
  const LLT V10S32 = LLT::vector(10, 32);
  const LLT V11S32 = LLT::vector(11, 32);
  const LLT V12S32 = LLT::vector(12, 32);
  const LLT V13S32 = LLT::vector(13, 32);
  const LLT V14S32 = LLT::vector(14, 32);
  const LLT V15S32 = LLT::vector(15, 32);
  const LLT V16S32 = LLT::vector(16, 32);

  const LLT V2S64 = LLT::vector(2, 64);
  const LLT V3S64 = LLT::vector(3, 64);
  const LLT V4S64 = LLT::vector(4, 64);
  const LLT V5S64 = LLT::vector(5, 64);
  const LLT V6S64 = LLT::vector(6, 64);
  const LLT V7S64 = LLT::vector(7, 64);
  const LLT V8S64 = LLT::vector(8, 64);

  std::initializer_list<LLT> AllS32Vectors =
    {V2S32, V3S32, V4S32, V5S32, V6S32, V7S32, V8S32,
     V9S32, V10S32, V11S32, V12S32, V13S32, V14S32, V15S32, V16S32};
  std::initializer_list<LLT> AllS64Vectors =
    {V2S64, V3S64, V4S64, V5S64, V6S64, V7S64, V8S64};

  const LLT GlobalPtr = GetAddrSpacePtr(AMDGPUAS::GLOBAL_ADDRESS);
  const LLT ConstantPtr = GetAddrSpacePtr(AMDGPUAS::CONSTANT_ADDRESS);
  const LLT LocalPtr = GetAddrSpacePtr(AMDGPUAS::LOCAL_ADDRESS);
  const LLT FlatPtr = GetAddrSpacePtr(AMDGPUAS::FLAT_ADDRESS);
  const LLT PrivatePtr = GetAddrSpacePtr(AMDGPUAS::PRIVATE_ADDRESS);

  const LLT CodePtr = FlatPtr;

  const LLT AddrSpaces[] = {
    GlobalPtr,
    ConstantPtr,
    LocalPtr,
    FlatPtr,
    PrivatePtr
  };

  setAction({G_BRCOND, S1}, Legal);

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_UMULH, G_SMULH})
    .legalFor({S32})
    .clampScalar(0, S32, S32)
    .scalarize(0);

  // Report legal for any types we can handle anywhere. For the cases only legal
  // on the SALU, RegBankSelect will be able to re-legalize.
  getActionDefinitionsBuilder({G_AND, G_OR, G_XOR})
    .legalFor({S32, S1, S64, V2S32, V2S16, V4S16})
    .clampScalar(0, S32, S64)
    .scalarize(0);

  getActionDefinitionsBuilder({G_UADDO, G_SADDO, G_USUBO, G_SSUBO,
                               G_UADDE, G_SADDE, G_USUBE, G_SSUBE})
    .legalFor({{S32, S1}})
    .clampScalar(0, S32, S32);

  getActionDefinitionsBuilder(G_BITCAST)
    .legalForCartesianProduct({S32, V2S16})
    .legalForCartesianProduct({S64, V2S32, V4S16})
    .legalForCartesianProduct({V2S64, V4S32})
    // Don't worry about the size constraint.
    .legalIf(all(isPointer(0), isPointer(1)));

  getActionDefinitionsBuilder(G_FCONSTANT)
    .legalFor({S32, S64, S16});

  // G_IMPLICIT_DEF is a no-op so we can make it legal for any value type that
  // can fit in a register.
  // FIXME: We need to legalize several more operations before we can add
  // a test case for size > 512.
  getActionDefinitionsBuilder(G_IMPLICIT_DEF)
    .legalIf([=](const LegalityQuery &Query) {
        return Query.Types[0].getSizeInBits() <= 512;
    })
    .clampScalar(0, S1, S512);


  // FIXME: i1 operands to intrinsics should always be legal, but other i1
  // values may not be legal.  We need to figure out how to distinguish
  // between these two scenarios.
  // FIXME: Pointer types
  getActionDefinitionsBuilder(G_CONSTANT)
    .legalFor({S1, S32, S64})
    .clampScalar(0, S32, S64)
    .widenScalarToNextPow2(0);

  setAction({G_FRAME_INDEX, PrivatePtr}, Legal);

  getActionDefinitionsBuilder({G_FADD, G_FMUL, G_FNEG, G_FABS, G_FMA})
      .legalFor({S32, S64})
      .scalarize(0)
      .clampScalar(0, S32, S64);

  getActionDefinitionsBuilder(G_FPTRUNC)
    .legalFor({{S32, S64}, {S16, S32}})
    .scalarize(0);

  getActionDefinitionsBuilder(G_FPEXT)
    .legalFor({{S64, S32}, {S32, S16}})
    .lowerFor({{S64, S16}}) // FIXME: Implement
    .scalarize(0);

  getActionDefinitionsBuilder(G_FSUB)
      // Use actual fsub instruction
      .legalFor({S32})
      // Must use fadd + fneg
      .lowerFor({S64, S16, V2S16})
      .scalarize(0)
      .clampScalar(0, S32, S64);

  getActionDefinitionsBuilder({G_SEXT, G_ZEXT, G_ANYEXT})
    .legalFor({{S64, S32}, {S32, S16}, {S64, S16},
               {S32, S1}, {S64, S1}, {S16, S1},
               // FIXME: Hack
               {S128, S32}, {S128, S64}, {S32, LLT::scalar(24)}})
    .scalarize(0);

  getActionDefinitionsBuilder({G_SITOFP, G_UITOFP})
    .legalFor({{S32, S32}, {S64, S32}})
    .scalarize(0);

  getActionDefinitionsBuilder({G_FPTOSI, G_FPTOUI})
    .legalFor({{S32, S32}, {S32, S64}})
    .scalarize(0);

  getActionDefinitionsBuilder({G_INTRINSIC_TRUNC, G_INTRINSIC_ROUND})
    .legalFor({S32, S64})
    .scalarize(0);

  for (LLT PtrTy : AddrSpaces) {
    LLT IdxTy = LLT::scalar(PtrTy.getSizeInBits());
    setAction({G_GEP, PtrTy}, Legal);
    setAction({G_GEP, 1, IdxTy}, Legal);
  }

  // FIXME: When RegBankSelect inserts copies, it will only create new registers
  // with scalar types. This means we can end up with G_LOAD/G_STORE/G_GEP
  // instruction with scalar types for their pointer operands. In assert builds,
  // the instruction selector will assert if it sees a generic instruction which
  // isn't legal, so we need to tell it that scalar types are legal for pointer
  // operands
  setAction({G_GEP, S64}, Legal);

  setAction({G_BLOCK_ADDR, CodePtr}, Legal);

  getActionDefinitionsBuilder({G_ICMP, G_FCMP})
    .legalFor({{S1, S32}, {S1, S64}})
    .widenScalarToNextPow2(1)
    .clampScalar(1, S32, S64)
    .scalarize(0);

  // FIXME: fexp, flog2, flog10 needs to be custom lowered.
  getActionDefinitionsBuilder({G_FPOW, G_FEXP, G_FEXP2,
                               G_FLOG, G_FLOG2, G_FLOG10})
    .legalFor({S32})
    .scalarize(0);

  setAction({G_CTLZ, S32}, Legal);
  setAction({G_CTLZ_ZERO_UNDEF, S32}, Legal);
  setAction({G_CTTZ, S32}, Legal);
  setAction({G_CTTZ_ZERO_UNDEF, S32}, Legal);
  setAction({G_BSWAP, S32}, Legal);
  setAction({G_CTPOP, S32}, Legal);

  getActionDefinitionsBuilder(G_INTTOPTR)
    .legalIf([](const LegalityQuery &Query) {
      return true;
    });

  getActionDefinitionsBuilder(G_PTRTOINT)
    .legalIf([](const LegalityQuery &Query) {
      return true;
    });

  getActionDefinitionsBuilder({G_LOAD, G_STORE})
    .narrowScalarIf([](const LegalityQuery &Query) {
        unsigned Size = Query.Types[0].getSizeInBits();
        unsigned MemSize = Query.MMODescrs[0].SizeInBits;
        return (Size > 32 && MemSize < Size);
      },
      [](const LegalityQuery &Query) {
        return std::make_pair(0, LLT::scalar(32));
      })
    .fewerElementsIf([=, &ST](const LegalityQuery &Query) {
        unsigned MemSize = Query.MMODescrs[0].SizeInBits;
        return Query.Types[0].isVector() && (MemSize == 96) &&
               ST.getGeneration() < AMDGPUSubtarget::SEA_ISLANDS;
      },
      [=](const LegalityQuery &Query) {
        return std::make_pair(0, V2S32);
      })
    .legalIf([=, &ST](const LegalityQuery &Query) {
        const LLT &Ty0 = Query.Types[0];

        unsigned Size = Ty0.getSizeInBits();
        unsigned MemSize = Query.MMODescrs[0].SizeInBits;
        if (Size > 32 && MemSize < Size)
          return false;

        if (Ty0.isVector() && Size != MemSize)
          return false;

        // TODO: Decompose private loads into 4-byte components.
        // TODO: Illegal flat loads on SI
        switch (MemSize) {
        case 8:
        case 16:
          return Size == 32;
        case 32:
        case 64:
        case 128:
          return true;

        case 96:
          // XXX hasLoadX3
          return (ST.getGeneration() >= AMDGPUSubtarget::SEA_ISLANDS);

        case 256:
        case 512:
          // TODO: constant loads
        default:
          return false;
        }
      })
    .clampScalar(0, S32, S64);


  auto &ExtLoads = getActionDefinitionsBuilder({G_SEXTLOAD, G_ZEXTLOAD})
    .legalForTypesWithMemSize({
        {S32, GlobalPtr, 8},
        {S32, GlobalPtr, 16},
        {S32, LocalPtr, 8},
        {S32, LocalPtr, 16},
        {S32, PrivatePtr, 8},
        {S32, PrivatePtr, 16}});
  if (ST.hasFlatAddressSpace()) {
    ExtLoads.legalForTypesWithMemSize({{S32, FlatPtr, 8},
                                       {S32, FlatPtr, 16}});
  }

  ExtLoads.clampScalar(0, S32, S32)
          .widenScalarToNextPow2(0)
          .unsupportedIfMemSizeNotPow2()
          .lower();

  auto &Atomics = getActionDefinitionsBuilder(
    {G_ATOMICRMW_XCHG, G_ATOMICRMW_ADD, G_ATOMICRMW_SUB,
     G_ATOMICRMW_AND, G_ATOMICRMW_OR, G_ATOMICRMW_XOR,
     G_ATOMICRMW_MAX, G_ATOMICRMW_MIN, G_ATOMICRMW_UMAX,
     G_ATOMICRMW_UMIN, G_ATOMIC_CMPXCHG})
    .legalFor({{S32, GlobalPtr}, {S32, LocalPtr},
               {S64, GlobalPtr}, {S64, LocalPtr}});
  if (ST.hasFlatAddressSpace()) {
    Atomics.legalFor({{S32, FlatPtr}, {S64, FlatPtr}});
  }

  // TODO: Pointer types, any 32-bit or 64-bit vector
  getActionDefinitionsBuilder(G_SELECT)
    .legalFor({{S32, S1}, {S64, S1}, {V2S32, S1}, {V2S16, S1}})
    .clampScalar(0, S32, S64)
    .fewerElementsIf(
      [=](const LegalityQuery &Query) {
        if (Query.Types[1].isVector())
          return true;

        LLT Ty = Query.Types[0];

        // FIXME: Hack until odd splits handled
        return Ty.isVector() &&
          (Ty.getScalarSizeInBits() > 32 || Ty.getNumElements() % 2 != 0);
      },
      scalarize(0))
    // FIXME: Handle 16-bit vectors better
    .fewerElementsIf(
      [=](const LegalityQuery &Query) {
        return Query.Types[0].isVector() &&
               Query.Types[0].getElementType().getSizeInBits() < 32;},
      scalarize(0))
    .scalarize(1)
    .clampMaxNumElements(0, S32, 2);

  // TODO: Only the low 4/5/6 bits of the shift amount are observed, so we can
  // be more flexible with the shift amount type.
  auto &Shifts = getActionDefinitionsBuilder({G_SHL, G_LSHR, G_ASHR})
    .legalFor({{S32, S32}, {S64, S32}});
  if (ST.has16BitInsts()) {
    Shifts.legalFor({{S16, S32}, {S16, S16}});
    Shifts.clampScalar(0, S16, S64);
  } else
    Shifts.clampScalar(0, S32, S64);
  Shifts.clampScalar(1, S32, S32);

  for (unsigned Op : {G_EXTRACT_VECTOR_ELT, G_INSERT_VECTOR_ELT}) {
    unsigned VecTypeIdx = Op == G_EXTRACT_VECTOR_ELT ? 1 : 0;
    unsigned EltTypeIdx = Op == G_EXTRACT_VECTOR_ELT ? 0 : 1;
    unsigned IdxTypeIdx = 2;

    getActionDefinitionsBuilder(Op)
      .legalIf([=](const LegalityQuery &Query) {
          const LLT &VecTy = Query.Types[VecTypeIdx];
          const LLT &IdxTy = Query.Types[IdxTypeIdx];
          return VecTy.getSizeInBits() % 32 == 0 &&
            VecTy.getSizeInBits() <= 512 &&
            IdxTy.getSizeInBits() == 32;
        })
      .clampScalar(EltTypeIdx, S32, S64)
      .clampScalar(VecTypeIdx, S32, S64)
      .clampScalar(IdxTypeIdx, S32, S32);
  }

  getActionDefinitionsBuilder(G_EXTRACT_VECTOR_ELT)
    .unsupportedIf([=](const LegalityQuery &Query) {
        const LLT &EltTy = Query.Types[1].getElementType();
        return Query.Types[0] != EltTy;
      });

  // FIXME: Doesn't handle extract of illegal sizes.
  getActionDefinitionsBuilder({G_EXTRACT, G_INSERT})
    .legalIf([=](const LegalityQuery &Query) {
        const LLT &Ty0 = Query.Types[0];
        const LLT &Ty1 = Query.Types[1];
        return (Ty0.getSizeInBits() % 16 == 0) &&
               (Ty1.getSizeInBits() % 16 == 0);
      });

  getActionDefinitionsBuilder(G_BUILD_VECTOR)
      .legalForCartesianProduct(AllS32Vectors, {S32})
      .legalForCartesianProduct(AllS64Vectors, {S64})
      .clampNumElements(0, V16S32, V16S32)
      .clampNumElements(0, V2S64, V8S64)
      .minScalarSameAs(1, 0)
      // FIXME: Sort of a hack to make progress on other legalizations.
      .legalIf([=](const LegalityQuery &Query) {
        return Query.Types[0].getScalarSizeInBits() < 32;
      });

  // TODO: Support any combination of v2s32
  getActionDefinitionsBuilder(G_CONCAT_VECTORS)
    .legalFor({{V4S32, V2S32},
               {V8S32, V2S32},
               {V8S32, V4S32},
               {V4S64, V2S64},
               {V4S16, V2S16},
               {V8S16, V2S16},
               {V8S16, V4S16}});

  // Merge/Unmerge
  for (unsigned Op : {G_MERGE_VALUES, G_UNMERGE_VALUES}) {
    unsigned BigTyIdx = Op == G_MERGE_VALUES ? 0 : 1;
    unsigned LitTyIdx = Op == G_MERGE_VALUES ? 1 : 0;

    auto notValidElt = [=](const LegalityQuery &Query, unsigned TypeIdx) {
      const LLT &Ty = Query.Types[TypeIdx];
      if (Ty.isVector()) {
        const LLT &EltTy = Ty.getElementType();
        if (EltTy.getSizeInBits() < 8 || EltTy.getSizeInBits() > 64)
          return true;
        if (!isPowerOf2_32(EltTy.getSizeInBits()))
          return true;
      }
      return false;
    };

    getActionDefinitionsBuilder(Op)
      .widenScalarToNextPow2(LitTyIdx, /*Min*/ 16)
      // Clamp the little scalar to s8-s256 and make it a power of 2. It's not
      // worth considering the multiples of 64 since 2*192 and 2*384 are not
      // valid.
      .clampScalar(LitTyIdx, S16, S256)
      .widenScalarToNextPow2(LitTyIdx, /*Min*/ 32)

      // Break up vectors with weird elements into scalars
      .fewerElementsIf(
        [=](const LegalityQuery &Query) { return notValidElt(Query, 0); },
        scalarize(0))
      .fewerElementsIf(
        [=](const LegalityQuery &Query) { return notValidElt(Query, 1); },
        scalarize(1))
      .clampScalar(BigTyIdx, S32, S512)
      .widenScalarIf(
        [=](const LegalityQuery &Query) {
          const LLT &Ty = Query.Types[BigTyIdx];
          return !isPowerOf2_32(Ty.getSizeInBits()) &&
                 Ty.getSizeInBits() % 16 != 0;
        },
        [=](const LegalityQuery &Query) {
          // Pick the next power of 2, or a multiple of 64 over 128.
          // Whichever is smaller.
          const LLT &Ty = Query.Types[BigTyIdx];
          unsigned NewSizeInBits = 1 << Log2_32_Ceil(Ty.getSizeInBits() + 1);
          if (NewSizeInBits >= 256) {
            unsigned RoundedTo = alignTo<64>(Ty.getSizeInBits() + 1);
            if (RoundedTo < NewSizeInBits)
              NewSizeInBits = RoundedTo;
          }
          return std::make_pair(BigTyIdx, LLT::scalar(NewSizeInBits));
        })
      .legalIf([=](const LegalityQuery &Query) {
          const LLT &BigTy = Query.Types[BigTyIdx];
          const LLT &LitTy = Query.Types[LitTyIdx];

          if (BigTy.isVector() && BigTy.getSizeInBits() < 32)
            return false;
          if (LitTy.isVector() && LitTy.getSizeInBits() < 32)
            return false;

          return BigTy.getSizeInBits() % 16 == 0 &&
                 LitTy.getSizeInBits() % 16 == 0 &&
                 BigTy.getSizeInBits() <= 512;
        })
      // Any vectors left are the wrong size. Scalarize them.
      .scalarize(0)
      .scalarize(1);
  }

  computeTables();
  verify(*ST.getInstrInfo());
}
