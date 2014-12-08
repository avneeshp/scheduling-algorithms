; ModuleID = 'wang.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y1 = common global i16 0, align 2
@y5 = common global i16 0, align 2
@y7 = common global i16 0, align 2
@y3 = common global i16 0, align 2
@y2 = common global i16 0, align 2
@y8 = common global i16 0, align 2
@y6 = common global i16 0, align 2
@y4 = common global i16 0, align 2

define void @wang(i16 signext %x11, i16 signext %x12, i16 signext %x13, i16 signext %x14, i16 signext %x15, i16 signext %x16, i16 signext %x17, i16 signext %x18) nounwind uwtable {
entry:
  %add = add i16 %x18, %x11
  %add5 = add i16 %x17, %x12
  %add9 = add i16 %x16, %x13
  %add13 = add i16 %x15, %x14
  %sub = sub i16 %x14, %x15
  %sub20 = sub i16 %x13, %x16
  %sub24 = sub i16 %x12, %x17
  %sub28 = sub i16 %x11, %x18
  %add32 = add i16 %add, %add13
  %add36 = add i16 %add5, %add9
  %sub40 = sub i16 %add5, %add9
  %sub44 = sub i16 %add, %add13
  %add48 = add i16 %add32, %add36
  %mul = mul i16 %add48, 3535
  store i16 %mul, i16* @y1, align 2, !tbaa !0
  %sub52 = sub i16 %add32, %add36
  %mul53 = mul i16 %sub52, 3535
  store i16 %mul53, i16* @y5, align 2, !tbaa !0
  %mul56 = mul i16 %sub44, 1913
  %tmp0 = mul i16 %sub40, -4619
  %sub63 = add i16 %mul56, %tmp0
  store i16 %sub63, i16* @y7, align 2, !tbaa !0
  %mul66 = mul i16 %sub44, 4619
  %mul70 = mul i16 %sub40, 1913
  %add73 = add i16 %mul66, %mul70
  store i16 %add73, i16* @y3, align 2, !tbaa !0
  %mul76 = mul i16 %sub28, 7071
  %mul79 = mul i16 %sub, 7071
  %add83 = add i16 %sub24, %sub20
  %mul84 = mul i16 %add83, 4999
  %sub88 = sub i16 %sub24, %sub20
  %mul89 = mul i16 %sub88, 4999
  %add93 = add i16 %mul76, %mul84
  %add98 = add i16 %mul89, %mul79
  %sub103 = sub i16 %mul76, %mul84
  %sub108 = sub i16 %mul79, %mul89
  %mul112 = mul i16 %add93, 8209
  %mul116 = mul i16 %add98, 25890
  %add119 = add i16 %mul112, %mul116
  store i16 %add119, i16* @y2, align 2, !tbaa !0
  %mul122 = mul i16 %add93, 25890
  %tmp1 = mul i16 %add98, -8209
  %sub129 = add i16 %mul122, %tmp1
  store i16 %sub129, i16* @y8, align 2, !tbaa !0
  %mul132 = mul i16 %sub103, 23341
  %mul136 = mul i16 %sub108, 2502
  %add139 = add i16 %mul132, %mul136
  store i16 %add139, i16* @y6, align 2, !tbaa !0
  %mul142 = mul i16 %sub103, 2502
  %tmp2 = mul i16 %sub108, -23341
  %sub149 = add i16 %mul142, %tmp2
  store i16 %sub149, i16* @y4, align 2, !tbaa !0
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
