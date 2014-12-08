; ModuleID = 'pr.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y10 = common global i16 0, align 2
@y14 = common global i16 0, align 2
@y12 = common global i16 0, align 2
@y16 = common global i16 0, align 2
@y11 = common global i16 0, align 2
@y17 = common global i16 0, align 2
@y13 = common global i16 0, align 2
@y15 = common global i16 0, align 2

define void @pr(i16 signext %x10, i16 signext %x11, i16 signext %x12, i16 signext %x13, i16 signext %x14, i16 signext %x15, i16 signext %x16, i16 signext %x17) nounwind uwtable {
entry:
  %add = add i16 %x17, %x10
  %add5 = add i16 %x12, %x11
  %sub = sub i16 %x11, %x12
  %add12 = add i16 %x14, %x13
  %sub16 = sub i16 %x13, %x14
  %add20 = add i16 %x16, %x15
  %sub24 = sub i16 %x15, %x16
  %sub28 = sub i16 %x10, %x17
  %add32 = add i16 %add, %add12
  %add36 = add i16 %add20, %add5
  %sub40 = sub i16 %sub, %sub24
  %sub44 = sub i16 %add, %add12
  %sub48 = sub i16 %add5, %add20
  %add52 = add i16 %sub24, %sub
  %add56 = add i16 %add32, %add36
  %sub60 = sub i16 %add32, %add36
  %mul = mul i16 %sub48, 7071
  %mul65 = mul i16 %add52, 7071
  %add69 = add i16 %mul65, %sub16
  %add73 = add i16 %mul, %sub28
  %sub77 = sub i16 %sub16, %mul65
  %sub81 = sub i16 %sub28, %mul
  %mul84 = mul i16 %add56, 7071
  store i16 %mul84, i16* @y10, align 2, !tbaa !0
  %mul87 = mul i16 %sub60, 7071
  store i16 %mul87, i16* @y14, align 2, !tbaa !0
  %mul90 = mul i16 %sub40, 3827
  %mul92 = mul i16 %sub44, 9239
  %add93 = add i16 %mul92, %mul90
  store i16 %add93, i16* @y12, align 2, !tbaa !0
  %mul96 = mul i16 %sub44, 3827
  %tmp0 = mul i16 %sub40, -9239
  %sub99 = add i16 %mul96, %tmp0
  store i16 %sub99, i16* @y16, align 2, !tbaa !0
  %mul102 = mul i16 %add73, 9807
  %mul104 = mul i16 %add69, 1951
  %add105 = add i16 %mul102, %mul104
  store i16 %add105, i16* @y11, align 2, !tbaa !0
  %mul108 = mul i16 %add73, 1951
  %tmp1 = mul i16 %add69, -9807
  %sub111 = add i16 %mul108, %tmp1
  store i16 %sub111, i16* @y17, align 2, !tbaa !0
  %mul114 = mul i16 %sub81, 8315
  %tmp2 = mul i16 %sub77, -5556
  %sub117 = add i16 %mul114, %tmp2
  store i16 %sub117, i16* @y13, align 2, !tbaa !0
  %mul120 = mul i16 %sub81, 5556
  %mul122 = mul i16 %sub77, 8315
  %add123 = add i16 %mul120, %mul122
  store i16 %add123, i16* @y15, align 2, !tbaa !0
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
