; ModuleID = 'lee.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@y10 = common global i16 0, align 2
@y11 = common global i16 0, align 2
@y12 = common global i16 0, align 2
@y13 = common global i16 0, align 2
@y14 = common global i16 0, align 2
@y15 = common global i16 0, align 2
@y16 = common global i16 0, align 2
@y17 = common global i16 0, align 2

define void @lee(i16 signext %x10, i16 signext %x11, i16 signext %x12, i16 signext %x13, i16 signext %x14, i16 signext %x15, i16 signext %x16, i16 signext %x17) nounwind uwtable {
entry:
  %add = add i16 %x17, %x10
  %add5 = add i16 %x16, %x11
  %add9 = add i16 %x14, %x13
  %add13 = add i16 %x15, %x12
  %sub = sub i16 %x10, %x17
  %sub20 = sub i16 %x11, %x16
  %sub24 = sub i16 %x13, %x14
  %sub28 = sub i16 %x12, %x15
  %mul = mul i16 %sub, 5098
  %mul33 = mul i16 %sub20, 6013
  %mul36 = mul i16 %sub24, 25629
  %mul39 = mul i16 %sub28, 8999
  %add43 = add i16 %add, %add9
  %add47 = add i16 %add5, %add13
  %sub51 = sub i16 %add, %add9
  %sub55 = sub i16 %add5, %add13
  %add59 = add i16 %mul, %mul36
  %add63 = add i16 %mul33, %mul39
  %sub67 = sub i16 %mul, %mul36
  %sub71 = sub i16 %mul33, %mul39
  %mul74 = mul i16 %sub51, 5412
  %mul77 = mul i16 %sub55, 13065
  %mul80 = mul i16 %sub67, 5412
  %mul83 = mul i16 %sub71, 13065
  %add87 = add i16 %add43, %add47
  %sub91 = sub i16 %add43, %add47
  %sub99 = sub i16 %mul74, %mul77
  %sub107 = sub i16 %add59, %add63
  %sub115 = sub i16 %mul80, %mul83
  %mul121 = mul i16 %sub99, 7071
  %mul124 = mul i16 %sub107, 7071
  %mul127 = mul i16 %sub115, 7071
  %add95 = add i16 %mul74, %mul77
  %add131 = add i16 %add95, %mul121
  %add111 = add i16 %mul80, %mul83
  %add135 = add i16 %add111, %mul127
  %add103 = add i16 %add59, %add63
  %add139 = add i16 %add103, %add135
  %mul124200 = add i16 %sub115, %sub107
  %add147 = add i16 %add135, %mul124
  %mul150 = mul i16 %add87, 25
  store i16 %mul150, i16* @y10, align 2, !tbaa !0
  %mul153 = mul i16 %add139, 25
  store i16 %mul153, i16* @y11, align 2, !tbaa !0
  %mul156 = mul i16 %add131, 25
  store i16 %mul156, i16* @y12, align 2, !tbaa !0
  %mul159 = mul i16 %add147, 25
  store i16 %mul159, i16* @y13, align 2, !tbaa !0
  %mul162 = mul i16 %sub91, -19833
  store i16 %mul162, i16* @y14, align 2, !tbaa !0
  %mul165 = mul i16 %mul124200, -19833
  store i16 %mul165, i16* @y15, align 2, !tbaa !0
  %mul168 = mul i16 %sub99, -19833
  store i16 %mul168, i16* @y16, align 2, !tbaa !0
  %mul171 = mul i16 %sub115, -19833
  store i16 %mul171, i16* @y17, align 2, !tbaa !0
  ret void
}

!0 = metadata !{metadata !"short", metadata !1}
!1 = metadata !{metadata !"omnipotent char", metadata !2}
!2 = metadata !{metadata !"Simple C/C++ TBAA"}
